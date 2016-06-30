/**
 * BMS project 2
 * Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * @date 2013/12/05
 */

#ifndef TSSTREM_H_
#define TSSTREM_H_

#include <iostream>
#include <fstream>
#include <cerrno>
#include <list>
#include <cassert>
#include <map>

#include "ts.h"
#include "tspacket.h"

/**
 * Stream representation
 */
class TSstream {
  private:
    static const unsigned kNITunknown = ~0;
    std::ifstream m_in;
    std::list<TSpacket *> m_buffer;
    std::map<TSpid, unsigned> m_pid_map;

    TSstream() {
        m_nit_network_name  = 0;
        m_nit_network_id    = kNITunknown;
        m_nit_bendwidth     = kNITunknown;
        m_nit_constellation = kNITunknown;
        m_nit_guard         = kNITunknown;
        m_nit_code_rate     = kNITunknown;
        m_update_info       = false;
    }
    ~TSstream() { m_in.close(); if (m_nit_network_name) delete [] m_nit_network_name; }

  public:
    bool m_update_info;
    bool getPacket(TSpacket * packet, TSpid pid = TS_PID_ANY, unsigned continuity = 0);
    bool init(const char * fname);
    void print_stats(std::ostream & out);

    bool eof() const;
    bool empty() const;
    void reset();
    static TSstream & getInstance() { static TSstream singleton; return singleton; }

    bool updateNITinfo(TSpacket * packet);
    bool updateNITinfo(TSpacket & packet) { return updateNITinfo(&packet); }

  private:
    double getTotalBitRate();
    bool readPacket(TSraw_packet & raw_packet);
    bool readPacket(TSraw_packet * raw_packet) {
        assert(raw_packet);
        return readPacket(*raw_packet);
    }

    char   * m_nit_network_name;
    unsigned m_nit_network_id;
    unsigned m_nit_bendwidth;
    unsigned m_nit_constellation;
    unsigned m_nit_guard;
    unsigned m_nit_code_rate;
};

/**
 * Are we at the end of stream?
 */
inline bool TSstream::eof() const { return m_in.eof(); }

/**
 * Is stream empty?
 */
inline bool TSstream::empty() const { return m_in.eof() && m_buffer.empty(); }

/**
 * Reset the stream
 */
inline void TSstream::reset() {
    m_in.clear();
    m_in.seekg(0, std::ios::beg);
    for (auto it = m_buffer.begin(); it != m_buffer.end(); ) {
        delete (*it);
        it = m_buffer.erase(it);
    }
}


#endif // TSSTREM_H_

