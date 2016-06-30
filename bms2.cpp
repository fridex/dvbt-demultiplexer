/**
 * BMS project 2
 * Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * @date 2013/12/05
 */

#include "ts.h"
#include "tsstream.h"
#include "tspacket.h"
#include "epginfo.h"
#include "proginfo.h"
#include "cfg.h"
#include "stream.h"

/**
 * Return values
 */
enum {
    RET_OK,
    RET_HELP,
    RET_ERR_FILE,
    RET_ERR
};

const char * HELP_START = "Demultiplex transport stream\n\nUsage:\t";
const char * HELP_END   = "[OPTION|FILE]\n"
                          "\n\tFILE\t- file with stream\n\t-h\t- print this help\n\n"
                          "Written by Fridolin Pokorny <fridex.devel@gmail.com>\n";

/**
 * Print help
 * @param pname program name
 * @return always RET_HELP
 */
int print_help(const char * pname) {
    std::cerr << HELP_START
              << pname
              << HELP_END;

    return RET_HELP;
}

/**
 * Entry point ;)
 *
 * @param argc argument count
 * @param argv argument vector
 * @return RET_OK on success otherwise error code
 */
int main(int argc, char * argv[]) {
    bool pat_analysed = false;
    bool sdt_analysed = false;
    bool nit_analysed = false;
    bool pmt_analysed = false;
    bool tot_analysed = false;
    bool dirs_created  = false;

    if (argc != 2 || ! strcmp(argv[1], "-h"))
        return print_help(argv[0]);

    if (! TSstream::getInstance().init(argv[1]))
        return RET_ERR_FILE;

    std::string fname = argv[1];
    if (! Cfg::getInstance().filename(fname))
        return RET_ERR_FILE;

    // http://www.etsi.org/deliver/etsi_en/300400_300499/300468/01.13.01_40/en_300468v011301o.pdf
    // https://www.fit.vutbr.cz/study/courses/BMS/public/proj2013/p2.html
    // http://soominho.tistory.com/214
    //  http://cmm.khu.ac.kr/korean/files/02.mpeg2ts1_es_pes_ps_ts_psi.pdf
    TSpacket packet;
    while (! TSstream::getInstance().empty()) {
        if (TSstream::getInstance().getPacket(&packet)) {
            if (packet.header.transport_error_indicator)
                continue;

            if (packet.getPid() == TS_PID_NIT) {
                TSstream::getInstance().updateNITinfo(packet);
            } else if (dirs_created && sdt_analysed && packet.getPid() == TS_PID_EIT) {
                EPGinfo::getInstance().analyse(packet);
            } else if (! sdt_analysed && packet.getPid() == TS_PID_SDT) {
                sdt_analysed = ProgInfo::getInstance().analyse_sdt(packet);
            } else if (! pat_analysed && packet.getPid() == TS_PID_PAT) {
                pat_analysed = ProgInfo::getInstance().analyse_pat(packet);
            } else if (pat_analysed && Cfg::getInstance().search_pat(packet.getPid())) {
                ProgInfo::getInstance().analyse_pmt(packet);
            } else if (packet.getPid() == TS_PID_TOT) {
                ProgInfo::getInstance().analyse_tot(packet);
            } else if (dirs_created) {
                Stream::parse(packet);
            }

            if (pat_analysed && sdt_analysed && ! dirs_created) {
                if (! Cfg::getInstance().create_dirs())
                    return RET_ERR_FILE;
                TSstream::getInstance().m_update_info = true;
                TSstream::getInstance().reset();
                dirs_created = true;
            }
        }
    }
#if 0


    TSpacket packet;
    while (! TSstream::getInstance().empty()) {
        if (TSstream::getInstance().getPacket(&packet)) {
            if (packet.header.transport_error_indicator)
                continue;

            if (! nit_analysed && packet.getPid() == TS_PID_NIT) {
                nit_analysed = TSstream::getInstance().updateNITinfo(packet);
            } else if (! sdt_analysed && packet.getPid() == TS_PID_SDT) {
                sdt_analysed = ProgInfo::getInstance().analyse_sdt(packet);
            } else if (! pat_analysed && packet.getPid() == TS_PID_PAT) {
                pat_analysed = ProgInfo::getInstance().analyse_pat(packet);
            } else if (!pmt_analysed && pat_analysed && Cfg::getInstance().search_pat(packet.getPid())) {
                pmt_analysed = ProgInfo::getInstance().analyse_pmt(packet);
            } else if (! tot_analysed && packet.getPid() == TS_PID_TOT) {
                tot_analysed = ProgInfo::getInstance().analyse_tot(packet);
            }

            if (pat_analysed && sdt_analysed
                    && nit_analysed && pmt_analysed && tot_analysed)
                break;
        }
    }
    if (! Cfg::getInstance().create_dirs())
        return RET_ERR_FILE;
    TSstream::getInstance().m_update_info = true;
    TSstream::getInstance().reset();


    // second run
    while (! TSstream::getInstance().empty()) {
        if (TSstream::getInstance().getPacket(&packet)) {
            if (packet.header.transport_error_indicator)
                continue;

            if (packet.getPid() == TS_PID_EIT)
                    EPGinfo::getInstance().analyse(packet);
            else
                Stream::parse(packet);
        }
    }
#endif

    // write stats to info.txt
    if (! Cfg::getInstance().open_info())
        return RET_ERR_FILE;
    TSstream::getInstance().print_stats(Cfg::getInstance().info);
    Cfg::getInstance().close_info();

    // flush all data
    if (! Cfg::getInstance().flush())
        return RET_ERR_FILE;

    return RET_OK;
}

