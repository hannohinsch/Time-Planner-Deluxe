#ifndef __worktime__
#define __worktime__

_CLASSDEF( TWorkTime )
_CLASSDEF( TNote )

#ifndef __duration__
#include "duration.h"
#endif

class _CLASSTYPE TWorkTime: public TDuration {

    public:

    TWorkTime( void );
    TWorkTime( RTWorkTime workTime );
    TWorkTime( RTDuration duration );
    TWorkTime( RTTime start, RTTime end );
    TWorkTime( TTimeUnit unit, long numUnits );
    TWorkTime( StreamableInit ) {};
    virtual ~TWorkTime( void );

    long GetNumUnits( TTimeUnit unit );
    void Format( TTimeScale scale, BOOL mixed, char *buf );
    void Format( TTimeUnit unit, BOOL mixed, char *buf );
    void CalculateEnd( RTTime start, RTTime end );
    void CalculateStart( RTTime start, RTTime end );
    void Set( RTTime start, RTTime end );
    void Set( RTDuration duration );
    void Set( TTimeUnit unit, long numUnits );

    private:

    void CalculateShortStart( RTTime start, RTTime end );
    void CalculateLongStart( RTTime start, RTTime end );
    void CalculateShortEnd( RTTime start, RTTime end );
    void CalculateLongEnd( RTTime start, RTTime end );
};


#endif