/**
 * BMS project 2
 * Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * @date 2013/12/05
 */

#include <sstream>
#include <iomanip>
#include <algorithm>

#include "epginfo.h"
#include "tsstream.h"
#include "cfg.h"

#define IS_PRESENT(X)      (X->payload[0] == 0x4e && X->header.payload_unit_start_indicator)
#define IS_SCHEDULED(X)    (X->payload[0] >= 0x50 && X->payload[0] <= 0x5F \
                                && X->header.payload_unit_start_indicator)

/**
 * Analyse scheduled/present EPG
 * @param packet packet to analyse
 *
 * @return true on success
 */
bool EPGinfo::analyse(TSpacket * packet) {
    if (IS_PRESENT(packet))
        return analyse_epg(packet, true);
    else if (IS_SCHEDULED(packet))
        return analyse_epg(packet, false);

    return false;
}

/**
 * Analyse EPG info
 *
 * @param packet packet to analyse
 * @param present true if present EPG
 * @return true on success
 */
bool EPGinfo::analyse_epg(TSpacket * packet, bool present) {
    unsigned service_id = get16b(packet->payload[3], packet->payload[4]);

    if (present && packet->payload[13] != 0x4e)
        return false;
    else if (! present && (packet->payload[13] > 0x5f || packet->payload[13] < 0x50))
        return false;

    unsigned section_len = get12b(packet->payload[1], packet->payload[2]);

    // merge packets if segmented
    unsigned continuity = packet->header.continuity_counter;
    while (section_len > packet->payload.size()) {
        TSpacket packet2;

        packet->payload.reserve(packet->payload.size() + packet2.payload.size());

        continuity = (continuity + 1) % 16;
        TSstream::getInstance().getPacket(&packet2, packet->getPid(), continuity);
        packet->payload.insert(packet->payload.end(), packet2.adaptation.begin(), packet2.adaptation.end());
        packet->payload.insert(packet->payload.end(), packet2.payload.begin(), packet2.payload.end());
    }

    unsigned index = 0;
    unsigned base = 3 /* bits before/with len */ + 11;
    while (index + base < section_len + 3 - 4/*CRC*/) {
        unsigned event_id = (unsigned) get16b(packet->payload[index + base], packet->payload[index + base + 1]);
        int nDay, nMonth, nYear;

        unsigned descriptor_loop_len = get12b(packet->payload[index + base + 10], packet->payload[index + base + 11]);
        unsigned index2 = 0;

        while (index2 < descriptor_loop_len) {
            if (packet->payload[index + base + 12 + index2] == 0x4d) { // short_event desc
                unsigned len = packet->payload[index + base + 12 + index2 + 5];
                char * event_name = new char[len + 1];
                memcpy(event_name, &(packet->payload[index + base + 12 + index2 + 6]), len);
                event_name[len] = '\0';

                unsigned len2 = packet->payload[index + base + 12 + index2 + 6 + len];
                char * event_text = new char[len2 + 1];
                memcpy(event_text, &(packet->payload[index + base + 12 + index2 + 6 + len + 1]), len2);
                event_text[len2] = '\0';

                std::stringstream out;
                uint64_t time = ((((uint64_t) packet->payload[index + base + 2]) << 32)
                                | (((uint64_t)packet->payload[index + base + 3]) << 24)
                                | (((uint64_t)packet->payload[index + base + 4]) << 16)
                                | (((uint64_t)packet->payload[index + base + 5]) << 8)
                                | (((uint64_t)packet->payload[index + base + 6]) << 0));
                uint64_t duration = ((((uint64_t) packet->payload[index + base + 7]) << 16)
                                    | (((uint64_t)packet->payload[index + base + 8]) << 8)
                                    | (((uint64_t)packet->payload[index + base + 9]) << 0));

                out << event_name << " - " << event_text;

                if (Cfg::getInstance().data.find(service_id) != Cfg::getInstance().data.end()) {
                    if (present) {
                        auto it = std::find(Cfg::getInstance().data[service_id]->present_ids.begin(), Cfg::getInstance().data[service_id]->present_ids.end(), event_id);
                        if (it != Cfg::getInstance().data[service_id]->present_ids.end()) {
                            for (auto jt = Cfg::getInstance().data[service_id]->present.begin(); jt != Cfg::getInstance().data[service_id]->present.end(); ++jt) {
                                if ((*jt).first == event_id) {
                                    Cfg::getInstance().data[service_id]->present.erase(jt);
                                    break;
                                }
                            }
                        }

                        for (auto jt = Cfg::getInstance().data[service_id]->present.begin(); jt != Cfg::getInstance().data[service_id]->present.end(); ++jt) {
                            if ((*jt).second.first.first == time) {
                                jt = Cfg::getInstance().data[service_id]->present.erase(jt);
                            }
                        }

                        Cfg::getInstance().data[service_id]->present.push_back(std::make_pair(event_id, std::make_pair(std::make_pair(time, duration), out.str())));
                        Cfg::getInstance().data[service_id]->present_ids.push_back(event_id);
                    } else {
                        auto it = std::find(Cfg::getInstance().data[service_id]->scheduled_ids.begin(), Cfg::getInstance().data[service_id]->scheduled_ids.end(), event_id);
                        if (it != Cfg::getInstance().data[service_id]->scheduled_ids.end()) {
                            for (auto jt = Cfg::getInstance().data[service_id]->scheduled.begin(); jt != Cfg::getInstance().data[service_id]->scheduled.end(); ++jt) {
                                if ((*jt).first == event_id) {
                                    Cfg::getInstance().data[service_id]->scheduled.erase(jt);
                                    break;
                                }
                            }
                        }

                        for (auto jt = Cfg::getInstance().data[service_id]->scheduled.begin(); jt != Cfg::getInstance().data[service_id]->scheduled.end(); ++jt) {
                            if ((*jt).second.first.first == time) {
                                jt = Cfg::getInstance().data[service_id]->scheduled.erase(jt);
                            }
                        }

                        Cfg::getInstance().data[service_id]->scheduled.push_back(std::make_pair(event_id, std::make_pair(std::make_pair(time, duration), out.str())));
                        Cfg::getInstance().data[service_id]->scheduled_ids.push_back(event_id);
                    }
                }
            }

            index2 += packet->payload[index + base + 12 + index2 + 1] + 2; /* descriptor len */
        }

        index += descriptor_loop_len + 12;
    }

    return true;
}

