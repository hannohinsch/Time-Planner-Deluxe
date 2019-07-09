#ifndef __error__
#define __error__

_CLASSDEF( TError )

#ifndef __profile__
#include "profile.h"
#endif

// system error handler macros (used to get file and line info )

// syserr puts up messagebox, then continues
#define SYSERR( errnum ) (TError::SysError( errnum, __FILE__, __LINE__ ))

// syserr_str puts up messagebox with a message, not a number, then continues
#define SYSERR_STR( message ) (TError::SysErrorStr( message, __FILE__, __LINE__ ))

//systhrow does immediate longjmp to message loop, then puts up message box
#define SYSTHROW( errnum ) (TError::SysThrow( errnum, __FILE__, __LINE__ ))

//reasonably fast tests
#define ASSERT( errnum, condition )  {if( TProfile::debugLevel > 0 && !(condition) ) SYSERR( errnum ); else ; }
#define ASSERT_STR( message, condition )  {if( TProfile::debugLevel > 0 && !(condition) ) SYSERR_STR( message ); else ; }

//really slow tests...
#define CHKBUG( errnum, condition )  {if( TProfile::debugLevel > 1 && !(condition) ) SYSERR( errnum ); else ; }

typedef enum
{
    ERR_GEN,         //generic app err
    ERR_NEW,         //new() returned NULL
    ERR_LOWMEM,      //safety pool exhausted
    ERR_TIMER,       //couldn't allocate timer
    ERR_CAT,         //cat not found in global tree
    ERR_BADCAT,      //on file read, cat didn't pass sanity check
    ERR_VER,         //on file read, encountered bad version of object
    ERR_CHK,         //checksum error on safstr read
    ERR_ASSIGN,      //using implicit assignment operator
    ERR_GDI,         //unable to create or select GDI object
    ERR_RES,         //unable to load resource
    ERR_FILE,        //generic file op error
    ERR_OBJSTRM,     //objstrm.cpp error
    ERR_NULLPTR,     //null ptr found
    ERR_HEALTH,      //object healthcheck failed
    ERR_VERSION,     //file version error
    ERR_HEADER       //file header error

} TErrCode;

class _CLASSTYPE TError
{
public:
    static void SysError(         TErrCode, char * errFile, int errLine);
    static void SysThrow(         TErrCode, char * errFile, int errLine );
    static void SysErrorStr( char *message, char * errFile, int errLine);
    static void ProcessSysCatch( void );

    static LPCATCHBUF SetCatchBuf( LPCATCHBUF pBuf );

    static BOOL MemOK( BOOL bTell );

private:
    static TErrCode     itsErrNum;
    static char *       itsErrFile;
    static int          itsErrLine;
    static LPCATCHBUF   itsCatchBufP;
    static BOOL         itsMemMessageB;
};

#endif
