/**
 * BMS project 2
 * Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * @date 2013/12/05
 */

#ifndef EPG_H_
#define EPG_H_

#include "tspacket.h"

/**
 * Get EPG info
 */
class EPGinfo {
  public:
    bool analyse(TSpacket * packet);
    bool analyse(TSpacket & packet) { return analyse(&packet); }
    static EPGinfo & getInstance() { static EPGinfo singleton; return singleton; }

    bool analyse_epg(TSpacket * packet, bool present);

  private:
    EPGinfo() {}
};

#endif // EPG_H_

