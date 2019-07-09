#ifndef __apptime__
#define __apptime__

_CLASSDEF( TTime )
_CLASSDEF( TBounds )
_CLASSDEF( TDuration )
_CLASSDEF( TWorkTime )
_CLASSDEF( TFreq )
_CLASSDEF( TParse )
_CLASSDEF( TSafStr )

// -------------------------------------------------------------------------
//
//                             Types
//
// -------------------------------------------------------------------------
typedef enum {

	SCALE_HOUR = 0,
	SCALE_DAY,
	SCALE_WEEK,
	SCALE_MONTH,
	SCALE_QUARTER,
	SCALE_YEAR

	} TTimeScale;

typedef enum {

    k1Min,
    k5Min,
    k6Min,
    k10Min,
    k12Min,
    k15Min,
    k20Min,
    k30Min,
    k1Hour,
    k1Day,
    k1Week

    } TTimeUnit, *PTTimeUnit;

typedef enum {

	k1st	= 1,
	k2nd	= 2,
	k3rd	= 3,
	k4th	= 4,
	kLast	= 5

	} TWeekSpecifier;

// -------------------------------------------------------------------------
//
//                             Constants
//
// -------------------------------------------------------------------------
extern const int kMinutesPerTimeUnit[11];

#ifndef __duration__
#include "duration.h"
#endif

class _CLASSTYPE TTime: Sortable, TStreamable {

	friend TBounds;
	friend TDuration;
    friend TWorkTime;
    friend TFreq;
    friend TParse;

public:

	// constructors
    TTime( void );
	TTime( RTTime );
	TTime( RTTime, TTimeScale scale, int offset );
	TTime( RTTime, TTimeUnit  unit,  int offset );
	TTime( RTTime, RTDuration duration );
	TTime( StreamableInit ) {};
    virtual ~TTime( void );

    // setting
    void Clear( void );
    void ClearDate( void );
    void ClearTime( void );
    void CopyDate( RTTime time );
    void CopyTime( RTTime time );

    void SetToMidnight( void );
    void SetToMidnightTomorrow( void );
    void SetNow( void );
	void SetLastDayOfMonth( void );

    // worktime
    void ForwardToWorkTime( void );
    void BackwardToWorkTime( void );
    BOOL IsWorkTime( void );
    BOOL IsBeforeWorkTime( void );
    BOOL IsAfterWorkTime( void );

	void Normalize( TTimeScale scale );
	void Normalize( TTimeUnit unit );

    void PlusEquals( TTimeScale scale, int offset );
    void PlusEquals( TTimeUnit  unit,  int offset );
    void PlusEquals( RTDuration  duration );
    TTime Plus( TTimeScale scale, int offset );
    TTime Plus( TTimeUnit  unit,  int offset );
    TTime Plus( RTDuration  duration );

	void SetDayOfMonth( TWeekSpecifier week, int dayInWeek );
    void SetDate( int day, int mon, int year );
    void SetYear( int year );
    void SetMonth( int mon );
    void SetDay( int day );
    void SetHour( int hour );
    void SetMinute( int min );

    // queries
    BOOL IsSameDay( RTTime time, BOOL checkMidnights = TRUE );
    BOOL IsMidnight( void ) { return itsJulianMinute == 0; };
    BOOL IsWorkDay( void );
    BOOL IsHoliday( void );
	BOOL IsLastDayOfMonth( void );
    BOOL HasTime( void ) const;
	BOOL HasDate( void ) const;
    int  GetWeekDay( void ) const;
	void GetDayOfMonth( TWeekSpecifier &week, int &dayInWeek, BOOL allowLast = TRUE );
	int  LastDayOfMonth( void );

    int  MonthDelta( RTTime end );
    int  WeekDelta(  RTTime end );
    int  DayDelta(   RTTime end );

    int  GetYear( void );
    int  GetMonth( void );
    int  GetDay( void );
    int  GetHour( void );
    int  GetMinute( void );
    static void GetUnitName( TTimeUnit, RTSafStr );

	// operators
    RTTime operator =  (const TTime &t);
	BOOL   operator == (const TTime &a) const;
	BOOL   operator != (const TTime &a) const;
	BOOL   operator >  (const TTime &a) const;
	BOOL   operator >= (const TTime &a) const;
	BOOL   operator <  (const TTime &a) const;
	BOOL   operator <= (const TTime &a) const;

    BOOL   HealthCheck( void );

	// object functions
    classType     isA(         void )            const;
    char         *nameOf(      void )            const;
    hashValueType hashValue(   void )            const;
    int           isEqual(     const Object &)   const;
    int           isLessThan(  const Object &a ) const;
    void          printOn(     ostream &)        const;
    static        PTStreamable build(  );

    void write( Ropstream );
    Pvoid read( Ripstream );

protected:

    const Pchar streamableName() const { return "TTime"; };

    BOOL   itsDateValid;  // TRUE iff date defined
	int    itsDay;
	int    itsMonth;
	int    itsYear;
    long   itsJulianDate;

	BOOL   itsTimeValid;  // TRUE iff time defined
	int    itsHour;
	int    itsMinute;
    long   itsJulianMinute;

private:

    void SetJulians();

    void NormalizeUnit( int &smallerUnit, int &largerUnit, int conversion );
    void Adjust( void );
};

	inline Ripstream operator >> (Ripstream is, RTTime cl)
        { return is >> (RTStreamable) cl; }

	inline Ripstream operator >> (Ripstream is, RPTTime cl)
        { return is >> (RPvoid) cl; }

	inline Ropstream operator << (Ropstream os, RTTime cl)
        { return os << (RTStreamable) cl; }

	inline Ropstream operator << (Ropstream os, PTTime cl)
        { return os << (PTStreamable) cl; }


#endif
