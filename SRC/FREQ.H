#ifndef __freqtime__
#define __freqtime__

_CLASSDEF( TFreq )
_CLASSDEF( TFrequencyDialog )
_CLASSDEF( TNote )
_CLASSDEF( TXArray )

#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __classid__
#include "classid.h"
#endif

// NB this enum is saved in file; don't resequence!
typedef enum {

	kOnce,
	kDaysApart,               	// eg. every 2 days
	kWeeksApart,              	// eg. every 2 weeks
	kMonthsApartOnDayNumber,	// eg. every 2 months on the 5 of the month
	kMonthsApartOnLastDay,		// eg. every 2 months on the last day of the month
    kMonthsApartOnDayName,      // eg. every 2 months on the 2nd Tuesday of the month
    kTueThu,                    // eg. every Tuesday/Thursday
    kMonWedFri                  // eg. every Monday/Wednesday/Friday

	} TFrequencyType;


class _CLASSTYPE TFreq: public TStreamable {

friend TFrequencyDialog;

public:

    typedef enum {

        kNormalCell,
        kStartCell,
        kSecondCell,
        kFrequencyCell,
        kEndCell,
        kAdditionCell,
        kExcludeCell

    } TCellType;

    TFreq( void );
    TFreq( RTTime startDate );
    TFreq( RTFreq freq );
	TFreq( StreamableInit )        { };
    virtual ~TFreq( void );

    void Clear( void );

    void InferFrequency( PTWindowsObject parent, RTTime firstDate, RTTime secondDate );
    void First( RTTime first );
    void Last( RTTime last ) { last = itsEndDate; };
    BOOL Next( RTTime next );
    BOOL NextDisplay( RTTime next, TCellType &type );

    void AddDate( RTTime date );
    void ExcludeDate( RTTime date );
    BOOL IsExclusion( RTTime date );
    BOOL IsAddition( RTTime date );
    void ModifyForAddition( RTTime date );
    void GetStart( RTTime start );
    void SetTime(  RTTime time );

    void GetFreqText( char *s );
    BOOL IsRealFreq( void ) { return itsFrequencyType != kOnce; };
    RTFreq operator =( RTFreq rhs );

    // basic object functions
    classType     isA( void )                     const { return IDFreq; };
    char         *nameOf( void )                  const { return "TFreq"; };
    hashValueType hashValue( void )               const { return 0; };
    void          printOn(ostream &)              const { }
    const         Pchar streamableName()          const { return "TFreq"; };
    static  PTStreamable  build( void )                         { return( new TFreq( streamableInit )); };

    void          write( Ropstream );
    Pvoid         read( Ripstream );

private:

    BOOL   operator ==( RTFreq rhs );

    BOOL NextSub( RTTime next );

    TTime           itsStartDate;
    TTime           itsEndDate;
    TTime           itsCurrentDate;
    TTime           itsLastAdditionDate;

    // frequency information
    TFrequencyType  itsFrequencyType;
    short           itsFrequency;
    TWeekSpecifier  itsFrequencyWeek;
    short           itsFrequencyDay;

    PTXArray        itsAdditionsP;
    PTXArray        itsExclusionsP;


};

    inline Ripstream operator >> (Ripstream is, RTFreq cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTFreq cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTFreq cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTFreq cl)
        { return os << (PTStreamable) cl; }


#endif


