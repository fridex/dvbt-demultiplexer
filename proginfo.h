/**
 * BMS project 2
 * Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * @date 2013/12/05
 */

#ifndef PROGINFO_H_
#define PROGINFO_H_

#include "tspacket.h"

/**
 * Get program info from SDT/PAT/PMT/TOT
 */
class ProgInfo {
  public:
    static ProgInfo & getInstance() { static ProgInfo singleton; return singleton; }
    bool analyse_sdt(TSpacket * packet);
    bool analyse_sdt(TSpacket & packet) { return analyse_sdt(&packet); }

    bool analyse_pat(TSpacket * packet);
    bool analyse_pat(TSpacket & packet) { return analyse_pat(&packet); }

    bool analyse_pmt(TSpacket * packet);
    bool analyse_pmt(TSpacket & packet) { return analyse_pmt(&packet); }

    bool analyse_tot(TSpacket * packet);
    bool analyse_tot(TSpacket & packet) { return analyse_tot(&packet); }

  private:
    ProgInfo() { }
};

#endif // PROGINFO_H_

