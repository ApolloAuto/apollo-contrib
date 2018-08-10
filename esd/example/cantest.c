/************************************************************************/
/*                                                                      */
/*   Test/Demonstration program for CAN driver with NTCAN-API           */
/*                                                                      */
/*          Copyright 1997 - 2016 esd - electronic system design gmbh   */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*      Filename:      cantest.c                                        */
/*      Date:          09.11.99                                         */
/*      Language:      ANSI C                                           */
/*      Targetsystem:  Win NT/2K/XP/Vista/7/8+, Win 9x/ME, WinCE, QNX,  */
/*                     UNIX, VxWorks, Linux (RTAI/RT), NET+OS, RTOS-UH, */
/*                     RTX/RTX64, OnTime RTOS-32                        */
/*                                                                      */
/*      Purpose:       Test and demonstration for drivers which support */
/*                     NTCAN-API                                        */
/*----------------------------------------------------------------------*/
/* Revision history:                                                    */
/*----------------------------------------------------------------------*/
/* 2C6,16feb16,ot  * Fixed MinGW (GCC) warnings                         */
/* 2C5,20apr15,mk  * Fixed transceiver constants                        */
/* 2C4,08feb15,ot  * Decode CAN transceiver type returned in NTCAN_INFO */
/*                 *  and indicate required FW update in help()         */
/* 2C3,26jan15,mk  * Retry canOpen without NTCAN_MODE_FD                */
/* 2C2,25aug14,ot  * Added support for evaluation of NTCAN_INFO and     */
/*                 *  extended features.                                */
/* 2C1,21aug14,mk  * Adapted to new NTCAN macro names                   */
/* 2C0,19jun14,mt  * New tests 60-63, 66, 72, 73, 82, 100, 110          */
/*                 *  to support the NTCAN API functions based on       */
/*                 *  CMSG_X (CAN-FD ready) messages.                   */
/* 2B7,12dec13,mk  * Added dummy signal handler for SIGUSR1 (unix only) */
/* 2B6,15jul13,ju  * Fix for NTCAN_IOCTL_GET_TX_TS_WIN: TX counter in   */
/*                 *  CAN data accidentally was incremented by two      */
/* 2B5,17may13,stm * Crude fix for the output of the fraction of the    */
/*                 *  TimestampFreq.                                    */
/* 2B4,26apr13,bl  * Changed SLEEP macros for Linux and QNX             */
/* 2B3,11jan13,ot  * Support to dynamically load canSendT()/canWriteT() */
/* 2B2,10jan13,ot  * Decode features IRIG-B and TIMESTAMPED_TX          */
/* 2B1,16nov12,ot  * New test -3 like -2 with decoded feature flags     */
/*                 * Added controller state information in overview     */
/* 2B0,14aug12,ot  * Allocated test context instead of using of global  */
/*                 *  variables to support more than one instance on    */
/*                 *  operating systems with common global namespace    */
/*                 * Fixed canOverlappedT() and canFormatError() not    */
/*                 *  loaded dynamically which breaks compatibility to  */
/*                 *  very old versions of NTCAN library on Windows     */
/* 2A5,24may12,fj  * Added support for RTOS-32 (#define RTOS32)         */
/* 2A4,23mar12,ot  * Show timestamp frequency and timestamp in overview */
/*                 *  correctly without 64-Bit support in printf()      */
/* 2A3,11aug11,ot  * Added test 50 and use Tx timeout in canSend test   */
/*                 *  as delay between bursts.                          */
/* 2A2,22jun11,ot  * Tweak some defaults according to test type         */
/* 2A1,22mar11,ot  * Allow special constants "auto", "disable", "no"    */
/*                 *  for parameter baudrate to configure auto baudrate */
/*                 *  disable device or no baudrate change              */
/*                 * Display CAN controller clock in device overview    */
/* 2A0,23jul10,mk  * Added -1 for baudrate does not set baudrate        */
/*                 * Added negative test numbers lead to help output    */
/*                 * Test -2 leads to shorter output (no cmdline help)  */
/* 29F,06jul10,ot  * Added plain text for some more controller types    */
/*                 * Support extended EVMSG union in print_event()      */
/* 29E,21may10,mk  * Added use of canFormatEvent                        */
/* 29D,15apr10,bl  * Changed CAN node information output a bit          */
/*                 * Changed output of CMSGs a bit                      */
/* 29C,08apr10,ot  * Different output format in print_cmsg() for        */
/*                 *  object mode results and support for interaction   */
/*                 *  marking in FIFO mode                              */
/*                 * A negative value for 'count' will open the handle  */
/*                 *  in 'mark interaction' mode for blocking Rx tests  */
/*                 * A value of 0 for 'count' will open the handle      */
/*                 *  in 'no interaction' mode for blocking Rx tests    */
/*                 * A negative value for 'count' will send RTR instead */
/*                 *  of data frames for blocking/non-blocking Tx tests */
/*                 * Decode CAN controller type in 'boardstatus'        */
/*                 * Textual description of extended error events       */
/* 29B,13jan10,ot  * Fixed canTest() called without args with VxWorks   */
/*                 *  does not always shows the expected behaviour.     */
/* 29A,26aug09,mk  * Removed tests from d3xtest                         */
/* 299,25aug09,mk  * Prevent deprecated warning for d3xtest (for DDK)   */
/* 298,10jun09,stm * Made compilable again under RTOS-UH for driver     */
/*                 * release 2.2.4.                                     */
/* 297,05feb09,ot  * Support to configure bus load event period         */
/* 296,12sep08,mk  * Repaired busload event decoding to work again      */
/* 295,21aug08,ot  * New entry print_event() as common code to decode   */
/*                 *  events                                            */
/* 294,03jan08,ot  * Test 4 and test 5 are changed to avoid using the   */
/*                 *  obsolete API calls canReadEvent()/canSendEvent()  */
/*                 * New test 16 to check Win32 overlapped canReadT()   */
/*                 * Moved code to create event for overlapped I/O to   */
/*                 *  the initialization part of the test.              */
/*                 * Added missing cleanup code for overlapped tests.   */
/*                 * Moved boilerplate code to get timestamp frequency  */
/*                 *  into get_timestamp_freq()                         */
/*                 * Moved boilerplate code to enable a CAN identifier  */
/*                 *  into common set_can_id()                          */
/* 293,21jan08,mt  * New test 43 for timestamp sanity check.            */
/*                 * Test 100 changed to test sched disable/enable      */
/*                 *  feature and sched counters                        */
/* 292,09nov07,mk  * Added nice busload output for test 23              */
/*                 * Baudrate is now set after adding event ids         */
/* 291,15aug07,ot  * Check if hardware/driver supports timestamps       */
/*                 *  before performing related tests.                  */
/*                 * Adapted to changes in Windows CE 6.x environment   */
/* 290,19jul07,ot  * Try loading ntcan64.dll dynamically in addition to */
/*                 *  ntcan.dll if running on Windows 64-Bit            */
/*     25may07,mt  * Sending 0 byte-frames if 1st data arg is -1        */
/* 289,22feb07,fj  * Fixed some compiler warnings (linux, gcc-4.1)      */
/* 288,31jan07,ot  * Support for canFormatError()                       */
/*                 * Changed duration time unit for Win32 from ms to us */
/* 287,13oct06,ot  * Changed preprocessor check if scheduling is        */
/*                 *  supported.                                        */
/* 286,17feb06,mk  * Added test 54 to check closing of duped handles    */
/* 285,11oct06,bl  * Added detailed output of baudrate change event     */
/*                 *  to event read test (4)                            */
/* 284,31jul06,stm * To be compiled successfully under VW54 (rel 2.4.1) */
/*                 * the variable <udtTimestampFreq> also needs to be   */
/*                 * excluded using the define NTCAN_IOCTL_GET_TIMESTAMP*/
/* 283,05jul06,ot  * Wait configured tx timeout after test 0 before     */
/*                 *  closing the handle, to prevent driver discarding  */
/*                 *  pending messages if delayed close is unsupported. */
/* 282,12jun06,ot  * Force thread affinity to one processor or core for */
/*                 *  Win32 kernel with SMP support to prevent wrong    */
/*                 *  time differences caused by CPU TSCs out of sync.  */
/* 281,14mar06,ot  * Support for dynamically loaded entries for Win32   */
/*                 *  to make current version run with old NTCAN lib    */
/* 280,03feb06,ot  * Changed test 22 to 32 to be orthogonal with read   */
/*                 * Added test 22 canTakeT() and 42 canTakeT() in      */
/*                 *  object mode                                       */
/*                 * Different output format in print_cmsg_t() for      */
/*                 *  object mode results.                              */
/* 275,23nov05,ot  * Time difference in print_cmsg_t() calculated and   */
/*                 *  displayed for each frame type.                    */
/* 274,23nov05,mk  * Added test 53 to check NTCAN_IOCTL_GET_TIMESTAMP   */
/* 273,21nov05,mf  * Mask idstart for 20B filter                        */
/* 272,12sep05,ot  * New entries print_cmsg() and print_cmsg_t() to     */
/*                 *  remove redundant code                             */
/* 271,08sep05,ot  * Added auto RTR test (8)                            */
/* 270,04aug05,ot  * Support for serial number and 20B filter mask if   */
/*                 *  supported by driver                               */
/*                 * Using strtoul() instead of atoi() for converting   */
/*                 *  data part of command line                         */
/*                 * Fixed last net is 255 instead 254 in help()        */
/*                 * Use namespace clean NTCAN_HANDLE instead of HANDLE */
/* 2615,12jul05,mt * Support for MAIN_CALLTYPE                          */
/* 2614,07may05,ot * Missing defines for 64 bit support in Windows      */
/*                 * NET+OS 6.x support                                 */
/*                 * Additional error codes in get_error_str()          */
/* 2613,19apr05,bl * Removed warnings after 64-Bit port of driver       */
/* 2612,14apr05,mf * Removed timestamp union - use unsigned long long   */
/* 2611,03may04,bl * Limiting count to size of rxmsg or txmsg           */
/* 2610,15mar04,fj * Added NTCAN_SOCK_xxx return-codes                  */
/* 269,07jan04,mf  * changed timestamping ioctl from period to freq     */
/* 268,05jan04,stm * Fixed compiler warnings.                           */
/* 267,07oct03,mf  * added timestamping test (23)                       */
/*                 *  (timestamping code must be enabled by defining    */
/*                 *  NTCAN_HAVE_TIMESTAMPS                             */
/*                 * removed C++ style comments                         */
/*                 * added test 51 (written by mt)                      */
/*                 * merged with new esdcan tree's cantest.c            */
/*                 * minor changes in RTAI code                         */
/* 265,23apr03,ot  * Error message if opening handle failed with other  */
/*                 *  reason than 'no device' if called w/o parameter.  */
/*                 * Error codes listed in err2str[] now depend on      */
/*                 *  NTCAN library and not on operating system.        */
/* 264,22oct02,stm * Fixed compiler warnings.                           */
/* 263b,10oct02    * See CVS log                                        */
/* 263a,09sep02    * See CVS log                                        */
/* 263,13aug02,stm * Use compiler defines __RTOSUH__ +  __RTOSUHPPC__   */
/* 262,08jul02,ot  * Added support for NET+OS (define NET_OS)           */
/* 261,16mar02,ot  * Added support for RTX (define UNDER_RTSS)          */
/*                 * Added support for time unit in duration output     */
/* 260,05Nov01,mt  * Added test 22 Object mode                          */
/* 257,10may01,ot  * If possible use performance counter for Win32      */
/*                 *  instead of GetTickCount() to have a more accurate */
/*                 *  timing                                            */
/* 256,27feb01,fj  * Added start_rt_timer() and stop_rt_timer() for     */
/*                   rtai-version                                       */
/* 255,05feb01,fj  * Added support for rtlinux-3.0                      */
/* 254,06jul00,ot  * Display feature flags of each interface            */
/*                 * Added new error codes from NTCAN for Win32         */
/* 253,06jul00,ot  * Caller priority check for Vxworks (not used yet)   */
/* 252,05jul00,fj  * rtai: added kernel module parameter infifo and     */
/*                 *       outfifo. printf() now via /dev/rtfx, with    */
/*                 *       x=outfifo (printf() no longer via printk()   */
/*                 *       to syslog). If not specified, infifo defaults*/
/*                 *       to 0 and outfifo defaults to 1.              */
/* 251,27jun00,sr  * some // comments removed for vxworks               */
/* 250,09jun00,fj  * support for linux-rtai                             */
/* 249,17apr00,sr  * rtos-uh-support included                           */
/* 248,13jan00,mt  * rmos-support included                              */
/* 247,05jan00,ot  * Fixed bug in previous version that idend is set to */
/*                 *  idstart                                           */
/* 246,16dec99,ot  * Using strtol() instead of atoi() to parse baudrate */
/*                 *  and CAN-IDs to support hex and oct values         */
/*                 * Changes in value display for baudrate and idf      */
/* 245,01dec99,mt  * Test 19                                            */
/* 244,09nov99,mt  * canGetBaudrate                                     */
/* 243,20may99,ot  * Removed test 13 from D3X version                   */
/*                 * Return immediately if test is undefined            */
/*                 * Special test 6 for async I/O using signal handler  */
/*                 *  and poll for Solaris (D3X) version                */
/* 242,17may99,mt  * decimal net numbers in help()                      */
/* 241,03may99,ot  * Windows CE support included (no test 6 and 7)      */
/*                 * Changed GET_CURRENT_TIME macro from WIN32 API call */
/*                 *  GetCurrentTime() to GetTickCount() as only the    */
/*                 *  latter is present in Windows CE                   */
/* 240,19mar99,mt  * Test 7 added                                       */
/* 230,19mar99,ot  * Support for VxWorks included                       */
/*                 * Minor changes to prevent GCC compiler warnings     */
/*                 * Replaced GetCurrentTime() call in Rev 2.2.1 with   */
/*                 *  macro                                             */
/*                 * Display code revision in help()                    */
/* 221,09mar99,mt  * NO_ASCII_OUTPUT => canRead/Take measures           */
/*                 * Frames per second                                  */
/* 220,01mar99,ot  * New macros GET_CURRENT_TIME and SLEEP for multi    */
/*                 *  OS support                                        */
/*                 * Replace DWORD with long                            */
/*                 * Replace ERROR_SUCCESS with NTCAN_SUCCESS           */
/*                 * Fixed bug with non initialized board status        */
/*                 * Minor changes to remove some BC and GCC compiler   */
/*                 *  warnings                                          */
/*                 * Support for UNIX (Solaris) based on macro unix     */
/*                 * Replaced macro SIEMENS with D3X                    */
/*                 * Description of test 9 in help()                    */
/*                 * Included ASCII output support for test 3 and 9     */
/*                 * Removed some junk code                             */
/*                 * Added RCS/CVS id                                   */
/*                 * Made help() and get_error_str() static             */
/*                 * Check for new NTCAN_INSUFICIENT_RESOURCES after    */
/*                 *  canIdAdd() and solve situation with sleep         */
/*                 * Rename module from nttest.c to cantest.c           */
/* 210,09dec98,mt  * New Parameter for nttest                           */
/* 202,18dec97,mt  * New Error-Code "NTCAN_INVALID_HARDWARE"            */
/* 201,06nov97,mt  * Bug-Fixes & different Default-values for testcount */
/* 200,28oct97,mt  * D3x-Version                                        */
/* 010,19jun97,mt  * Birth of module                                    */
/*----------------------------------------------------------------------*/
#define LEVEL    2
#define REVISION 12
#define CHANGE   6

/************************************************************************/
/************************************************************************/
/* Special includes, defines and functions for all Windows-platforms    */
/*                    RTX and OnTime RTOS32                             */
/************************************************************************/
/************************************************************************/
#ifdef _WIN32

  /* Prevent warnings about deprecated ANSI C library functions in VC8 :-( */
#if defined (_MSC_VER) && (_MSC_VER >= 1400)
# ifndef _CRT_SECURE_NO_DEPRECATE
#  define _CRT_SECURE_NO_DEPRECATE
# endif
# ifndef _CRT_NONSTDC_NO_DEPRECATE
#  define _CRT_NONSTDC_NO_DEPRECATE
# endif
#endif

#if defined(__MINGW32__) || defined (__MINGW64__)
# if (__GNUC__ == 4 && 3 <= __GNUC_MINOR__) || 4 < __GNUC__
#  pragma GCC diagnostic ignored "-Wstrict-aliasing"
# endif
# include <stdint.h>      /* Include stdint.h if available */
#endif

# include "windows.h"
#define MAIN_CALLTYPE __cdecl
# ifndef INT64_C
#  define INT64_C(c)  (c ## I64) /* INT64 constant according to ISO C99 */
#  define UINT64_C(c)  (c ## UI64) /* UINT64 constant according to ISO C99 */
# endif
  /*
   * Macros for printing 64 bit format specifiers according to ISO C99
   * missing in Windows
   */
#ifndef __PRI64_PREFIX
# define __PRI64_PREFIX  "I64"
# define PRId64           __PRI64_PREFIX "d"
# define PRIi64           __PRI64_PREFIX "i"
# define PRIu64           __PRI64_PREFIX "u"
# define PRIx64           __PRI64_PREFIX "x"
# define PRIX64           __PRI64_PREFIX "X"
#endif

#ifndef RTOS32
# define CANTEST_MULTIPROCESSOR
# define GET_CURRENT_TIME mtime()
# define TIME_UNITS   "us"
static unsigned long mtime(void);
static int ForceThreadAffinity(void);
# define SLEEP(arg)  Sleep(arg)
#else
# include <Rttarget.h>
# include <Rtk32.h>
# include <Finetime.h>
# include <Clock.h>
# define GET_CURRENT_TIME (CLKTicksToMilliSecs(RTKGetTime()))
# define SLEEP(arg) { register RTKDuration ticks = CLKMilliSecsToTicks(arg); \
                               (ticks == 0 ? RTKDelay(1) : RTKDelay(ticks)); }
# define main _canTest   /* Redefine main for to be called from helper */
#endif /* !RTOS32 */


/* BL: not quite sure, if 400 is the version this behaviour was changed.
       Might have been earlier.*/
# ifdef _WIN32_WCE
#  if _WIN32_WCE < 400
#  define atoi _wtoi   /* As we work with unicode redefine this */
int CreateArgvArgc(TCHAR *pProgName, TCHAR *argv[30], TCHAR *pCmdLine);
#  endif /* of _WIN32_WCE < 400 */
#  undef CANTEST_MULTIPROCESSOR
# endif
# ifdef UNDER_RTSS
#  include "rtapi.h"
/* #  define TIME_UNITS   "us" */
#  undef CANTEST_MULTIPROCESSOR
# endif /* of UNDER_RTSS */

# if defined (_MSC_VER) && defined(D3X)
__pragma(warning(disable:4996)) /* disable deprecated warning for canSendEvent and canReadEvent */
# endif

#endif /* _WIN32 */


/************************************************************************/
/************************************************************************/
/* Special includes, defines and functions for all unix platforms       */
/* except RTAI- and RT-Linux                                            */
/************************************************************************/
/************************************************************************/
#if defined(unix) && !defined(RTAI) && !defined(RTLINUX)
# include <errno.h>
# include <sys/time.h>
# include <unistd.h>
# include <inttypes.h>  /* Needed since 64-Bit port of driver! */
# include <string.h>
# include <signal.h>
# ifdef D3X   /* Solaris driver only */
  void sigio_handler(int arg);
# endif /* of D3X */
  static unsigned long mtime(void);
  void sigusr1_handler(int signum);
# define CANTEST_USE_SIGUSR1_HANDLER
# define SLEEP(arg)                     \
  do {                                  \
    if ( (arg) >= 1000 ) {              \
      sleep((arg)/1000);                \
      usleep(((arg)%1000) * 1000);      \
    } else {                            \
      usleep((arg)*1000);               \
    }                                   \
  } while(0)
# define GET_CURRENT_TIME mtime()
#endif /* unix */


/************************************************************************/
/************************************************************************/
/* Special includes, defines and functions for QNX                      */
/************************************************************************/
/************************************************************************/
#ifdef qnx
# include <sys/time.h>
# include <unistd.h>
  static unsigned long mtime(void);
# define GET_CURRENT_TIME mtime()
# define SLEEP(arg) delay(arg)
#endif /* qnx */


/************************************************************************/
/************************************************************************/
/* Special includes, defines and functions for VxWorks                  */
/************************************************************************/
/************************************************************************/
#ifdef VXWORKS
# include "vxWorks.h"
# include "sysLib.h"
# include "taskLib.h"
# include "semLib.h"
# include "tickLib.h"
# include "string.h"
# include "timers.h"
# define GET_CURRENT_TIME ((tickGet() & 0x003fffff) * 1000 / sysClkRateGet())
# define SLEEP(arg) { register long ticks = arg * sysClkRateGet() / 1000; \
                      (ticks == 0 ? taskDelay(1) : taskDelay(ticks)); }
# ifndef INT64_C
#  define INT64_C(c)  (c ## LL)   /* INT64 constant according to ISO C99 */
#  define UINT64_C(c)  (c ## ULL) /* UINT64 constant according to ISO C99 */
# endif

  /*
   * Macros for printing 64 bit format specifiers according to ISO C99
   * missing in Windows
   */
#ifndef __PRI64_PREFIX
# define __PRI64_PREFIX  "ll"
# define PRId64           __PRI64_PREFIX "d"
# define PRIi64           __PRI64_PREFIX "i"
# define PRIu64           __PRI64_PREFIX "u"
# define PRIx64           __PRI64_PREFIX "x"
# define PRIX64           __PRI64_PREFIX "X"
#endif

# define main _canTest   /* Redefine main for to be called from helper */
# define WANT_PRIO_CHECK    0      /* 1 to enable priority check */
# if WANT_PRIO_CHECK == 1
  static STATUS _checkPriority(int net);
# endif
#endif /* VXWORKS */


/************************************************************************/
/************************************************************************/
/* Special includes, defines and functions for RMOS                     */
/************************************************************************/
/************************************************************************/
#ifdef RMOS
# include <errno.h>
# include <rmapi.h>        /* RMOS3-Include                                  */
  static unsigned long mtime(void);
# define GET_CURRENT_TIME mtime()
# define SLEEP(arg) RmPauseTask( arg )

# define __PRI64_PREFIX  "ll"
# define PRId64           __PRI64_PREFIX "d"
# define PRIi64           __PRI64_PREFIX "i"
# define PRIu64           __PRI64_PREFIX "u"
# define PRIx64           __PRI64_PREFIX "x"
# define PRIX64           __PRI64_PREFIX "X"

# ifndef INT64_C
#  define INT64_C(c)  (c ## LL)
#  define UINT64_C(c)  (c ## ULL)
# endif

#endif /* RMOS */

/************************************************************************/
/************************************************************************/
/* Special includes, defines and functions for LynxOS                   */
/************************************************************************/
/************************************************************************/
#ifdef __Lynx__
# define __PRI64_PREFIX  "ll"
# define PRId64           __PRI64_PREFIX "d"
# define PRIi64           __PRI64_PREFIX "i"
# define PRIu64           __PRI64_PREFIX "u"
# define PRIx64           __PRI64_PREFIX "x"
# define PRIX64           __PRI64_PREFIX "X"
# ifndef INT64_C
#  define INT64_C(c)  (c ## LL)
#  define UINT64_C(c)  (c ## ULL)
# endif

#endif /* __Lynx__ */
/************************************************************************/
/************************************************************************/
/* Special includes, defines and functions for RTOS-UH                  */
/************************************************************************/
/************************************************************************/
#if defined(__RTOSUH__) || defined(__RTOSUHPPC__)

typedef unsigned long long uint64_t;
typedef          long long  int64_t;

typedef unsigned long   uint32_t;
typedef          long   int32_t;
typedef unsigned short  uint16_t;
typedef unsigned char   uint8_t;
typedef          char   int8_t;

# include <sys/types.h>
# include <fcntl.h>
# include <unistd.h>
# include <ctype.h>
# include <errno.h>
# define GET_CURRENT_TIME clock_()
/*# define SLEEP(arg)       ttire(0x80000000 | arg) */
PROGRAM_TYPE(PT_SHM);
SHELL_MODULE_SPEC(CANTEST, CANTEST, 30);
#endif /* RTOSUH */


/************************************************************************/
/************************************************************************/
/* Special includes, defines and functions for NET-OS                   */
/************************************************************************/
/************************************************************************/
#ifdef NET_OS
# if NET_OS < 6
#  include <bspconf.h>
#else
#  include <bsp.h>
# endif
# include <narmapi.h>
# include <tx_api.h>
  static unsigned long mtime(void);
# define GET_CURRENT_TIME mtime()
# define SLEEP(msec) tx_thread_sleep((msec * BSP_TICKS_PER_SECOND) / 1000)
# define main canTest   /* Redefine main for to be called from helper */
#endif /* NET_OS */


/************************************************************************/
/************************************************************************/
/* Special includes, defines and functions for RT-Linux and RTAI-Linux  */
/************************************************************************/
/************************************************************************/
#if defined(RTAI) || defined(RTLINUX)
# include <linux/module.h>

# ifdef RTAI
#  include <rtai.h>
#  include <rtai_fifos.h>
#  include <rtai_sched.h>
RT_TASK thread;
#  define RT_TIMER_TICK_1MS 1000000 /* ns (!!!!! CAREFULL NEVER GREATER THAN 1E7 !!!!!) */
# else
#  include <rtl.h>
#  include <rtl_fifo.h>
#  include <rtl_sched.h>
#  include <rtl_time.h>
#  include <rtl_sched.h>
#  include <pthread.h>
#  define WPX(x) printk("### %s:%d ###\n", x,  __LINE__);
pthread_attr_t     attr;
pthread_t          thread;
struct sched_param sched_param;
# endif /* RTAI */

# define STACK_SIZE        0x2000
# define CANTEST_PRIO      RT_HIGHEST_PRIORITY + 100
# define DEFAULT_IN_FIFO   1
# define DEFAULT_OUT_FIFO  2
# define FIFO_BUF_SIZE     256
# define MAX_ARGC          32
static int infifo = DEFAULT_IN_FIFO;
static int outfifo = DEFAULT_OUT_FIFO;
int printf(const char *fmt, ...)
{
  char sbuf[FIFO_BUF_SIZE];

  va_list args;
  va_start(args, fmt);
  vsprintf(sbuf, fmt, args);
  va_end(args);
  rtf_put(outfifo, sbuf, strlen(sbuf));

  return 0;
}
long strtol(const char *cp,char **endp,unsigned int base)
{
  if (*cp == '-') {
    return -simple_strtoul(cp+1,endp,base);
  }
  return simple_strtoul(cp,endp,base);
}
# ifdef RTAI
#  define TIME_TYPE RTIME
#  define GET_CURRENT_TIME rt_get_time_ns()
#  define SLEEP(x) rt_sleep(nano2count(((unsigned long long)(x) * 1000LL * 1000LL)))
# else
#  define TIME_TYPE hrtime_t
#  define GET_CURRENT_TIME gethrtime()
#  if 0
#   define SLEEP(x) do { pthread_make_periodic_np(pthread_self(), gethrtime() + (x) * 1000 * 1000,  0); pthread_wait_np(); } while(0)
#  else
#   define SLEEP(x) usleep((x) > 1000 ? 999999 : (x) * 1000)
#  endif /* if 0 */
# endif /* RTAI */
# define TIME_UNITS "ns"
MODULE_PARM(infifo,  "1i");
MODULE_PARM(outfifo, "1i");
int atoi(char *str)
{
  char *endp;
  int   ul;

  ul = strtoul(str, &endp, 10);
  return ul;
}

/*
** Build argc and argv[] and call test program
*/
void fifoHandler(int fifo)
{
  int   status, i, argc = 0;
  char  buf[FIFO_BUF_SIZE], *cp;
  char *argv[MAX_ARGC];
  char *argv0 = "cantest";
  int   main(int argc, char *argv[]);

  while (1) {
    buf[0] = '\0';
    buf[FIFO_BUF_SIZE - 1] = 0;
    i = 0;
    while (!strchr(buf,'\n')) {
      status = rtf_get(fifo, &buf[i] , FIFO_BUF_SIZE-i);
      if (status > 0) {
        i += status;
        buf[i] = '\0';
      } else {
        SLEEP(500);
      }
    }
    i = 1;
    argv[0] = argv0;
    argc = 1;
    cp = strchr(buf, '\n');
    if (cp && (cp != buf)) {
      *cp = '\0';
      cp = strtok(buf, " ");
      while (cp && i < (MAX_ARGC - 2)) {
        argv[i] = rt_malloc(strlen(cp) + 1);
        strcpy(argv[i], cp);
        i++;
        cp = strtok(0, " ");
      }
    }
    argc = i;
    argv[MAX_ARGC - 1] = 0;

    /* call test program */
    i = main(argc, argv);

    for (i = 1; i < argc; i++) {
      rt_free(argv[i]);
    }
  } /* while(1) */
}

int init_module(void)
{
  int status;

  rtf_destroy(infifo);
  if ((status = rtf_create(infifo, 256)) < 0) {
    printk("CANTEST: Can't create infifo [%d=%#x]\n", status, status);
    return status;
  }
  rtf_destroy(outfifo);
  if ((status = rtf_create(outfifo, 4096)) < 0) {
    printk("CANTEST: Can't create outfifo [%d=%#x]\n", status, status);
    return status;
  }
# ifdef RTAI
  {
    RTIME __attribute__ ((unused)) ticks;
#  ifdef OSIF_ARCH_PPC
    /*#warning "USING ONESHOT MODE for ARCH PPC"*/
    rt_set_oneshot_mode();
#  endif
    ticks = start_rt_timer((int)nano2count(RT_TIMER_TICK_1MS));
    printk("CANTEST: rtai-timer started with tick time 1ms [%ld]\n", (long)ticks);
  }
  /*
   * rtai offers fifo_handlers, but semaphores doesn't work with fifo-handlers in rtai-1.2 !?
   * So we spawn a task and poll for fifo-data
   */
  if ((status = rt_task_init(&thread, fifoHandler, infifo, STACK_SIZE, CANTEST_PRIO, 0, 0)) < 0) {
    printk("CANTEST: rt_task_init() failed [%d=%#x]\n", status, status);
    return status;
  }
  rt_task_resume(&thread);
# else
  pthread_attr_init(&attr);
  sched_param.sched_priority = 1;
  pthread_attr_setschedparam(&attr, &sched_param);
  if ((status = pthread_create(&thread, &attr,  (void* (*)(void*))fifoHandler, (void*)infifo))) {
    printk("CANTEST: pthread_create() failed [%d=%#x]\n", status, status);
    return status;
  }
# endif /* RTAI */
  printk("CANTEST: Initialized using /dev/rtf%d as input and /dev/rtf%d as output-device\n",
         infifo, outfifo);
  return 0;
}

void cleanup_module(void)
{
  rtf_destroy(infifo);
  rtf_destroy(outfifo);
# ifdef RTAI
  rt_task_delete(&thread);
# else
  pthread_delete_np(thread);
# endif /* RTAI */
  stop_rt_timer();
  printk("CANTEST: cleanup ...\n");
  return;
}
#else
# include <stdio.h>
# include <stdlib.h>
# ifndef TIME_TYPE
#  define TIME_TYPE unsigned long
# endif
# ifndef TIME_UNITS
#  define TIME_UNITS "msec"
# endif
#endif /* RTAI || RTLINUX */


#include "ntcan.h"

/*
 * RCS/CVS id with support to prevent compiler warnings about unused vars
 */
#if ((__GNUC__ > 2) || (__GNUC__ == 2 && __GNUC_MINOR__ >=7))
static char* rcsid __attribute__((unused)) = "$Id: cantest.c 15230 2016-02-16 08:58:34Z oliver $";
#else  /* No or old GNU compiler */
# define USE(var) static void use_##var(void *x) {if(x) use_##var((void *)var);}
static char* rcsid = "$Id: cantest.c 15230 2016-02-16 08:58:34Z oliver $";
USE(rcsid);
#endif /* of GNU compiler */

/*
 * Defines
 */
#define NO_ASCII_OUTPUT         0x00000001

/*
 * Macros
 */

    /* Check NTCAN-API used by test number */
#define IS_SEND_TEST(test) (0 == ((test) % 10))
#define IS_WRITE_TEST(test) (1 == ((test) % 10))
#define IS_TAKE_TEST(test) (2 == ((test) % 10))
#define IS_READ_TEST(test) (3 == ((test) % 10))

    /* Use C library memory allocator if not defined OS specific */
#ifndef CANTEST_ALLOC
# define CANTEST_ALLOC(s)    malloc(s)
#endif

#ifndef CANTEST_FREE
# define CANTEST_FREE(s)     free(s)
#endif

/*
 * Typedefs
 */
#define MAX_RX_MSG_CNT 8192
#define MAX_TX_MSG_CNT 2048

typedef struct {
    CMSG     rxmsg[MAX_RX_MSG_CNT];
    CMSG     txmsg[MAX_TX_MSG_CNT];
#ifdef NTCAN_IOCTL_GET_TIMESTAMP
    CMSG_T   rxmsg_t[MAX_RX_MSG_CNT];  /* needs to have same number of messages as rxmsg */
    CMSG_T   txmsg_t[MAX_TX_MSG_CNT];  /* needs to have same number of messages as txmsg */
    uint64_t udtTimestampFreq;
    uint64_t ullLastTime;
#endif
#ifdef NTCAN_FD
    CMSG_X   rxmsg_x[MAX_RX_MSG_CNT];  /* needs to have same number of messages as rxmsg */
    CMSG_X   txmsg_x[MAX_TX_MSG_CNT];  /* needs to have same number of messages as txmsg */
#endif
    uint32_t mode;
    uint8_t  ctrl_type;
#if defined(unix) && defined(D3X)
    NTCAN_HANDLE h1;            /* Handle for async I/O test */
#endif /* of unix && D3X */
} CANTEST_CTX;

/*
 * Forward declarations
 */
static void help(int testnr);
static int8_t *get_error_str(int8_t *str_buf, NTCAN_RESULT ntstatus);
static void print_cmsg(CMSG *pCmsg, CANTEST_CTX *pCtx);
static NTCAN_RESULT set_can_id(NTCAN_HANDLE handle, int32_t id);
#ifdef NTCAN_IOCTL_GET_TIMESTAMP
static int get_timestamp_freq(NTCAN_HANDLE handle, uint64_t *pFreq, int verbose);
static void print_cmsg_t(CMSG_T *pCmsgT, CANTEST_CTX *pCtx);
#endif
#ifdef NTCAN_FD
static void print_cmsg_x(CMSG_X *pCmsgX, CANTEST_CTX *pCtx);
#endif
static void print_event(EVMSG *e, uint64_t ts_diff, CANTEST_CTX *pCtx);

/*
 * Dynamically loaded functions, if supported by OS
 */
#if defined(_WIN32) && defined(FUNCPTR_CAN_READ_T)
# define CANTEST_DYNLOAD
static int DynLoad(void);
PFN_CAN_IOCTL                   pfnIoctl                = NULL;
PFN_CAN_TAKE_T                  pfnTakeT                = NULL;
PFN_CAN_READ_T                  pfnReadT                = NULL;
PFN_CAN_SEND_T                  pfnSendT                = NULL;
PFN_CAN_WRITE_T                 pfnWriteT               = NULL;
PFN_CAN_GET_OVERLAPPED_RESULT_T pfnGetOverlappedResultT = NULL;
PFN_CAN_FORMAT_ERROR            pfnFormatError          = NULL;
PFN_CAN_FORMAT_EVENT            pfnFormatEvent          = NULL;
# if defined (NTCAN_FD)
PFN_CAN_TAKE_X                  pfnTakeX                = NULL;
PFN_CAN_READ_X                  pfnReadX                = NULL;
PFN_CAN_SEND_X                  pfnSendX                = NULL;
PFN_CAN_WRITE_X                 pfnWriteX               = NULL;
PFN_CAN_GET_OVERLAPPED_RESULT_X pfnGetOverlappedResultX = NULL;
# endif /* of NTCAN_FD */
# define canIoctl(hnd, cmd, pArg) \
    (NULL == pfnIoctl) ? NTCAN_NOT_IMPLEMENTED : \
    pfnIoctl((hnd), (cmd), (pArg))
# define canTakeT(hnd, pCmsg, pLen) \
    (NULL == pfnTakeT) ? NTCAN_NOT_IMPLEMENTED : \
    pfnTakeT((hnd), (pCmsg), (pLen))
# define canReadT(hnd, pCmsg, pLen, pOvr) \
    (NULL == pfnReadT) ? NTCAN_NOT_IMPLEMENTED : \
    pfnReadT((hnd), (pCmsg), (pLen), (pOvr))
# define canSendT(hnd, pCmsg, pLen) \
    (NULL == pfnSendT) ? NTCAN_NOT_IMPLEMENTED : \
    pfnSendT((hnd), (pCmsg), (pLen))
# define canWriteT(hnd, pCmsg, pLen, pOvr) \
    (NULL == pfnWriteT) ? NTCAN_NOT_IMPLEMENTED : \
    pfnWriteT((hnd), (pCmsg), (pLen), (pOvr))
# define canGetOverlappedResultT(hnd, pOvr, pLen, bWait) \
    (NULL == pfnGetOverlappedResultT) ? NTCAN_NOT_IMPLEMENTED : \
    pfnGetOverlappedResultT((hnd), (pOvr), (pLen), (bWait))
# define canFormatEvent(ev,pParams,pBuf,bufsize) \
    (NULL == pfnFormatEvent) ? NTCAN_NOT_IMPLEMENTED : \
    pfnFormatEvent((ev), (pParams), (pBuf), (bufsize))
# define canFormatError(err, type, pBuf, bufsize) \
    (NULL == pfnFormatError) ? NTCAN_NOT_IMPLEMENTED : \
    pfnFormatError((err), (type), (pBuf), (bufsize))
# if defined (NTCAN_FD)
#  define canTakeX(hnd, pCmsg, pLen) \
     (NULL == pfnTakeX) ? NTCAN_NOT_IMPLEMENTED : \
     pfnTakeX((hnd), (pCmsg), (pLen))
#  define canReadX(hnd, pCmsg, pLen, pOvr) \
     (NULL == pfnReadX) ? NTCAN_NOT_IMPLEMENTED : \
     pfnReadX((hnd), (pCmsg), (pLen), (pOvr))
#  define canSendX(hnd, pCmsg, pLen) \
     (NULL == pfnSendX) ? NTCAN_NOT_IMPLEMENTED : \
     pfnSendX((hnd), (pCmsg), (pLen))
#  define canWriteX(hnd, pCmsg, pLen, pOvr) \
     (NULL == pfnWriteX) ? NTCAN_NOT_IMPLEMENTED : \
     pfnWriteX((hnd), (pCmsg), (pLen), (pOvr))
#  define canGetOverlappedResultX(hnd, pOvr, pLen, bWait) \
     (NULL == pfnGetOverlappedResultX) ? NTCAN_NOT_IMPLEMENTED : \
     pfnGetOverlappedResultX((hnd), (pOvr), (pLen), (bWait))
# endif /* NTCAN_FD */
/* Disable warning about potentially uninitialized local variables */
# if defined (_MSC_VER)
__pragma(warning(disable:4701))
# endif
#endif

static uint32_t num_baudrate;
static void set_num_baudrate(uint32_t baud) {
    switch (baud) {
        case NTCAN_BAUD_1000:   num_baudrate=1000000;break;
        case NTCAN_BAUD_800:    num_baudrate= 800000;break;
        case NTCAN_BAUD_500:    num_baudrate= 500000;break;
        case NTCAN_BAUD_250:    num_baudrate= 250000;break;
        case NTCAN_BAUD_125:    num_baudrate= 125000;break;
        case NTCAN_BAUD_100:    num_baudrate= 100000;break;
        case NTCAN_BAUD_50:     num_baudrate=  50000;break;
        case NTCAN_BAUD_20:     num_baudrate=  20000;break;
        case NTCAN_BAUD_10:     num_baudrate=  10000;break;
        default: num_baudrate=0;break;
    }
}

#ifndef MAIN_CALLTYPE
#define MAIN_CALLTYPE
#endif

/************************************************************************/
/************************************************************************/
/* Function: main()                                                     */
/* The "#ifdef-massacre" simply adapts the prototype of main() to       */
/* peculiarities of certain systems.                                    */
/************************************************************************/
/************************************************************************/
/* AB: not quite sure, if 400 is the version this behaviour was changed.
       Might have been earlier.*/
#if defined(_WIN32_WCE) && (_WIN32_WCE < 400)
int main(DWORD  hInstance,      /* Handle to current instance of application */
         DWORD  hPrevInstance,  /* Handle to prev. instance of application */
         TCHAR *pCmdLine,       /* Command line for application w/o appname */
         int    nShowCmd)       /* Windows show mode */
#else /* of _WIN32_WCE < 400 */
# ifdef RMOS
int _FAR main(int argc, char *argv[])
# else /* of RMOS */
int MAIN_CALLTYPE main(int argc, char *argv[])
# endif /* of RMOS */
#endif /* of _WIN32_WCE < 400 */
{
  CANTEST_CTX   *pCtx;
  NTCAN_HANDLE  h0;
  CMSG          cmdmsg;             /* can-data from the command-line */
  CMSG_T        cmdmsg_t;           /* can-data from the command-line */
#if defined(NTCAN_FD)
  CMSG_X        cmdmsg_x;           /* can-data from the command-line */
#endif
  EVMSG         evmsg;
  int32_t       test;
  int32_t       net           = 0;
  int32_t       idstart       = 0;
  int32_t       idend         = 0;
  int32_t       maxcount      = 1;
  int32_t       txbufsize     = 10;
  int32_t       rxbufsize     = 100;
  int32_t       txtout        = 1000;
  int32_t       rxtout        = 5000;
  int32_t       idnow         = 0;
#ifdef D3X
  uint32_t      baudrate      = 7;
#else
  uint32_t      baudrate      = 2;
  uint32_t      time_1;
#endif /* DX3 */
  int32_t       testcount     = 10;
  int32_t       ever          = 0;
  int8_t        str_buf[100];
  int32_t       data_from_cmd = 0;
  TIME_TYPE     start_time, stop_time;
  TIME_TYPE     start_test_time, stop_test_time;
  NTCAN_RESULT  ret;
  int32_t       h, i, j, k;
  int32_t       frames = -1;
  CMSG         *cm;
#if defined(NTCAN_IOCTL_GET_TX_TS_WIN) || defined(NTCAN_IOCTL_GET_TIMESTAMP)
    CMSG_T     *cmt;
#endif
#if defined(NTCAN_FD)
CMSG_X         *cmx;
#endif
  int32_t       len;
  char         *pend;
#ifdef NTCAN_IOCTL_GET_TIMESTAMP
  uint64_t     lastStamp = 0;
#endif
#ifdef NTCAN_IOCTL_SET_BUSLOAD_INTERVAL
  uint32_t     bl_event_period = 0;
#endif
#ifdef _WIN32
  /* BL: not quite sure, if 400 is the version this behaviour was changed.
     Might have been earlier.*/
# if defined(_WIN32_WCE) && (_WIN32_WCE < 400)
  TCHAR      *argv[30];
  int         argc;

  argc = CreateArgvArgc(TEXT("cantest.exe"), argv, pCmdLine);
# elif UNDER_RTSS
# else
  HANDLE        hEvent = NULL;      /* Event for overlapped operation */
  OVERLAPPED    overlapped;         /* overlapped-structure           */

# endif /* of _WIN32_WCE */
#endif /* of _WIN32 */
  uint8_t       rtr = 0;

  /*
   * Initialize function pointer if parts of the API are dynamically loaded
   */
#ifdef CANTEST_DYNLOAD
  DynLoad();
#endif /* of CANTEST_DYNLOAD */

  /*
   * Parse command line parameters and assign test specific default values
   */
  if (argc > 1) {       /* Parameter 'test' */
    test = atoi(argv[1]);
    if (test<0) {
        help(test);
        return(0);
    }
  } else {
    help(-1);
    return(-1);
  }

  /* Adapt some default values according to test type */
  if(64 == test || 74 == test || 84 == test) {
    testcount = 1;
    baudrate  = 0xFFFFFFFFU;
  }

  if (argc > 2) {       /* Parameter 'net' */
    net = atoi(argv[2]);
    if((net < 0) || (net > NTCAN_MAX_NETS)) {
      printf("!! Parameter 'net' invalid !!\n");
      return(-1);
    }
  }

  if (argc > 3) {       /* Parameter 'id-1st' */
    idstart = strtoul(argv[3], &pend, 0);
    if (*pend != '\0') {
      printf("!! Parameter 'id-1st' invalid !!\n");
      return(-1);
    }
  }

  if (argc > 4) {       /* Parameter 'id-last' */
    idend = strtoul(argv[4], &pend, 0);
    if (*pend != '\0') {
      printf("!! Parameter 'id-last' invalid !!\n");
      return(-1);
    }
  }

  /* Allocate memory of context */
  pCtx = CANTEST_ALLOC(sizeof(CANTEST_CTX));
  if(NULL == pCtx) {
      printf("!! Error allocating context !!\n");
      return(-1);
  } else {
      memset(pCtx, 0, sizeof(CANTEST_CTX));
  }

  if (argc > 5) {       /* Parameter 'count' */
    maxcount = atoi(argv[5]);
    /*
     * Negative value of maxcount for receive operations means opening the
     * handle with NTCAN_MODE_MARK_INTERACTION, for transmit operations RTR
     * frames are send instead of data frames.
     */
    if(maxcount < 0) {
        if(IS_READ_TEST(test)) {
            pCtx->mode |= NTCAN_MODE_MARK_INTERACTION;
            maxcount *= -1;
        } else if(IS_WRITE_TEST(test) || IS_SEND_TEST(test)) {
            rtr = NTCAN_RTR;
            maxcount *= -1;
        } else {
            printf("Parameter 'count' invalid !!\n");
            CANTEST_FREE(pCtx);
            return(-1);
        }
    } else if(0 == maxcount) {
        if(IS_READ_TEST(test)) {
            pCtx->mode |= NTCAN_MODE_NO_INTERACTION;
            maxcount = 1;
        }
    }
    if (IS_SEND_TEST(test) || IS_WRITE_TEST(test) || (5 == test)) {
      if (maxcount > MAX_TX_MSG_CNT) {
        maxcount = MAX_TX_MSG_CNT;
        printf("Limited count to %ld!\n", (long)MAX_TX_MSG_CNT);
      }
    } else {
      if (maxcount > MAX_RX_MSG_CNT) {
        maxcount = MAX_RX_MSG_CNT;
        printf("Limited count to %ld!\n", (long)MAX_RX_MSG_CNT);
      }
    }
  }

  if (argc > 6) {           /* Parameter 'txbufsize' */
    txbufsize = atoi(argv[6]);
  }

  if (argc > 7) {           /* Parameter 'rxbufsize' */
    rxbufsize = atoi(argv[7]);
  }

  if (argc > 8) {           /* Parameter 'txtout' */
    txtout = atoi(argv[8]);
  }

  if (argc > 9) {           /* Parameter 'rxtout' */
    rxtout = atoi(argv[9]);
  }

  if (argc > 10) {           /* Parameter 'baudrate' */
      if(!strcmp(argv[10], "auto")) {
          baudrate = NTCAN_AUTOBAUD;
      } else if(!strcmp(argv[10], "disable")) {
          baudrate = NTCAN_NO_BAUDRATE;
      } else if(!strcmp(argv[10], "no")) {
          baudrate = 0xFFFFFFFFU;
      } else {
          baudrate = strtoul(argv[10], &pend, 0);
          if (*pend != '\0') {
              printf("!! Parameter 'baudrate' invalid !!\n");
              CANTEST_FREE(pCtx);
              return(-1);
          }
      }
  }

  if (argc > 11) {           /* Parameter 'testcount' */
    testcount = atoi(argv[11]);
  } else {
      /* Default testcount to 'endless' for receive operations */
      if (IS_TAKE_TEST(test) || IS_READ_TEST(test) || 4 == test) {
          testcount = -1;
      }
  }

  /*
   * Prepare default-data
   */
  k = 1;
  for (i = 0; i < MAX_TX_MSG_CNT; i++) {
    cm = &pCtx->txmsg[i];
    cm->id = idstart;
    cm->len = 8 | rtr;
    *((uint32_t*)(&cm->data[0])) = 0;
    *((uint32_t*)(&cm->data[4])) = k;
#if defined(NTCAN_IOCTL_GET_TX_TS_WIN)
    cmt = &pCtx->txmsg_t[i];
    cmt->id = idstart;
    cmt->len = 8 | rtr;
    *((uint32_t*)(&cmt->data[0])) = 0;
    *((uint32_t*)(&cmt->data[4])) = k;
#endif
#if defined(NTCAN_FD)
    cmx = &pCtx->txmsg_x[i];
    cmx->id = idstart;
    cmx->len = (uint8_t)(NTCAN_FD | NTCAN_DATASIZE_TO_DLC(64));
    *((uint32_t*)(&cmx->data[0])) = 0;
    *((uint32_t*)(&cmx->data[4])) = k;
    *((uint32_t*)(&cmx->data[8])) = 2;
    *((uint32_t*)(&cmx->data[12])) = 3;
    *((uint32_t*)(&cmx->data[16])) = 4;
    *((uint32_t*)(&cmx->data[20])) = 5;
    *((uint32_t*)(&cmx->data[24])) = 6;
    *((uint32_t*)(&cmx->data[28])) = 7;
    *((uint32_t*)(&cmx->data[32])) = 8;
    *((uint32_t*)(&cmx->data[36])) = 9;
    *((uint32_t*)(&cmx->data[40])) = 10;
    *((uint32_t*)(&cmx->data[44])) = 11;
    *((uint32_t*)(&cmx->data[48])) = 12;
    *((uint32_t*)(&cmx->data[52])) = 13;
    *((uint32_t*)(&cmx->data[56])) = 14;
    *((uint32_t*)(&cmx->data[60])) = 15;
#endif
    k++;
  }

  for (i = 0; i <= (idend>2047?2047:idend)-idstart; i++) {
    cm = &pCtx->rxmsg[i];
    cm->id = idstart+i;
#ifdef NTCAN_IOCTL_GET_TIMESTAMP
    cmt = &pCtx->rxmsg_t[i];
    cmt->id = idstart+i;
#endif
#if defined(NTCAN_FD)
    cmx = &pCtx->rxmsg_x[i];
    cmx->id = idstart+i;
#endif
  }


  /*
   * Try to fetch data-bytes from command-line
   */
  evmsg.evid = (idstart & 0xff) + NTCAN_EV_BASE;
  cmdmsg.id = idstart;
  cmdmsg.len = 0 | rtr;
#if defined(NTCAN_IOCTL_GET_TIMESTAMP)
  cmdmsg_t.id = cmdmsg.id;
  cmdmsg_t.len = cmdmsg.len;
#endif
#if defined(NTCAN_FD)
  cmdmsg_x.id = cmdmsg.id;
  cmdmsg_x.len = cmdmsg.len | NTCAN_FD;
#endif
  evmsg.len = 0;
  for (i = 12; (i < argc) && (i < 12+64); i++) {
      uint32_t ulData = strtoul(argv[i], &pend, 0);

      if(*pend != '\0' || ulData > 255 )
      {
        if( ulData == (uint32_t)-1) {
          cmdmsg.len = 0 | rtr;
#if defined(NTCAN_IOCTL_GET_TIMESTAMP)
          cmdmsg_t.len = cmdmsg.len;
#endif
#if defined(NTCAN_FD)
          cmdmsg_x.len = cmdmsg.len | NTCAN_FD;
#endif
          i = 13;
          break;
        }

        printf("!! Parameter data%ld invalid !!\n", (long)(i - 12));
        CANTEST_FREE(pCtx);
        return(-1);
      }

      if(i < 20)
          cmdmsg.data[i-12] = (char)ulData;
      if(i < 27)
          cmdmsg.len ++;

#if defined(NTCAN_IOCTL_GET_TIMESTAMP)
      if(i < 20)
          cmdmsg_t.data[i-12] = (char)ulData;
      if(i < 27)
          cmdmsg_t.len ++;
#endif
#if defined(NTCAN_FD)
      if(i < 76)
          cmdmsg_x.data[i-12] = (char)ulData;

      if(i < 20) {
          cmdmsg_x.len ++;
      } else {
         switch(i) {
                  case 20:
                  case 24:
                  case 28:
                  case 32:
                  case 36:
                  case 44:
                  case 60:
                      cmdmsg_x.len ++;
                      break;
         }
      }
#endif
      if(i < 20)
          evmsg.evdata.c[i-12] = (char)ulData;
      if(i < 27)
          evmsg.len ++;
  }

  if (i > 12) {
    data_from_cmd = 1;
    for (i = 0; i < MAX_TX_MSG_CNT; i++) {
      pCtx->txmsg[i] = cmdmsg;
#if defined(NTCAN_IOCTL_GET_TX_TS_WIN)
      pCtx->txmsg_t[i] = cmdmsg_t;
#endif
#if defined(NTCAN_FD)
      pCtx->txmsg_x[i] = cmdmsg_x;
#endif
    }
  }

  if (testcount == -1) {
    ever = 1;
  }

  /*
   * Print test configuration
   */
  printf("test=%ld net=%ld ", (long)test, (long)net);
  if ((idstart > 0x7FF) || (idend > 0x7FF)) {
    printf("id-1st=0x%lx id-last=0x%lx ", (unsigned long)idstart, (unsigned long)idend);
  } else {
    printf("id-1st=%ld id-last=%ld ", (unsigned long)idstart, (unsigned long)idend);
  }
  printf("count=%ld\ntxbuf=%ld rxbuf=%ld txtout=%ld rxtout=%ld ",
         (unsigned long)maxcount, (unsigned long)txbufsize, (unsigned long)rxbufsize, (unsigned long)txtout, (unsigned long)rxtout);
  if (baudrate == 0xFFFFFFFFU) {
    printf("baudrate=(don't change) ");
  } else if (baudrate >= 15) {
    printf("baudrate=0x%lx ", (unsigned long)baudrate);
  } else {
    printf("baudrate=%ld ", (unsigned long)baudrate);
  }
  set_num_baudrate(baudrate);
  if (num_baudrate) {
        printf("(%u baud) ",(unsigned int)num_baudrate);
  }
  printf("\ntestcount=%ld ", (unsigned long)testcount);

  /*
   * Set object-mode, if needed
   */
  if ((32 == test) || (42 == test)|| (82 == test)) {
    pCtx->mode |= NTCAN_MODE_OBJECT;
  }

#if defined(NTCAN_FD)
  /* Set CAN-FD mode bit if header defines support for it */
  if( (60 == test) || (61 == test) || (62 == test) || (72 == test) ||
      (82 == test) || (63 == test) || (73 == test)) {
    pCtx->mode |= NTCAN_MODE_FD;
  }
#endif

  /*
   * Mark overlapped for Windows overlapped I/O tests and create
   * the event parameter of the Win32 overlapped structure
   */
#if defined(_WIN32) && !defined(_WIN32_WCE) && !defined(UNDER_RTSS)
  if ((6 == test) || (7 == test) || (16 == test)) {
    pCtx->mode |= NTCAN_MODE_OVERLAPPED;

    hEvent = CreateEvent(NULL,
                         TRUE,      /* flag for manual-reset  */
                         FALSE,     /* flag for initial state */
                         NULL);
    if (NULL == hEvent) {
        printf("Creating event for overlapped operation failed\n");
        CANTEST_FREE(pCtx);
        return(-1);
    }
    overlapped.hEvent = hEvent;
  }
#endif /* _WIN32 && !_WIN32_WCE && !UNDER_RTSS */

  /*
   *  Async I/O test for UNIX(Solaris) D3X interface driver
   *  Install signal handler and make driver call SIGIO for each received frame
   */
#if defined(unix) && defined(D3X)
  if (test == 6) {
    if (signal(SIGIO, sigio_handler) == SIG_ERR) {
      printf("Initialising signal handler failed\n");
      CANTEST_FREE(pCtx);
      return(-1);
    }
    pCtx->mode |= (NTCAN_MODE_OVERLAPPED | SIGIO);
  }
#endif /* unix && D3X */

#if (defined VXWORKS) && WANT_PRIO_CHECK == 1
  if (ERROR == _checkPriority(net)) {
    CANTEST_FREE(pCtx);
    return(ERROR);      /* Check priority of caller */
  }
#endif

#ifdef RMOS
  switch (test) {
  case 21:
    pCtx->mode |= NTCAN_MODE_SLOW;
    break;
  case 31:
    pCtx->mode |= NTCAN_MODE_FAST;
    break;
  case 41:
    pCtx->mode |= NTCAN_MODE_BURST;
    break;
  }
#endif

  if(pCtx->mode & NTCAN_MODE_MARK_INTERACTION) {
      printf(" (Mode: Mark Interaction)");
  } else if(pCtx->mode & NTCAN_MODE_NO_INTERACTION) {
      printf(" (Mode: No Interaction)");
  }
  printf("\n");

#if defined(NTCAN_IOCTL_GET_TX_TS_WIN)
  if ( (21 == test) || (20 == test) ) {
    pCtx->mode |= NTCAN_MODE_TIMESTAMPED_TX;
  }
#endif

  /*
   * Force thread affinity to one CPU or core on multiprocessor/multicore
   * hardware, if TSC of different CPUs/cores get out of sync.
   */
#ifdef CANTEST_MULTIPROCESSOR
  if(ForceThreadAffinity() != 0)
      printf("Forcing thread affinity to dedicated processor failed !!\n");
#endif /* of CANTEST_MULTIPROCESSOR */

#ifdef CANTEST_USE_SIGUSR1_HANDLER
  /*
   * Setup dummy signal handler for testing
   */
    if (signal(SIGUSR1, &sigusr1_handler) == SIG_ERR) {
      printf("Initialising USR1 signal handler failed\n");
      CANTEST_FREE(pCtx);
      return(-1);
    }
#endif



  /*
   * Open CAN handle and check success
   */
  for(;;) {
      if (21 == test) {
          /* Timeouts are (mis-)used for Timestamped TX purposes */
          ret = canOpen(net, pCtx->mode, txbufsize, rxbufsize, 0, 0, &h0);
      } else {
          ret = canOpen(net, pCtx->mode, txbufsize, rxbufsize, txtout,
                        rxtout, &h0);
      }
#if defined(NTCAN_FD)
      if ((ret != NTCAN_SUCCESS) && (pCtx->mode & NTCAN_MODE_FD)) {
          printf("canOpen returned: %s, retrying without NTCAN_MODE_FD\n",
                 get_error_str(str_buf,ret));
          pCtx->mode &= ~NTCAN_MODE_FD;
          continue;
      }
#endif
      break;
  } /* of for(;;) */

  if (ret != NTCAN_SUCCESS) {
    printf("canOpen returned: %s\n", get_error_str(str_buf,ret));
    CANTEST_FREE(pCtx);
    return(-1);
  } else {
      CAN_IF_STATUS cstat;

      /* Get the CAN controller type */
      ret = canStatus(h0, &cstat);
      if (NTCAN_SUCCESS == ret) {
#ifdef NTCAN_GET_CTRL_TYPE
          pCtx->ctrl_type = (uint8_t)NTCAN_GET_CTRL_TYPE(cstat.boardstatus);
#endif
      }
  }

  /*
   * For tests which require timestamp support check if driver/hardware
   * supports timestamps and return timestamp frequency
   */
#ifdef NTCAN_IOCTL_GET_TIMESTAMP
  if ((23 == test) || (22 == test) || (21 == test) || (20 == test) || (42 == test) || (100 == test) || (16 == test) ||
      (63 == test) || (62 == test) || (61 == test) || (60 == test) || (82 == test) || (110 == test) || (66 == test)  )
  {
      if(get_timestamp_freq(h0, &pCtx->udtTimestampFreq, 1) != 0) {
          CANTEST_FREE(pCtx);
          return(-1);
      }
  }
#endif

  /*
   * Change idstart/idend to event range for the read event test
   */
  if(4 == test) {
      idstart = NTCAN_EV_BASE + (idstart & 0xFF);
      idend   = NTCAN_EV_BASE + (idend   & 0xFF);
  }

  /*
  * Configure the CAN-ID filter for events before setting the baudrate
  * to e.g. capture baudrate change events.
  */
  if((idend <= NTCAN_EV_LAST) && (idstart >= NTCAN_EV_BASE)) {
      for (i = idstart; i <= idend; i++) {
          ret = set_can_id(h0, i);
          if (ret != NTCAN_SUCCESS) {
              printf("canIdAdd for event %ld returns error %s\n",
                  (long)i, get_error_str(str_buf, ret));
              break;
          }
      }

#ifdef NTCAN_IOCTL_SET_BUSLOAD_INTERVAL
      /*
       * If we just want to capture the busload event use txtout as
       * event period
       */
      if((NTCAN_EV_BUSLOAD == idstart) && (idstart == idend)) {
          bl_event_period = txtout;
          ret = canIoctl(h0,  NTCAN_IOCTL_SET_BUSLOAD_INTERVAL, &bl_event_period);
          if (ret != NTCAN_SUCCESS) {
              printf("NTCAN_IOCTL_SET_BUSLOAD_INTERVAL failed with: %s\n",
                  get_error_str(str_buf,ret));
          } else {
              /* Get configured bus load event period */
              ret = canIoctl(h0, NTCAN_IOCTL_GET_BUSLOAD_INTERVAL,
                             &bl_event_period);
              if (ret != NTCAN_SUCCESS) {
                  printf("NTCAN_IOCTL_GET_BUSLOAD_INTERVAL failed with: %s\n",
                      get_error_str(str_buf,ret));
              } else {
                  printf("Bus load event period set to %ld ms\n",
                         (long)bl_event_period);
              }
          }
      } /* of if(idstart == idend == NTCAN_EV_BUSLOAD) */
#endif
  } /* of if((idend <= NTCAN_EV_LAST ) && (idstart >= NTCAN_EV_BASE)) */

  /*
   * Configure the baudrate for all tests but sending events.
   */
  if ( (test != 5) && (test != 53) && (baudrate!=0xFFFFFFFFU) ) {
      ret = canSetBaudrate(h0, baudrate);
      if (ret != NTCAN_SUCCESS) {
          printf("canSetBaudrate returned: %s\n", get_error_str(str_buf,ret));
          CANTEST_FREE(pCtx);
          return(-1);
      }
  }

#ifdef NTCAN_IOCTL_SET_20B_HND_FILTER
  /*
  * Configure the CAN-ID filter using 20B handle filter
  */
  if(((idstart & 0x60000000) == 0x60000000) &&
      ((idend & 0x60000000) == 0x60000000))
  {
      unsigned long ulFilterMask = idend & 0x1FFFFFFF;

      ret = canIdAdd(h0, idstart & 0x2fffffff);
      if(ret != NTCAN_SUCCESS)
      {
          printf("canIdAdd for id %lx returns error %s\n",
              (long)idend, get_error_str(str_buf,ret));
          CANTEST_FREE(pCtx);
          return(-1);
      }
      else
      {
          ret = canIoctl(h0, NTCAN_IOCTL_SET_20B_HND_FILTER, &ulFilterMask);
          if(ret != NTCAN_SUCCESS)
          {
              printf("canIoctl to set filter mask %lx returns error %s\n",
                  (unsigned long)ulFilterMask, get_error_str(str_buf,ret));
              CANTEST_FREE(pCtx);
              return(-1);
          }
      }

      printf("Using CAN 2.0B Acceptance Code: 0x%08lx / Acceptance Mask: 0x%08lx\n",
          (unsigned long)(idstart & 0x1fffffff), (unsigned long)ulFilterMask);

      idstart = 2048;	/* No further canIdAdd() */
      idend   = 0;
  }
  else
#endif
  {
      /*
      * Receive 2.0B IDs if either idend is in 2.0B or event range and
      * idstart is 2.0A ID or both idend and idstart are in 2.0B range
      */
      if(idstart <= idend) {
          if (((idend >= NTCAN_20B_BASE) && (idstart < 2048)) ||
              (((idstart & NTCAN_20B_BASE) == NTCAN_20B_BASE) &&
              ((idend & NTCAN_20B_BASE) == NTCAN_20B_BASE))) {
                  ret = canIdAdd(h0, NTCAN_20B_BASE);
                  if (ret != NTCAN_SUCCESS) {
                      printf("canIdAdd for all 29-Bit ids returns error %s\n",
                          get_error_str(str_buf,ret));
                  }
          }
      } /* of if(idstart <= idend) */
  }

  if (idend > 2047) {
      idend = 2047;
  }

  /*
  * Enable the 11-bit identifier
  */
  for (i = idstart; i <= idend; i++) {
      ret = set_can_id(h0, i);
      if (ret != NTCAN_SUCCESS) {
          printf("canIdAdd for can-id %ld returns error %s\n",
              (long)i, get_error_str(str_buf,ret));
          break;
      }
  }

  idnow = idstart;
  start_test_time = GET_CURRENT_TIME;

  /*
   * The following loop repeats the entire test "testcount"-times
   */
  for (h = 0; (h < testcount) || ever; h++) {
    /*
     * Choose the desired test
     */
    switch (test) {
#ifndef D3X
      /*
       * canSend()
       * non-blocking write or write without wait
       */
    case 0:
    case 50:
      len = maxcount;
      if (!data_from_cmd) {
          for (i = 0; i < len; i++) {
            *((uint32_t*)(&pCtx->txmsg[i].data[0])) = h;
          }
      }

      if (50 == test) {
          for (i = 0; i < len; i++) {
              pCtx->txmsg[i].id = idnow++;
              if (idnow > idend) {
                  idnow = idstart;
              }
          }
      }
      start_time = GET_CURRENT_TIME;
      ret = canSend(h0, pCtx->txmsg, &len);
      stop_time = GET_CURRENT_TIME;
      printf("Duration=%9lu %s Can-Messages=%ld\n",
         (unsigned long)(stop_time-start_time), TIME_UNITS, (unsigned long)len);
      if (ret != NTCAN_SUCCESS) {
        printf("canSend returned: %s\n", get_error_str(str_buf,ret));
        SLEEP(1000);
      } else {
        SLEEP(txtout);
      }
      break;

      /*
       * canSendT()/canWriteT()/canSendX()/canWriteT()
       * non-blocking timestamped write
       */
#if defined(NTCAN_IOCTL_GET_TX_TS_WIN)
    case 20:
    case 21:
#if defined(NTCAN_FD)
    case 60:
    case 61:
#endif
      len = maxcount;
      if (!data_from_cmd) {
          for (i = 0; i < len; i++) {
            *((uint32_t*)(&pCtx->txmsg_t[i].data[0])) = h;
#if defined(NTCAN_FD)
            *((uint32_t*)(&pCtx->txmsg_x[i].data[0])) = h;
#endif
          }
      }
      ret = canIoctl(h0, NTCAN_IOCTL_GET_TIMESTAMP, &pCtx->txmsg_t[0].timestamp);
      if (ret != NTCAN_SUCCESS) {
        printf("canIoctl returned: %s (failed to acquire timestamp)\n", get_error_str(str_buf, ret));
      }
#if defined(NTCAN_FD)
      pCtx->txmsg_x[0].timestamp = pCtx->txmsg_t[0].timestamp;
      pCtx->txmsg_x[0].timestamp += ((pCtx->udtTimestampFreq * (uint64_t)txtout) / 1000ULL);  /* Schedule first frame to be send in "TX-timeout" milliseconds */
      for (i = 1; i < len; i++) {
        pCtx->txmsg_x[i].timestamp = pCtx->txmsg_x[i-1].timestamp + ((pCtx->udtTimestampFreq * (uint64_t)rxtout) / 1000ULL); /* Every other frame will follow with "RX-timeout" milliseconds offset */
      }
#endif
      pCtx->txmsg_t[0].timestamp += ((pCtx->udtTimestampFreq * (uint64_t)txtout) / 1000ULL);  /* Schedule first frame to be send in "TX-timeout" milliseconds */
      for (i = 1; i < len; i++) {
        pCtx->txmsg_t[i].timestamp = pCtx->txmsg_t[i-1].timestamp + ((pCtx->udtTimestampFreq * (uint64_t)rxtout) / 1000ULL); /* Every other frame will follow with "RX-timeout" milliseconds offset */
      }

      start_time = GET_CURRENT_TIME;
      switch(test) {
          case 20:
              ret = canSendT(h0, pCtx->txmsg_t, &len);
              break;
          case 21:
              ret = canWriteT(h0, pCtx->txmsg_t, &len, NULL);
              break;
#if defined(NTCAN_FD)
          case 60:
              ret = canSendX(h0, pCtx->txmsg_x, &len);
              break;
          case 61:
              ret = canWriteX(h0, pCtx->txmsg_x, &len, NULL);
              break;
#endif
      }
      stop_time = GET_CURRENT_TIME;
      printf("Duration=%9lu %s Can-Messages=%ld\n",
         (unsigned long)(stop_time-start_time), TIME_UNITS, (unsigned long)len);
      if (ret != NTCAN_SUCCESS) {
        printf("canSend returned: %s\n", get_error_str(str_buf,ret));
        SLEEP(1000);
      } else {
          if(test == 20 || test == 21) {
              /* Sleep for some time (500ms + duration of scheduled frames)
               * in order to prevent close from aborting our running canSendT scheduling... */
              SLEEP(500 + txtout + ((maxcount-1) * rxtout));
          }
      }
      break;
#endif

#endif

      /*
       * canWrite()
       * blocking write or write with wait
       */
    case 1:
#ifdef RMOS
    case 21:
#endif
    case 31:
    case 41:
    case 51:
      len = maxcount;
      if (!data_from_cmd) {
        for (i = 0; i < len; i++) {
          *((uint32_t*)(&pCtx->txmsg[i].data[0])) = h;
        }
      }
      if (51 == test) {
        for (i = 0; i < len; i++) {
          pCtx->txmsg[i].id = idnow++;
          if (idnow > idend) {
            idnow = idstart;
          }
        }
      }
      start_time = GET_CURRENT_TIME;

      ret = canWrite(h0, pCtx->txmsg, &len, NULL);

      stop_time = GET_CURRENT_TIME;
      printf("Duration=%9lu %s Can-Messages=%ld \n",
             (unsigned long)(stop_time-start_time), TIME_UNITS, (unsigned long)len);
      if (ret != NTCAN_SUCCESS) {
        printf("canWrite returned: %s\n", get_error_str(str_buf,ret) );
        SLEEP(1000);
      }
      break;

    case 11:
      if (!data_from_cmd) {
        for (i = 0; i < maxcount; i++) {
          *((uint32_t*)(&pCtx->txmsg[i].data[0])) = h;
        }
      }
      start_time = GET_CURRENT_TIME;
#if 0
      for (i = 0; i < maxcount; i++) {
        len = 1;
        (void)canWrite(h0, &txmsg[i], &len, NULL);
      }
#else
      for (i = 0; i < maxcount; i++) {
        do {
          len = 1;
          ret = canSend(h0, &pCtx->txmsg[i], &len);
        } while (ret == NTCAN_CONTR_BUSY);
      }
#endif
      stop_time = GET_CURRENT_TIME;
      printf("Duration=%6ld %s Can-Messages=%ld \n",
             (unsigned long)(stop_time-start_time),
             TIME_UNITS, (unsigned long)len);
      if (ret != NTCAN_SUCCESS) {
        printf("canWrite returned: %s\n",
               get_error_str(str_buf,ret));
        SLEEP(1000);
      }
      break;

      /*
       * canTake()
       * non-blocking read or read without wait
       */
#ifndef D3X
    case 32:
      len = idend-idstart+1;
      if(rxtout) {
          SLEEP(rxtout);
      }
      goto take_common;
    case 12:    /* test with output disabled */
      pCtx->mode |= NO_ASCII_OUTPUT;
    case 2:
      len = maxcount;
    take_common:
      start_time = GET_CURRENT_TIME;
#ifdef RTAI
      /* To better see canTake working ... */
      SLEEP(100);
      start_time += 100000; /* us */
      start_test_time += 100000; /* us */
#endif
      ret = canTake(h0, pCtx->rxmsg, &len);

      stop_time = GET_CURRENT_TIME;
      if (frames >= 10000) {
        stop_test_time = GET_CURRENT_TIME;
        printf("Test-Duration=%lu %s frames=%ld\n",
               (unsigned long)(stop_test_time-start_test_time), TIME_UNITS, (unsigned long)frames);
        start_test_time = GET_CURRENT_TIME;
        frames = 0;
      }
      if (!(pCtx->mode & NO_ASCII_OUTPUT)) {
        printf("Duration=%9lu %s Can-Messages=%ld\n",
               (unsigned long)(stop_time-start_time), TIME_UNITS, (unsigned long)len);
      }
      if (ret == NTCAN_SUCCESS) {
        if ((pCtx->mode & NO_ASCII_OUTPUT)) {
          if (frames == -1) {
            start_test_time = GET_CURRENT_TIME;
            frames = 0;
            continue;
          }
        }
        for (i = 0; i < len; i++) {
            print_cmsg(&pCtx->rxmsg[i], pCtx);
            frames ++;
        }
      } else {
        printf("canTake returned: %s\n",
               get_error_str(str_buf,ret));
        SLEEP(1000);
      }
      break;

      /*
       * canRead()
       * blocking read or read with wait
       */
    case 33:     /* test with output disabled and data validation */
    case 13:     /* test with output disabled */
      pCtx->mode |= NO_ASCII_OUTPUT;
#endif /* of !D3X */
    case 3:
      len = maxcount;
      start_time = GET_CURRENT_TIME;

      ret = canRead(h0, pCtx->rxmsg, &len, NULL);
#ifdef RTAI
      /* To better see canRead working ... */
      SLEEP(100);
      start_time += 100000; /* us */
      start_test_time += 100000; /* us */
#endif
      stop_time = GET_CURRENT_TIME;
      if (frames >= 10000) {
        stop_test_time = GET_CURRENT_TIME;
        printf("Test-Duration=%lu %s frames=%ld\n",
               (unsigned long)(stop_test_time-start_test_time), TIME_UNITS, (unsigned long)frames);
        start_test_time = GET_CURRENT_TIME;
        frames = 0;
      }
      if (!(pCtx->mode & NO_ASCII_OUTPUT)) {
        printf("Duration=%6lu %s Can-Messages=%ld \n",
               (unsigned long)(stop_time-start_time), TIME_UNITS, (unsigned long)len);
      }
      if (ret == NTCAN_SUCCESS) {
        if ((pCtx->mode & NO_ASCII_OUTPUT)) {
          if (frames == -1) {
            start_test_time = GET_CURRENT_TIME;
            frames = 0;
          }
        }

        for (i = 0; i < len; i++) {
          if (33 == test) {
            static uint32_t counter;
            pCtx->mode |= NO_ASCII_OUTPUT;
            if ( (pCtx->rxmsg[i].msg_lost != 0) ||
                 (*((uint32_t*)&pCtx->rxmsg[i].data[4]) == 1) ) {
              counter = *((uint32_t*)&pCtx->rxmsg[i].data[4]);
            }
            if ( (pCtx->rxmsg[i].len != 8) ||
                (pCtx->rxmsg[i].id != idstart) ||
                (counter != *((uint32_t*)&pCtx->rxmsg[i].data[4]) ) ) {
              printf("Wrong Frame received. Counter=0x%lx  0x%lx  0x%lx %u!\n",
                     (unsigned long)counter, (unsigned long)*((uint32_t*)&pCtx->rxmsg[i].data[4]), (unsigned long)pCtx->rxmsg[i].id, (unsigned int)pCtx->rxmsg[i].msg_lost);
              pCtx->mode &= ~NO_ASCII_OUTPUT;
              print_cmsg(&pCtx->rxmsg[i], pCtx);
              exit(1);
            }
            counter++;
          }
          print_cmsg(&pCtx->rxmsg[i], pCtx);
          frames ++;
        }
      } else {
        printf("canRead returned: %s\n", get_error_str(str_buf,ret));
        SLEEP(100);
#ifdef RTAI
        start_test_time += 100000; /* us, this should be done more genric for other OS */
#endif
      }
      break;

#ifdef NTCAN_IOCTL_GET_TIMESTAMP

      /*
       * canTakeT()/canTakeX()
       * non-blocking read
       */
#ifdef NTCAN_FD
    case 82:
#endif
    case 42:
      len = idend-idstart+1;
      if(rxtout) {
        SLEEP(rxtout);
      }
      goto taket_common;
#ifdef NTCAN_FD
    case 72:
      pCtx->mode |= NO_ASCII_OUTPUT;
#endif
    case 22:
#ifdef NTCAN_FD
    case 62:
#endif
      len = maxcount;
    taket_common:
      start_time = GET_CURRENT_TIME;
#ifdef RTAI
      /* To better see canTakeT working ... */
      SLEEP(100);
      start_time += 100000; /* us */
      start_test_time += 100000; /* us */
#endif
#ifdef NTCAN_FD
      if(test == 62 || test == 72 || test == 82)
          { ret = canTakeX(h0, pCtx->rxmsg_x, &len); }
      else
#endif
          { ret = canTakeT(h0, pCtx->rxmsg_t, &len); }

      stop_time = GET_CURRENT_TIME;
      if (frames >= 10000) {
        stop_test_time = GET_CURRENT_TIME;
        printf("Test-Duration=%lu %s frames=%ld\n",
               (unsigned long)(stop_test_time-start_test_time), TIME_UNITS, (unsigned long)frames);
        start_test_time = GET_CURRENT_TIME;
        frames = 0;
      }
      if (!(pCtx->mode & NO_ASCII_OUTPUT)) {
        printf("Duration=%9lu %s Can-Messages=%ld\n",
               (unsigned long)(stop_time-start_time), TIME_UNITS, (unsigned long)len);
      }
      if (ret == NTCAN_SUCCESS) {
        if ((pCtx->mode & NO_ASCII_OUTPUT)) {
          if (frames == -1) {
            start_test_time = GET_CURRENT_TIME;
            frames = 0;
            continue;
          }
        }
        for (i = 0; i < len; i++) {
#ifdef NTCAN_FD
            if(test == 62 || test == 72 || test == 82)
                { print_cmsg_x(&pCtx->rxmsg_x[i], pCtx);
            } else
#endif
                { print_cmsg_t(&pCtx->rxmsg_t[i], pCtx); }

            frames ++;
        }
      } else {
        printf("canTake* returned: %s\n",
               get_error_str(str_buf,ret));
        SLEEP(1000);
      }
      break;

      /*
       * canReadT()/canReadX()
       * blocking read with timestamp support
       */
#ifdef NTCAN_FD
    case 73:     /* test with output disabled and timestamp validation */
#endif
    case 43:     /* test with output disabled and timestamp validation */
      pCtx->mode |= NO_ASCII_OUTPUT;
    case 23:
#ifdef NTCAN_FD
    case 63:
#endif
      len = maxcount;
      start_time = GET_CURRENT_TIME;
#ifdef NTCAN_FD
      if(test == 63 || test == 73) {
        ret = canReadX(h0, pCtx->rxmsg_x, &len, NULL);
      } else
#endif
      { ret = canReadT(h0, pCtx->rxmsg_t, &len, NULL); }
#ifdef RTAI
      /* To better see canRead working ... */
      SLEEP(100);
#endif
      stop_time = GET_CURRENT_TIME;
      if (frames >= 10000) {
        stop_test_time = GET_CURRENT_TIME;
        printf("Test-Duration=%lu %s frames=%ld\n",
               (unsigned long)(stop_test_time-start_test_time), TIME_UNITS, (unsigned long)frames);
        start_test_time = GET_CURRENT_TIME;
        frames = 0;
      }
      if (!(pCtx->mode & NO_ASCII_OUTPUT)) {
        printf("Duration=%6lu %s Can-Messages=%ld \n",
               (unsigned long)(stop_time-start_time), TIME_UNITS, (unsigned long)len);
      }

      if (ret == NTCAN_SUCCESS) {
          if (frames == -1) {
              start_test_time = GET_CURRENT_TIME;
#ifdef NTCAN_FD
              if(test == 63 || test == 73)
              { lastStamp = pCtx->rxmsg_x[0].timestamp; }
              else
#endif
              { lastStamp = pCtx->rxmsg_t[0].timestamp; }
              frames = 0;
          }

          for (i = 0; i < len; i++ ) {

              if (!(pCtx->mode & NO_ASCII_OUTPUT)) {
#ifdef NTCAN_FD
                  if(test == 63 || test == 73)  {
                    print_cmsg_x(&pCtx->rxmsg_x[i], pCtx);
                  } else
#endif
                  { print_cmsg_t(&pCtx->rxmsg_t[i], pCtx); }
              }

              if ((pCtx->mode & NO_ASCII_OUTPUT)) {
#ifdef NTCAN_FD
                  if(test == 63 || test == 73) {
                    if( lastStamp > pCtx->rxmsg_x[i].timestamp) {
                        printf("Negative timestamp interval= 0x%"PRIx64",0x%"PRIx64"\n",
                            lastStamp, pCtx->rxmsg_x[i].timestamp);
                    }

                    lastStamp = pCtx->rxmsg_x[i].timestamp;
                  } else
#endif
                  {
                    if( lastStamp > pCtx->rxmsg_t[i].timestamp) {
                        printf("Negative timestamp interval= 0x%"PRIx64",0x%"PRIx64"\n",
                            lastStamp, pCtx->rxmsg_t[i].timestamp);
                    }

                    lastStamp = pCtx->rxmsg_t[i].timestamp;
                  }
              }

              frames++;
          }
      } else {
          printf("canRead* returned: %s\n", get_error_str(str_buf, ret));
          SLEEP(100);
      }
      break;
#endif

      /*
       * canReadEvent()
       * blocking read of status information and customer specific events
       */
    case 4:
      start_time = GET_CURRENT_TIME;

#ifndef D3X
      len = 1;
      ret = canRead(h0, (CMSG *)&evmsg, &len, NULL);
#else
      ret = canReadEvent(h0, &evmsg, NULL);
#endif

      stop_time = GET_CURRENT_TIME;
      printf("Duration=%lu %s\n",
             (unsigned long)(stop_time-start_time), TIME_UNITS);
      if (ret == NTCAN_SUCCESS) {
        printf("EVENT-ID=%8lx len=%01X data= ",
               (long)evmsg.evid, evmsg.len);
        for (j = 0; j < evmsg.len; j++) {
          printf("%02X ", evmsg.evdata.c[j]);
        }

        printf("\n");

        print_event(&evmsg, 0, pCtx);

      } else {
        printf("Reading event returned: %s\n", get_error_str(str_buf,ret));
        SLEEP(100);
      }
      break;

#if 0 /* test-only !!!!!!!!!!!!!!!! */
    case 44:
      len = maxcount;
      start_time = GET_CURRENT_TIME;
      SLEEP(1000);
      ret = canIoctl(h0, NTCAN_IOCTL_FLUSH_RX_FIFO, NULL);
      stop_time = GET_CURRENT_TIME;
      printf("Duration=%u %s\n",
             (unsigned long)(stop_time-start_time), TIME_UNITS);
      printf("pre canTake: len=%d maxcount=%d\n",
             (long)len, (long)maxcount);
      ret = canTake(h0, rxmsg, &len);
      printf("canTake returns: ret=%x len=%d\n",
             (long)ret, (long)len);
      break;

    case 45:
      len = maxcount;
      start_time = GET_CURRENT_TIME;
      SLEEP(1000);
      stop_time = GET_CURRENT_TIME;
      printf("Duration=%u %s\n", (uint32_t)(stop_time-start_time), TIME_UNITS);
      ret = canTake(h0, rxmsg, &len);
      printf("canTake returns: ret=%x len=%d\n", (int32_t)ret, (int32_t)len);
      break;
    case 46:
      {
        uint32_t val;
        len = maxcount;
        start_time = GET_CURRENT_TIME;
        SLEEP(1000);
        ret = canIoctl(h0, NTCAN_IOCTL_GET_RX_MSG_COUNT, &val);
        printf("GET_RX_MSG_COUNT=%d\n", (int32_t)val);
        ret = canIoctl(h0, NTCAN_IOCTL_FLUSH_RX_FIFO, NULL);
        stop_time = GET_CURRENT_TIME;
        printf("Duration=%u %s\n", (unsigned long)(stop_time-start_time), TIME_UNITS);
        ret = canTake(h0, rxmsg, &len);
        printf("canTake returns: ret=%x len=%d\n", (long)ret, (long)len);
        break;
      }
#endif

#ifdef NTCAN_IOCTL_GET_TIMESTAMP
    case 53:
      {
        uint64_t ts, ts_old, ts_mindiff;
        int cnt;
        cnt = 100000;
        ts_mindiff = UINT64_C(9999999999);
        ts_old = 0;
        printf("canIoctl(GET_TIMESTAMP), %d calls, ",cnt);
        start_time = GET_CURRENT_TIME;
        do {
                ret = canIoctl(h0, NTCAN_IOCTL_GET_TIMESTAMP, &ts);
                if (ret != NTCAN_SUCCESS) {
                  printf("NTCAN_IOCTL_GET_TIMESTAMP failed with: %s\n", get_error_str(str_buf,ret));
                  break;
                }
                if (ts<ts_old) {
                  printf("Timestamps not increasing!! Old=0x%" PRIx64 " New=0x%" PRIx64 ".\n",ts_old,ts);
                  break;
                }
                ts-=ts_old;
                if (ts_mindiff>ts) ts_mindiff=ts;
                ts_old=ts;
        } while (--cnt);
        stop_time = GET_CURRENT_TIME;
        printf("Min_diff=%" PRIu64 " Duration=%lu %s\n", ts_mindiff, (unsigned long)(stop_time-start_time), TIME_UNITS);
        break;
      }
#endif
#ifdef unix
    case 54:
    {
      int iii,ppp;
      const int lens[10]={555,111,1234,765,777,3333,2345,888,1000,10};
      if (!data_from_cmd) {
        for (i = 0; i < MAX_TX_MSG_CNT; i++) {
          *((uint32_t*)(&pCtx->txmsg[i].data[0])) = h;
        }
      }
      len=maxcount;
      printf("Parent: Testcount=%d.\n",h);
      start_time = GET_CURRENT_TIME;
      for (iii = 0; iii< 10 ; iii++) {
        if ((ppp=fork())==0) { /* we are the child */
                for (i = 0; i < len; i++) {
                 pCtx->txmsg[i].id = iii;
                }
                start_time = GET_CURRENT_TIME;

                SLEEP(lens[iii]);
                ret = canSend(h0, pCtx->txmsg, &len);

                stop_time = GET_CURRENT_TIME;
                printf("Child %d: Duration=%9lu %s Can-Messages=%ld \n",
                     iii,(unsigned long)(stop_time-start_time), TIME_UNITS, (unsigned long)len);
                if (ret != NTCAN_SUCCESS) {
                        printf("Child %d: canSend returned: %s\n", iii, get_error_str(str_buf,ret) );
                }
                SLEEP(500);
#ifndef nto
                SLEEP(4500); /* fork not working that way here, canClose would render the handle useless for all forked childs */
#endif
                printf("Child %d: Exiting...\n",iii);
                h=testcount; /* last testcount for this child, child shouldn't run any (more) tests ... */
                break;
        } else {
                printf("Parent: Started child %d with pid %d.\n",iii,ppp);
        }
      }
      if (iii==10) {
                stop_time = GET_CURRENT_TIME;
                printf("Parent: Duration=%9lu %s\n",(unsigned long)(stop_time-start_time), TIME_UNITS);
                printf("Parent: Sleeping 5 secs...\n");fflush(stdout);SLEEP(5000);
                printf("Parent: Sleeping done.\n");fflush(stdout);
      }
      break;
    }
#endif
    case 55: /* close - send test */
      ret = canClose(h0);
      printf("canClose returned: %s\n",get_error_str(str_buf,ret)); /* should be SUCCESS */
      len = maxcount;
      ret = canSend(h0, pCtx->txmsg, &len);
      printf("canSend returned: %s - len=%u\n", get_error_str(str_buf,ret),
            (unsigned int)len); /* should be INVALID HANDLE */
      break;

#if 0
    case 56:
      {
        int cnt;
        uint32_t bb,bb0;
        cnt = maxcount * 1000;if (cnt>100000) cnt=100000;
        printf("canGetBaudrate(), %d calls, ",cnt);
        ret = canGetBaudrate(h0, &bb0);
        if (ret != NTCAN_SUCCESS) {
          printf("canGetBaudrate failed with: %s\n", get_error_str(str_buf,ret));
        }
        start_time = GET_CURRENT_TIME;
        do {
                ret = canGetBaudrate(h0, &bb);
                if (ret != NTCAN_SUCCESS) {
                  printf("canGetBaudrate failed with: %s\n", get_error_str(str_buf,ret));
                  break;
                }
                if (bb!=bb0) {
                  printf("Baudrate has suddenly changed! Old=0x%x New=0x%x\n",(unsigned int)bb0,(unsigned int)bb);
                  break;
                }
        } while (--cnt);
        stop_time = GET_CURRENT_TIME;
        printf("Duration=%lu %s\n", (unsigned long)(stop_time-start_time), TIME_UNITS);
        break;
      }
#endif

#if defined NTCAN_IOCTL_GET_BUS_STATISTIC && !defined D3X
    case 64:
      {
          NTCAN_BUS_STATISTIC stat;
          NTCAN_CTRL_STATE ctrl_state;

          ret = canIoctl(h0, NTCAN_IOCTL_GET_BUS_STATISTIC, &stat);
          if (ret != NTCAN_SUCCESS) {
              printf("NTCAN_IOCTL_GET_BUS_STATISTIC failed with: %s\n",
                  get_error_str(str_buf, ret));
              break;
          }
          ret = canIoctl(h0, NTCAN_IOCTL_GET_CTRL_STATUS, &ctrl_state);
          if (ret != NTCAN_SUCCESS) {
              printf("NTCAN_IOCTL_GET_CTRL_STATUS failed with: %s\n",
                  get_error_str(str_buf, ret));
              break;
          }

          printf("CAN bus statistic:\n------------------\n");
          printf("Rcv frames      : Std(Data/RTR): %ld/%ld Ext(Data/RTR) %ld/%ld\n",
              (long)stat.rcv_count.std_data, (long)stat.rcv_count.std_rtr,
              (long)stat.rcv_count.ext_data, (long)stat.rcv_count.ext_rtr);
          printf("Xmit frames     : Std(Data/RTR): %ld/%ld Ext(Data/RTR) %ld/%ld\n",
              (long)stat.xmit_count.std_data, (long)stat.xmit_count.std_rtr,
              (long)stat.xmit_count.ext_data, (long)stat.xmit_count.ext_rtr);
          printf("Bytes           : (Rcv/Xmit): %ld/%ld\n",
              (long)stat.rcv_byte_count, (long)stat.xmit_byte_count);
          printf("Overruns        : (Controller/FIFO): %ld/%ld\n",
              (long)stat.ctrl_ovr, (long)stat.fifo_ovr);
          printf("Err frames      : %ld\n", (long)stat.err_frames);
          printf("Aborted frames  : %ld\n", (long)stat.aborted_frames);
          printf("Err counter     : (Rx/Tx): %d/%d Status: %02x\n",
              ctrl_state.rcv_err_counter,
              ctrl_state.xmit_err_counter,
              ctrl_state.status);
          printf("Rcv bits        : %" PRIu64 "\n", stat.bit_count);
          SLEEP(txtout);
      }
      break;

    case 74:
      {
          ret = canIoctl(h0, NTCAN_IOCTL_GET_BUS_STATISTIC, NULL);
          if (ret != NTCAN_SUCCESS) {
              printf("NTCAN_IOCTL_GET_BUS_STATISTIC (NULL) failed with: %s\n",
                  get_error_str(str_buf, ret));
              break;
          }
          printf("CAN bus statistic reset.\n");
          SLEEP(txtout);
      }
      break;
#endif /* of NTCAN_IOCTL_GET_BUS_STATISTIC */
#ifdef NTCAN_IOCTL_GET_BITRATE_DETAILS
    case 84:
      {
          NTCAN_BITRATE bitrate;
          long          sp;

          ret = canIoctl(h0, NTCAN_IOCTL_GET_BITRATE_DETAILS, &bitrate);
          if (ret != NTCAN_SUCCESS) {
              printf("NTCAN_IOCTL_GET_BITRATE_DETAILS failed with: %s\n",
                  get_error_str(str_buf, ret));
              break;
          }
          printf("CAN bitrate:\n------------\n");
          printf("Value set by canSetBaudrate()  : 0x%08lX\n",
              (long)bitrate.baud);
          if (NTCAN_SUCCESS == bitrate.valid) {
              printf("Actual Bitrate                 : %ld Bits/s\n",
                  (long)bitrate.rate);
              printf("Timequantas per Bit            : %ld\n",
                  (long)(bitrate.tq_pre_sp + bitrate.tq_post_sp));
              printf("Timequantas before samplepoint : %ld\n",
                  (long)bitrate.tq_pre_sp);
              printf("Timequantas after samplepoint  : %ld\n",
                  (long)bitrate.tq_post_sp);
              printf("Syncronization Jump Width      : %ld\n",
                  (long)bitrate.sjw);
              printf("Additional flags               : 0x%08lX\n",
                  (long)bitrate.flags);
              sp = (long)((bitrate.tq_pre_sp * 10000) /
                  (bitrate.tq_pre_sp + bitrate.tq_post_sp));
              printf("Position samplepoint           : %ld.%ld%%\n",
                  sp/100, sp%100);
              printf("Deviation from configured rate : %ld.%02ld%%\n",
                  (long)(bitrate.error/100), (long)(bitrate.error%100));
              printf("Controller clockrate           : %ld.%ldMHz\n",
                  (long)(bitrate.clock/1000000), (long)(bitrate.clock%1000000));
          }
          SLEEP(txtout);
      }
      break;
#endif /* NTCAN_IOCTL_GET_BITRATE_DETAILS */
      /*
       * canSendEvent()
       * non-blocking send of customer specific events
       */
    case 5:
#ifndef D3X
      len = 1;
      ret = canSend(h0, (CMSG *)&evmsg, &len);
#else
      ret = canSendEvent( h0, &evmsg );
#endif
      if (ret == NTCAN_SUCCESS) {
        printf("Event %08lx, data written: ", (long)evmsg.evid);
        for (j = 0; j < evmsg.len; j++) {
          printf("%02X ", evmsg.evdata.c[j]);
        }
        printf ("\n" );
      } else {
        printf("Sending event returned: %s\n", get_error_str(str_buf,ret));
      }
      SLEEP(1000);
      break;

#if defined(_WIN32) && !defined(_WIN32_WCE) && !defined(UNDER_RTSS)
    case 6:
      len = MAX_RX_MSG_CNT;
      start_time = GET_CURRENT_TIME;
      ret = canRead(h0, pCtx->rxmsg, &len, &overlapped);
      stop_time = GET_CURRENT_TIME;
      printf("async-Read triggered; Duration=%lu %s\n",
             stop_time - start_time, TIME_UNITS);
      printf("canRead returned %s \n", get_error_str(str_buf,ret));
      if ((ret != NTCAN_SUCCESS) && (ret != NTCAN_IO_PENDING)) {
        break;
      }
      start_time = GET_CURRENT_TIME;
      ret = WaitForSingleObject(hEvent,    /* event-handle      */
                                INFINITE); /* wait indefinitely */
      stop_time = GET_CURRENT_TIME;
      printf("WaitForSingleObject returned %s; Duration=%lu %s\n",
             get_error_str(str_buf,ret), stop_time-start_time, TIME_UNITS);
      ret = canGetOverlappedResult(h0,            /* filehandle     */
                                   &overlapped,
                                   &len,          /* ret cmsg-count */
                                   FALSE);        /* do not wait    */
      if (ret == NTCAN_SUCCESS) {
        printf("Len(cmsg)=%d\n", len);
        for (i = 0; i < len; i++) {
            print_cmsg(&pCtx->rxmsg[i], pCtx);
        }
      } else {
        printf("canOverlappedResult returned: %s\n",
               get_error_str(str_buf,ret));
      }
      break;

# if defined(NTCAN_IOCTL_GET_TIMESTAMP) && !defined(D3X)
    case 66:
    case 16:
      len = MAX_RX_MSG_CNT;
      start_time = GET_CURRENT_TIME;
#  if defined(NTCAN_FD)
      if(test == 66)  {
          ret = canReadX(h0, pCtx->rxmsg_x, &len, &overlapped);
      } else
#  endif
      { ret = canReadT(h0, pCtx->rxmsg_t, &len, &overlapped); }
      stop_time = GET_CURRENT_TIME;
      printf("async-Read triggered; Duration=%lu %s\n",
             stop_time - start_time, TIME_UNITS);
      printf("canRead returned %s \n", get_error_str(str_buf,ret));
      if ((ret != NTCAN_SUCCESS) && (ret != NTCAN_IO_PENDING)) {
        break;
      }
      start_time = GET_CURRENT_TIME;
      ret = WaitForSingleObject(hEvent,    /* event-handle      */
                                INFINITE); /* wait indefinitely */
      stop_time = GET_CURRENT_TIME;
      printf("WaitForSingleObject returned %s; Duration=%lu %s\n",
             get_error_str(str_buf,ret), stop_time-start_time, TIME_UNITS);
      ret = canGetOverlappedResultT(h0,            /* handle               */
                                    &overlapped,   /* overlapped structure */
                                    &len,          /* ret cmsg-count       */
                                    FALSE);        /* do not wait          */
      if (ret == NTCAN_SUCCESS) {
        printf("Len(cmsg)=%d\n", len);
        for (i = 0; i < len; i++) {
#  if defined(NTCAN_FD)
            if(test == 66)  {
                print_cmsg_x(&pCtx->rxmsg_x[i], pCtx);
            } else
#  endif
            { print_cmsg_t(&pCtx->rxmsg_t[i], pCtx); }

        }
      } else {
        printf("canOverlappedResult returned: %s\n",
			   get_error_str(str_buf,ret));
      }
      break;
# endif

    case 7:
      len = maxcount;
      if (!data_from_cmd) {
        for (i = 0; i < len; i++) {
          *((uint32_t*)(&pCtx->txmsg[i].data[0])) = h;
        }
      }
      start_time = GET_CURRENT_TIME;
      ret = canWrite(h0, pCtx->txmsg, &len, &overlapped);
      stop_time =GET_CURRENT_TIME;
      printf("async-Write triggered; Duration=%lu %s\n",
             stop_time - start_time, TIME_UNITS);
      printf("canWrite returned %s \n", get_error_str(str_buf,ret));
      if ((ret != NTCAN_SUCCESS) && (ret != NTCAN_IO_PENDING)) {
        break;
      }
      start_time = GET_CURRENT_TIME;
#if 1
      ret = WaitForSingleObject(hEvent,           /* event-handle       */
                                INFINITE);        /* wait indefinitely  */
      printf("WaitForSingleObject returned %s; \n",
             get_error_str(str_buf,ret));
      ret = canGetOverlappedResult(h0,            /* filehandle         */
                                   &overlapped,
                                   &len,          /* ret cmsg-count     */
                                   FALSE);        /* do not wait        */
#else
      ret = canGetOverlappedResult(h0,            /* filehandle         */
                                   &overlapped,
                                   &len,          /* ret cmsg-count     */
                                   TRUE);         /* wait               */
#endif
      stop_time = GET_CURRENT_TIME;
      if (ret == NTCAN_SUCCESS) {
        printf("Duration=%6lu %s Can-Messages=%d \n",
               stop_time-start_time, TIME_UNITS, len);
      } else {
        printf("canOverlappedResult returned: %s\n",
               get_error_str(str_buf,ret));
      }
      break;
#endif /* of _WIN32 */

#if defined(unix) && defined(D3X)
    case 6:
      h1 = h0;   /* Store handle in global variable, start of test */
      printf("Async Read triggred\n");
      while(1);
      start_time = GET_CURRENT_TIME;
      break;
#endif /* of unix && D3X */

#ifndef D3X
# ifdef NTCAN_IOCTL_TX_OBJ_SCHEDULE
    case 8:
      /*
       * Create auto RTR object
       */
      len = 1;
      if (!data_from_cmd)
        *((uint32_t*)(&pCtx->txmsg[0].data[0])) = h;
      ret = canIoctl(h0, NTCAN_IOCTL_TX_OBJ_CREATE, &pCtx->txmsg[0]);
      if (ret != NTCAN_SUCCESS) {
        printf("NTCAN_IOCTL_TX_OBJ_CREATE failed with :%s\n",
               get_error_str(str_buf,ret));
        break;
      }
      ret = canIoctl(h0, NTCAN_IOCTL_TX_OBJ_AUTOANSWER_ON, &pCtx->txmsg[0]);
      if (ret != NTCAN_SUCCESS) {
        printf("NTCAN_IOCTL_TX_OBJ_AUTOANSWER_ON failed with :%s\n",
               get_error_str(str_buf,ret));
        break;
      }
      ret = canIoctl(h0, NTCAN_IOCTL_TX_OBJ_UPDATE, &pCtx->txmsg[0]);
      if (ret != NTCAN_SUCCESS) {
        printf("NTCAN_IOCTL_TX_OBJ_UPDATE failed with :%s\n",
               get_error_str(str_buf,ret));
        break;
      }
      SLEEP(rxtout);
      ret = canIoctl(h0, NTCAN_IOCTL_TX_OBJ_AUTOANSWER_OFF, &pCtx->txmsg[0]);
      if (ret != NTCAN_SUCCESS) {
        printf("NTCAN_IOCTL_TX_OBJ_AUTOANSWER_OFF failed with :%s\n",
               get_error_str(str_buf,ret));
        break;
      }
      SLEEP(rxtout);
      ret = canIoctl(h0, NTCAN_IOCTL_TX_OBJ_DESTROY, &pCtx->txmsg[0]);
      if (ret != NTCAN_SUCCESS) {
        printf("NTCAN_IOCTL_TX_OBJ_DESTROY failed with :%s\n",
               get_error_str(str_buf,ret));
        break;
      }
      break;

# endif /* of NTCAN_IOCTL_TX_OBJ_SCHEDULE */

    case 19:
      pCtx->mode |= NO_ASCII_OUTPUT;
    case 9:
      /*
       * Send RTR of len 0 and keep system time
       */
      len = 1;
      pCtx->txmsg->len |= NTCAN_RTR; /* rtr */
      start_time = GET_CURRENT_TIME;
      ret = canSend(h0, pCtx->txmsg, &len);
      if (ret != NTCAN_SUCCESS) {
        printf("canSend returned: %s\n", get_error_str(str_buf,ret));
        break;
      }
      /*
       * Wait for reply to this remote request and keep system time
       */
      time_1 = GET_CURRENT_TIME;
      len = maxcount;
      ret = canRead(h0, pCtx->rxmsg, &len, NULL);
      stop_time = GET_CURRENT_TIME;
      /*
       * Print durations and reply
       */
      if (!(pCtx->mode & NO_ASCII_OUTPUT)) {
        printf("Duration=%9lu %s Duration2=%9lu %s Can-Messages=%ld \n",
               (unsigned long)(time_1-start_time), TIME_UNITS,
               (unsigned long)(stop_time-time_1), TIME_UNITS, (long)len);
      }
      if (ret == NTCAN_SUCCESS) {
        if (!(pCtx->mode & NO_ASCII_OUTPUT)) {
          for (i = 0; i < len; i++) {
              print_cmsg(&pCtx->rxmsg[i], pCtx);
          }
        }
      } else {
        printf("canRead returned: %s\n", get_error_str(str_buf,ret));
      }
      break;
#endif /* of !D3X */

#ifdef NTCAN_IOCTL_TX_OBJ_SCHEDULE
#ifdef NTCAN_FD
    case 110:
#endif
    case 100:
      for(j=0; j < maxcount; j ++) {
#ifdef NTCAN_FD
        CMSG_X *cmx = &pCtx->txmsg_x[j];
#endif
        CMSG *cm = &pCtx->txmsg[j];
        CSCHED sched;

#ifdef NTCAN_FD
        cmx->id = j+idstart;
#endif
        cm->id = j+idstart;

#ifdef NTCAN_FD
        if(test == 110)
        { ret = canIoctl(h0, NTCAN_IOCTL_TX_OBJ_CREATE_X, cmx); }
        else
#endif
        { ret = canIoctl(h0, NTCAN_IOCTL_TX_OBJ_CREATE, cm); }
        if( 0 != ret) {
            printf("NTCAN_IOCTL_TX_OBJ_CREATE returned 0x%x!\n",
                   (unsigned int)ret);
        }

        if ((j & 1)==0) {
          sched.id = cm->id;
          sched.flags = NTCAN_SCHED_FLAG_INC16 | NTCAN_SCHED_FLAG_OFS6;
          sched.time_start= pCtx->udtTimestampFreq * (1+j);
          sched.time_interval = pCtx->udtTimestampFreq / 1000 * (j+4);
          sched.count_start = 0x1;
          sched.count_stop = 0x1234;

          ret = canIoctl(h0, NTCAN_IOCTL_TX_OBJ_SCHEDULE, &sched);
          if(0 != ret) {
            printf("NTCAN_IOCTL_TX_OBJ_SCHEDULE returned 0x%x!\n",
                   (unsigned int)ret);
          }
        } else {
          ret = canIoctl(h0, NTCAN_IOCTL_TX_OBJ_AUTOANSWER_ON, cm);
          if(0 != ret) {
            printf("NTCAN_IOCTL_TX_OBJ_AUTOANSWER_ON returned 0x%x!\n",
                   (unsigned int)ret);
          }
        }
      }

      ret = canIoctl(h0, NTCAN_IOCTL_TX_OBJ_SCHEDULE_START, NULL);
      printf("NTCAN_IOCTL_TX_OBJ_SCHED_START returned 0x%x!\n", (unsigned int)ret);

      for(k=0; k < testcount || ever; k ++) {
        CSCHED sched;
        SLEEP(1000);
        for(j=0; j < maxcount; j ++) {
          CMSG *cm = &pCtx->txmsg[j];
#ifdef NTCAN_FD
          CMSG_X *cmx = &pCtx->txmsg_x[j];
#endif
          if ( ( k == 64  || k == 128) && (j & 1)==0 ) {
            if( (j & 3) == 0) {
              sched.id = cm->id;

              if( k == 64 ) {
                sched.flags = NTCAN_SCHED_FLAG_DIS;
              } else {
                sched.flags = NTCAN_SCHED_FLAG_EN;
              }

              ret = canIoctl(h0, NTCAN_IOCTL_TX_OBJ_SCHEDULE, &sched);
              if( 0 != ret) {
                printf("NTCAN_IOCTL_TX_OBJ_SCHEDULE returned 0x%x!\n", (unsigned int)ret);
              } else {
                printf("NTCAN_IOCTL_TX_OBJ_SCHEDULE ID=0x%03lx Flags=0x%08lx!\n",
                       (long)sched.id, (long)sched.flags );
              }
            }
          }

#ifndef NTCAN_FD
          *((int32_t *)&cm->data[0]) +=(j+1);
#else
          *((int32_t *)&cmx->data[0]) +=(j+1);
          if(test == 110)
          { ret = canIoctl(h0, NTCAN_IOCTL_TX_OBJ_UPDATE_X, cmx); }
          else
#endif
          { ret = canIoctl(h0, NTCAN_IOCTL_TX_OBJ_UPDATE, cm); }
          if( 0 != ret) {
            printf("NTCAN_IOCTL_TX_OBJ_UPDATE returned 0x%x!\n", (unsigned int)ret);
          }
        }
      }

      ret = canIoctl(h0, NTCAN_IOCTL_TX_OBJ_SCHEDULE_STOP, NULL);
      printf("NTCAN_IOCTL_TX_OBJ_SCHED_STOP returned 0x%x\n", (unsigned int)ret);


      SLEEP(1000);
      testcount = 0;
      ever = 0;
      break;
#endif /* of NTCAN_IOCTL_TX_OBJ_SCHEDULE */


    default:
      printf("Undefined Test!\n");
      ret=canClose(h0);
      if (ret != NTCAN_SUCCESS) {
        printf("canClose returned: %s\n", get_error_str(str_buf,ret));
      }
      CANTEST_FREE(pCtx);
      return(-1);
    } /* of switch */

    /* If we return with NTCAN_NO_ID_ENABLED stop testing */
    if(NTCAN_NO_ID_ENABLED == ret)
      break;
  } /* of for */

  stop_test_time = GET_CURRENT_TIME;

  /*
   * To prevent that pending Tx messages are discarded by the driver if
   * a non-blocking test is executed and the driver does not support a
   * delayed close we wait the configured Tx timeout before closing the handle
   */
  if((0 == test) && (NTCAN_SUCCESS == ret)) {
      SLEEP(txtout);
  }

#ifdef NTCAN_IOCTL_SET_BUSLOAD_INTERVAL
  /* Stop the busload event */
  if(bl_event_period != 0) {
    bl_event_period = 0;
    ret = canIoctl(h0, NTCAN_IOCTL_SET_BUSLOAD_INTERVAL, &bl_event_period);
    if (ret != NTCAN_SUCCESS) {
      printf("NTCAN_IOCTL_SET_BUSLOAD_INTERVAL failed with: %s\n",
             get_error_str(str_buf, ret));
    }
  }
#endif

  /*
   * Close the CAN handle
   */
  ret = canClose(h0);
  if (ret != NTCAN_SUCCESS) {
    printf("canClose returned: %s\n", get_error_str(str_buf,ret));
  }

  printf("Test-Duration=%lu %s \n",
         (unsigned long)(stop_test_time-start_test_time), TIME_UNITS);

#if defined(_WIN32) && !defined(_WIN32_WCE) && !defined(UNDER_RTSS)

  /* Close the event handle of Win32 overlapped tests */
  if(hEvent != NULL)
      CloseHandle(hEvent);

#endif

  /* Free allocated memory */
  CANTEST_FREE(pCtx);

  return((NTCAN_SUCCESS == ret) ? 0 : -1);
}

/************************************************************************/
/************************************************************************/
/* Function: help()                                                     */
/* Print usage and number of available CAN devices                      */
/************************************************************************/
/************************************************************************/
#ifndef D3X
static void help(int testnr)
{
  NTCAN_RESULT  ret;
  int           i;
  int           fwUpdateRequired = 0;
  NTCAN_HANDLE  h0;
  CAN_IF_STATUS cstat;
  int8_t        str_buf[100];
  uint32_t      baudrate;
  uint32_t      features;
  uint32_t      ulSerial = 0;
  char          cBuffer[16];
  char          boardid[48];
  uint16_t      fw2 = 0;

#ifdef NTCAN_IOCTL_GET_TIMESTAMP
  uint64_t udtTimestampFreq;
  uint64_t ullLastTime;
#endif /* ifdef NTCAN_IOCTL_GET_TIMESTAMP */

#ifdef NTCAN_IOCTL_GET_BITRATE_DETAILS
  NTCAN_BITRATE bitrate;
#endif /* ifdef NTCAN_IOCTL_GET_BITRATE_DETAILS */

#ifdef NTCAN_IOCTL_GET_INFO
  NTCAN_INFO info;
#endif /* ifdef NTCAN_IOCTL_GET_INFO */

  printf("CAN Test Rev %d.%d.%d  -- (c) 1997-2015 esd electronic system design gmbh\n\n",
         LEVEL, REVISION, CHANGE);
  printf("Available CAN-Devices: \n");
  for (i = 0; i <= NTCAN_MAX_NETS; i++) {
    ret = canOpen(i, 0, 1, 1, 0, 0, &h0);
    if (ret == NTCAN_SUCCESS) {
      ret = canStatus(h0, &cstat);
      if (ret != NTCAN_SUCCESS) {
        printf("Cannot get Status of Net-Device %02X (ret = 0x%x)\n",
               i, (unsigned int)ret);
      } else {

        features = (uint32_t)cstat.features;
        strncpy(boardid, (const char *)cstat.boardid, sizeof(boardid));
#ifdef NTCAN_IOCTL_GET_INFO
        memset(&info, 0, sizeof(info));
        ret = canIoctl(h0, NTCAN_IOCTL_GET_INFO, &info);
        if(NTCAN_SUCCESS == ret) {
            features = info.features;
            fw2      = info.firmware2;
            sprintf(boardid, "%s (%d ports)", cstat.boardid, info.ports);
        }
#endif

        baudrate = NTCAN_NO_BAUDRATE;
        ret = canGetBaudrate(h0, &baudrate);
        if(NTCAN_INVALID_FIRMWARE == ret) {
            fwUpdateRequired = 1;
            ret = NTCAN_SUCCESS;
        }
        if (ret != NTCAN_SUCCESS) {
          printf("Cannot get Baudrate of Net-Device %02X (ret: 0x%x)\n",
                  i, (unsigned int)ret);
        } else {
#ifdef NTCAN_IOCTL_GET_BITRATE_DETAILS
            bitrate.valid = 0;
            ret = canIoctl(h0, NTCAN_IOCTL_GET_BITRATE_DETAILS, &bitrate);
            if (ret != NTCAN_SUCCESS) {
                bitrate.valid = NTCAN_INVALID_PARAMETER;
            }
#endif /* ifdef NTCAN_IOCTL_GET_BITRATE_DETAILS */

            /*
             * Get the serial number.
             */
#ifdef NTCAN_IOCTL_GET_SERIAL
            ret = canIoctl(h0, NTCAN_IOCTL_GET_SERIAL, &ulSerial);
            if(ret != NTCAN_SUCCESS) {
                ulSerial = 0;
            }
#endif /* ifdef NTCAN_IOCTL_GET_SERIAL */

            if(0 == ulSerial) {
                sprintf(cBuffer,"N/A");
            } else {
                sprintf(cBuffer, "%c%c%06ld",
                    (char)('A' + (ulSerial >> 28 & 0xF)),
                    (char)('A' + (ulSerial >> 24 & 0xF)),
                    (long)(ulSerial & 0xFFFFFF));
            }
#ifdef NTCAN_IOCTL_GET_INFO
            if(info.serial_string[0] != '\0') {
                strncpy(cBuffer, info.serial_string, sizeof(cBuffer));
            }
#endif  /* of NTCAN_IOCTL_GET_INFO */

            printf("Net %3d: ID=%s Serial no.: %s\n"
                "         Versions (hex): Lib=%1X.%1X.%02X"
                " Drv=%1X.%1X.%02X"
                " HW=%1X.%1X.%02X"
                " FW=%1X.%1X.%02X (%1X.%1X.%02X)\n",
                i, boardid, cBuffer,
                cstat.dll     >>12, (cstat.dll     >>8) & 0xf, cstat.dll      & 0xff,
                cstat.driver  >>12, (cstat.driver  >>8) & 0xf, cstat.driver   & 0xff,
                cstat.hardware>>12, (cstat.hardware>>8) & 0xf, cstat.hardware & 0xff,
                cstat.firmware>>12, (cstat.firmware>>8) & 0xf, cstat.firmware & 0xff,
                fw2 >> 12, fw2 >> 8, fw2 & 0xff);
#if defined (NTCAN_IOCTL_GET_INFO)
            if (-3 == testnr) {
                if(info.drv_build_info[0] != '\0') {
                    printf("         Drv build: %s\n", info.drv_build_info);
                }
                if(info.lib_build_info[0] != '\0') {
                    printf("         Lib build: %s\n", info.lib_build_info);
                }
            }
#endif /* NTCAN_IOCTL_GET_INFO */

            printf("         Baudrate=%08lx", (unsigned long)baudrate);
#ifdef NTCAN_IOCTL_GET_BITRATE_DETAILS
          if(NTCAN_SUCCESS == bitrate.valid) {
              printf(" (%d KBit/s", (int)(bitrate.rate / 1000));
              if(bitrate.baud & NTCAN_LISTEN_ONLY_MODE) {
                  printf(", LOM");
              }
              printf(")");
          } else if(NTCAN_NO_BAUDRATE == bitrate.baud) {
              printf(" (Not set)");
          }
#endif /* ifdef NTCAN_IOCTL_GET_BITRATE_DETAILS */

          printf(" Status=%04x Features=%08x\n",
#ifdef NTCAN_GET_BOARD_STATUS
              (unsigned int)NTCAN_GET_BOARD_STATUS(cstat.boardstatus),
#else /* ifdef NTCAN_GET_BOARD_STATUS */
              (unsigned int)cstat.boardstatus,
#endif /* ifdef NTCAN_GET_BOARD_STATUS */
              (unsigned int)features);

        } /* of if (ret != NTCAN_SUCCESS) */

        /* Decode feature flags */
        if (-3 == testnr) {
            printf(
            "           %c CAN 2.0B support   %c Rx Object Mode    %c Timestamp support\n"
            "           %c Listen Only Mode   %c Smart Disconnect  %c Local Echo\n"
            "           %c Smart ID filter    %c Tx Scheduling     %c Enhanced Bus Diagnostic\n"
            "           %c Error Injection    %c IRIG-B Support    %c PXI Support\n"
            "           %c CAN-FD support     %c Self Test Mode",
            features & NTCAN_FEATURE_CAN_20B ? '+' : '-',
            features & NTCAN_FEATURE_RX_OBJECT_MODE ? '+' : '-',
            features & NTCAN_FEATURE_TIMESTAMP ? '+' : '-',
            features & NTCAN_FEATURE_LISTEN_ONLY_MODE ? '+' : '-',
            features & NTCAN_FEATURE_SMART_DISCONNECT ? '+' : '-',
            features & NTCAN_FEATURE_LOCAL_ECHO ? '+' : '-',
            features & NTCAN_FEATURE_SMART_ID_FILTER ? '+' : '-',
            features & NTCAN_FEATURE_SCHEDULING ? '+' : '-',
            features & NTCAN_FEATURE_DIAGNOSTIC ? '+' : '-',
            features & NTCAN_FEATURE_ERROR_INJECTION ? '+' : '-',
            features & NTCAN_FEATURE_IRIGB ? '+' : '-',
            features & NTCAN_FEATURE_PXI ? '+' : '-',
            features & NTCAN_FEATURE_CAN_FD ? '+' : '-',
            features & NTCAN_FEATURE_SELF_TEST ? '+' : '-');
            if(cstat.driver > 0x3000) {
              printf("    %c Timestamped Tx\n",
                     features & NTCAN_FEATURE_TIMESTAMPED_TX ? '+' : '-');
            } else {
              printf("    %c Cyclic Tx\n",
                     features & NTCAN_FEATURE_TIMESTAMPED_TX ? '+' : '-');
            }
       }

#ifdef NTCAN_GET_CTRL_TYPE
        printf("         Ctrl=");
        switch(NTCAN_GET_CTRL_TYPE(cstat.boardstatus)) {
            case NTCAN_CANCTL_SJA1000: printf("NXP SJA1000"); break;
            case NTCAN_CANCTL_I82527:  printf("Intel C527"); break;
            case NTCAN_CANCTL_FUJI:    printf("Fujitsu MBxxxxx MCU"); break;
            case NTCAN_CANCTL_LPC:     printf("NXP LPC2xxx MCU"); break;
            case NTCAN_CANCTL_MSCAN:   printf("Freescale MCU"); break;
            case NTCAN_CANCTL_ATSAM:   printf("Atmel ARM MCU"); break;
            case NTCAN_CANCTL_ESDACC:  printf("esd Advanced CAN Core"); break;
            case NTCAN_CANCTL_STM32:   printf("ST STM32Fxx MCU"); break;
            case NTCAN_CANCTL_CC770:   printf("Bosch CC770"); break;
            case NTCAN_CANCTL_SPEAR:   printf("ST SPEAr320 MCU"); break;
            case NTCAN_CANCTL_FLEXCAN: printf("Freescale iMX MCU"); break;
            case NTCAN_CANCTL_SITARA:  printf("TI AM335x (Sitara) MCU"); break;
            default:
                printf("Unknown (0x%02x)",
                       (unsigned int)NTCAN_GET_CTRL_TYPE(cstat.boardstatus));
        }
#ifdef NTCAN_IOCTL_GET_BITRATE_DETAILS
        if(bitrate.valid != NTCAN_INVALID_PARAMETER) {
            printf(" @ %d MHz", (int)(bitrate.clock / 1000000));
        }
#endif /* ifdef NTCAN_IOCTL_GET_BITRATE_DETAILS */
        if(features & NTCAN_FEATURE_FULL_CAN) {
            printf(" -- FullCAN");
        }

#ifdef NTCAN_IOCTL_GET_CTRL_STATUS
        {
            NTCAN_CTRL_STATE ctrl_state;

            ret = canIoctl(h0, NTCAN_IOCTL_GET_CTRL_STATUS, &ctrl_state);
            if (NTCAN_SUCCESS == ret) {
                printf(" (");
                switch(ctrl_state.status)
                {
                case 0x00:
                    printf("Error Active");
                    break;
                case 0x40:
                    printf("Error Warn");
                    break;
                case 0x80:
                    printf("Error Passive");
                    break;
                case 0xC0:
                    printf("Bus-Off");
                    break;
                default:
                    printf("Unknown");
                    break;
                }
                printf(" / REC:%d / TEC:%d)",
                    ctrl_state.rcv_err_counter,
                    ctrl_state.xmit_err_counter);
            }
        }
#endif /* ifdef NTCAN_IOCTL_GET_CTRL_STATUS */
        printf("\n");
#endif /* ifdef NTCAN_GET_CTRL_TYPE */

#if defined (NTCAN_IOCTL_GET_INFO)
        printf("         Transceiver=");
        switch(info.transceiver) {
        case NTCAN_TRX_PCA82C251:  printf("NXP PCA82C251"); break;
        case NTCAN_TRX_SN65HVD251: printf("TI SN65HVD251"); break;
        case NTCAN_TRX_SN65HVD265: printf("TI SN65HVD265"); break;
        default:
            printf("Unknown (0x%02x)", info.transceiver);
        }
        printf("\n");
#endif

#ifdef NTCAN_IOCTL_GET_TIMESTAMP
        /*
         * Print timestamp frequency and current timestamp if supported
         */
        if(0 == get_timestamp_freq(h0, &udtTimestampFreq, 0)) {
            ret = canIoctl(h0, NTCAN_IOCTL_GET_TIMESTAMP, &ullLastTime);
            printf("         TimestampFreq=%ld.%06ld MHz",
                   (unsigned long)(udtTimestampFreq / 1000000),
                   (unsigned long)(udtTimestampFreq % 1000000));
            if (NTCAN_SUCCESS == ret) {
                printf(" Timestamp=%08lX%08lX",
                       (unsigned long)((ullLastTime >> 32) & 0xFFFFFFFF),
                       (unsigned long)(ullLastTime & 0xFFFFFFFF));
            }
            printf("\n");
        }

        /* Indicate a required FW update */
        if(fwUpdateRequired != 0) {
            printf("\n         ----> !!!! THIS BOARD REQUIRES A FW UPDATE !!!! <----\n\n");
        }
#endif /* ifdef NTCAN_IOCTL_GET_TIMESTAMP */
      }
      canClose(h0);
    } else if(ret != NTCAN_NET_NOT_FOUND) { /* Error-code sanity check */
      printf("Net %3d: Opening device returned with error %s\n",
             i, get_error_str(str_buf,ret));
    }
  }
  if (testnr <= -4) {
      printf("\n");
      printf("  |======================================================================|\n");
      printf("  | Bitrate (KBit/s) | 1000 | 800 | 500 | 250 | 125 | 100 | 50 | 20 | 10 |\n");
      printf("  |------------------+------+-----+-----+-----+-----+-----+----+----+----|\n");
      printf("  | Index            |    0 |  14 |   2 |   4 |   6 |   7 |  9 | 11 | 13 |\n");
      printf("  |======================================================================|\n");
  }
  if (testnr <= -2) return;

#if defined(VXWORKS)
  printf("\nSyntax: canTest test-Nr "
#else /* defined(VXWORKS) */
  printf("\nSyntax: cantest test-Nr "
#endif /* defined(VXWORKS) */
         "[net id-1st id-last count\n"
         "        txbuf rxbuf txtout rxtout baud testcount data0 data1 ...]\n");
  printf("Test   0:  canSend()\n");
#if defined(NTCAN_IOCTL_GET_TX_TS_WIN)
  printf("Test  20:  canSendT()\n");
#endif /* defined(NTCAN_IOCTL_GET_TX_TS_WIN) */
  printf("Test  50:  canSend() with incrementing ids\n");
#if defined(NTCAN_FD)
  printf("Test  60:  canSendX()\n");
#endif /* defined(NTCAN_FD) */
  printf("Test   1:  canWrite()\n");
#if defined(NTCAN_IOCTL_GET_TX_TS_WIN)
  printf("Test  21:  canWriteT()\n");
#endif /* defined(NTCAN_IOCTL_GET_TX_TS_WIN) */
#ifdef RMOS
  printf("Test  21:  canWrite() fifo SLOW\n");
  printf("Test  31:  canWrite() fifo FAST\n");
  printf("Test  41:  canWrite() fifo BURST\n");
#endif /* ifdef RMOS */
  printf("Test  51:  canWrite() with incrementing ids\n");
#if defined(NTCAN_FD)
  printf("Test  61:  canWriteX()\n");
#endif /* defined(NTCAN_FD) */
  printf("Test   2:  canTake()\n");
  printf("Test  12:  canTake() with time-measurement for 10000 can-frames\n");
#ifdef NTCAN_IOCTL_GET_TIMESTAMP
  printf("Test  22:  canTakeT()\n");
#endif /* ifdef NTCAN_IOCTL_GET_TIMESTAMP */
  printf("Test  32:  canTake() in Object-Mode\n");
#ifdef NTCAN_IOCTL_GET_TIMESTAMP
  printf("Test  42:  canTakeT() in Object-Mode\n");
#endif /* ifdef NTCAN_IOCTL_GET_TIMESTAMP */
#if defined(NTCAN_FD)
  printf("Test  62:  canTakeX()\n");
  printf("Test  72:  canTakeX() with time-measurement for 10000 can-frames\n");
  printf("Test  82:  canTakeX() in Object-Mode\n");
#endif /* defined(NTCAN_FD) */
  printf("Test   3:  canRead()\n");
  printf("Test  13:  canRead() with time-measurement for 10000 can-frames\n");
#ifdef NTCAN_IOCTL_GET_TIMESTAMP
  printf("Test  23:  canReadT()\n");
#endif /* ifdef NTCAN_IOCTL_GET_TIMESTAMP */
#if defined(NTCAN_FD)
  printf("Test  63:  canReadX()\n");
  printf("Test  73:  canReadX() with time-measurement for 10000 can-frames\n");
#endif /* defined(NTCAN_FD) */
  printf("Test   4:  canReadEvent()\n");
#if defined(NTCAN_IOCTL_GET_BUS_STATISTIC) && !defined(D3X)
  printf("Test  64:  Retrieve bus statistics (every tx timeout)\n");
  printf("Test  74:  Reset bus statistics\n");
#endif /* if defined(NTCAN_IOCTL_GET_BUS_STATISTIC) && !defined(D3X) */
#if defined(NTCAN_IOCTL_GET_BITRATE_DETAILS) && !defined(D3X)
  printf("Test  84:  Retrieve bitrate details (every tx timeout)\n");
#endif /* if defined(NTCAN_IOCTL_GET_BITRATE_DETAILS) && !defined(D3X) */
  printf("Test   5:  canSendEvent()\n");
#if defined(_WIN32) && !defined(_WIN32_WCE) && !defined(UNDER_RTSS)
  printf("Test   6:  Overlapped-canRead()\n");
# if defined(NTCAN_IOCTL_GET_TIMESTAMP) && !defined(D3X)
  printf("Test  16:  Overlapped-canReadT()\n");
#  if defined(NTCAN_FD)
  printf("Test  66:  Overlapped-canReadX()\n");
#  endif
# endif /* if defined(NTCAN_IOCTL_GET_TIMESTAMP) && !defined(D3X) */
  printf("Test   7:  Overlapped-canWrite()\n");
#endif /* if defined(_WIN32) && !defined(_WIN32_WCE) && !defined(UNDER_RTSS) */
#if defined(unix) && defined(D3X)
  printf("Test   6:  Async canRead()\n");
#endif /* if defined(unix) && defined(D3X) */
#ifndef D3X
# ifdef NTCAN_IOCTL_TX_OBJ_SCHEDULE
  printf("Test   8:  Create auto RTR object\n");
# endif /* ifdef NTCAN_IOCTL_TX_OBJ_SCHEDULE */
  printf("Test   9:  Wait for RTR reply\n");
  printf("Test  19:  Wait for RTR reply without text-output\n");
# ifdef NTCAN_IOCTL_TX_OBJ_SCHEDULE
  printf("Test 100:  Object Scheduling test\n");
#  if defined(NTCAN_FD)
  printf("Test 110:  Object Scheduling test with cmsg_x\n");
#  endif
# endif
#endif /* ifndef D3X */
  printf("Test  -2:  Overview without syntax help\n");
  printf("Test  -3:  Overview without syntax help but with feature flags details\n");
}
#else
static void help(int testnr)
{
  NTCAN_RESULT  ret;
  int           i;
  NTCAN_HANDLE  h0;
  CAN_IF_STATUS cstat;
  int8_t        str_buf[100];

  printf("CAN Test Rev %d.%d.%d  -- (c) 1997-2013 esd electronic system design gmbh\n\n",
         LEVEL, REVISION, CHANGE);
  printf("Available CAN-Devices: \n");
  for (i = 0; i <= NTCAN_MAX_NETS; i++) {
    ret = canOpen(i, 0, 1, 1, 0, 0, &h0);
    if (ret == NTCAN_SUCCESS) {
      ret = canStatus(h0, &cstat);
      if (ret != NTCAN_SUCCESS) {
        printf("Cannot get Status of Net-Device %02X (ret = %d)\n",
               i, ret);
      } else {
          printf("Net %3d: ID=%s\n"
                 "         Versions (hex): Dll=%1X.%1X.%02X "
                 " Drv=%1X.%1X.%02X"
                 " FW=%1X.%1X.%02X"
                 " HW=%1X.%1X.%02X\n"
                 "         Status=%08x\n",
                 i, cstat.boardid,
                 cstat.dll     >>12, (cstat.dll     >>8) & 0xf, cstat.dll      & 0xff,
                 cstat.driver  >>12, (cstat.driver  >>8) & 0xf, cstat.driver   & 0xff,
                 cstat.firmware>>12, (cstat.firmware>>8) & 0xf, cstat.firmware & 0xff,
                 cstat.hardware>>12, (cstat.hardware>>8) & 0xf, cstat.hardware & 0xff,
                 (unsigned long)cstat.boardstatus);
      }
      canClose(h0);
    } else if(ret != NTCAN_NET_NOT_FOUND) { /* Error-code sanity check */
      printf("Net %3d: Opening device returned with error %s\n",
             i, get_error_str(str_buf,ret));
    }
  }
  if (testnr <= -2) return;

  printf("\nSyntax: d3xtest test-Nr "
         "[net id-1st id-last count\n"
         "        txbuf rxbuf txtout rxtout baud testcount data0 data1 ...]\n");
  printf("Test  1:  canWrite()\n");
  printf("Test  3:  canRead()\n");
  printf("Test  4:  canReadEvent()\n");
  printf("Test  5:  canSendEvent()\n");
#if defined(_WIN32) && !defined(_WIN32_WCE) && !defined(UNDER_RTSS)
  printf("Test  6:  Overlapped-canRead()\n");
  printf("Test  7:  Overlapped-canWrite()\n");
#endif /* of _WIN32 && !_WIN32_WCE && !UNDER_RTSS */
#if defined(unix)
  printf("Test  6:  Async canRead()\n");
#endif /* of unix */
}
#endif /* of ! defined D3X */

/************************************************************************/
/************************************************************************/
/* Function: set_can_id()                                               */
/* Boilerplate code to enable a single ID of the CAN handle filter      */
/************************************************************************/
/************************************************************************/
static NTCAN_RESULT set_can_id(NTCAN_HANDLE handle, int32_t id)
{
    int          i;
    NTCAN_RESULT ret = NTCAN_SUCCESS;

    /*
    * For some active CAN hardware old driver implementations might return
    * NTCAN_INSUFFICIENT_RESOURCES if the communication FIFO of the device
    * is full and can not accept further requests because the host performs
    * consecutive calls of canIdAdd()/canIdDelete much faster than it can be
    * processed by the CAN hardware.
    *
    * All current driver can handle this situation internally more efficient
    * so this boileplate code around canIdAdd(), which waits several ms in this
    * situation, isn't usually necessary in a user's application.
    */
    for (i = 0; i < 2; i++) {
        ret = canIdAdd(handle, id);
        if (NTCAN_INSUFFICIENT_RESOURCES == ret) {
            SLEEP(100);
            continue;
        }
        break;
    }

    return ret;
}

/************************************************************************/
/************************************************************************/
/* Function: get_timestamp_freq()                                       */
/* Boilerplate code to check if driver/device supports timestamps and   */
/* to request the frequency of the timestamp counter.                   */
/* Returns 0 on success -1 otherwise                                    */
/************************************************************************/
/************************************************************************/
#ifdef NTCAN_IOCTL_GET_TIMESTAMP
static int get_timestamp_freq(NTCAN_HANDLE handle, uint64_t *pFreq, int verbose)
{
    CAN_IF_STATUS cstat;
    NTCAN_RESULT  ret;
    int8_t        str_buf[100];

    /* Check for timestamp support */
    ret = canStatus(handle, &cstat);
    if ((ret != NTCAN_SUCCESS) ||
        (0 == (cstat.features & NTCAN_FEATURE_TIMESTAMP))) {
            if(verbose != 0)
                printf("Driver/Hardware does not support timestamps\n");
            return(-1);
    }

    /* Read timestamp frequency */
    ret = canIoctl(handle, NTCAN_IOCTL_GET_TIMESTAMP_FREQ, pFreq);
    if (ret != NTCAN_SUCCESS) {
        if(verbose != 0)
            printf("canIoctl returned: %s\n", get_error_str(str_buf, ret));
        return(-1);
    }

    if(verbose != 0)
        printf("TimestampFreq=%" PRId64 " Hz\n", *pFreq);

    return(0);
}
#endif /* of  NTCAN_IOCTL_GET_TIMESTAMP */

/************************************************************************/
/************************************************************************/
/* Function: get_error_str()                                            */
/* Return ASCII representation of NTCAN return code                     */
/************************************************************************/
/************************************************************************/
static int8_t *get_error_str(int8_t *str_buf, NTCAN_RESULT ntstatus)
{
  struct ERR2STR {
    NTCAN_RESULT  ntstatus;
    const char   *str;
  };

  static const struct ERR2STR err2str[] = {
    { NTCAN_SUCCESS            , "NTCAN_SUCCESS"            },
    { NTCAN_RX_TIMEOUT         , "NTCAN_RX_TIMEOUT"         },
    { NTCAN_TX_TIMEOUT         , "NTCAN_TX_TIMEOUT"         },
    { NTCAN_TX_ERROR           , "NTCAN_TX_ERROR"           },
    { NTCAN_CONTR_OFF_BUS      , "NTCAN_CONTR_OFF_BUS"      },
    { NTCAN_CONTR_BUSY         , "NTCAN_CONTR_BUSY"         },
    { NTCAN_CONTR_WARN         , "NTCAN_CONTR_WARN"         },
    { NTCAN_NO_ID_ENABLED      , "NTCAN_NO_ID_ENABLED"      },
    { NTCAN_ID_ALREADY_ENABLED , "NTCAN_ID_ALREADY_ENABLED" },
    { NTCAN_ID_NOT_ENABLED     , "NTCAN_ID_NOT_ENABLED"     },
    { NTCAN_INVALID_FIRMWARE   , "NTCAN_INVALID_FIRMWARE"   },
    { NTCAN_MESSAGE_LOST       , "NTCAN_MESSAGE_LOST"       },
    { NTCAN_INVALID_PARAMETER  , "NTCAN_INVALID_PARAMETER"  },
    { NTCAN_INVALID_HANDLE     , "NTCAN_INVALID_HANDLE"     },
    { NTCAN_NET_NOT_FOUND      , "NTCAN_NET_NOT_FOUND"      },
#ifdef NTCAN_IO_INCOMPLETE
    { NTCAN_IO_INCOMPLETE      , "NTCAN_IO_INCOMPLETE"      },
#endif
#ifdef NTCAN_IO_PENDING
    { NTCAN_IO_PENDING         , "NTCAN_IO_PENDING"         },
#endif
#ifdef NTCAN_INVALID_HARDWARE
    { NTCAN_INVALID_HARDWARE   , "NTCAN_INVALID_HARDWARE"   },
#endif
#ifdef NTCAN_PENDING_WRITE
    { NTCAN_PENDING_WRITE      , "NTCAN_PENDING_WRITE"      },
#endif
#ifdef NTCAN_PENDING_READ
    { NTCAN_PENDING_READ       , "NTCAN_PENDING_READ"       },
#endif
#ifdef NTCAN_INVALID_DRIVER
    { NTCAN_INVALID_DRIVER     , "NTCAN_INVALID_DRIVER"     },
#endif
#ifdef NTCAN_OPERATION_ABORTED
    { NTCAN_OPERATION_ABORTED  , "NTCAN_OPERATION_ABORTED"  },
#endif
#ifdef NTCAN_WRONG_DEVICE_STATE
    { NTCAN_WRONG_DEVICE_STATE , "NTCAN_WRONG_DEVICE_STATE"  },
#endif
    { NTCAN_INSUFFICIENT_RESOURCES, "NTCAN_INSUFFICIENT_RESOURCES"},
#ifdef NTCAN_HANDLE_FORCED_CLOSE
    { NTCAN_HANDLE_FORCED_CLOSE, "NTCAN_HANDLE_FORCED_CLOSE"  },
#endif
#ifdef NTCAN_NOT_IMPLEMENTED
    { NTCAN_NOT_IMPLEMENTED    , "NTCAN_NOT_IMPLEMENTED"  },
#endif
#ifdef NTCAN_NOT_SUPPORTED
    { NTCAN_NOT_SUPPORTED      , "NTCAN_NOT_SUPPORTED"  },
#endif
#ifdef NTCAN_SOCK_CONN_TIMEOUT
    { NTCAN_SOCK_CONN_TIMEOUT  , "NTCAN_SOCK_CONN_TIMEOUT"     },
#endif
#ifdef NTCAN_SOCK_CMD_TIMEOUT
    { NTCAN_SOCK_CMD_TIMEOUT   , "NTCAN_SOCK_CMD_TIMEOUT"      },
#endif
#ifdef NTCAN_SOCK_HOST_NOT_FOUND
    { NTCAN_SOCK_HOST_NOT_FOUND, "NTCAN_SOCK_HOST_NOT_FOUND"   },
#endif
#ifdef NTCAN_CONTR_ERR_PASSIVE
    { NTCAN_CONTR_ERR_PASSIVE  , "NTCAN_CONTR_ERR_PASSIVE"   },
#endif
#ifdef NTCAN_ERROR_NO_BAUDRATE
    { NTCAN_ERROR_NO_BAUDRATE  , "NTCAN_ERROR_NO_BAUDRATE"   },
#endif
#ifdef NTCAN_ERROR_LOM
    { NTCAN_ERROR_LOM          , "NTCAN_ERROR_LOM"   },
#endif
    { (NTCAN_RESULT)0xffffffff , "NTCAN_UNKNOWN"               }    /* stop-mark */
  };

  const struct ERR2STR *es = err2str;

  do {
    if (es->ntstatus == ntstatus) {
      break;
    }
    es++;
  }
  while((uint32_t)es->ntstatus != 0xffffffff);

#ifdef NTCAN_ERROR_FORMAT_LONG
  {
      NTCAN_RESULT res;
      char szErrorText[60];

      res = canFormatError(ntstatus, NTCAN_ERROR_FORMAT_LONG, szErrorText,
                           sizeof(szErrorText) - 1);
      if(NTCAN_SUCCESS == res) {
          sprintf((char *)str_buf, "%s - %s", es->str, szErrorText);
      } else {
          sprintf((char *)str_buf, "%s(0x%08x)", es->str, (unsigned int)ntstatus);
      }
  }
#else

  sprintf((char *)str_buf, "%s(0x%08x)", es->str, ntstatus);

#endif  /* of NTCAN_ERROR_FORMAT_LONG */


  return str_buf;
}

/************************************************************************/
/************************************************************************/
/* Function: print_event()                                              */
/* Print interpreted version of a CAN event                             */
/************************************************************************/
/************************************************************************/
static void print_event(EVMSG *e, uint64_t ts, CANTEST_CTX *pCtx)
{
    if(NTCAN_EV_CAN_ERROR == e->evid) {
        printf("   CAN controller state: ");
        switch(e->evdata.error.can_status) {
            case 0x00:
                printf("OK");
                break;
            case 0x40:
                printf("WARN");
                break;
            case 0x80:
                printf("ERROR PASSIVE");
                break;
            case 0xC0:
                printf("BUS-OFF");
                break;
            default:
                printf("UNKNOWN State ?!?");
                break;
        }
        printf(" - Lost messages (Ctrl: %d, Driver: %d)\n",
            e->evdata.error.ctrl_overrun, e->evdata.error.fifo_overrun);
    }
#if defined NTCAN_EV_BAUD_CHANGE
    else if (e->evid == NTCAN_EV_BAUD_CHANGE) {
        if(NTCAN_NO_BAUDRATE == (uint32_t)e->evdata.baud_change.baud) {
            printf("  CAN controller removed from bus\n");
        }
#ifdef NTCAN_AUTOBAUD
        else if(NTCAN_AUTOBAUD == (uint32_t)e->evdata.baud_change.baud) {
            printf("  CAN controller changes in auto baudrate detection\n");
        }
#endif
        else {
            printf("  New baudrate : 0x%08lX", (unsigned long)e->evdata.baud_change.baud);
            if ( 4 < e->len ) {
                printf(" (%ld baud)", (unsigned long)e->evdata.baud_change.num_baud);
                num_baudrate = e->evdata.baud_change.num_baud;
            }
#ifdef NTCAN_LISTEN_ONLY_MODE
            if(((uint32_t)e->evdata.baud_change.baud & NTCAN_LISTEN_ONLY_MODE) != 0) {
                printf(" Listen only enabled");
            } else {
                printf(" Listen only disabled");
            }
#endif
            printf("\n");
        }
    }
# if defined NTCAN_EV_BUSLOAD && defined NTCAN_IOCTL_GET_TIMESTAMP
    else if (NTCAN_EV_BUSLOAD == e->evid) {
        static EVMSG e_last;
        static uint64_t last_bl_ts;  /* Timestamp of last busload event */
        if (0 == ts) {
            printf("  Busload-Error! ts?\n");
        } else if (0 == last_bl_ts) {
            printf("  First Busload event\n");
        } else if (0 == num_baudrate) {
            printf("  Busload-Error! baud?\n");
        } else if (((int64_t)(ts-last_bl_ts))<=0) {
            /* prevent division by zero */
            printf("  Busload-Error! ts_last>=ts");
        } else {
            unsigned int can_load;
            uint64_t     dbits;

            dbits     = e->evdata.q;
            dbits    -= e_last.evdata.q;
            dbits    *= pCtx->udtTimestampFreq;
            dbits    *= 100; /* for percentage */
            can_load  = (unsigned int)(dbits/(ts-last_bl_ts));
            can_load /= num_baudrate;

            printf("  Busload %u%%\n", (can_load > 100) ? 100 : can_load);
        }
        last_bl_ts=ts;
        e_last=*e;
    }
# endif /* of NTCAN_EV_BUSLOAD */
#endif /* of NTCAN_EV_BAUD_CHANGE */

#if defined NTCAN_EV_CAN_ERROR_EXT && defined NTCAN_FORMATEVENT_SHORT
    else if(NTCAN_EV_CAN_ERROR_EXT == e->evid) {
        NTCAN_RESULT result = NTCAN_NOT_IMPLEMENTED;
        NTCAN_FORMATEVENT_PARAMS ev;
        char buffer[80];
        memset(&ev, 0, sizeof(ev));
        ev.ctrl_type = pCtx->ctrl_type;
        result = canFormatEvent(e,&ev,buffer,sizeof(buffer));
        if(NTCAN_SUCCESS == result){
            printf("  %s\n", buffer);
        }
    }
#endif /* of NTCAN_EV_CAN_ERROR_EXT */
    return;
}

/************************************************************************/
/************************************************************************/
/* Function: print_cmsg()                                               */
/* Print a formatted CAN message to stdout if NO_ASCII_OUTPUT flag is   */
/* not set                                                              */
/************************************************************************/
/************************************************************************/
static void print_cmsg(CMSG *pCmsg, CANTEST_CTX *pCtx)
{
    int j, c;

    if(pCtx->mode & NTCAN_MODE_OBJECT) {
        if (pCmsg->len & NTCAN_NO_DATA) {
            printf("RX-ID=%9ld (0x%08lx) NO DATA\n",
                (unsigned long)pCmsg->id, (unsigned long)pCmsg->id);
        } else if (pCmsg->len & NTCAN_RTR) {
            printf("RX-RTR-ID=%9ld (0x%08lx) len=%02X\n",
                (unsigned long)pCmsg->id, (unsigned long)pCmsg->id, pCmsg->len & 0x0f );
        } else {
            pCmsg->len &= 0x0f;
            printf("RX-ID=%9ld (0x%08lx) len=%02X data= ",
                (unsigned long)pCmsg->id, (unsigned long)pCmsg->id, pCmsg->len );
            if(pCmsg->len > 8)
                pCmsg->len = 8;
            for (j = 0; j < pCmsg->len; j++) {
                printf("%02X ", pCmsg->data[j]);
            }
            for (j = pCmsg->len; j < 8; j++) {
                printf("   ");
            }
            printf("  [");
            {
                for (j = 0; j < pCmsg->len; j++) {
                    c = pCmsg->data[j];
                    printf("%c", c > 31 && c < 128 ? c : '.' );
                }
                for (j = pCmsg->len; j < 8; j++) {
                    printf(" ");
                }
            }
            printf("]\n");
        }
    } else {

        /* Check for lost messages */
        if (pCmsg->msg_lost != 0) {
            printf("%02x Messages lost !\n", pCmsg->msg_lost);
        }

        /* Return if console output is disabled */
        if ((pCtx->mode & NO_ASCII_OUTPUT))
            return;

        /* Mark interaction messages */
        if (pCmsg->len & NTCAN_INTERACTION) {
            printf("*");
        }

        if (pCmsg->len & NTCAN_RTR) {
            printf("RX-RTR-ID=%9ld (0x%08lx) len=%02X\n",
                (unsigned long)pCmsg->id, (unsigned long)pCmsg->id, pCmsg->len & 0x0f );
        } else {
            pCmsg->len &= 0x0f;
            printf("RX-ID=%9ld (0x%08lx) len=%02X data= ",
                (unsigned long)pCmsg->id, (unsigned long)pCmsg->id, pCmsg->len );
            if(pCmsg->len > 8)
                pCmsg->len = 8;
            for (j = 0; j < pCmsg->len; j++) {
                printf("%02X ", pCmsg->data[j]);
            }
            for (j = pCmsg->len; j < 8; j++) {
                printf("   ");
            }
            printf("  [");
            {
                for (j = 0; j < pCmsg->len; j++) {
                    c = pCmsg->data[j];
                    printf("%c", c > 31 && c < 128 ? c : '.' );
                }
                for (j = pCmsg->len; j < 8; j++) {
                    printf(" ");
                }
            }
            printf("]\n");
        }

        /* Decode events */
        if((pCmsg->id & NTCAN_EV_BASE) != 0) {
            print_event((EVMSG *)pCmsg, 0, pCtx);
        }
    }

    return;
}

/************************************************************************/
/************************************************************************/
/* Function: print_cmsg_t()                                             */
/* Print a formatted timestamped CAN message to stdout if               */
/*  NO_ASCII_OUTPUT flag is not set                                     */
/************************************************************************/
/************************************************************************/
#ifdef NTCAN_IOCTL_GET_TIMESTAMP
static void print_cmsg_t(CMSG_T *pCmsgT, CANTEST_CTX *pCtx)
{
    int       j;
    uint32_t  ulTimeDiff = 0;

    if(pCtx->mode & NTCAN_MODE_OBJECT) {
        pCtx->ullLastTime = pCmsgT->timestamp;

        if (pCmsgT->len & NTCAN_NO_DATA) {
            printf("RX-ID= %4ld (0x%03lx) - %016" PRIx64 " - NO DATA\n",
                (unsigned long)pCmsgT->id, (unsigned long)pCmsgT->id,
                pCtx->ullLastTime);
        } else if (pCmsgT->len & NTCAN_RTR) {
            printf("RTR-ID=%4ld (0x%03lx) - %016" PRIx64 " - len=%02X\n",
                (unsigned long)pCmsgT->id, (unsigned long)pCmsgT->id,
                pCtx->ullLastTime, pCmsgT->len & 0x0f);
        } else {
            pCmsgT->len &= 0x0f;

            printf("RX-ID= %4ld (0x%03lx) - %016" PRIx64 " - len=%02X data= ",
                (unsigned long)pCmsgT->id, (unsigned long)pCmsgT->id,
                pCtx->ullLastTime, pCmsgT->len);
            if(pCmsgT->len > 8)
                pCmsgT->len = 8;
            for (j = 0; j < pCmsgT->len; j++) {
                printf("%02X ", pCmsgT->data[j]);
            }
            printf("\n");
        }
    } else {
        /* Check for lost messages */
        if (pCmsgT->msg_lost != 0) {
            printf("%02x Messages lost !\n", pCmsgT->msg_lost);
        }

        /* Return if console output is disabled */
        if ((pCtx->mode & NO_ASCII_OUTPUT))
            return;

        /* Calculate time difference */
        if (pCtx->ullLastTime != 0) {
            ulTimeDiff = (uint32_t)((pCmsgT->timestamp - pCtx->ullLastTime) *
                INT64_C(1000000) / pCtx->udtTimestampFreq); /*us*/
        }
        pCtx->ullLastTime = pCmsgT->timestamp;

        /* Mark interaction messages */
        if (pCmsgT->len & NTCAN_INTERACTION) {
            printf("*");
        }

        if (pCmsgT->len & NTCAN_RTR) {
            printf("RTR-ID=%9ld (0x%08lx) - %05ld.%03ld - len=%02X\n",
                (unsigned long)pCmsgT->id, (unsigned long)pCmsgT->id,
                (unsigned long)(ulTimeDiff / 1000),
                (unsigned long)(ulTimeDiff % 1000),
                pCmsgT->len & 0x0f);
        } else {
            pCmsgT->len &= 0x0f;

#if 0
            if (ullLastTime != 0) {
                ulTimeDiff = (uint32_t)((pCmsgT->timestamp - ullLastTime) *
                    INT64_C(1000000) / udtTimestampFreq); /*us*/
            }
            ullLastTime = pCmsgT->timestamp;
#endif
            printf("RX-ID=%9ld (0x%08lx) - %05ld.%03ld - len=%02X data=",
                (unsigned long)pCmsgT->id, (unsigned long)pCmsgT->id, (unsigned long)(ulTimeDiff / 1000),
                (unsigned long)(ulTimeDiff % 1000), pCmsgT->len);
            if(pCmsgT->len > 8)
                pCmsgT->len = 8;
            for (j = 0; j < pCmsgT->len; j++) {
                printf("%02X ", pCmsgT->data[j]);
            }
            printf("\n");

            /* Decode events */
            if((pCmsgT->id & NTCAN_EV_BASE) != 0) {
                print_event((EVMSG *)pCmsgT, pCmsgT->timestamp, pCtx);
            }
        }
    }
}

#endif /* of NTCAN_IOCTL_GET_TIMESTAMP */

/************************************************************************/
/************************************************************************/
/* Function: print_cmsg_x()                                             */
/* Print a formatted timestamped CAN message to stdout if               */
/*  NO_ASCII_OUTPUT flag is not set                                     */
/************************************************************************/
/************************************************************************/
#ifdef NTCAN_FD
static void print_cmsg_x(CMSG_X *pCmsgX, CANTEST_CTX *pCtx)
{
    int       j;
    uint32_t  ulTimeDiff = 0;
    uint32_t  len;
    int       dPos;
    char      fd = ' ';

    if(pCmsgX->len & NTCAN_FD) {
        fd = 'f';
    }

    if(pCtx->mode & NTCAN_MODE_OBJECT) {
        pCtx->ullLastTime = pCmsgX->timestamp;

        if (pCmsgX->len & NTCAN_NO_DATA) {
            printf("RX-ID= %4ld (0x%03lx) - %016" PRIx64 " - NO DATA\n",
                (unsigned long)pCmsgX->id, (unsigned long)pCmsgX->id,
                pCtx->ullLastTime);
        } else if (pCmsgX->len & NTCAN_RTR) {
            printf("RTR-ID=%4ld (0x%03lx) - %016" PRIx64 " - len=%02X\n",
                (unsigned long)pCmsgX->id, (unsigned long)pCmsgX->id,
                pCtx->ullLastTime, pCmsgX->len & 0x0f);
        } else {
            len = NTCAN_LEN_TO_DATASIZE(pCmsgX->len);

            dPos = printf("RX-ID= %4ld (0x%03lx) - %016" PRIx64 " - len=%02X %cdat=",
                (unsigned long)pCmsgX->id, (unsigned long)pCmsgX->id,
                pCtx->ullLastTime, len, fd);

            for (j = 0; j < (int)len; j++) {
                if(j && !(j & 7)) {
                    int k = 0;
                    printf("\n");
                    while( k ++ < dPos-3 ){
                        printf(" ");
                    }
                    printf("%02X=", j);
                }
                printf("%02X ", pCmsgX->data[j]);
            }
            printf("\n");
        }
    } else {
        /* Check for lost messages */
        if (pCmsgX->msg_lost != 0) {
            printf("%02x Messages lost !\n", pCmsgX->msg_lost);
        }

        /* Return if console output is disabled */
        if ((pCtx->mode & NO_ASCII_OUTPUT))
            return;

        /* Calculate time difference */
        if (pCtx->ullLastTime != 0) {
            ulTimeDiff = (uint32_t)((pCmsgX->timestamp - pCtx->ullLastTime) *
                INT64_C(1000000) / pCtx->udtTimestampFreq); /*us*/
        }
        pCtx->ullLastTime = pCmsgX->timestamp;

        /* Mark interaction messages */
        if (pCmsgX->len & NTCAN_INTERACTION) {
            printf("*");
        }

        if (pCmsgX->len & NTCAN_RTR) {
            printf("RTR-ID=%9ld (0x%08lx) - %05ld.%03ld - len=%02X\n",
                (unsigned long)pCmsgX->id, (unsigned long)pCmsgX->id,
                (unsigned long)(ulTimeDiff / 1000),
                (unsigned long)(ulTimeDiff % 1000),
                pCmsgX->len & 0x0f);
        } else {
            len = NTCAN_LEN_TO_DATASIZE(pCmsgX->len);
#if 0
            if (ullLastTime != 0) {
                ulTimeDiff = (uint32_t)((pCmsgX->timestamp - ullLastTime) *
                    INT64_C(1000000) / udtTimestampFreq); /*us*/
            }
            ullLastTime = pCmsgX->timestamp;
#endif
            dPos = printf("RX-ID=%9ld (0x%08lx) - %05ld.%03ld - len=%02X %cdat=",
                (unsigned long)pCmsgX->id, (unsigned long)pCmsgX->id, (unsigned long)(ulTimeDiff / 1000),
                (unsigned long)(ulTimeDiff % 1000), len, fd);
            for (j = 0; j < (int)len; j++) {
                if(j && !(j & 7)) {
                    int k = 0;
                    printf("\n");
                    while( k ++ < dPos - 3){
                        printf(" ");
                    }
                    printf("%02X=", j);
                }
                printf("%02X ", pCmsgX->data[j]);
            }
            printf("\n");

            /* Decode events */
            if((pCmsgX->id & NTCAN_EV_BASE) != 0) {
                print_event((EVMSG *)pCmsgX, pCmsgX->timestamp, pCtx);
            }
        }
    }
}

#endif /* of NTCAN_FD */

#if defined(unix) && !defined(RTAI) && !defined(RTLINUX)
static unsigned long mtime(void)
{
  struct timeval  tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);
  return( tv.tv_sec *1000 + tv.tv_usec/1000);
}


#ifdef D3X
void sigio_handler(int arg)
{
  register CMSG *cm;
  int            i, j;
  NTCAN_RESULT   err;
  int32_t        len;
  int8_t         str_buf[100];

  printf("Received signal SIGIO %d\n", arg);
  len = 1;
  err = canRead(h1, rxmsg, &len, NULL);
  if (err == NTCAN_SUCCESS) {
    if (!(pCtx->mode & NO_ASCII_OUTPUT)) {
      printf("Signal = %d Can-Messages=%d \n", arg, len);
    }
    for (i = 0; i < len; i++) {
      cm = &rxmsg[i];
      if (cm->len & NTCAN_RTR) {
        if (cm->msg_lost != 0) {
          printf("%02x Messages lost !\n", cm->msg_lost);
        }
        if (!(pCtx->mode & NO_ASCII_OUTPUT)) {
          printf("RX-RTR-ID=%4d len=%01X\n",
                 cm->id, cm->len & 0x0f);
        }
      } else {
        cm->len &= 0x0f;
        if (cm->msg_lost != 0) {
          printf("%02x Messages lost !\n", cm->msg_lost);
        }
        if (!(pCtx->mode & NO_ASCII_OUTPUT)) {
          printf("RX-ID=%4d len=%01X data= ", cm->id, cm->len);
          for (j = 0; j < cm->len; j++) {
            printf("%02X ", cm->data[j]);
          }
          printf("  [");
          {
            int c;
            for (j = 0; j < cm->len; j++) {
              c = cm->data[j];
              printf("%c", c > 31 && c < 128 ? c : '.' );
            }
          }
          putchar(']');
          printf("\n");
        }
      }
#if 0
      frames ++;
#endif
    }
  } else {
    printf("canRead returned:%s\n", get_error_str(str_buf,err));
    SLEEP(100);
  }
  if (signal(SIGIO, sigio_handler) == SIG_ERR) {
    printf("Re-Initialising signal handler failed\n");
  } else {
    printf("Re-Initialising signal handler succesfull\n");
  }
}
# endif /* of D3X */
#endif /* of unix */

#ifdef CANTEST_USE_SIGUSR1_HANDLER
void sigusr1_handler(int signum) {
    static unsigned int sigcnt;
    if (signum != SIGUSR1) return;
    sigcnt++;
    if ((sigcnt % 1000)==0) {
        printf("sigusr1_handler: sigcnt=%u\n",sigcnt);
    }
}
#endif

#ifdef qnx
static unsigned long mtime(void)
{
  struct timespec tv;

  clock_gettime( CLOCK_REALTIME, &tv );
  return (tv.tv_sec *1000 + tv.tv_nsec/1000000);
}
#endif /* of qnx */

#if defined(VXWORKS) || defined(RTOS32)
# define CT_MAX_ARGS 30 /* Maximum arguments for command line */
/*
 * Helper code for VxWorks limited capability to deal with many cmd line args
 */
int canTest(char *args)
{
  char *token;
  int   argc = 1;
  char *argv[CT_MAX_ARGS] = {"canTest"};

  /*
   * Make comand line from given string
   */
  if(args != NULL) {
    for (token = strtok(args, " "); token && argc < (CT_MAX_ARGS - 1); argc++) {
      argv[argc] = token;
      token = strtok(NULL, " ");
    }
  }

  /*
   * Call real test program. Main is redefined for VxWorks
   */
  return(main(argc, argv));
}
#endif /* VXWORKS || RTOS32 */

#ifdef VXWORKS
#if WANT_PRIO_CHECK == 1
/*
 * Helper code for VxWorks to prevent start with priority higher than
 * backend
 */
static STATUS _checkPriority(int net)
{
  char *taskName = "CANx";
  int   prioSelf, prioBackend;
  int   tid;

  taskName[3] = (char)('0' + net);
  /*
   * Get TID of backend
   */
  tid = taskNameToId(taskName);
  if (ERROR == tid) {
    printf("\nError: No backend %s running\n", taskName);
    return(ERROR);
  }
  /*
   * Return error if getting own or backend priority failed
   */
  if (ERROR == taskPriorityGet(tid, &prioBackend)) {
    printf("\nError getting priority of backend %s\n", taskName);
    return(ERROR);
  }
  /*
   * Return error if getting backend priority failed
   */
  if (ERROR == taskPriorityGet(0, &prioSelf)) {
    printf("\nError getting own priority\n");
    return(ERROR);
  }
  /*
   * Return error if own prio is higher than backend prio
   */
  if (prioSelf <= prioBackend) {
    printf("\nOwn priority (%d) is higher than priority of backend"
           " '%s' (%d) which can cause unexpected results."
           " Spawn canTest with lower priority !!\n\n",
           prioSelf, taskName, prioBackend);
    return(ERROR);
  }
  return(OK);
}
#endif /* #if WANT_PRIO_CHECK == 1 */
#endif /* of VXWORKS */

#ifdef NET_OS
static unsigned long mtime(void)
{
  unsigned long ulHigh, ulLow;

  NATotalTicks(&ulHigh, &ulLow);
  return((ulLow * 1000) / BSP_TICKS_PER_SECOND);
}
#endif /* of NET_OS */

#ifdef _WIN32_WCE
/*
 * Helper code for VxWorks to create argc/argv arrays for programs by
 * parsing the command line given as unicode string without the
 * program name into individual arguments.
 * It does not handle quoted strings.
 */
int CreateArgvArgc(TCHAR *pProgName, TCHAR *argv[30], TCHAR *pCmdLine)
{
  TCHAR *pEnd;
  int    argc = 0;

  /* Insert the program name as argc 1 */
  argv[argc++] = pProgName;

  while (*pCmdLine != TEXT('\0')) {
    while (iswspace (*pCmdLine)) {
      pCmdLine++;             /* Skip to first whitsacpe */
    }
    if (*pCmdLine == TEXT('\0')) {
      break;                  /* Break at EOL */
    }
    /*
     * Check for '' or ""
     */
    if ((*pCmdLine == TEXT('"')) || (*pCmdLine == TEXT('\''))) {
      TCHAR cTerm = *pCmdLine++;
      for (pEnd = pCmdLine; (*pEnd != cTerm) && (*pEnd != TEXT('\0'));) {
        pEnd++;
      }
    } else {
      /* Find the end.*/
      for (pEnd = pCmdLine; !iswspace(*pEnd) && (*pEnd != TEXT('\0'));) {
        pEnd++;
      }
    }
    if (*pEnd != TEXT('\0')) {
      *pEnd = TEXT('\0');
      pEnd++;
    }
    argv[argc] = pCmdLine;
    argc++;
    pCmdLine = pEnd;
  }
  return argc;
}
#endif /* of _WIN32_WCE */


#ifdef RMOS
static unsigned long mtime(void)
{
  int             rmStatus;
  RmAbsTimeStruct time;

  rmStatus = RmGetAbsTime(&time);
  if (rmStatus != RM_OK) {
    printf("Cannot get RMOS-Time! Err=%d!\n", rmStatus);
    return 0;
  } else {
    return time.lotime;
  }
}
#endif


#ifdef _WIN32
# if defined(UNDER_RTSS)
static unsigned long mtime(void)
{
  LARGE_INTEGER ticks;

  RtGetClockTime(CLOCK_FASTEST, &ticks);        /* Get 100 ns tick */

  return ((unsigned long)(ticks.QuadPart / 10));                        /* Return us tick */
}
# elif defined(RTOS32)
# else
static unsigned long mtime(void)
{
  LARGE_INTEGER frequency;

  if (QueryPerformanceFrequency(&frequency)) {
    LARGE_INTEGER ticks;

    QueryPerformanceCounter(&ticks);
    return (unsigned long) (((ticks.QuadPart) * 1000000) / frequency.QuadPart);
  } else {
    return GetTickCount();
  }
}

/*
 * Helper code to dynamically load entries of NTCAN libraries >= 4.x.x
 * to be prevent load errors with previous versions which do not contain
 * these exports.
 */
static int DynLoad(void)
{
    HMODULE hDll = NULL;

#  ifndef _WIN32_WCE

    /* Try loading NTCAN DLL (32 or 64 bit) dynamically and check success */
    hDll = LoadLibrary("ntcan.dll");   /* 32 bit version */

    if (NULL == hDll) {
        /*
        * In 64-bit driver revision 2.4.x the NTCAN library was named
        * ntcan64.dll. Cope with this legacy name.
        */
#   if defined (_MSC_VER)
__pragma(warning(disable:4127))
#   endif
        if(8 == sizeof(INT_PTR)) {
            hDll = LoadLibrary("ntcan64.dll");   /* 64-bit legacy name */
        }
#   if defined (_MSC_VER)
__pragma(warning(default:4127))
#   endif

        if (NULL == hDll)
            return -1;
    }
#  else

    /* Try loading NTCAN DLL dynamically and check success */
    hDll = LoadLibrary(TEXT("ntcan.dll"));

    if (NULL == hDll)
        return -1;
#  endif /* of !defined _WIN32_WCE */

    /*
     * NTCAN >= 2.3.0:
     *   Get function ptr of canIoctl()
     */
    pfnIoctl = FUNCPTR_CAN_IOCTL(hDll);

    /*
     * NTCAN > 4.0.x
     *   Get function ptr of canTakeT() and canReadT()
     */
    pfnTakeT = FUNCPTR_CAN_TAKE_T(hDll);
    pfnReadT = FUNCPTR_CAN_READ_T(hDll);

    /*
     * NTCAN > 4.1.x:
     *   Get function ptr of canFormatError()
     */
    pfnFormatError = FUNCPTR_CAN_FORMAT_ERROR(hDll);

    /*
     * NTCAN > 4.1.x:
     *   Get function ptr of canGetOverlappedResultT()
     */
    pfnGetOverlappedResultT = FUNCPTR_CAN_GET_OVERLAPPED_RESULT_T(hDll);

    /*
     * NTCAN > 4.3.0:
     *   Get function ptr of canFormatEvent()
     */
    pfnFormatEvent = FUNCPTR_CAN_FORMAT_EVENT(hDll);

    /*
     * NTCAN > 4.7.x:
     *   Get function ptr of canSendT() and canWriteT()
     */
    pfnSendT = FUNCPTR_CAN_SEND_T(hDll);
    pfnWriteT = FUNCPTR_CAN_WRITE_T(hDll);

# if defined (NTCAN_FD)
    /*
     * NTCAN > 5.0.x:
     *   Get function ptr of canTakeX(), canReadX(), canWriteX(), canSendX(),
     *   canGetOverlappedResultX()
     */
    pfnTakeX                = FUNCPTR_CAN_TAKE_X(hDll);
    pfnReadX                = FUNCPTR_CAN_READ_X(hDll);
    pfnSendX                = FUNCPTR_CAN_SEND_X(hDll);
    pfnWriteX               = FUNCPTR_CAN_WRITE_X(hDll);
    pfnGetOverlappedResultX = FUNCPTR_CAN_GET_OVERLAPPED_RESULT_X(hDll);
#endif /* of NTCAN_FD) */

    return(0);
}

#  ifndef _WIN32_WCE
static int ForceThreadAffinity(void) {
    SYSTEM_INFO udtSysInfo;
    int i;
    /*
     * GetProcessAffinityMask() parameter changed with the introduction of
     * 64-bit Windows from DWORD to DWORD_PTR. As earlier revisions of the
     * SDK didn't define DWORD_PTR we leave the definition for 32-bit
     * Windows as is to stay backward compatible.
     */
#   ifdef _WIN64
    DWORD_PTR   dwProcessAffinityMask, dwSystemAffinityMask, dwMask;
#   else
    DWORD       dwProcessAffinityMask, dwSystemAffinityMask, dwMask;
#   endif

    /* Get system information */
    GetSystemInfo(&udtSysInfo);

    /*
     * In case of more than one processor force execution to the 1st processor
     * to be sure that using QueryPerformanceCounter() does not return TSC
     * counter values from different cores/processors.
     */
    if(udtSysInfo.dwNumberOfProcessors <= 1)
        return 0;

    if(GetProcessAffinityMask(GetCurrentProcess(),
       &dwProcessAffinityMask,
       &dwSystemAffinityMask))
    {
        /*
         *	Search the processor mask for the 1st allowed CPU and use this one.
         */
        for(dwMask = 1, i = 0; i < (int)(sizeof(dwMask)<<3); i++, dwMask <<= 1)
        {
            if((dwProcessAffinityMask & dwMask) != 0)
            {
                SetThreadAffinityMask(GetCurrentThread(), dwMask);
                return(0);
            }
        }
    }

    return(1);
}
#  endif /* of !defined(_WIN32_WCE) */
# endif  /* of UNDER_RTSS */
#endif   /* of _WIN32 */
