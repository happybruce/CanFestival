#ifndef __CANFESTIVAL_SYSDEP_H__
#define __CANFESTIVAL_SYSDEP_H__

#include "config.h"

#ifdef CANOPEN_BIG_ENDIAN

/* Warning: the argument must not update pointers, e.g. *p++ */

#define UNS16_LE(v)  ((((UNS16)(v) & 0xFF00) >> 8) | \
              (((UNS16)(v) & 0x00FF) << 8))

#define UNS32_LE(v)  ((((UNS32)(v) & 0xFF000000) >> 24) |    \
              (((UNS32)(v) & 0x00FF0000) >> 8)  |    \
              (((UNS32)(v) & 0x0000FF00) << 8)  |    \
              (((UNS32)(v) & 0x000000FF) << 24))

#else

#define UNS16_LE(v)  (v)

#define UNS32_LE(v)  (v)

#endif /* CANOPEN_BIG_ENDIAN */

#endif /* __CANFESTIVAL_SYSDEP_H__ */

