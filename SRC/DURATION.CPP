//*****************************************************************************
// manipulate the duration between two times
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "duration.h"
#ifndef __error__
#include "error.h"
#endif
#ifndef __profile__
#include "profile.h"
#endif
#ifndef __utility__
#include "utility.h"
#endif

static const int kMinutesPerHour = 60;
static const int kMinutesPerDay  = 60 * 24;
static const int kMinutesPerWeek = 60 * 24 * 7;
static const int kHoursPerDay    = 24;
static const int kHoursPerWeek   = 24 * 7;
static const int kDaysPerWeek    = 7;

// *************************************************************************
//
//                          Constructors
//
// *************************************************************************

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
TDuration::TDuration( void )
{
    Clear();
}

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
TDuration::TDuration( RTDuration old )
{
    (*this) = old;
}

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
TDuration::TDuration( TTimeUnit unit, long numUnits )
{
    (*this).Set( unit, numUnits );
}

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
TDuration::TDuration( RTTime start, RTTime end )
{
    (*this).Set( start, end );
}

// -------------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------------
TDuration::~TDuration( void )
{
}

// *************************************************************************
//
//                          Setting
//
// *************************************************************************

// -------------------------------------------------------------------------
// Clear
// -------------------------------------------------------------------------
void TDuration::Clear( void )
{
    itsMinutes = 0;
}

// -------------------------------------------------------------------------
// Set from time units
// -------------------------------------------------------------------------
void TDuration::Set( TTimeUnit unit, long numUnits )
{
    itsMinutes = kMinutesPerTimeUnit[unit] * numUnits;
}

// -------------------------------------------------------------------------
// Set from start and end
// -------------------------------------------------------------------------
void TDuration::Set( RTTime start, RTTime end )
{
    // items, floats milestones don't have durations
    if (!(start.HasDate() && end.HasDate())) {

        itsMinutes   = 0;
    }

    else if (start.HasTime()) {

        long days    = end.itsJulianDate   - start.itsJulianDate;
        long minutes = end.itsJulianMinute - start.itsJulianMinute;
        minutes     += days * kMinutesPerDay;

        itsMinutes   = minutes;
    }

    // if there are no times, add 1, since oct 5 to oct 5 is supposed
    // to be 1 day
    else {

        long days    = end.itsJulianDate - start.itsJulianDate + 1;
        itsMinutes   = days * kMinutesPerDay;
    }
}



// -------------------------------------------------------------------------
// Normalize
// -------------------------------------------------------------------------
void TDuration::Normalize( TTimeUnit unit )
{
    itsMinutes = (itsMinutes / kMinutesPerTimeUnit[unit]) * kMinutesPerTimeUnit[unit];
}

// -------------------------------------------------------------------------
// PlusEquals
// -------------------------------------------------------------------------
void TDuration::PlusEquals( TTimeUnit unit, long numUnits )
{
    itsMinutes += kMinutesPerTimeUnit[unit] * numUnits;
}

// -------------------------------------------------------------------------
// PlusEquals with duration
// -------------------------------------------------------------------------
void TDuration::PlusEquals( RTDuration duration )
{
    itsMinutes += duration.itsMinutes;
}

// *************************************************************************
//
//                          Queries
//
// *************************************************************************

// -------------------------------------------------------------------------
// IsZero
// -------------------------------------------------------------------------
BOOL TDuration::IsZero( void )
{
    return (itsMinutes == 0);
}

// -------------------------------------------------------------------------
// GetNumUnits
// -------------------------------------------------------------------------
long TDuration::GetNumUnits( TTimeUnit unit )
{
    return itsMinutes / kMinutesPerTimeUnit[unit];
}


// *************************************************************************
//
//                          Operators
//
// *************************************************************************

// -------------------------------------------------------------------------
// operator =
// -------------------------------------------------------------------------
RTDuration TDuration::operator=( RTDuration rhs )
{
    itsMinutes = rhs.itsMinutes;

    return (*this);
}

// -------------------------------------------------------------------------
// operator -
// -------------------------------------------------------------------------
TDuration TDuration::operator-( void )
{
    TDuration duration;

    duration.itsMinutes = -itsMinutes;

    return (duration);
}

// -------------------------------------------------------------------------
// operator ==
// -------------------------------------------------------------------------
BOOL TDuration::operator==( RTDuration rhs ) const
{
    return (itsMinutes == rhs.itsMinutes);
}

// -------------------------------------------------------------------------
// operator <
// -------------------------------------------------------------------------
BOOL TDuration::operator<( RTDuration rhs ) const
{
    return (itsMinutes < rhs.itsMinutes);
}

// -------------------------------------------------------------------------
// Format
// -------------------------------------------------------------------------
void TDuration::Format( TTimeUnit unit, BOOL mixed, char *buf )
{
    if (unit <= k1Hour) {

        Format( SCALE_HOUR, mixed, buf );
    }
    else if (unit == k1Day) {

        Format( SCALE_DAY, mixed, buf );
    }
    else if (unit == k1Week) {

        Format( SCALE_WEEK, mixed, buf );
    }
}


// -------------------------------------------------------------------------
// Format
// -------------------------------------------------------------------------
void TDuration::Format( TTimeScale scale, BOOL mixed, char *buf )
{
    long minutes = itsMinutes;

    if (mixed) {

        long m, h, d, w;
        switch( scale ) {

            case SCALE_HOUR:

                h = minutes / kMinutesPerHour;
                m = minutes % 60;

                if (m > 0) {

                    wsprintf( buf, "%ld:%0.2ld h", h, m );
                }
                else {

                    wsprintf( buf, "%ld h", h );
                }
                break;

            case SCALE_DAY:

                d =  minutes / kMinutesPerDay;
                h = (minutes % kMinutesPerDay) / kMinutesPerHour;
                m = (minutes % kMinutesPerDay) % kMinutesPerHour;

                if (m > 0) {

                    wsprintf( buf, "%ld d %ld:%0.2ld h", d, h, m );
                }
                else if (h > 0) {

                    wsprintf( buf, "%ld d %ld h", d, h );
                }
                else {

                    wsprintf( buf, "%ld d", d );
                }
                break;

            case SCALE_WEEK:

                w =  minutes / kMinutesPerWeek;
                d = (minutes % kMinutesPerWeek) / kMinutesPerDay;
                h = (minutes % kMinutesPerDay)  / kMinutesPerHour;
                m =  minutes % kMinutesPerHour;

                if (m > 0) {

                    wsprintf( buf, "%ld w %ld d %ld:%0.2ld h", w, d, h, m );
                }
                else if (h > 0) {

                    wsprintf( buf, "%ld w %ld d %ld h", w, d, h );
                }
                else if (d > 0) {

                    wsprintf( buf, "%ld w %ld d", w, d );
                }
                else {

                    wsprintf( buf, "%ld w", w );
                }
                break;
        }
    }

    // not mixed
    else {

        double length;

        switch( scale ) {

            case SCALE_HOUR:

                length = ((double) minutes) / kMinutesPerHour;
                break;

            case SCALE_DAY:

                length = ((double) minutes) / kMinutesPerDay;
                break;

            case SCALE_WEEK:

                length = ((double) minutes) / kMinutesPerWeek;
                break;
        }

        static const char abbreviations[3] = { 'h', 'd', 'w' };
        char rawBuf[80];

        FormatDouble( length, rawBuf );
        wsprintf( buf, "%s %c", rawBuf, abbreviations[scale] );
    }
}



// -------------------------------------------------------------------------
// Read
// -------------------------------------------------------------------------
TStreamableClass RegTDuration( "TDuration", TDuration::build, __DELTA(TDuration) );
Pvoid TDuration::read( Ripstream is )
{
    int version;

    is >> version;
    if (version == 2) {

        is >> itsMinutes;
    }
    else {

        BOOL       valid;
        TTimeScale scale;
        double     length;
        is >> valid;
        is >> (int) scale;
        is >> length;

        if( !valid ) {

            itsMinutes= 0;
        }
        else {

            switch( scale ) {

                case SCALE_HOUR:

                    itsMinutes = length * kMinutesPerHour;
                    break;

                case SCALE_DAY:

                    itsMinutes = length * kMinutesPerDay;
                    break;

                case SCALE_WEEK:

                    itsMinutes = length * kMinutesPerWeek;
                    break;

                default:

                    SYSERR_STR( "Read: unexpected case" );

            }
        }
    }

    return this;
}

// -------------------------------------------------------------------------
// Write
//
// version 1: itsValid, itsTimeScale, itsLength
// version 2: itsMinutes
// -------------------------------------------------------------------------
void TDuration::write( Ropstream os )
{
    os << (int) 2;  //object version
    os << itsMinutes;
}


// -------------------------------------------------------------------------
// HealthCheck
//
// Return TRUE if it's ok
// -------------------------------------------------------------------------
BOOL TDuration::HealthCheck( void )
{
    return (0 <= itsMinutes);
}


