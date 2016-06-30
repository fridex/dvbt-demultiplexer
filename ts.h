/**
 * BMS project 2
 * Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * @date 2013/12/05
 */

#ifndef TS_H_
#define TS_H_

#include <iostream>

typedef unsigned        TSpid;
typedef unsigned char   TSbyte;

#define TS_SYNC_BYTE    ((TSbyte) 0x47)

#define TS_PID_PAT      ((TSpid) 0x0000)
#define TS_PID_CAT      ((TSpid) 0x0001)
#define TS_PID_NIT      ((TSpid) 0x0010)
#define TS_PID_SDT      ((TSpid) 0x0011)
#define TS_PID_EIT      ((TSpid) 0x0012)
#define TS_PID_TOT      ((TSpid) 0x0014)
#define TS_PID_ANY      ((TSpid) ~0)

#define TS_PID_STREAM1  ((TSpid) 0x0020)
#define TS_PID_STREAM2  ((TSpid) 0x1FFA)

// DEBUG???
//#define TS_DEBUG

#ifdef TS_DEBUG
# define TSDBG          std::cerr << ">>> DBG: (" << __FILE__ << ":" \
                                  << __func__ << ":" << __LINE__ << ") "
#else
# define TSDBG          while(0) std::cerr
#endif

#ifdef TS_DEBUG
# define WARN            std::cerr << "Warning (" << __FILE__ << ":" \
                                  << __func__ << ":" << __LINE__ << "): "
#else
# define WARN            std::cerr << "Warning: "
#endif

#ifdef TS_DEBUG
# define ERROR           std::cerr << "Error (" << __FILE__ << ":" \
                                  << __func__ << ":" << __LINE__ << "): "
#else
# define ERROR           std::cerr << "Error: "
#endif

#define get12b(A, B)    (((((unsigned) A) & 0xF) << 8) | B)
#define get13b(A, B)    (((((unsigned) A) & 0x1F) << 8) | B)
#define get16b(A, B)    ((((unsigned) A) << 8) | B)

#endif // TS_H_

