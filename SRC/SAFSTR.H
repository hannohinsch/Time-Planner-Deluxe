#ifndef __safstr__
#define __safstr__

_CLASSDEF(TSafStr)
_CLASSDEF(TSafStrIter)

#ifndef __error__
#include "error.h"
#endif

//KSTR is an alias for TSafStr, to be used for declaring
//string constants.  The idea is to initialize them with strings
//when first programming, then go back and convert them to
//resource ids for production.  The KSTR moniker makes them
//easier to find.
//
#define KSTR TSafStr
#define KSET Set
//

//
//cast safstr to char *  (needed all over, cause library routines
// typically aren't declared correctly with const)
//
#define SAF2CP(s)   ((char *)( const char *)(s))

class _CLASSTYPE TSafStr: public Sortable, TStreamable
{
friend TSafStrIter;

public:
	TSafStr( void );
	TSafStr( const char * );
    TSafStr( const TSafStr& );
    TSafStr( const WORD );
    TSafStr( StreamableInit ) {};
    virtual ~TSafStr();

    void TSafStr::StreamFix( void );

    // ptr may be NULL!
    const char * Addr( void );

    //itsLength may be 0; hence return -1 !
    sizeType Len( void ) const;

    void Get( char *p, int n ) const;

    void Set( const TSafStr & );
    void Set( const char * );
    void Set( const WORD );

    void Cat( const TSafStr & );
    void Cat( const char );
    void Cat( const char *p );

    void ToLower( void );
    void ToUpper( void );

    int Printf( const char *fmt, ... );

    void Clear( void );

    BOOL Contains( const char *lookFor ) const;
    BOOL IsBlank( void ) const;
    BOOL HealthCheck( void ) const;

    //cast to const char *
    operator const char *() const;

    BOOL operator == (const TSafStr &a) const;
    BOOL operator != (const TSafStr &a) const;
    BOOL operator > (const TSafStr &a) const;
    BOOL operator >= (const TSafStr &a) const;
    BOOL operator < (const TSafStr &a) const;
    BOOL operator <= (const TSafStr &a) const;

    BOOL operator == (const char *a) const;
    BOOL operator != (const char *a) const;
    BOOL operator > (const char *a) const;
    BOOL operator >= (const char *a) const;
    BOOL operator < (const char *a) const;
    BOOL operator <= (const char *a) const;

    TSafStr &operator =( const TSafStr &other );

    int isEqual( const Object & ) const;
    int isLessThan( const Object & ) const;
    classType isA() const;
    char *nameOf() const;
    hashValueType hashValue() const;
    void printOn( ostream& ) const;

    static PTStreamable build();

    void write( Ropstream );
    Pvoid read( Ripstream );

private:
    const Pchar streamableName() const { return "TSafStr"; };

    int OurCmp( const char *a, const char *b) const;

    static char     itsScratchBuffer[ 1024 ];
    static char     itsNullString[1];
    sizeType        itsLength;
    char *          itsStringP;
    hashValueType   itsCheckSum;

};


//*****************************************************************************
// iterator class for strings -- properly handles two-byte chars
// (just in case we ever go Japanese)
//*****************************************************************************
class _CLASSTYPE TSafStrIter
{
public:
    TSafStrIter( const TSafStr &a ) { itsP= itsStartP= a.itsStringP; };
    virtual ~TSafStrIter() {};

    operator const char *() const { return itsP; };

    const char * First( void ) { itsP= itsStartP; return itsP; };

    const char * Next( void ) { itsP= AnsiNext( itsP ); return itsP; };
    const char * operator ++() { itsP= AnsiNext( itsP ); return itsP; };              //++p
    const char * operator ++(int) { char *oldP= itsP; itsP= AnsiNext( itsP ); return oldP; };    //p++

    const char * Prev( void ) { itsP= AnsiPrev( itsStartP, itsP ); return itsP; };
    const char * operator --() { itsP= AnsiPrev( itsStartP, itsP ); return itsP; };     //++p
    const char * operator --(int) { char *oldP= itsP; itsP= AnsiPrev( itsStartP, itsP ); return oldP; };    //p++


private:
    char *itsP;
    char *itsStartP;

};

    inline Ripstream operator >> (Ripstream is, RTSafStr cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTSafStr cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTSafStr cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTSafStr cl)
        { return os << (PTStreamable) cl; }

#endif
