#ifndef __SYS_OSIFTYPES_H__
#define __SYS_OSIFTYPES_H__

#include <linux/types.h>

typedef   void VOID;

typedef   u8   UINT8;
typedef   s8   INT8;
typedef   u16  UINT16;
typedef   s16  INT16;
typedef   u32  UINT32;
typedef   s32  INT32;
typedef   u64  UINT64;
typedef   s64  INT64;
typedef   char CHAR8;

typedef   unsigned long UINTPTR;
typedef   signed long   INTPTR;

/* Some definitions for POSIX conform constant declaration,
 * not present in all linux kernels */
#ifndef INT32_C
# define INT32_C(c)   (c ## L)
# define UINT32_C(c)  (c ## UL)
# define INT64_C(c)   (c ## LL)
# define UINT64_C(c)  (c ## ULL)
#endif

#endif /* __SYS_OSIFTYPES_H__ */
