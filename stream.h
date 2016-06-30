/**
 * BMS project 2
 * Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * @date 2013/12/05
 */

#ifndef STREAM_H_
#define STREAM_H_

#include "tspacket.h"

/**
 * TS stream representation
 */
class Stream {
  public:
      static bool parse(TSpacket * packet);
      static bool parse(TSpacket & packet) { return parse(&packet); }

  private:
      Stream() { }
};

#endif // STREAM_H_

