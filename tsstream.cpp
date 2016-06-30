/**
 * BMS project 2
 * Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * @date 2013/12/05
 */

#include <cassert>
#include <cstring>

#include "tsstream.h"

//#define DBG_getPacket if (1)
#define DBG_getPacket if (0)

/**
 * Get next packet from stream
 *
 * @param packet output packet
 * @param pid get packet with specified pid
 * @param contiuity check continuity if any
 * @return
 */
bool TSstream::getPacket(TSpacket * packet, TSpid pid /* = TS_PID_ANY */, unsigned continuity) {
assert(packet);

    DBG_getPacket TSDBG << "Looking for " << std::hex << pid << std::endl;

    if (! m_buffer.empty()) {
        for (auto it = m_buffer.begin(); it != m_buffer.end(); ++it) {
            if (pid == TS_PID_ANY || ((*it)->getPid() == pid && (*it)->header.continuity_counter == continuity)) {
                DBG_getPacket TSDBG << "Pop packet PID: 0x" << std::hex << (*it)->getPid() << std::endl;
                *packet = *(*it);
                delete (*it);
                m_buffer.erase(it);
                return true;
            }
        }
    }

    if (m_in.eof()) {
        TSDBG << std::endl;
        return false;
    }

    TSraw_packet   raw_packet;
    TSpacket     * new_packet;

    do {
        new_packet = new TSpacket;

        if (! readPacket(raw_packet)) {
            TSDBG << std::endl;
            return false;
        } else {
            DBG_getPacket TSDBG << "Read new raw packet\n";
        }

        if (! new_packet->parse(raw_packet)) {
            TSDBG << std::endl;
            return false;
        } else {
            DBG_getPacket TSDBG << "Parsed new packet\n";
        }

        // update stats
        if (m_update_info)
            m_pid_map[new_packet->getPid()]++;

        if (pid == TS_PID_ANY || (new_packet->getPid() == pid && new_packet->header.continuity_counter == continuity)) {
            DBG_getPacket TSDBG << std::hex << "Returning packet PID: 0x" << new_packet->getPid() << std::endl;
            *packet = *new_packet;
            delete new_packet;
            return true;
        } else {
            DBG_getPacket TSDBG << std::hex << "Buffering packet PID: 0x" << new_packet->getPid() << std::endl;
            m_buffer.push_back(new_packet);
        }

    } while (! m_in.eof());

    return true;
}

/**
 * Read raw packet from input
 *
 * @param param output raw packet
 * @return true on success
 */
bool TSstream::readPacket(TSraw_packet & raw_packet) {
    raw_packet.clear();

    for (size_t i = 0; i < TSpacket::kTSpacket_size; ++i) {
        if (m_in.good() && ! m_in.eof()) {
            raw_packet.push_back(m_in.get());
        } else {
            TSDBG << i << std::endl;
            WARN << "Failed to read a whole packet (eof?)!\n";
            return false;
        }
    }

    return true;
}

/**
 * Init packet stream
 *
 * @param fname name of the file with stream
 * @return true on success
 */
bool TSstream::init(const char * fname) {
    m_in.open(fname, std::ios::binary);

    if (! m_in.is_open()) {
        ERROR << fname << ":"
                 << strerror(errno) << std::endl;
        TSDBG << std::endl;
        return false;
    } else
        return true;
}


