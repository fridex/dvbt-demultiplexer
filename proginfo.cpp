/**
 * BMS project 2
 * Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * @date 2013/12/05
 */

#include "proginfo.h"
#include "tsstream.h"
#include "cfg.h"

/**
 * Analyse SDT
 *
 * @param packet SDT packet
 * @return true on success
 */
bool ProgInfo::analyse_sdt(TSpacket * packet) {
    if (packet->payload[0] != 0x42) // service description section
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
    unsigned base = 11;
    while (index + base < section_len + 3 - 4) {
        unsigned service_id = get16b(packet->payload[index + base], packet->payload[index + base + 1]);

        unsigned desc_loop_len = get12b(packet->payload[index + base + 3], packet->payload[index + base + 4]);

        unsigned index2 = 0;
        while (index2 < desc_loop_len) {
            if (packet->payload[index + base + index2 + 5] == 0x48) { // service descriptor
                if (packet->payload[index + base + index2 + 5 + 2] == 0x01) { // digital television service
                    struct ServiceInfo * si = new ServiceInfo;

                    unsigned len = packet->payload[index + base + index2 + 5 + 3];
                    char * provider_name = new char[len + 1];
                    memcpy(provider_name, &(packet->payload[index + base +  index2 + 5 + 4]), len);
                    provider_name[len] = '\0';

                    si->provider = provider_name;
                    delete [] provider_name;

                    unsigned len2 = packet->payload[index + base + index2 + 5 + 3 + len + 1];
                    char * service_name = new char[len2 + 1];
                    memcpy(service_name, &(packet->payload[index + base + index2 + 5 + 3 + len + 2]), len2);
                    service_name[len2] = '\0';

                    si->name = service_name;
                    delete [] service_name;
                    Cfg::getInstance().data[service_id] = si;
                }
            }

            index2 += packet->payload[index + base + index2 + 5 + 1] + 2; /* descriptor len */
        }

        index = index + desc_loop_len + 5;
    }

    return true;
}

/**
 * Analyse PAT
 *
 * @param packet PAT to analyse
 * @return true on success
 */
bool ProgInfo::analyse_pat(TSpacket * packet) {
    if (packet->payload[0] != 0x00)
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

    unsigned base = 8;
    unsigned index = 0;
    while (index + base < section_len + 3 - 4) {
        unsigned program_no = get16b(packet->payload[index + base], packet->payload[index + base + 1]);
        unsigned pid = get12b(packet->payload[index + base + 2], packet->payload[index + base + 3]);

        if (program_no != 0) {
            Cfg::getInstance().programs.push_back(std::make_pair(program_no, pid));
        }

        index += 4;
    }

    return true;
}

/**
 * Analyse PMT
 *
 * @param packet PMT packet to analyse
 * @return true on success
 */
bool ProgInfo::analyse_pmt(TSpacket * packet) {
    if (packet->payload[0] != 0x02)
        return false;

    unsigned section_len = get12b(packet->payload[1], packet->payload[2]);
    //std::cerr << "section_len " << std::hex << section_len << std::endl;

    unsigned program_no = get16b(packet->payload[3], packet->payload[4]);
    //std::cerr << "program_no " << std::hex << program_no << std::endl;

    unsigned program_info_len = get12b(packet->payload[10], packet->payload[11]);
    //std::cerr << "program_info_len " << std::hex << program_info_len << std::endl;

    unsigned base = program_info_len + 12;
    unsigned index = 0;
    while (index + base < section_len + 3 - 4) {
        unsigned stream_type = packet->payload[index + base];
        //std::cerr << "stream_type " << std::hex << stream_type << std::endl;

        unsigned elementary_pid = get13b(packet->payload[index + base + 1], packet->payload[index + base + 2]);
        //std::cerr << "elementary_pid " << std::hex << elementary_pid << std::endl;

        if (stream_type == 0x02) { // video
            if (! Cfg::getInstance().videos_find(program_no)) {
                Cfg::getInstance().videos.push_back(std::make_pair(program_no, elementary_pid));
            }
        } else if (stream_type == 0x03) { // audio
            if (! Cfg::getInstance().audios_find(program_no)) {
                Cfg::getInstance().audios.push_back(std::make_pair(program_no, elementary_pid));
            }
        }

        unsigned es_len = get12b(packet->payload[base + index + 3], packet->payload[base + index + 4]);
        //std::cerr << "es_len " << std::hex << es_len << std::endl;

        index += es_len + 5;
        //std::cerr << index + base << " < " << section_len + 3 - 4 << std::endl;
    }

    return true;
}

/**
 * Analyse TOT
 *
 * @param packet TOT packet to analyse
 * @return true on success
 */
bool ProgInfo::analyse_tot(TSpacket * packet) {
    if (packet->payload[0] != 0x73) { // TOT?
        return false;
    }

    unsigned section_len = get12b(packet->payload[1], packet->payload[2]);
    //std::cerr << "section_len " << std::hex << section_len << std::endl;

    unsigned loop_len = get12b(packet->payload[8], packet->payload[9]);
    //std::cerr << "loop_len " << std::hex << loop_len << std::endl;

    unsigned index = 0;
    unsigned base = 10;
    while (index + base < section_len + 3 - 4) {
        unsigned desc_len = packet->payload[index + base + 1];
        unsigned index2 = 0;

        while (index2 < desc_len) {
            if (packet->payload[index + base] == 0x58) {
                unsigned offset_polarity = packet->payload[index + base + index2 + 2 + 3] & 1;
                //std::cerr << "local_time_offset_polarity " << offset_polarity << std::endl;
                Cfg::getInstance().local_time_offset_polarity = offset_polarity;

                unsigned local_time_offset = get16b(packet->payload[index + base + index2 + 2 + 4], packet->payload[index + base + index2 + 2 + 5]);
                //std::cerr << "local_time_offset " << local_time_offset << std::endl;
                Cfg::getInstance().local_time_offset = local_time_offset;
            }

            index2 += 13;
            //std::cerr << " >> " << index2 << " <  " << desc_len << std::endl;
        }

        index += desc_len + 2;
        //std::cerr << " > " << index + base << " < " <<  section_len + 3 - 4 << std::endl;
    }

    return true;
}

