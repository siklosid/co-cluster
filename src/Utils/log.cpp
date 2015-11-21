#include <stdarg.h> // for vararg va_list and vsnprintf
#include <string.h>
#include "malloc.h"
#include <stdlib.h>
#include <string>
#ifdef WIN32
#include <tchar.h>
#else // !WIN32
#include <execinfo.h> // for stack trace
#endif

using namespace std;

#include "log.h"

// Make very sure that no syslog msg exceeds 1024 bytes
#define LOG_BUF_SIZE          512
// Buffer size for strerrno_r
#define LOG_STRERRNO_BUF_SIZE 128
// Buffer size for format strings
#define LOG_FORMAT_BUF_SIZE 128

static int   __log_runtime_level = INT_MAX; // for run time log level filtering
                                            // default: none
static FILE *__log_file = stderr;	        // used for logging iff non NULL
                                            // default: stderr
static bool  __log_use_syslog    = false;   // syslog is used iff __log_use_syslog is true
                                            // default: don't use syslog
static bool  __log_use_stderr    = false;    // log is written to stderr
                                            // default: use stderr

static bool  __log_use_colors    = false;   // colors are used iff __log_use_colors is true
                                            // default: don't use colors

// LOG_PERROR = stderr
// XXX: solarison a LOG_PERROR nem megy

void log_open_syslog( const char *prg, const char *prg_group )
{
#ifndef WIN32 // We dont use syslog on Windows
   // Needs to be static, because openlog in log_open_syslog
   // just stores the char * pointer
   static string ident;
   ident  = prg_group;
   ident += '.';
   ident += prg;
   // This gives no output and does no harm for use_syslog == false   
   openlog( ident.c_str(), LOG_NDELAY|LOG_PID, LOG_USER );
#endif
}
 
void set_log( int runtime_level, FILE *log_file, bool use_syslog, bool use_stderr, bool use_colors )
{
#ifdef WIN32
   if (use_syslog) 
   {
	   // Disable it, but do not complain
	   // log_warn( "Syslog does not work on Windows, disabling it automatically" );
	   use_syslog = false;
   }
#endif

   if( log_file == NULL && !use_syslog && !use_stderr)
   {
      // Be safe - complain everywhere
      set_log( INT_MAX, stderr, true, true, use_colors ); 
      log_err( "Fatal error: both file and syslog based logging are "
               "disabled. Exiting." );
      exit(1);
   }
   
   __log_runtime_level = runtime_level;
   __log_file          = log_file;
   __log_use_syslog    = false;
   __log_use_stderr    = use_stderr;
   __log_use_colors    = use_colors;
}

static void update_log_dest( FILE **log_file, bool &use_syslog,
							 bool &use_stderr,
                             const char *token,
                             const char *log_dest_str )
{
   if( strcmp(token,"syslog") == 0 )
   {
      use_syslog = true;
   }
   else
   {
      if( *log_file != NULL )
      {
         log_err( "Fatal error: Multiple log files given in %s. Exiting",
                  log_dest_str );
         exit(1);
      }
    
      if( strcmp(token,"stderr") == 0 )
      {
         //*log_file = stderr;
		 use_stderr = true;
      }
      else if( strcmp(token,"stdout") == 0 )
      {
         *log_file = stdout;
      }
      else
      {
         *log_file = fopen(token,"a");
      }
   }
}

void set_log( int runtime_log_level, const char *log_destination, bool use_colors )
{
   // temporary log settings for possible error reporting in update_log_dest
   // Be safe - complain everywhere
   set_log( INT_MAX, stderr, true, false, use_colors);    
   
   bool use_syslog = false;
   bool use_stderr = false;
   FILE *log_file  = NULL;
   const char *comma = strchr(log_destination,',');
   if( comma )
   {
      string s( log_destination, comma-log_destination );
      update_log_dest( &log_file, use_syslog, use_stderr, s.c_str(), log_destination );
      update_log_dest( &log_file, use_syslog, use_stderr, comma+1, log_destination );      
   }
   else
   {
      update_log_dest( &log_file, use_syslog, use_stderr, log_destination, log_destination );
   }
   
   // permanent log settings
   set_log( runtime_log_level, log_file, use_syslog, use_stderr, use_colors );
}

void close_logfile() {
	fclose(__log_file);
	__log_file = NULL;
}

void set_color(const char *prefix, char *color) {
    int c;
    switch (prefix[0]) {
        case 'E': c = 31;
                  break;
        case 'W': c = 33;
                  break;
        case 'D': c = 0;
                  break;
        default:  c = 0;
                  break;
    }
    snprintf(color, 10, "\033[%dm", c);
}

// Helper classes for creating logging functions with different 
// char types (char, wchar, tchar)

struct charFunctions {
	inline static int snprintf(char *buf, size_t size, const char *format, ...) {
		va_list ap; va_start(ap, format);
		return ::vsnprintf(buf, size, format, ap);}
	inline static int vsnprintf(char *buf, size_t size, const char *format, va_list args)
	{return ::vsnprintf(buf, size, format, args);}
	inline static int fprintf(FILE *f, const char *format, const char *arg)
	{return ::fprintf(f, format, arg);}
	inline static char *strncat(char *dest, const char *src, size_t count)
	{return ::strncat(dest, src, count);}
	inline static char *convert(char *dest, const char *src, size_t size)
	{return ::strncpy(dest, src, size);}
};

#ifdef WIN32

struct wchar_tFunctions {
	inline static int snprintf(wchar_t *buf, size_t size, const wchar_t *format, ...) {
		va_list ap; va_start(ap, format);
		return	_vsnwprintf(buf, size, format, ap);}
	inline static int vsnprintf(wchar_t *buf, size_t size, const wchar_t *format, va_list args)
		{return _vsnwprintf(buf, size, format, args);}
	inline static int fprintf(FILE *f, const wchar_t *format, const wchar_t *arg)
		{return fwprintf(f, format, arg);}
	inline static wchar_t *strncat(wchar_t *dest, const wchar_t *src, size_t count)
	{return wcsncat(dest, src, count);}
	inline static wchar_t *convert(wchar_t *dest, const char *src, size_t size) 
	{mbstowcs(dest, src, size);return dest;}
};
struct TCHARFunctions {
	inline static int snprintf(TCHAR *buf, size_t size, const TCHAR *format, ...) {
		va_list ap; va_start(ap, format);
        return	_vsntprintf(buf, size, format, ap);}
	inline static int vsnprintf(TCHAR *buf, size_t size, const TCHAR *format, va_list args)
		{return _vsntprintf(buf, size, format, args);}
	inline static int fprintf(FILE *f, const TCHAR *format, const TCHAR *arg)
		{return _ftprintf(f, format, arg);}
	inline static TCHAR *strncat(TCHAR *dest, const TCHAR *src, size_t count)
	{return _tcsncat(dest, src, count);}
	inline static TCHAR *convert(TCHAR *dest, const char *src, size_t size) {
#ifdef _UNICODE
		return wchar_tFunctions::convert(dest, src ,size);
#else // !_UNICODE
		return charFunctions::convert(dest, src ,size);
#endif // _UNICODE
	}
};
#endif

//
// Does the real work of logging.
// Pre: va_start(ap) has been called
// Post: va_end(ap) has benn called
// 

template<class CharType, class Functions> 
static void do_log_( const int level, 
					 const CharType *prefix, const CharType *postfix,
                     const CharType *srcfile, const int srcline,
                     const CharType *format, va_list ap )
{
   if( level >  __log_runtime_level ) // do the run time filtering
   {
      va_end(ap);
      return;
   }

   // snprintf helyett strncat mert nem lehet fix tipusu formatot hasznalni.
#define LOG_CTIME_BUF_SIZE 128
   CharType buf[LOG_BUF_SIZE];
   CharType fmt_buf[LOG_FORMAT_BUF_SIZE];
   CharType ctime_buf[LOG_CTIME_BUF_SIZE];
   char ctime_buf_c[LOG_CTIME_BUF_SIZE];
   time_t t = time(NULL);
   ctime_r(&t, ctime_buf_c);
   char *nl = strchr(ctime_buf_c, '\n');
   if (nl != NULL) *nl = '\0';
   int len = Functions::snprintf( buf, LOG_BUF_SIZE, 
	   Functions::convert(fmt_buf, "%s(%s %s:%d): ", LOG_FORMAT_BUF_SIZE), 
	   prefix, 
	   Functions::convert(ctime_buf, ctime_buf_c, LOG_CTIME_BUF_SIZE),
	   srcfile, srcline );

   /*
   char buf[LOG_BUF_SIZE];
   int len = snprintf( buf, LOG_BUF_SIZE, "%s(%s:%d): ", prefix, srcfile, srcline );
   */

   // if the full prefix was not truncated
   // snprintf should never return negative values(output error), but make very sure
   if( 0 <= len && len < LOG_BUF_SIZE ) 
   {
	  int len2 = Functions::vsnprintf( buf+len, LOG_BUF_SIZE-len, format, ap );
      len += len2;
      // if the buffer still has more space in it
      if( 0 <= len2 && len < LOG_BUF_SIZE ) 
      {
         // Let's try to add the postfix, strncat writes at most size+1 chars!
		 Functions::strncat( buf+len, postfix, LOG_BUF_SIZE-len-1 ); 
      }
   }
   va_end( ap );
   if( __log_use_syslog )
   { 
#ifndef WIN32
      syslog(level, "%s\n", buf ); 
#endif
   } 
   if (__log_use_stderr) {
		Functions::fprintf(stderr, 
		   Functions::convert(fmt_buf, "%s\n", LOG_FORMAT_BUF_SIZE), 
		   buf );
   }
   if( __log_file )
   { 
	   Functions::fprintf(__log_file, 
		   Functions::convert(fmt_buf, "%s\n", LOG_FORMAT_BUF_SIZE), 
		   buf ); 
	   fflush(__log_file);
/*
	  if (__log_use_colors) {
		  char color[10] = "";
		  set_color(prefix, color);
		  fprintf(__log_file, "%s%s\033[0m\n", color, buf ); 
	  } else {
		  fprintf(__log_file, "%s\n", buf ); 
	  }
	  */
   }
}

//
// Called by DOLOG_REAL macro
// 
/*template<class CharType, class Functions> 
static void do_log( const int level, const CharType *prefix,
             const char *srcfile, const int srcline,
             const CharType *format, ... )
{
   va_list ap;
   va_start( ap, format );
   do_log_<CharType, Functions>( level, prefix, "", srcfile, srcline, format, ap );
}*/

//
// Called by DOLOG_REAL_ERRNO macro
// Saves errnoval, because if it is the real errno, our_strerror may spoil it
// (if it returned non-zero) then restores it.
//
template<class CharType, class Functions> 
static void do_log_errno_( const int level, const CharType *prefix, int &errnoval,
                   const CharType *srcfile, const int srcline,
                   const CharType *format, va_list ap )
{
   int  errno_saved = errnoval;
   char errno_buf[LOG_STRERRNO_BUF_SIZE];
   CharType converted_errno_buf[LOG_STRERRNO_BUF_SIZE];
   CharType postfix[LOG_BUF_SIZE];
   CharType fmt_buf[LOG_FORMAT_BUF_SIZE];
      
   if( strerror( errno_saved ) == 0 )
   {
	  Functions::snprintf( postfix, LOG_BUF_SIZE, 
				Functions::convert(fmt_buf, ": errno %d: %s.", LOG_FORMAT_BUF_SIZE), 
                errno_saved, 
				Functions::convert(converted_errno_buf, errno_buf, LOG_STRERRNO_BUF_SIZE));
   }
   else
   {
	  Functions::snprintf( postfix, LOG_BUF_SIZE, 
				Functions::convert(fmt_buf, ": errno %d: strerror errno %d.", 
								   LOG_FORMAT_BUF_SIZE),
                errno_saved, errno );
      errnoval = errno_saved;
   }
   do_log_<CharType, Functions>( level, prefix, postfix, srcfile, srcline, format, ap );
}


#define DEFINE_DO_LOG(name, type, empty_string) \
void name( const int level, const type *prefix, \
             const type *srcfile, const int srcline, \
			 const type *format, ... ) \
{ \
   va_list ap; \
   va_start( ap, format ); \
   do_log_<type, type ## Functions>( level, prefix, empty_string, srcfile, srcline, format, ap ); \
}

#define DEFINE_DO_LOG_ERRNO(name, type) \
void name( const int level, const type *prefix, int &errnoval, \
             const type *srcfile, const int srcline, \
			 const type *format, ... ) \
{ \
   va_list ap; \
   va_start( ap, format ); \
   do_log_errno_<type, type ## Functions>( level, prefix, errnoval, srcfile, srcline, format, ap ); \
}

DEFINE_DO_LOG(do_log_a, char, "")
DEFINE_DO_LOG_ERRNO(do_log_errno_a, char)

#ifdef WIN32

DEFINE_DO_LOG(do_log_w, wchar_t, L"")
DEFINE_DO_LOG_ERRNO(do_log_errno_w, wchar_t)

DEFINE_DO_LOG(do_log_t, TCHAR, _T(""))
DEFINE_DO_LOG_ERRNO(do_log_errno_t, TCHAR)

#else // !WIN32

DEFINE_DO_LOG(do_log_w, char, "")
DEFINE_DO_LOG_ERRNO(do_log_errno_w, char)

DEFINE_DO_LOG(do_log_t, char, "")
DEFINE_DO_LOG_ERRNO(do_log_errno_t, char)

#endif // WIN32

// returns stack trace. Thread-unsafe!
const char *stack_trace() {
#ifndef WIN32
#define ST_MAXSIZE 20
#define ST_BUFSIZE 1024
     static string s;
     s.clear();
     void *stack[ST_MAXSIZE];
     int size = backtrace(stack, ST_MAXSIZE);
     char **stack_symbols = backtrace_symbols(stack, size);
     for(int i = 0; i < size; i++) {
         s.append(stack_symbols[i]);
         s.push_back('\n');
     }
     free(stack_symbols);
     return s.c_str();
#else // WIN32
    log_err("stack trace is not implemented on Windows");
    return strdup("");
#endif

}
