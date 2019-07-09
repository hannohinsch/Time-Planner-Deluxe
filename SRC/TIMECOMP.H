#ifndef __timecomp
#define __timecomp

#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __duration__
#include "duration.h"
#endif
#ifndef __worktime__
#include "worktime.h"
#endif

_CLASSDEF( TTimeComplex )

class _CLASSTYPE TTimeComplex: public TStreamable {

    public:

    TTimeComplex( void );
	TTimeComplex( StreamableInit );
    virtual ~TTimeComplex( void );

    RTTimeComplex operator =( const RTTimeComplex );
    BOOL ExactlyMatches( const RTTimeComplex );

    void SetStartAndEnd( RTTime start, RTTime time, TTimeUnit unit );
    void SetStartAndKeepDuration( RTTime start );
    void SetDurationAndKeepStart( RTDuration duration );
    void SetStartAndKeepEnd( RTTime start, TTimeUnit unit, int number );
    void SetEndAndKeepStart( RTTime end, TTimeUnit unit, int number );
    void SetStartToStartOfDayAndKeepEnd( void );
    void SetEndToEndOfDayAndKeepStart( void );
    void SetStartToStartOfDayAndKeepDuration( void );
    void SetEndToEndOfDayAndKeepDuration( void );
    void SetStartToJustBeforeEnd( TTimeUnit unit, int number );
    void SetEndToJustAfterStart( TTimeUnit unit, int number );
    void SetTo1DayOn( RTTime day );
    void SetTo2HoursAt( RTTime time );
    void SetDurationFromNote( PTNote noteP, RTTime start );
    void SetStartAndClearDuration( RTTime start );
    void SetFromRawData( RTTime start, RTDuration duration,
            TTimeUnit unit, BOOL isWorkTime );

    void ConstrainToDay( RTTime day );
    void ConstrainStartToDayAndEnd( TTimeUnit unit, int number );
    void ConstrainEndToDayAndStart( TTimeUnit unit, int number );
    void ConstrainStartToTimeAndEnd( RTTime time, TTimeUnit unit, int number );
    void ConstrainEndToTimeAndStart( RTTime time, TTimeUnit unit, int number );
    void ConstrainToInterval( RTTime start1, RTTime end1,
        PTTime start2P = NULL, PTTime end2P = NULL );

    TTime      GetStart( void )    const;
    TTime      GetEnd( void )      const;
    TDuration  GetDuration( void ) const;
    TWorkTime  GetWorkTime( void ) const;
    TTimeUnit  GetTimeUnit( void ) const;
    BOOL       IsWorkTime( void )  const;

    void       SetWorkTime( BOOL worktime );
    void       SetTimeUnit( TTimeUnit unit );

    void       UpdateUnitLength( void );
    BOOL       ModifyForChangedWorkTime( void );

    BOOL       HealthCheck( void );

	// object nonsense
    classType isA( void ) const;
    char *nameOf( void ) const;
    hashValueType hashValue( void ) const;
    void printOn(ostream &) const;
    static PTStreamable build();
    const Pchar streamableName() const { return "TTimeComplex"; };
    void write( Ropstream );
    Pvoid read( Ripstream );


    private:

    void ForwardToWorkTime( RTTime time );
    void BackwardToWorkTime( RTTime time );

    TTime     itsStart;
    TTime     itsEnd;
    TDuration itsDuration;
    TWorkTime itsWorkTime;
    TTimeUnit itsTimeUnit;
    BOOL      itsUseWorkTime;
    long      itsUnitLength;
};


    inline Ripstream operator >> (Ripstream is, RTTimeComplex cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTTimeComplex cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTTimeComplex cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTTimeComplex cl)
        { return os << (PTStreamable) cl; }
#endif
