/******************************************************************************
** Az alábbi program és információk a Nemzeti Kutatási Fejlesztési Programok
** 2002. évi pályázatán a 2. sz. programban elfogadott NKFP-2/0017/2002 számú
** ,,Adatrosta - webtárházak nagytömegû adatbázisainak elemzése adatbányászati
** és statisztikai eszközök segítségével'' elnevezésû projekt terméke.
**
** A program a projekt folyamán feldolgozásra kerülõ naplóállományok
** (web, access és mail) bináris kódolását és tömörítését végzi.
**
** A program és információk az Adatrosta projekt konzorciumának tulajdona.
** Engedély nélküli felhasználása tilos.
**
** Az Adatrosta projekt konzorcium tagjai:
** ELTE TTK Számítógéptudományi Tsz.
** BME Matematika Intézet
** MTA SZTAKI Informatikai Kutatólaboratórium
** Axelero Rt.
** econet.hu Rt.
**
** http://adatrosta.hu/
** email: info@adatrosta.hu
**
** (C) 2003. Minden jog fenntartva.
**
**
** This data and information are property of the "Data Riddle" consortium.
** Modification or use without permission is strictly prohibited.
**
** (C) 2003 All rights reserved.
**
** Author: RACZ Balazs, 2003.
**
******************************************************************************/

/*!
  \file   log.h
  \brief  logging of status, warnings and error messages

  Provides routines for displaying and/or logging status, giving warnings.

  Set DEBUG_LEVEL to your needs with compiler option or rewrite debug
  information processing. There is a slot to provide debug level
  selection based on the message source (domain), which is currently
  __FILE__. This is yet unused (i.e. constant), you may set the debug
  level based on the source file.

  \author RACZ Balazs
  \date 2003-01-19 */


/*
  flockfile (stderr) / funlockfile (stderr) added around messages
  to make logging thread safe
  benczur 2003-11-29

  removed it, cause syslog & stdio _are_ thread safe when using -lpthread
  stamas 2004-02-22
 */

#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#ifndef WIN32 // We don't use syslog on Windows,
   #include <syslog.h>
   #define LOG_FDEBUG      8       /* very frequent debug messages */
#else // but we need its constants, for run-time log firltering.
      // The definitions are copified from Linux.
   #define LOG_EMERG       0       /* system is unusable */
   #define LOG_ALERT       1       /* action must be taken immediately */
   #define LOG_CRIT        2       /* critical conditions */
   #define LOG_ERR         3       /* error conditions */
   #define LOG_WARNING     4       /* warning conditions */
   #define LOG_NOTICE      5       /* normal but significant condition */
   #define LOG_INFO        6       /* informational */
   #define LOG_DEBUG       7       /* debug-level messages */
   #define LOG_FDEBUG      8       /* very frequent debug messages */
#endif


#define LOG_DEF_PRG       "unknown"
#define LOG_DEF_PRG_GROUP "co-cluster"

// Call this 1st
//! \brief open with name ident=prog:prog_group. This will be prepended to every message
//! This gives no output and does no harm for use_syslog == false

void log_open_syslog( const char *prg = LOG_DEF_PRG,
                      const char *prg_group = LOG_DEF_PRG_GROUP );

// Call this to change default log destinations
// Filters messages above runtime_level
// NOTE: runtime_level is according to "/usr/include/sys/syslog.h", i.e.
//       error = 3, debug = 7 which is different from the LEVEL_*
//       macros used for compile time filtering
// logs to log_file iff log_file != NULL
// opens and logs to syslog with ident syslog_ident iff syslog_ident != NULL

void set_log( int runtime_level = INT_MAX, FILE *log_file = stderr,
              bool use_syslog = false, bool use_stderr = false, bool use_colors = false );

// As above, log_file and use_syslog are parsed from log_destination

void set_log( int runtime_log_level = INT_MAX,
              const char *log_destination = "/dev/stderr",
              bool use_colors = false );

void close_logfile();

//! Base debug level of errors
#define LEVEL_ERR	0
//! Base debug level of warnings
#define LEVEL_WARN	10
//! Base level of performance (statistics) messages
#define LEVEL_PERF	20
//! Base debug level of status messages
#define LEVEL_STATUS	30
//! Base debug level of informative messages
#define LEVEL_INFO	35
//! Debug info from external web servers, bad html etc
#define LEVEL_CRAWLER_WARNING	45
//! Base debug level of debugging messages
#define LEVEL_DBG	50
//! Debug level of frequent debugging message
#define LEVEL_FDBG 60

//implementation


//! \brief append to log (generic)
/**
    \param domain is a string literal, specifies the source of the message

    \param level is a number, the greater, the less important the log
    entry. loglevel specification will filter levels to less than the
    specified number.

    \param levelchr is a string literal denoting the level.
    \param format is a printf() like format string
    \param arg are the arguments to the format string.
*/
/*
// We use SLOG (syslog) instead
#define LOG( domain, level, levelchr, format, ... ) { \
  if (level<=debug_level(domain)) { \
    fprintf(stderr, domain "(" levelchr ", %d ): ",__LINE__); \
    fprintf(stderr, format, ## arg); \
    fprintf(stderr, "\n"); \
  } \
}
*/

//! \brief defines the debug level as a function of the domain.
/** currently constant
 */
#define debug_level( x )  DEBUG_LEVEL

#ifndef DEBUG_LEVEL
#ifndef WIN32
#warning "You should define DEBUG_LEVEL or expect full debugging output everywhere"
//! You should define DEBUG_LEVEL externally (makefile, perhaps).
#else // Windows does not have #warning directive
#pragma message "You should define DEBUG_LEVEL or expect full debugging output everywhere"
#endif
#define DEBUG_LEVEL 255
#endif

//////////////////////////////////////////////////////////////
// logging with syslog
//////////////////////////////////////////////////////////////

// Extra log_perr style routine log_err_errno for syscalls
// which return an errno like value

#if LEVEL_ERR <= DEBUG_LEVEL
#define log_err( ... )  {DOLOG(LOG_ERR, "E", __VA_ARGS__ )}
#define log_err_errno( errnoval, ... ) {DOLOG_ERRNO(LOG_ERR, "E", errno, __VA_ARGS__ )}
#define log_perr( ... ) {log_err_errno( errno, __VA_ARGS__ )}
#else
#define log_err( ... )  {}
#define log_err_errno( errnoval, ... ) {}
#define log_perr( ... ) {}
#endif

#if LEVEL_WARN <= DEBUG_LEVEL
#define log_warn( ... )  {DOLOG(LOG_WARNING, "W", __VA_ARGS__ )}
#define log_pwarn( ... ) {DOLOG_ERRNO(LOG_WARNING, "W", errno, __VA_ARGS__ )}
#else
#define log_warn( ... )  {}
#define log_pwarn( ... ) {}
#endif

#if LEVEL_PERF <= DEBUG_LEVEL
#define log_perf( ... ) {DOLOG(LOG_NOTICE, "P", __VA_ARGS__ )}
#else
#define log_perf( ... ) {}
#endif

#if LEVEL_STATUS <= DEBUG_LEVEL
#define log_status( ... ) {DOLOG(LOG_NOTICE, "S", __VA_ARGS__ )}
#else
#define log_status( ... ) {}
#endif

#if LEVEL_INFO <= DEBUG_LEVEL
#define log_info( ... )  {DOLOG(LOG_INFO, "I", __VA_ARGS__ )}
#define log_pinfo( ... ) {DOLOG_ERRNO(LOG_INFO, "I", errno, __VA_ARGS__ )}
#else
#define log_info( ... )  {}
#define log_pinfo( ... ) {}
#endif

#if LEVEL_CRAWLER_WARNING <= DEBUG_LEVEL
#define log_cw( ... ) {DOLOG(LOG_DEBUG, "C", __VA_ARGS__ )}
#else
#define log_cw( ... ) {}
#endif

#if LEVEL_DBG <= DEBUG_LEVEL
#define log_dbg( ... ) {DOLOG(LOG_DEBUG, "D", __VA_ARGS__ )}
#else
#define log_dbg( ... ) {}
#endif

#if LEVEL_FDBG <= DEBUG_LEVEL
#define log_fdbg( ... ) {DOLOG(LOG_FDEBUG, "F", __VA_ARGS__ )}
#else
#define log_fdbg( ... ) {}
#endif



// Next two are the real logging functions
void do_log_a( const int level, const char *prefix,
             const char *srcfile, const int srcline,
             const char *format, ... );
//
// errnoval is in-out, since strerror_r may spoil the real errno,
// in that case we restore it.
//
void do_log_errno_a( const int level, const char *prefix, int &errnoval,
                   const char *srcfile, const int srcline,
                   const char *format, ... );

// Substitute the params and call the real logger
#define DOLOG( level, levelchr, ... ) {  \
   do_log_a( level, levelchr, __FILE__, __LINE__, __VA_ARGS__ ); \
}

#define DOLOG_ERRNO( level, levelchr, errnoval, ... ) { \
   do_log_errno_a( level, levelchr, errnoval, __FILE__, __LINE__, __VA_ARGS__ ); \
}

#define log_assert(cond, ... ) { 	               \
   if(!(cond)) {		  		               \
      log_err( "assert: " #cond " is FALSE: " __VA_ARGS__); \
   }					                       \
}

#ifdef WIN32

// wchar_t version of the above

#if LEVEL_ERR <= DEBUG_LEVEL
#define log_err_w( ... )  {DOLOG_W(LOG_ERR, "E", __VA_ARGS__ )}
#define log_err_errno_w( errnoval, ... ) {DOLOG_ERRNO_W(LOG_ERR, "E", errno, __VA_ARGS__ )}
#define log_perr_w( ... ) {log_err_errno_w( errno, __VA_ARGS__ )}
#else
#define log_err_w( ... )  {}
#define log_err_errno_w( errnoval, ... ) {}
#define log_perr_w( ... ) {}
#endif

#if LEVEL_WARN <= DEBUG_LEVEL
#define log_warn_w( ... )  {DOLOG_W(LOG_WARNING, "W", __VA_ARGS__ )}
#define log_pwarn_w( ... ) {DOLOG_ERRNO_W(LOG_WARNING, "W", errno, __VA_ARGS__ )}
#else
#define log_warn_w( ... )  {}
#define log_pwarn_w( ... ) {}
#endif

#if LEVEL_PERF <= DEBUG_LEVEL
#define log_perf_w( ... ) {DOLOG_W(LOG_NOTICE, "P", __VA_ARGS__ )}
#else
#define log_perf_w( ... ) {}
#endif

#if LEVEL_STATUS <= DEBUG_LEVEL
#define log_status_w( ... ) {DOLOG_W(LOG_NOTICE, "S", __VA_ARGS__ )}
#else
#define log_status_w( ... ) {}
#endif

#if LEVEL_INFO <= DEBUG_LEVEL
#define log_info_w( ... )  {DOLOG_W(LOG_INFO, "I", __VA_ARGS__ )}
#define log_pinfo_w( ... ) {DOLOG_ERRNO_W(LOG_INFO, "I", errno, __VA_ARGS__ )}
#else
#define log_info_w( ... )  {}
#define log_pinfo_w( ... ) {}
#endif

#if LEVEL_CRAWLER_WARNING <= DEBUG_LEVEL
#define log_cw_w( ... ) {DOLOG_W(LOG_DEBUG, "C", __VA_ARGS__ )}
#else
#define log_cw_w( ... ) {}
#endif

#if LEVEL_DBG <= DEBUG_LEVEL
#define log_dbg_w( ... ) {DOLOG_W(LOG_DEBUG, "D", __VA_ARGS__ )}
#else
#define log_dbg_w( ... ) {}
#endif

#if LEVEL_FDBG <= DEBUG_LEVEL
#define log_fdbg_w( ... ) {DOLOG_W(LOG_FDEBUG, "D", __VA_ARGS__ )}
#else
#define log_fdbg_w( ... ) {}
#endif


void do_log_w( const int level, const wchar_t *prefix,
             const wchar_t *srcfile, const int srcline,
             const wchar_t *format, ... );
void do_log_errno_w( const int level, const wchar_t *prefix, int &errnoval,
                   const wchar_t *srcfile, const int srcline,
                   const wchar_t *format, ... );
// Ezek nelkul nem megy a L __FILE__
#define __W(x) L ## x
#define _W(x) __W(x)

#define DOLOG_W( level, levelchr, ... ) {  \
	do_log_w( level, _W(levelchr), _W(__FILE__), __LINE__, __VA_ARGS__ ); \
}

#define DOLOG_ERRNO_W( level, levelchr, errnoval, ... ) { \
   do_log_errno_w( level, _W(levelchr), errnoval, _W(__FILE__), __LINE__, __VA_ARGS__ ); \
}

// TCHAR version of the above

#if LEVEL_ERR <= DEBUG_LEVEL
#define log_err_t( ... )  {DOLOG_T(LOG_ERR, "E", __VA_ARGS__ )}
#define log_err_errno_t( errnoval, ... ) {DOLOG_ERRNO_T(LOG_ERR, "E", errno, __VA_ARGS__ )}
#define log_perr_t( ... ) {log_err_errno_t( errno, __VA_ARGS__ )}
#else
#define log_err_t( ... )  {}
#define log_err_errno_t( errnoval, ... ) {}
#define log_perr_t( ... ) {}
#endif

#if LEVEL_WARN <= DEBUG_LEVEL
#define log_warn_t( ... )  {DOLOG_T(LOG_WARNING, "W", __VA_ARGS__ )}
#define log_pwarn_t( ... ) {DOLOG_ERRNO_T(LOG_WARNING, "W", errno, __VA_ARGS__ )}
#else
#define log_tarn_t( ... )  {}
#define log_pwarn_t( ... ) {}
#endif

#if LEVEL_PERF <= DEBUG_LEVEL
#define log_perf_t( ... ) {DOLOG_T(LOG_NOTICE, "P", __VA_ARGS__ )}
#else
#define log_perf_t( ... ) {}
#endif

#if LEVEL_STATUS <= DEBUG_LEVEL
#define log_status_t( ... ) {DOLOG_T(LOG_NOTICE, "S", __VA_ARGS__ )}
#else
#define log_status_t( ... ) {}
#endif

#if LEVEL_INFO <= DEBUG_LEVEL
#define log_info_t( ... )  {DOLOG_T(LOG_INFO, "I", __VA_ARGS__ )}
#define log_pinfo_t( ... ) {DOLOG_ERRNO_T(LOG_INFO, "I", errno, __VA_ARGS__ )}
#else
#define log_info_t( ... )  {}
#define log_pinfo_t( ... ) {}
#endif

#if LEVEL_CRAWLER_WARNING <= DEBUG_LEVEL
#define log_cw_t( ... ) {DOLOG_T(LOG_DEBUG, "C",__VA_ARGS__ )}
#else
#define log_cw_t( ... ) {}
#endif

#if LEVEL_DBG <= DEBUG_LEVEL
#define log_dbg_t( ... ) {DOLOG_T(LOG_DEBUG, "D", __VA_ARGS__ )}
#else
#define log_dbg_t( ... ) {}
#endif

#if LEVEL_FDBG <= DEBUG_LEVEL
#define log_fdbg_w( ... ) {DOLOG_W(LOG_FDEBUG, "D", __VA_ARGS__ )}
#else
#define log_fdbg_w( ... ) {}
#endif


void do_log_t( const int level, const TCHAR *prefix,
             const TCHAR *srcfile, const int srcline,
             const TCHAR *format, ... );
void do_log_errno_t( const int level, const wchar_t *prefix, int &errnoval,
                   const TCHAR *srcfile, const int srcline,
                   const wchar_t *format, ... );
#define DOLOG_T( level, levelchr, ... ) {  \
   do_log_t( level, _T(levelchr), _T(__FILE__), __LINE__, __VA_ARGS__ ); \
}

#define DOLOG_ERRNO_T( level, levelchr, errnoval, ... ) { \
   do_log_errno_t( level, _T(levelchr), errnoval, _T(__FILE__), __LINE__, __VA_ARGS__ ); \
}

#else // !WIN32

#define log_err_w( ... ) log_err( __VA_ARGS__ );
#define log_perr_w( ... ) log_perr( __VA_ARGS__ );
#define log_err_w( ... ) log_err( __VA_ARGS__ );
#define log_perr_w( ... ) log_perr( __VA_ARGS__ );
#define log_warn_w( ... ) log_warn( __VA_ARGS__ );
#define log_pwarn_w( ... ) log_pwarn( __VA_ARGS__ );
#define log_warn_w( ... ) log_warn( __VA_ARGS__ );
#define log_pwarn_w( ... ) log_pwarn( __VA_ARGS__ );
#define log_perf_w( ... ) log_perf( __VA_ARGS__ );
#define log_perf_w( ... ) log_perf( __VA_ARGS__ );
#define log_status_w( ... ) log_status( __VA_ARGS__ );
#define log_status_w( ... ) log_status( __VA_ARGS__ );
#define log_info_w( ... ) log_info( __VA_ARGS__ );
#define log_pinfo_w( ... ) log_pinfo( __VA_ARGS__ );
#define log_info_w( ... ) log_info( __VA_ARGS__ );
#define log_pinfo_w( ... ) log_pinfo( __VA_ARGS__ );
#define log_cw_w( ... ) log_cw( __VA_ARGS__ );
#define log_cw_w( ... ) log_cw( __VA_ARGS__ );
#define log_dbg_w( ... ) log_dbg( __VA_ARGS__ );
#define log_dbg_w( ... ) log_dbg( __VA_ARGS__ );
#define log_err_t( ... ) log_err( __VA_ARGS__ );
#define log_perr_t( ... ) log_perr( __VA_ARGS__ );
#define log_err_t( ... ) log_err( __VA_ARGS__ );
#define log_perr_t( ... ) log_perr( __VA_ARGS__ );
#define log_warn_t( ... ) log_warn( __VA_ARGS__ );
#define log_pwarn_t( ... ) log_pwarn( __VA_ARGS__ );
#define log_tarn_t( ... ) log_tarn( __VA_ARGS__ );
#define log_pwarn_t( ... ) log_pwarn( __VA_ARGS__ );
#define log_perf_t( ... ) log_perf( __VA_ARGS__ );
#define log_perf_t( ... ) log_perf( __VA_ARGS__ );
#define log_status_t( ... ) log_status( __VA_ARGS__ );
#define log_status_t( ... ) log_status( __VA_ARGS__ );
#define log_info_t( ... ) log_info( __VA_ARGS__ );
#define log_pinfo_t( ... ) log_pinfo( __VA_ARGS__ );
#define log_info_t( ... ) log_info( __VA_ARGS__ );
#define log_pinfo_t( ... ) log_pinfo( __VA_ARGS__ );
#define log_cw_t( ... ) log_cw( __VA_ARGS__ );
#define log_cw_t( ... ) log_cw( __VA_ARGS__ );
#define log_dbg_t( ... ) log_dbg( __VA_ARGS__ );
#define log_dbg_t( ... ) log_dbg( __VA_ARGS__ );

#endif // WIN32

// additional debugging tools

// returns stack trace. thread-unsafe
// needs -rdynamic option when compile
const char *stack_trace();

#endif //__LOG_H__
