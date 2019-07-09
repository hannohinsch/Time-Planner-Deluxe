#ifndef __holiday__
#define __holiday__

_CLASSDEF( THoliday )
_CLASSDEF( THolidaySet )
_CLASSDEF( THolidayDialog )
_CLASSDEF( TXArray )

#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __classid__
#include "classid.h"
#endif

typedef enum {

    kNone,
    kUSA,
    kCanada

    } TCountry;

typedef enum {

    kDate,
    kDay

    } THolidayType;

typedef struct {

    THolidayType   type;
    int            day;
    TWeekSpecifier week;
    char           name[256];

    } TFixedHoliday;

typedef struct {

    int            holidaysInMonth;
    TFixedHoliday  *holidays;

    } THolidaySlot;

typedef THolidaySlot TCountryHolidayTable[13];
typedef TCountryHolidayTable THolidayTable[3];



// ------------------------------------------------------------------------
// Holiday
// ------------------------------------------------------------------------
class _CLASSTYPE THoliday: public Sortable, public TStreamable  {

    friend THolidaySet;

    public:

    THoliday( void );
    THoliday( RTHoliday );
    THoliday( RTTime date, RTSafStr name );
	THoliday( StreamableInit )        { };
    virtual ~THoliday( void )         { };

    BOOL operator == ( RTHoliday other ) { return ( itsDate == other.itsDate ); };
    BOOL operator <  ( RTHoliday other ) { return ( itsDate <  other.itsDate ); };

    // basic object functions
    classType     isA( void )                     const { return IDHoliday; };
    char         *nameOf( void )                  const { return "THoliday"; };
    hashValueType hashValue( void )               const { return 0; };
    int           isEqual(const Object &obj)      const { return( this == & obj); };
    int           isLessThan( const Object &obj ) const { return( this < &obj); };
    void          printOn(ostream &)              const { }
    const         Pchar streamableName()          const { return "THoliday"; };
    static  PTStreamable  build( void )                         { return( new THoliday( streamableInit )); };

    void          write( Ropstream );
    Pvoid         read( Ripstream );

    void GetDate( RTTime date   )  { date = itsDate; };
    void GetName( RTSafStr name )  { name = itsName; };

    private:

    TTime     itsDate;
    TSafStr   itsName;

};

    inline Ripstream operator >> (Ripstream is, RTHoliday cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTHoliday cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTHoliday cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTHoliday cl)
        { return os << (PTStreamable) cl; }

// ------------------------------------------------------------------------
// HolidaySet
// ------------------------------------------------------------------------
class _CLASSTYPE THolidaySet
{
    public:

    THolidaySet( void );
    virtual ~THolidaySet();

    static BOOL IsHoliday( RTTime date );

    static int  GetNumHolidays( void );
    static BOOL GetHoliday( int number, RTHoliday holiday );
    static BOOL GetHoliday( RTTime, RTHoliday holiday );
    static BOOL GetFixedHoliday( RTTime, RTHoliday holiday );

    static BOOL GetFirstFixedHoliday( RTTime start, RTTime end, RTTime holidate );
    static BOOL GetNextFixedHoliday( RTTime end, RTTime holidate );

    static TCountry Country( void ) { return itsCountry; };
    static void SetCountry( TCountry country ) { itsCountry = country; };

    static void Add( RTHoliday );
    static void Delete( RTHoliday );
    static void Flush( void );

    private:

    static BOOL IsFixedHoliday( RTTime date );
    static void GetNthFixedHoliday( int month, int year, int n, RTTime date );
    static BOOL MatchNthFixedHoliday( RTTime date, int n );

    static PTXArray       itsHolidaysP;
    static THolidayTable itsFixedHolidays;
    static TCountry      itsCountry;
    static TTime         itsCurrentDate;

};

#endif
