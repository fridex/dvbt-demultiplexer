/**
 * BMS project 2
 * Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * @date 2013/12/05
 */

#include <iostream>
#include <fstream>
#include <cstdint>
#include <iomanip>
#include <cctype>

#include "tspacket.h"

/**
 * Parse packet
 *
 * @param raw_packet raw packet to parse
 * @return true on success
 */
bool TSpacket::parse(const TSraw_packet & raw_packet) {
    if (raw_packet[0] != TS_SYNC_BYTE) {
        TSDBG << std::hex << (unsigned) raw_packet[0] << std::endl;
        WARN << "Bad packet sync byte!\n";
        return false;
    }

    // clear the packet
    adaptation.clear();
    payload.clear();

    header.sync_byte                    = raw_packet[0];
    header.transport_error_indicator    = raw_packet[1] >> 7 & 0x1;
    header.payload_unit_start_indicator = raw_packet[1] >> 6 & 0x1;
    header.transport_priority           = raw_packet[1] >> 5 & 0x1;
    header.pid                          = (((unsigned) (raw_packet[1] & 0x1F) << 8)) | raw_packet[2];
    header.scrambling_control           = raw_packet[3] >> 6 & 0x3;
    header.adaptation_field             = raw_packet[3] >> 4 & 0x3;
    header.continuity_counter           = raw_packet[3] & 0xF;

    size_t adaptation_len = 0;
    adaptation.push_back(raw_packet[4]);
    if (header.adaptation_field == 0x2 || header.adaptation_field == 0x3) {
        // adaptation field exists
        adaptation_len = raw_packet[4];
        for (unsigned i = 0; i < adaptation_len; ++i)
            adaptation.push_back(raw_packet[5 + i]);
    }

    if (header.adaptation_field == 0x1 || header.adaptation_field == 0x3) {
        for (int i = 0; i < (int)kTSpayload_maxsize - (int)adaptation_len - 1; ++i) {
            //std::cerr << std::hex << "i: " << i << std::endl;
            //std::cerr << std::hex << (unsigned) raw_packet[5 + adaptation_len + i] << std::endl;
            payload.push_back(raw_packet[5 + adaptation_len + i]);
        }
    }

    return true;
}

/**
 * Print printable chars
 *
 * @param out output stream
 * @param packet packet to print
 * @param start starting index
 * @param len length to print
 * @return void
 */
void TSpacket::print_printable(std::ostream & out, TSraw_packet & packet, size_t start, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        if (isprint(packet[start + i]))
            out << " " << packet[start + i];
        else
            out << " .";
    }

    out << std::endl;
}

/**
 * Debug print of a packet
 *
 * @param out ouput stream
 * @return void
 */
void TSpacket::dbg_print(std::ostream & out) {
    out << "=== BEGIN DBG PRINT ===\n";
    out << "HEADER:\n";
    out << "\tsync_byte:\t\t\t0x"                   << std::hex << header.sync_byte
        << "\n\ttransport_error_indicator:\t"       << std::hex << header.transport_error_indicator
        << "\n\tpayload_unit_start_indicator:\t"    << std::hex << header.payload_unit_start_indicator
        << "\n\ttransport_priority:\t\t"            << std::hex << header.transport_priority
        << "\n\tpid:\t\t\t\t\t0x"                   << std::setw(4) << std::setfill('0') << std::hex << header.pid
        << "\n\tscrambling_control:\t\t"            << std::setw(0) << std::hex << header.scrambling_control
        << "\n\tadaptation_field:\t\t"              << std::hex << header.adaptation_field
        << "\n\tcontinuity_counter:\t\t0x"          << std::hex << header.continuity_counter;
    out << "\nADAPTATION [" << std::dec << adaptation.size() << "]:\n";
    for (unsigned i = 0; i < adaptation.size(); ++i) {
        out << std::hex << std::setw(2) << std::setfill('0') << (unsigned) adaptation[i];
        if ((i+1) % 8 == 0)
            out << std::endl;
        else if (i + 1 < adaptation.size())
            out << " ";
    }
    out << "\nPAYLOAD [" << std::dec << payload.size() << "]:\n";
    for (unsigned i = 0; i < payload.size(); ++i) {
        out << std::hex << std::setw(2) << std::setfill('0') << (unsigned) payload[i];
        if ((i+1) % 8 == 0) {
            out << "\t\t[" << std::setw(3) << std::setfill(' ') << std::dec << i - 7 << "]";
            print_printable(out, payload, i - 7, 8);
            out << std::endl;
        } else if (i + 1 < payload.size())
            out << " ";
    }
    // print reminder
    out << "\t\t[" << std::setw(3) << std::setfill(' ') << std::dec << payload.size()  - payload.size() % 8 << "]";
    print_printable(out, payload, payload.size() - payload.size() % 8, payload.size() % 8);

    out << "\n=== END DBG PRINT ===\n";
}

/**
 * Print raw packet
 *
 * @param out output stream
 * @return void
 */
void TSpacket::raw_print(std::ostream & out) {
    //header
    uint32_t h;

    h =
        (((uint32_t) header.sync_byte) << 24)                       //:  8;
        | (((uint32_t) header.transport_error_indicator) << 23)     //:  1;
        | (((uint32_t) header.payload_unit_start_indicator) << 22)  //:  1;
        | (((uint32_t) header.transport_priority)<< 21)             //:  1;
        | (((uint32_t) header.pid) << 8)                            //: 13;
        | (((uint32_t) header.scrambling_control) << 6)             //:  2;
        | (((uint32_t) header.adaptation_field) << 4)               //:  2;
        | (((uint32_t) header.continuity_counter));                 //:  4;

    out << (char) (h >> 24);
    out << (char) (h >> 16);
    out << (char) (h >> 8);
    out << (char) (h);


    //adaptation
    for (size_t i = 0; i < adaptation.size(); ++i) {
        out << adaptation[i];
    }
    //payload
    for (size_t i = 0; i < payload.size(); ++i) {
        out << payload[i];
    }
}

