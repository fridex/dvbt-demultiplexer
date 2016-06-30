/**
 * BMS project 2
 * Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * @date 2013/12/05
 */

#include <cassert>
#include <cstring>
#include <functional>
#include <algorithm>
#include <iomanip>

#include "tsstream.h"

#include "cfg.h"

static const char * UNKNOWN = "UNKNOWN";

/**
 * Print bendwidth in human readable form
 *
 * @param bw NIT code
 * @return string
 */
static const char * getBendwidth(unsigned bw) {
    const char *s = UNKNOWN;

    switch (bw) {
        case 0: s = "8 MHz"; break;
        case 1: s = "7 MHz"; break;
        case 2: s = "6 MHz"; break;
        case 3: s = "5 MHz"; break;
    }

    return s;
}

/**
 * Print constellation in human readable form
 *
 * @param c NIT code
 * @return string
 */
static const char * getConstellation(unsigned c) {
    const char *s = UNKNOWN;

    switch (c) {
        case 0: s = "QPSK"; break;
        case 1: s = "16-QAM"; break;
        case 2: s = "64-QAM"; break;
    }

    return s;
}

/**
 * Print guard inerval in human readable form
 *
 * @param gi NIT code
 * @return string
 */
static const char * getGuardInterval(unsigned gi) {
    const char *s = UNKNOWN;

    switch (gi) {
        case 0: s = "1/32"; break;
        case 1: s = "1/16"; break;
        case 2: s = "1/8"; break;
        case 3: s = "1/4"; break;
    }

    return s;
}

/**
 * Print code rate in human readable form
 *
 * @param cr NIT code
 * @return string
 */
static const char * getCodeRate(unsigned cr) {
    const char *s = UNKNOWN;

    switch (cr) {
        case 0: s = "1/2"; break;
        case 1: s = "2/3"; break;
        case 2: s = "3/4"; break;
        case 3: s = "5/6"; break;
        case 4: s = "7/8"; break;
    }

    return s;
}

/**
 * Is first less than second?
 */
template<class T>
struct less_first : std::binary_function<T,T,bool> {
    inline bool operator()(const T& lhs, const T& rhs) {
        return lhs.second > rhs.second;
    }
};

#include "table.h"

/**
 * Print stream statistics
 *
 * @param out output stream
 * @return void
 */
void TSstream::print_stats(std::ostream & out) {

    if (m_nit_network_name)
        out << "Network name: "     << m_nit_network_name                << std::endl;
    else
        out << "Network name: "     << UNKNOWN                           << std::endl;

    if (m_nit_network_id != kNITunknown)
        out << "Network ID: "       << std::dec << m_nit_network_id      << std::endl;
    else
        out << "Network ID: "       << std::dec << UNKNOWN               << std::endl;

    out << "Bandwidth: "        << getBendwidth(m_nit_bendwidth)         << std::endl;
    out << "Constellation: "    << getConstellation(m_nit_constellation) << std::endl;
    out << "Guard interval: "   << getGuardInterval(m_nit_guard)         << std::endl;
    out << "Code rate: "        << getCodeRate(m_nit_code_rate)          << std::endl;

    out << std::endl;

    unsigned total = 0;

    // count total
    for (auto it = m_pid_map.begin(); it != m_pid_map.end(); ++it)
            total += it->second;

    // sort by rate
    typedef std::pair<TSpid, unsigned> data_t;
    std::vector<data_t> vec(m_pid_map.begin(), m_pid_map.end());
    std::sort(vec.begin(), vec.end(), less_first<data_t>());

    // print rates
    out << "Bitrate:\n";
    for (auto it = vec.begin(); it != vec.end(); ++it)
        out << "0x" << std::setfill('0')
            << std::setw(4) << std::hex << it->first
            << " "
            << std::dec << std::setprecision(2) << std::fixed
            << (double)it->second / total * getTotalBitRate() << " Mbps\n";
}

/**
 * Update NIT info from NIT packet
 *
 * @param packet a NIT packet
 * @return true if info was updated
 */
bool TSstream::updateNITinfo(TSpacket * packet) {
    m_nit_network_id = get16b(packet->payload[3], packet->payload[4]);

    if (packet->payload[0] != 0x40) // network information section - actual network
        return false;

    unsigned section_len = get12b(packet->payload[1], packet->payload[2]);
    unsigned continuity = packet->header.continuity_counter;
    while (section_len > packet->payload.size()) {
        TSpacket packet2;

        packet->payload.reserve(packet->payload.size() + packet2.payload.size());

        continuity = (continuity + 1) % 16;
        TSstream::getInstance().getPacket(&packet2, packet->getPid(), continuity);
        packet->payload.insert(packet->payload.end(), packet2.adaptation.begin(), packet2.adaptation.end());
        packet->payload.insert(packet->payload.end(), packet2.payload.begin(), packet2.payload.end());
    }

    size_t network_descriptor_length = packet->payload[9];
    size_t index = 0;
    while (index < network_descriptor_length) {
        if (packet->payload[10 + index] == 0x40) { // network name descriptor
            size_t len = 0;
            len = packet->payload[10 + index + 1];
            m_nit_network_name = new char[len + 1];
            memcpy(m_nit_network_name, &packet->payload[10 + index + 2], len);
            m_nit_network_name[len] = '\0';

            //std::cerr << std::hex << (unsigned) len << "\nindex " << index << "\nname " << m_nit_network_name << std::endl;
        }
        index = index + packet->payload[11 + index] + 2;
    }

    size_t transport_stream_length = get12b(packet->payload[10 + network_descriptor_length],
                                                packet->payload[11 + network_descriptor_length]);
    size_t transport_descriptor_length = 0;
    size_t base = 12 + network_descriptor_length;
    index = 0;
    size_t index1 = 0;
    size_t index2 = 0;
    size_t size = 0;

    //std::cerr << std::hex;
    while (index1 < transport_stream_length) {
        //std::cerr << "S: index1 < transport_stream_length " << index1 << " < " << transport_stream_length << std::endl;
        transport_descriptor_length = get12b(packet->payload[base + index1 + 4],
                                             packet->payload[base + index1 + 5]);

        index2 = 0;
        while (index2 < transport_descriptor_length) {
            //std::cerr << "S: index2 < transport_descriptor_length " << index2 << " < " << transport_descriptor_length << std::endl;
            if (packet->payload[base + index1 + 6 + index2] == 0x5a) { // terrestrial delivery system descriptor
                m_nit_bendwidth = (packet->payload[base + index1 + 6 + index2 + 6] >> 5) & 0x7;
                m_nit_constellation = (packet->payload[base + index1 + 6 + index2 + 7] >> 6) & 0x3;
                m_nit_code_rate = packet->payload[base + index1 + 6 + index2 + 7] & 0x7;
                m_nit_guard = (packet->payload[base + index1 + 6 + index2 + 8] >> 3) & 0x3;
            }

            index2 = index2 + packet->payload[base + index1 + 7 + index2] + 2;
            //std::cerr << "E: index2 < transport_descriptor_length " << index2 << " < " << transport_descriptor_length << std::endl;
        }

        index1 = 6 + transport_descriptor_length;
        //std::cerr << "E: index1 < transport_stream_length " << index1 << " < " << transport_stream_length << std::endl;
    }

    return true;
}

