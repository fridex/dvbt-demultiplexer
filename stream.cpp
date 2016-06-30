/**
 * BMS project 2
 * Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * @date 2013/12/05
 */

#include "stream.h"
#include "cfg.h"

/**
 * Get index of mpeg header
 *
 * @param packet packet to get index from
 * @return index of mpeg header
 */
int get_mpeg_header_idx(TSpacket * packet) {
    unsigned index = 0;

    index = index + 3; // skip start code prefix
    index = index + 1; // skip stream id
    index = index + 2; // skip packet len

    if (((packet->payload[5] >> 6) & 0x3) == 0x2) {
        //std::cerr << std::hex << (unsigned) packet->payload[5] << std::endl;
        //std::cerr << "optional header\n";
        index = index + 2; // PES header
        unsigned pes_header_len = packet->payload[5 + 2];
        //std::cerr << std::hex << pes_header_len << std::endl;
        index = index + pes_header_len;
    }

    return index;
}

/**
 * Remove trailing 0xFF bytes from packet
 *
 * @param packet packet to remove bytes from
 * @return void
 */
static void schrink_packet(TSpacket * packet) {
    for (int i = packet->payload.size() -1; i > 0; --i) {
        if (packet->payload[i] == 0xFF)
            packet->payload.resize(i);
        else
            break;
    }
}

/**
 * Parse audio/video packet
 *
 * http://www.cs.columbia.edu/~delbert/docs/Dueck%20--%20MPEG-2%20Video%20Transcoding.pdf
 * @param packet packet to parse
 * @return true on success
 */
bool Stream::parse(TSpacket * packet) {
    unsigned pid;
    if (Cfg::getInstance().audios_find_pid(packet->getPid(), &pid)) {
        for (int i = 0; i < packet->adaptation.size(); ++i)
            Cfg::getInstance().data[pid]->audio << packet->adaptation[i];

        for (int i = 0; i < packet->payload.size(); ++i)
            Cfg::getInstance().data[pid]->audio << packet->payload[i];
    } else if (Cfg::getInstance().videos_find_pid(packet->getPid(), &pid)) {
        schrink_packet(packet); // remove 0XFF at the end of packet
        if (! Cfg::getInstance().data[pid]->video_started) {
            int idx = -1;
            for (int i = 0; i < (int)packet->payload.size() - 4; ++i) {
                if (packet->payload[i] == 0x0
                        && packet->payload[i + 1] == 0x0
                        && packet->payload[i + 2] == 0x01
                        && packet->payload[i + 3] == 0xb3) {
                    idx = i;
                }
            }

            if (idx == -1)
                return true;

            for (int i = idx; i < packet->payload.size(); ++i)
                Cfg::getInstance().data[pid]->video << packet->payload[i];

            Cfg::getInstance().data[pid]->video_started = true;
        } else if (Cfg::getInstance().data[pid]->video_started && packet->header.payload_unit_start_indicator) {
            // remove pes header
            unsigned idx = get_mpeg_header_idx(packet);
            for (int i = idx; i < (int)packet->payload.size(); ++i)
                Cfg::getInstance().data[pid]->video << packet->payload[i];

        } else if (Cfg::getInstance().data[pid]->video_started) {
            // pure data
            for (int i = 0; i < (int)packet->adaptation.size(); ++i)
                Cfg::getInstance().data[pid]->video << packet->adaptation[i];
            for (int i = 0; i < (int)packet->payload.size(); ++i)
                Cfg::getInstance().data[pid]->video << packet->payload[i];
        }
    }

    return true;
}
