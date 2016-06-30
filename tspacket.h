#ifndef TSPACKET_H_
#define TSPACKET_H_

#include <vector>
#include <cstring>
#include <cassert>

#include "ts.h"

typedef std::vector<TSbyte>     TSadaptation;
typedef std::vector<TSbyte>     TSpayload;
typedef std::vector<TSbyte>     TSraw_packet;

class TSpacket {
  public:
    static const size_t kTSheader_size = 4;
    static const size_t kTSpacket_size = 188;
    static const size_t kTSpayload_maxsize = kTSpacket_size - kTSheader_size;


    struct TSheader {
        unsigned sync_byte                    ; //:  8;
        unsigned transport_error_indicator    ; //:  1;
        unsigned payload_unit_start_indicator ; //:  1;
        unsigned transport_priority           ; //:  1;
        unsigned pid                          ; //: 13;
        unsigned scrambling_control           ; //:  2;
        unsigned adaptation_field             ; //:  2;
        unsigned continuity_counter           ; //:  4;
    };

    TSheader        header;
    TSadaptation    adaptation;
    TSpayload       payload;

    unsigned getPid() { return header.pid; }
    bool parse(const TSraw_packet & raw_packet);
    bool parse(const TSraw_packet * raw_packet) {
        assert(raw_packet);
        return parse(*raw_packet);
    }

    void print_printable(std::ostream & out, TSraw_packet & packet, size_t start, size_t end);
    void dbg_print(std::ostream & out);
    void raw_print(std::ostream & out);
};

#endif // TSPACKET_H_

