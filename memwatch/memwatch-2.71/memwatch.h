/*
** MEMWATCH.H
** Nonintrusive ANSI C memory leak / overwrite detection
** Copyright (C) 1992-2002 Johan Lindh
** All rights reserved.
** Version 2.71
**
************************************************************************
**
** PURPOSE:
**
**  MEMWATCH has been written to allow guys and gals that like to
**  program in C a public-domain memory error control product.
**  I hope you'll find it's as advanced as most commercial packages.
**  The idea is that you use it during the development phase and
**  then remove the MEMWATCH define to produce your final product.
**  MEMWATCH is distributed in source code form in order to allow
**  you to compile it for your platform with your own compiler.
**  It's aim is to be 100% ANSI C, but some compilers are more stingy
**  than others. If it doesn't compile without warnings, please mail
**  me the configuration of operating system and compiler you are using
**  along with a description of how to modify the source, and the version
**  number of MEMWATCH that you are using.
**
************************************************************************
**
**  The author may be reached by e-mail at the address below. If you
**  mail me about source code changes in MEMWATCH, remember to include
**  MW's version number.
**
**      Johan Lindh
**      johan@linkdata.se
**
** The latest version of MEMWATCH may be downloaded from
** http://www.linkdata.se/
*/

#ifndef __MEMWATCH_H
#define __MEMWATCH_H

/* Make sure that malloc(), realloc(), calloc() and free() are declared. */
/*lint -save -e537 */
#include <stdlib.h>
/*lint -restore */

#ifdef __cplusplus
extern "C" {
#endif

/*
** Constants used
**  All MEMWATCH constants start with the prefix MW_, followed by
**  a short mnemonic which indicates where the constant is used,
**  followed by a descriptive text about it.
*/

#define MW_ARI_NULLREAD 0x10    /* Null read (to start debugger) */
#define MW_ARI_ABORT    0x04    /* ARI handler says: abort program! */
#define MW_ARI_RETRY    0x02    /* ARI handler says: retry action! */
#define MW_ARI_IGNORE   0x01    /* ARI handler says: ignore error! */

#define MW_VAL_NEW      0xFE    /* value in newly allocated memory */
#define MW_VAL_DEL      0xFD    /* value in newly deleted memory */
#define MW_VAL_NML      0xFC    /* value in no-mans-land */
#define MW_VAL_GRB      0xFB    /* value in grabbed memory */

#define MW_TEST_ALL     0xFFFF  /* perform all tests */
#define MW_TEST_CHAIN   0x0001  /* walk the heap chain */
#define MW_TEST_ALLOC   0x0002  /* test allocations & NML guards */
#define MW_TEST_NML     0x0004  /* test all-NML areas for modifications */

#define MW_NML_NONE     0       /* no NML */
#define MW_NML_FREE     1       /* turn FREE'd memory into NML */
#define MW_NML_ALL      2       /* all unused memory is NML */
#define MW_NML_DEFAULT  0       /* the default NML setting */

#define MW_STAT_GLOBAL  0       /* only global statistics collected */
#define MW_STAT_MODULE  1       /* collect statistics on a module basis */
#define MW_STAT_LINE    2       /* collect statistics on a line basis */
#define MW_STAT_DEFAULT 0       /* the default statistics setting */

/*
** MemWatch internal constants
**  You may change these and recompile MemWatch to change the limits
**  of some parameters. Respect the recommended minimums!
*/
#define MW_TRACE_BUFFER 2048    /* (min 160) size of mwTrace()'s output buffer */
#define MW_FREE_LIST    64      /* (min 4) number of free()'s to track */


/*
** System functions
**  Normally, it is not nessecary to call any of these. MEMWATCH will
**  automatically initialize itself on the first MEMWATCH function call,
**  and set up a call to mwAbort() using atexit(). Some C++ implementations
**  run the atexit() chain before the program has terminated, so you
**  may have to use mwInit() or the MemWatch C++ class to get good
**  behaviour.
**  - mwInit() can be called to disable the atexit() usage. If mwInit()
**      is called directly, you must call mwTerm() to end MemWatch, or
**      mwAbort().
**  - mwTerm() is usually not nessecary to call; but if called, it will
**      call mwAbort() if it finds that it is cancelling the 'topmost'
**      mwInit() call.
**  - mwAbort() cleans up after MEMWATCH, reports unfreed buffers, etc.
*/
void  mwInit( void );
void  mwTerm( void );
void  mwAbort( void );
void  mwStatReport( void );


/*
** Setup functions
**  These functions control the operation of MEMWATCH's protective features.
**  - mwFlushNow() causes MEMWATCH to flush it's buffers.
**  - mwDoFlush() controls whether MEMWATCH flushes the disk buffers after
**      writes. The default is smart flushing: MEMWATCH will not flush buffers
**      explicitly until memory errors are detected. Then, all writes are
**      flushed until program end or mwDoFlush(0) is called.
**  - mwLimit() sets the allocation limit, an arbitrary limit on how much
**      memory your program may allocate in bytes. Used to stress-test app.
**      Also, in virtual-memory or multitasking environs, puts a limit on
**      how much MW_NML_ALL can eat up.
**  - mwGrab() grabs up X kilobytes of memory. Allocates actual memory,
**      can be used to stress test app & OS both.
**  - mwDrop() drops X kilobytes of grabbed memory.
**  - mwNoMansLand() sets the behaviour of the NML logic. See the
**      MW_NML_xxx for more information. The default is MW_NML_DEFAULT.
**  - mwStatistics() sets the behaviour of the statistics collector. See
**      the MW_STAT_xxx defines for more information. Default MW_STAT_DEFAULT.
**  - mwFreeBufferInfo() enables or disables the tagging of free'd buffers
**      with freeing information. This information is written in text form,
**      using sprintf(), so it's pretty slow. Disabled by default.
**  - mwAutoCheck() performs a CHECK() operation whenever a MemWatch function
**      is used. Slows down performance, of course.
**  - mwCalcCheck() calculates checksums for all data buffers. Slow!
**  - mwDumpCheck() logs buffers where stored & calc'd checksums differ. Slow!!
**  - mwMark() sets a generic marker. Returns the pointer given.
**  - mwUnmark() removes a generic marker. If, at the end of execution, some
**      markers are still in existence, these will be reported as leakage.
**      returns the pointer given.
*/
void        mwFlushNow( void );
void        mwDoFlush( int onoff );
void        mwLimit( long bytes );
unsigned    mwGrab( unsigned kilobytes );
unsigned    mwDrop( unsigned kilobytes );
void        mwNoMansLand( int mw_nml_level );
void        mwStatistics( int level );
//void        mwFreeBufferInfo( int onoff );
void        mwAutoCheck( int onoff );
//void        mwCalcCheck( void );
//void        mwDumpCheck( void );
void *      mwMark( void *p, const char *description, const char *file, unsigned line );
void *      mwUnmark( void *p, const char *file, unsigned line );

/*
** Testing/verification/tracing
**  All of these macros except VERIFY() evaluates to a null statement
**  if MEMWATCH is not defined during compilation.
**  - mwIsReadAddr() checks a memory area for read privilige.
**  - mwIsSafeAddr() checks a memory area for both read & write privilige.
**      This function and mwIsReadAddr() is highly system-specific and
**      may not be implemented. If this is the case, they will default
**      to returning nonzero for any non-NULL pointer.
**  - CHECK() does a complete memory integrity test. Slow!
**  - CHECK_THIS() checks only selected components.
**  - CHECK_BUFFER() checks the indicated buffer for errors.
**  - mwASSERT() or mwASSERT() If the expression evaluates to nonzero, execution continues.
**      Otherwise, the ARI handler is called, if present. If not present,
**      the default ARI action is taken (set with mwSetAriAction()).
**      mwASSERT() can be disabled by defining MW_NOASSERT.
**  - mwVERIFY() or VERIFY() works just like mwASSERT(), but when compiling without
**      MEMWATCH the macro evaluates to the expression.
**      VERIFY() can be disabled by defining MW_NOVERIFY.
**  - mwTRACE() or mwTrace() writes some text and data to the log. Use like printf().
**      mwTrace() can be disabled by defining MW_NOTRACE.
*/
int   mwIsReadAddr( const void *p, unsigned len );
int   mwIsSafeAddr( void *p, unsigned len );
int   mwTest( const char *file, int line, int mw_test_flags );
int   mwTestBuffer( const char *file, int line, void *p );
int   mwAssert( int, const char*, const char*, int );
int   mwVerify( int, const char*, const char*, int );

/*
** User I/O functions
**  - mwTrace() works like printf(), but dumps output either to the
**      function specified with mwSetOutFunc(), or the log file.
**  - mwPuts() works like puts(), dumps output like mwTrace().
**  - mwSetOutFunc() allows you to give the adress of a function
**      where all user output will go. (exeption: see mwSetAriFunc)
**      Specifying NULL will direct output to the log file.
**  - mwSetAriFunc() gives MEMWATCH the adress of a function to call
**      when an 'Abort, Retry, Ignore' question is called for. The
**      actual error message is NOT printed when you've set this adress,
**      but instead it is passed as an argument. If you call with NULL
**      for an argument, the ARI handler is disabled again. When the
**      handler is disabled, MEMWATCH will automatically take the
**      action specified by mwSetAriAction().
**  - mwSetAriAction() sets the default ARI return value MEMWATCH should
**      use if no ARI handler is specified. Defaults to MW_ARI_ABORT.
**  - mwAriHandler() is an ANSI ARI handler you can use if you like. It
**      dumps output to stderr, and expects input from stdin.
**  - mwBreakOut() is called in certain cases when MEMWATCH feels it would
**      be nice to break into a debugger. If you feel like MEMWATCH, place
**      an execution breakpoint on this function.
*/
void  mwTrace( const char* format_string, ... );
void  mwPuts( const char* text );
void  mwSetOutFunc( void (*func)(int) );
void  mwSetAriFunc( int (*func)(const char*) );
void  mwSetAriAction( int mw_ari_value );
int   mwAriHandler( const char* cause );
void  mwBreakOut( const char* cause );

/*
** Allocation/deallocation functions
**  These functions are the ones actually to perform allocations
**  when running MEMWATCH, for both C and C++ calls.
**  - mwMalloc() debugging allocator
**  - mwMalloc_() always resolves to a clean call of malloc()
**  - mwRealloc() debugging re-allocator
**  - mwRealloc_() always resolves to a clean call of realloc()
**  - mwCalloc() debugging allocator, fills with zeros
**  - mwCalloc_() always resolves to a clean call of calloc()
**  - mwFree() debugging free. Can only free memory which has
**      been allocated by MEMWATCH.
**  - mwFree_() resolves to a) normal free() or b) debugging free.
**      Can free memory allocated by MEMWATCH and malloc() both.
**      Does not generate any runtime errors.
*/
void* mwMalloc( size_t, const char*, int );
void* mwMalloc_( size_t );
void* mwRealloc( void *, size_t, const char*, int );
void* mwRealloc_( void *, size_t );
void* mwCalloc( size_t, size_t, const char*, int );
void* mwCalloc_( size_t, size_t );
void  mwFree( void*, const char*, int );
void  mwFree_( void* );
char* mwStrdup( const char *, const char*, int );

/*
** Enable/disable precompiler block
**  This block of defines and if(n)defs make sure that references
**  to MEMWATCH is completely removed from the code if the MEMWATCH
**  manifest constant is not defined.
*/

#define mwASSERT(exp)   while(mwAssert((int)(exp),#exp,__FILE__,__LINE__))

#define mwVERIFY(exp)   while(mwVerify((int)(exp),#exp,__FILE__,__LINE__))
#ifndef VERIFY
#define VERIFY          mwVERIFY
#endif /* !VERIFY */

/* some compilers use a define and not a function */
/* for strdup(). */
#ifdef strdup
#undef strdup
#endif

#define vos_malloc(n)       mwMalloc(n,__FILE__,__LINE__)
#define vos_strdup(p)       mwStrdup(p,__FILE__,__LINE__)
#define vos_realloc(p,n)    mwRealloc(p,n,__FILE__,__LINE__)
#define vos_calloc(n,m)     mwCalloc(n,m,__FILE__,__LINE__)
#define vos_free(p)         mwFree(p,__FILE__,__LINE__)

#define CHECK()         mwTest(__FILE__,__LINE__,MW_TEST_ALL)
#define CHECK_THIS(n)   mwTest(__FILE__,__LINE__,n)
#define CHECK_BUFFER(b) mwTestBuffer(__FILE__,__LINE__,b)
#define MARK(p)         mwMark(p,#p,__FILE__,__LINE__)
#define UNMARK(p)       mwUnmark(p,__FILE__,__LINE__)

#ifdef __cplusplus
    }
#endif


#endif /* __MEMWATCH_H */

/* EOF MEMWATCH.H */
