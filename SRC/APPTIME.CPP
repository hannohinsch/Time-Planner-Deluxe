//*****************************************************************************
// our time object
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "apptime.h"
#include <time.h>

#ifndef __classid__
#include "classid.h"
#endif
#ifndef __basecal__
#include "basecal.h"
#endif
#ifndef __error__
#include "error.h"
#endif
#ifndef __profile__
#include "profile.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __holiday__
#include "holiday.h"
#endif

const int kMinutesPerTimeUnit[11] = { 1, 5, 6, 10, 12, 15, 20, 30, 60, 1440, 10080 };

// ***********************************************************************
// ******************                                   ******************
// ******************         constructors              ******************
// ******************                                   ******************
// ***********************************************************************

// -----------------------------------------------------------------------
// constructor
// -----------------------------------------------------------------------
TTime::TTime( void )
{
    Clear(); 
}

// -----------------------------------------------------------------------
// constructor
// -----------------------------------------------------------------------
TTime::TTime( RTTime time )
{
    (*this) = time;
}

// -----------------------------------------------------------------------
// constructor
// -----------------------------------------------------------------------
TTime::TTime( RTTime time, TTimeScale scale, int offset )
{
    (*this) = time;
    PlusEquals( scale, offset );
}

// -----------------------------------------------------------------------
// constructor
// -----------------------------------------------------------------------
TTime::TTime( RTTime time, TTimeUnit unit, int offset )
{
    (*this) = time;
    PlusEquals( unit, offset );
}

// -----------------------------------------------------------------------
// constructor
// -----------------------------------------------------------------------
TTime::TTime( RTTime time, RTDuration duration )
{
    (*this) = time;
    PlusEquals( duration );
}

// -----------------------------------------------------------------------
// destructor
// -----------------------------------------------------------------------
TTime::~TTime( void )
{
}
// ***********************************************************************
// ******************                                   ******************
// ******************          set functions            ******************
// ******************                                   ******************
// ***********************************************************************

// 	----------------------------------------------------------------------------
// Clear
// 	----------------------------------------------------------------------------
void TTime::Clear( void )
{
    ClearDate();
    ClearTime();
}

// 	----------------------------------------------------------------------------
// ClearDate
// 	----------------------------------------------------------------------------
void TTime::ClearDate( void )
{
    itsDateValid     = FALSE;
    itsDay           = 0;
    itsMonth         = 0;
    itsYear          = 0;
    itsJulianDate    = 0;
}

// 	----------------------------------------------------------------------------
// ClearTime
// 	----------------------------------------------------------------------------
void TTime::ClearTime( void )
{
    itsTimeValid    = FALSE;
    itsHour         = 0;
    itsMinute       = 0;
    itsJulianMinute = 0;
}

// 	----------------------------------------------------------------------------
// CopyDate
// 	----------------------------------------------------------------------------
void TTime::CopyDate( RTTime time )
{
    itsDateValid   = time.itsDateValid;
    itsDay         = time.itsDay;
    itsMonth       = time.itsMonth;
    itsYear        = time.itsYear;
    itsJulianDate  = time.itsJulianDate;
}

// 	----------------------------------------------------------------------------
// CopyTime
// 	----------------------------------------------------------------------------
void TTime::CopyTime( RTTime time )
{
    itsTimeValid     = time.itsTimeValid;
    itsHour          = time.itsHour;
    itsMinute        = time.itsMinute;
    itsJulianMinute  = time.itsJulianMinute;
}


// -----------------------------------------------------------------------
// SetToMidnight
// -----------------------------------------------------------------------
void TTime::SetToMidnight( void )
{
    itsTimeValid    = TRUE;
    itsHour         = 0;
    itsMinute       = 0;
    itsJulianMinute = 0;
}

// -----------------------------------------------------------------------
// SetToMidnightTomorrow
// -----------------------------------------------------------------------
void TTime::SetToMidnightTomorrow( void )
{
    PlusEquals( SCALE_DAY, 1 );
    SetToMidnight();
}

// -----------------------------------------------------------------------
// SetNow
// -----------------------------------------------------------------------
void TTime::SetNow( void )
{
    struct tm nowTm;
    time_t now;

    now    = time( NULL );
    nowTm  = *localtime( &now );

    itsHour       = nowTm.tm_hour;
    itsMinute     = nowTm.tm_min;
    itsTimeValid  = TRUE;

    itsDay        = nowTm.tm_mday;
    itsMonth      = nowTm.tm_mon + 1;
    itsYear       = 1900 + nowTm.tm_year;
    itsDateValid  = TRUE;

    SetJulians();
}


// 	----------------------------------------------------------------------------
//	SetLastDayOfMonth
//
//	Set to the last day of the month, taking into account Feb 29
// 	----------------------------------------------------------------------------
void TTime::SetLastDayOfMonth( void )
{
	itsDay = LastDayOfMonth();

    SetJulians();
}

// 	----------------------------------------------------------------------------
// Normalize
//
// set TTime to beginning of current period
// 	----------------------------------------------------------------------------
void TTime::Normalize( TTimeScale timeScale )
{
    switch (timeScale) {

        case SCALE_HOUR:

		    itsMinute = 0;
            break;

        case SCALE_DAY:

		    if (itsTimeValid) {

                itsHour   = 0;
                itsMinute = 0;
            }
            break;

        case SCALE_WEEK:

		    if (itsTimeValid) {

                itsHour   = 0;
                itsMinute = 0;
            }

            // FIXME doesn't work across months (i.e. on 3 Oct, week starting Sep 29
            // should args be SIGNED? (obviously yes)
            itsDay -= GetWeekDay();
            break;

        case SCALE_MONTH:

		    if (itsTimeValid) {

                itsHour   = 0;
                itsMinute = 0;
            }

            itsDay = 1;
            break;

        case SCALE_QUARTER:

		    if (itsTimeValid) {

                itsHour   = 0;
                itsMinute = 0;
            }

            itsDay = 1;
            if( itsMonth <= 3 )
                itsMonth = 1;
            else if( itsMonth <= 6 )
                itsMonth= 4;
            else if( itsMonth <= 9 )
                itsMonth= 7;
            else
                itsMonth= 10;
            break;

        case SCALE_YEAR:

		    if (itsTimeValid) {

                itsHour   = 0;
                itsMinute = 0;
            }

            itsDay = 1;
            itsMonth = 1;
            break;
    }

    SetJulians();
}

// 	----------------------------------------------------------------------------
// Normalize
//
// set TTime to beginning of current unit
// 	----------------------------------------------------------------------------
void TTime::Normalize( TTimeUnit unit )
{
    switch (unit) {

        case k1Min:                                 break;
        case k5Min:    itsMinute -= itsMinute % 5;  break;
        case k6Min:    itsMinute -= itsMinute % 6;  break;
        case k10Min:   itsMinute -= itsMinute % 10; break;
        case k12Min:   itsMinute -= itsMinute % 12; break;
        case k15Min:   itsMinute -= itsMinute % 15; break;
        case k20Min:   itsMinute -= itsMinute % 20; break;
        case k30Min:   itsMinute -= itsMinute % 30; break;
        case k1Hour:   Normalize( SCALE_HOUR );     break;
        case k1Day:    Normalize( SCALE_DAY );      break;
        case k1Week:   Normalize( SCALE_WEEK );     break;
    }

    SetJulians();
}

// 	----------------------------------------------------------------------------
// PlusEquals
//
// Offset the time by the amount specified
// 	----------------------------------------------------------------------------
void TTime::PlusEquals( TTimeScale scale, int offset )
{
    switch (scale) {

       case SCALE_HOUR:         itsHour  += offset;     itsTimeValid = TRUE; break;
       case SCALE_DAY:          itsDay   += offset;                          break;
       case SCALE_WEEK:         itsDay   += offset * 7;                      break;
       case SCALE_MONTH:

           // make sure that the day isn't set beyond the end of the month
           int day   = itsDay;
           itsDay    = 1;
           itsMonth += offset;
           Adjust();
           int last  = LastDayOfMonth();
           itsDay    = (day <= last) ? day : last;
           SetJulians();
           return;

       case SCALE_QUARTER:

           // make sure that the day isn't set beyond the end of the month
           day       = itsDay;
           itsDay    = 1;
           itsMonth += offset * 3;
           Adjust();
           last      = LastDayOfMonth();
           itsDay    = (day <= last) ? day : last;
           SetJulians();
           return;

       case SCALE_YEAR:         itsYear  += offset;     break;
    }

    Adjust();
}

// 	----------------------------------------------------------------------------
// PlusEquals
//
// Offset the time by the amount specified
// 	----------------------------------------------------------------------------
void TTime::PlusEquals( TTimeUnit unit, int offset )
{
    switch (unit) {

        case k1Min:    itsMinute += offset;      itsTimeValid = TRUE; break;
        case k5Min:    itsMinute += offset * 5;  itsTimeValid = TRUE; break;
        case k6Min:    itsMinute += offset * 6;  itsTimeValid = TRUE; break;
        case k10Min:   itsMinute += offset * 10; itsTimeValid = TRUE; break;
        case k12Min:   itsMinute += offset * 12; itsTimeValid = TRUE; break;
        case k15Min:   itsMinute += offset * 15; itsTimeValid = TRUE; break;
        case k20Min:   itsMinute += offset * 20; itsTimeValid = TRUE; break;
        case k30Min:   itsMinute += offset * 30; itsTimeValid = TRUE; break;
        case k1Hour:   PlusEquals( SCALE_HOUR, offset );              break;
        case k1Day:    PlusEquals( SCALE_DAY,  offset );              break;
        case k1Week:   PlusEquals( SCALE_WEEK, offset );              break;
    }

    Adjust();
}


// 	----------------------------------------------------------------------------
// PlusEquals
//
// Offset the time by the amount specified
// 	----------------------------------------------------------------------------
void TTime::PlusEquals( RTDuration duration )
{
    const int kMinutesPerDay = 1440;

    if (duration.itsMinutes == 0) {

        return;
    }


    if ((duration.itsMinutes % kMinutesPerDay == 0) && !itsTimeValid) {

        int days = (int) (duration.itsMinutes / kMinutesPerDay);
        if (days > 0) {

            itsDay += days - 1;
        }
        else {

            itsDay += days + 1;
        }
    }
    else {

        itsDay    += (int) (duration.itsMinutes / kMinutesPerDay);
        itsMinute += (int) (duration.itsMinutes % kMinutesPerDay);
    }

    Adjust();
}


// 	----------------------------------------------------------------------------
// Plus
//
// Return a time offset by the amount specified
// 	----------------------------------------------------------------------------
TTime TTime::Plus( TTimeScale scale, int offset )
{
    TTime time( *this, scale, offset );
    return time;
}

// 	----------------------------------------------------------------------------
// Plus
//
// Return a time offset by the amount specified
// 	----------------------------------------------------------------------------
TTime TTime::Plus( TTimeUnit unit, int offset )
{
    TTime time( *this, unit, offset );
    return time;
}

// 	----------------------------------------------------------------------------
// Plus
//
// Offset the time by the amount specified. As a special case, if the
// duration is zero, clear the result
// 	----------------------------------------------------------------------------
TTime TTime::Plus( RTDuration duration )
{

    if (duration.IsZero()) {

        return *this;
    }
    else {

        TTime time( *this, duration );
        return time;
    }
}


// 	----------------------------------------------------------------------------
// SetDayOfMonth
// 	----------------------------------------------------------------------------
void TTime::SetDayOfMonth( TWeekSpecifier week, int dayInWeek )
{
    itsDay = 1;
    int d = (dayInWeek + (7 - GetWeekDay())) % 7 + (week - 1) * 7 + 1;
    if (d > LastDayOfMonth()) {

        d -= 7;
    }
    itsDay = d;

    Adjust();
}

// 	----------------------------------------------------------------------------
// SetDate
// 	----------------------------------------------------------------------------
void TTime::SetDate( int day, int mon, int year )
{
    itsDateValid = TRUE;
    itsDay       = day;
    itsMonth     = mon;
    itsYear      = year;

    SetJulians();
}

// 	----------------------------------------------------------------------------
// SetYear
// 	----------------------------------------------------------------------------
void TTime::SetYear( int year )
{
    itsYear   = year;
    Adjust();
}

// 	----------------------------------------------------------------------------
// SetMonth
// 	----------------------------------------------------------------------------
void TTime::SetMonth( int mon )
{
    itsMonth  = mon;
    Adjust();
}

// 	----------------------------------------------------------------------------
// SetDay
// 	----------------------------------------------------------------------------
void TTime::SetDay( int day )
{
    itsDay    = day;
    Adjust();
}

// 	----------------------------------------------------------------------------
// SetHour
// 	----------------------------------------------------------------------------
void TTime::SetHour( int hour )
{
    itsHour      = hour;
    itsTimeValid = TRUE;
    Adjust();
}

// 	----------------------------------------------------------------------------
// SetMinute
// 	----------------------------------------------------------------------------
void TTime::SetMinute( int min )
{
    itsMinute    = min;
    itsTimeValid = TRUE;
    Adjust();
}

// 	----------------------------------------------------------------------------
// ForwardToWorkTime
//
//  Increments the time to the first work hour after
//  its current time
// 	----------------------------------------------------------------------------
void TTime::ForwardToWorkTime( void )
{
    if (itsDateValid) {

        if (itsTimeValid) {

            // if this is not a work day, then
            // go to the start the next work day
            while (!IsWorkDay()) {

                PlusEquals( k1Day, 1 );
                SetToMidnight();
            }

            if (itsJulianMinute < TProfile::WorkDayStartMinute) {

                itsJulianMinute = TProfile::WorkDayStartMinute;
                itsHour   = (int) itsJulianMinute / 60;
                itsMinute = (int) itsJulianMinute % 60;
            }
            else if (itsJulianMinute > TProfile::WorkDayEndMinute) {

                PlusEquals( k1Day, 1 );
                itsJulianMinute = TProfile::WorkDayStartMinute;
                itsHour   = (int) itsJulianMinute / 60;
                itsMinute = (int) itsJulianMinute % 60;
            }
        }

        while (!IsWorkDay()) {

            PlusEquals( k1Day, 1 );
        }
    }
}

// 	----------------------------------------------------------------------------
// BackwardToWorkTime
//
//  Decrements the time to the last work hour before
//  its current time
// 	----------------------------------------------------------------------------
void TTime::BackwardToWorkTime( void )
{
    if (itsDateValid) {

        if (itsTimeValid) {

            // if this is not a work day, then
            // go to the end of the last work day
            while (!IsWorkDay()) {

                SetToMidnight();
                PlusEquals( k1Min, -1 );
            }

            if (itsJulianMinute < TProfile::WorkDayStartMinute) {

                PlusEquals( k1Day, -1 );
                itsJulianMinute = TProfile::WorkDayEndMinute;
                itsHour   = (int) itsJulianMinute / 60;
                itsMinute = (int) itsJulianMinute % 60;
            }
            else if (itsJulianMinute > TProfile::WorkDayEndMinute) {

                itsJulianMinute = TProfile::WorkDayEndMinute;
                itsHour   = (int) itsJulianMinute / 60;
                itsMinute = (int) itsJulianMinute % 60;
            }
        }

        while (!IsWorkDay()) {

            PlusEquals( k1Day, -1 );
        }
    }
}

// ***********************************************************************
// ******************                                   ******************
// ******************        query functions            ******************
// ******************                                   ******************
// ***********************************************************************

// 	----------------------------------------------------------------------------
// IsWorkDay
//
// Checks to see if the specified day is a work day.
// 	----------------------------------------------------------------------------
BOOL TTime::IsWorkDay( void )
{
    return TProfile::IsWorkDay[ GetWeekDay() ] &&
           !IsHoliday();
}

// 	----------------------------------------------------------------------------
// IsBeforeWorkTime
// 	----------------------------------------------------------------------------
BOOL TTime::IsBeforeWorkTime( void )
{
    if (!HasTime()) return FALSE;

    return itsJulianMinute < TProfile::WorkDayStartMinute;
}

// 	----------------------------------------------------------------------------
// IsAfterWorkTime
// 	----------------------------------------------------------------------------
BOOL TTime::IsAfterWorkTime( void )
{
    if (!HasTime()) return FALSE;

    return itsJulianMinute > TProfile::WorkDayEndMinute;
}

// 	----------------------------------------------------------------------------
// IsWorkTime
//
// Checks to see if the specified time is worktime.
// 	----------------------------------------------------------------------------
BOOL TTime::IsWorkTime( void )
{
    if (!HasTime()) return FALSE;

    return (TProfile::WorkDayStartMinute <= itsJulianMinute) &&
           (itsJulianMinute <= TProfile::WorkDayEndMinute );
}

// 	----------------------------------------------------------------------------
// IsHoliday
//
// Checks to see if the specified day is a holiday.
// 	----------------------------------------------------------------------------
BOOL TTime::IsHoliday( void )
{
    return THolidaySet::IsHoliday( *this );
}

// 	----------------------------------------------------------------------------
// IsSameDay
// 	----------------------------------------------------------------------------
BOOL TTime::IsSameDay( RTTime time, BOOL checkMidnights )
{
    // if arg has no date, then by convention it's the same day
    if (!time.HasDate()) {

        return TRUE;
    }

    if (itsJulianDate == time.itsJulianDate) {

        return TRUE;
    }

    if (checkMidnights) {

        // check to see if 'this' is midnight of the day after 'time'
        if ((itsJulianDate == time.itsJulianDate + 1) && (itsJulianMinute == 0)) {

            return TRUE;
        }

        // check to see if 'time' is midnight of the day after 'this'
        if ((itsJulianDate == time.itsJulianDate - 1) && (time.itsJulianMinute == 0)) {

            return TRUE;
        }
    }

    return FALSE;
}

// 	----------------------------------------------------------------------------
//	IsLastDayOfMonth
//
//	Is the date the last day of the month? Make sure to handle leap years
//	and February 29.
// 	----------------------------------------------------------------------------
BOOL TTime::IsLastDayOfMonth( void )
{
    return( itsDateValid && itsDay == LastDayOfMonth() );
}

// 	----------------------------------------------------------------------------
// HasDate
// 	----------------------------------------------------------------------------
BOOL TTime::HasDate( void ) const
{
    return itsDateValid;
}

// 	----------------------------------------------------------------------------
// HasTime
// 	----------------------------------------------------------------------------
BOOL TTime::HasTime( void ) const
{
    return itsTimeValid;
}

// 	----------------------------------------------------------------------------
// GetWeekDay
//
// 0 is Sunday, 6 is Saturday
// 	----------------------------------------------------------------------------
int TTime::GetWeekDay( void ) const
{
    return( weekday( daynumber( itsDay, itsMonth, itsYear )) - 1 );
}

// 	----------------------------------------------------------------------------
//	GetDayOfMonth
//
//	Get the day and week of the current day. Eg. the second Tuesday
// 	----------------------------------------------------------------------------
void TTime::GetDayOfMonth( TWeekSpecifier &week, int &dayInWeek, BOOL allowLast )
{

	// the day is easy
	dayInWeek = GetWeekDay();


	// are we in the last week (ie. one of the last 7 days of the month)?
    // (sometimes, we need to know the exact week number, not just
    // that it's the last week--allowLast is the flag to make sure
    // we are allowed to return kLast)
	if ((itsDay > (LastDayOfMonth() - 7)) && (allowLast)) {

		week = kLast;
	}


	// otherwise the week is specified by dividing by the day by 7
	else {

		week = (TWeekSpecifier) (itsDay / 7);


		// if the day is not exactly divisible by 7 then add 1 to the week
		// (eg. the 8th is in the second week)
		if (itsDay % 7) {

			week = (TWeekSpecifier) (week + 1);
		}
	}
}
// 	----------------------------------------------------------------------------
//	LastDayOfMonth
//
//	return the last day of the month, taking into account Feb 29
// 	----------------------------------------------------------------------------
int TTime::LastDayOfMonth( void )
{
	int y, m, d;

	caldate( ldom( itsYear, itsMonth ), &d, &m, &y );
	return( d );
}

// 	----------------------------------------------------------------------------
// MonthDelta
//
// Special case: just calculate the number of months between
// start and end stupidly.
// 	----------------------------------------------------------------------------
int TTime::MonthDelta( RTTime end )
{

    return (end.itsYear  - itsYear) * 12 +
           (end.itsMonth - itsMonth);

}

// 	----------------------------------------------------------------------------
// WeekDelta
//
// Special case: just calculate the number of weeks between
// start and end stupidly.
// 	----------------------------------------------------------------------------
int TTime::WeekDelta( RTTime end )
{
    return (int) (end.itsJulianDate - itsJulianDate) / 7;
}

// 	----------------------------------------------------------------------------
// DayDelta
//
// Special case: just calculate the number of days between
// start and end stupidly.
// 	----------------------------------------------------------------------------
int TTime::DayDelta( RTTime end )
{
    return (int) (end.itsJulianDate - itsJulianDate);
}

// -----------------------------------------------------------------------
// Get
// -----------------------------------------------------------------------
int TTime::GetYear(   void ) { return itsYear;   };
int TTime::GetMonth(  void ) { return itsMonth;  };
int TTime::GetDay(    void ) { return itsDay;    };
int TTime::GetHour(   void ) { return itsHour;   };
int TTime::GetMinute( void ) { return itsMinute; };

//  ----------------------------------------------------------------------------
// GetUnitName
//
// Return print string of given time unit
// 	----------------------------------------------------------------------------
void TTime::GetUnitName( TTimeUnit unit, RTSafStr str )
{
    switch (unit) {

        case k1Min:    str.KSET( ":01 mins" ); break;
        case k5Min:    str.KSET( ":05 mins" ); break;
        case k6Min:    str.KSET( ":06 mins" ); break;
        case k10Min:   str.KSET( ":10 mins" ); break;
        case k12Min:   str.KSET( ":12 mins" ); break;
        case k15Min:   str.KSET( ":15 mins" ); break;
        case k20Min:   str.KSET( ":20 mins" ); break;
        case k30Min:   str.KSET( ":30 mins" ); break;
        case k1Hour:   str.KSET( "hours" );   break;
        case k1Day:    str.KSET( "days" );    break;
        case k1Week:   str.KSET( "weeks" );   break;
    }

}

// ***********************************************************************
// ******************                                   ******************
// ******************           operators               ******************
// ******************                                   ******************
// ***********************************************************************

// -----------------------------------------------------------------------
// operator =
// -----------------------------------------------------------------------
TTime& TTime::operator =( const TTime &t )
{
    itsDateValid    = t.itsDateValid;
    itsDay          = t.itsDay;
    itsMonth        = t.itsMonth;
    itsYear         = t.itsYear;
    itsJulianDate   = t.itsJulianDate;

    itsTimeValid    = t.itsTimeValid;
    itsHour         = t.itsHour;
    itsMinute       = t.itsMinute;
    itsJulianMinute = t.itsJulianMinute;

    return *this;
}


// -----------------------------------------------------------------------
// operator ==
// -----------------------------------------------------------------------
BOOL TTime::operator == (const TTime &a) const
{
    if (!(itsDateValid || a.itsDateValid))    return TRUE;  // no dates at all
    if (itsDateValid != a.itsDateValid)       return FALSE; // one has a date
    if (itsJulianDate != a.itsJulianDate)     return FALSE; // dates don't match
    if (!(itsTimeValid || a.itsTimeValid))    return TRUE;  // no times at all
    if (itsTimeValid != a.itsTimeValid)       return FALSE; // one has a Time
    if (itsJulianMinute != a.itsJulianMinute) return FALSE; // Times don't match

    return TRUE;
}

// -----------------------------------------------------------------------
// operator !=
// -----------------------------------------------------------------------
 BOOL TTime::operator != (const TTime &a) const
{
    return !( *this == a );
}

// -----------------------------------------------------------------------
// operator >
// -----------------------------------------------------------------------
 BOOL TTime::operator > (const TTime &a) const
{
    if (!(itsDateValid || a.itsDateValid))                  return FALSE; // ==
    if (itsDateValid > a.itsDateValid)                      return TRUE;  // >
    if (itsDateValid < a.itsDateValid)                      return FALSE; // <

    if (itsJulianDate > a.itsJulianDate)      return TRUE;  // >
    if (itsJulianDate < a.itsJulianDate)      return FALSE; // <

    if (!(itsTimeValid || a.itsTimeValid))                  return FALSE; // ==
    if (itsTimeValid > a.itsTimeValid)                      return TRUE;  // >
    if (itsTimeValid < a.itsTimeValid)                      return FALSE; // <

    if (itsJulianMinute > a.itsJulianMinute)  return TRUE;  // >

    return FALSE;                                           // == or <
}

// -----------------------------------------------------------------------
// operator >=
// -----------------------------------------------------------------------
 BOOL TTime::operator >= (const TTime &a) const
{
    return ( *this == a || *this > a );
}

// -----------------------------------------------------------------------
// operator <
// -----------------------------------------------------------------------
 BOOL TTime::operator < (const TTime &a) const
{
    return !( *this >= a );
}

// -----------------------------------------------------------------------
// operator <=
// -----------------------------------------------------------------------
BOOL TTime::operator <= (const TTime &a) const
{
    return !( *this > a );
}


// ***********************************************************************
// ******************                                   ******************
// ******************     standard object functions     ******************
// ******************                                   ******************
// ***********************************************************************

// -----------------------------------------------------------------------
// isA
// -----------------------------------------------------------------------
classType TTime::isA( void ) const
{
    return IDTime;
}

// -----------------------------------------------------------------------
// nameOf
// -----------------------------------------------------------------------
char * TTime::nameOf( void ) const
{
    return "TTime";
}

// -----------------------------------------------------------------------
// hashValue
// -----------------------------------------------------------------------
hashValueType TTime::hashValue( void ) const
{
    return 0; //FIXME
}

// -----------------------------------------------------------------------
// isEqual
// -----------------------------------------------------------------------
int TTime::isEqual(const Object & obj) const
{
    return( (*this) == obj);
}

// -----------------------------------------------------------------------
// isLessThan
// -----------------------------------------------------------------------
int TTime::isLessThan(const Object & obj) const
{
    RTTime other = (RTTime) obj;
    return( *this < other );
}

// -----------------------------------------------------------------------
// printOn
// -----------------------------------------------------------------------
void TTime::printOn(ostream far&) const
{
    //later
}

// ***********************************************************************
// ******************                                   ******************
// ******************        under the hood             ******************
// ******************                                   ******************
// ***********************************************************************


// 	----------------------------------------------------------------------------
// NormalizeUnit
// 	----------------------------------------------------------------------------
void TTime::NormalizeUnit( int &smallerUnit, int &largerUnit, int conversion )
{
    if ((0 <= smallerUnit) && (smallerUnit < conversion)) {

        return;
    }

    largerUnit  += smallerUnit / conversion;
    smallerUnit %= conversion;
    if (smallerUnit < 0) {

        largerUnit--;
        smallerUnit += conversion;
    }
}



// 	----------------------------------------------------------------------------
// Adjust
// 	----------------------------------------------------------------------------
void TTime::Adjust( void )
{
    if( itsTimeValid ) {

        NormalizeUnit( itsMinute, itsHour, 60 );
        NormalizeUnit( itsHour, itsDay, 24 );
    }

    if( itsDateValid )
        {
        // months are a special case because they start at 1, not 0
        if ((1 <= itsMonth) && (itsMonth <= 12)) {

            // nothing!
        }
        else {

            itsMonth--;
            NormalizeUnit( itsMonth, itsYear, 12 );
            itsMonth++;
        }


        // normalize days (and months and years as necessary)
        if ((0 < itsDay) && (itsDay <= 28)) {

            // nothing
        }
        else {

            //FIXME daynumber and caldate should have regular arg ordering
            // and implement caching
            long daynum = daynumber( 1, itsMonth, itsYear ) + itsDay-1;
            caldate( daynum, &itsDay, &itsMonth, &itsYear );
        }
    }

    SetJulians();
}

// ----------------------------------------------------------------
// SetJulians
// ----------------------------------------------------------------
void TTime::SetJulians( void )
{
    itsJulianDate = daynumber( itsDay, itsMonth, itsYear );
    itsJulianMinute = itsHour * 60 + itsMinute;
}

// ***********************************************************************
// ******************                                   ******************
// ******************     input output functions        ******************
// ******************                                   ******************
// ***********************************************************************

// -----------------------------------------------------------------------
// build
// -----------------------------------------------------------------------
PTStreamable TTime::build( void )
{
    return( new TTime( streamableInit ) );
}

TStreamableClass RegTTime( "TTime", TTime::build, __DELTA(TTime) );

// -----------------------------------------------------------------------
// read
// -----------------------------------------------------------------------
Pvoid TTime::read( Ripstream is )
{
    int version, i, dummy;

    is >> version;

    if (version == 4) {         // removed second

	    is >> itsTimeValid;
	    is >> itsHour;
	    is >> itsMinute;
	    is >> itsDateValid;
	    is >> itsDay;
	    is >> itsMonth;
	    is >> itsYear;
        is >> itsJulianDate;
        is >> itsJulianMinute;
    }

    else if (version == 3) {

	    is >> itsTimeValid;
	    is >> itsHour;
	    is >> itsMinute;
	    is >> dummy;           // used to be second
	    is >> itsDateValid;
	    is >> itsDay;
	    is >> itsMonth;
	    is >> itsYear;
        is >> itsJulianDate;
        is >> itsJulianMinute;
    }

    else if( version == 2 ) {

	    is >> itsTimeValid;
	    is >> itsHour;
	    is >> itsMinute;
	    is >> dummy;           // used to be second
	    is >> itsDateValid;
	    is >> itsDay;
	    is >> itsMonth;
	    is >> itsYear;

	    is >> i;  // frequency type
        is >> i;  // frequency
        is >> i;  // week specifier
        is >> i;  // frequency day

        SetJulians();
	}
    else {

	    SYSERR( ERR_VER );
    }


    // clear out the fields if there is no date or time
    if (!itsDateValid) {

        Clear();
    }
    else if (!itsTimeValid) {

        ClearTime();
    }


    return this;
}

// -----------------------------------------------------------------------
// write
// -----------------------------------------------------------------------
void TTime::write( Ropstream os )
{
    os << (int) 4;  // version 4: removed second
                    // version 3: itsJulianDate, itsJulianMinute, no freq data
                    // version 2: object version  (knows abt freq)

    os << itsTimeValid;
    os << itsHour;
    os << itsMinute;
    os << itsDateValid;
    os << itsDay;
    os << itsMonth;
    os << itsYear;
    os << itsJulianDate;
    os << itsJulianMinute;

}



// -----------------------------------------------------------------------
// HealthCheck
//
//  Returns TRUE if it's ok
// -----------------------------------------------------------------------
BOOL TTime::HealthCheck( void )
{
    BOOL ok = TRUE;

    // time but no date?
    if (itsTimeValid && !itsDateValid) {

        return FALSE;
    }

    if (itsDateValid) {

        ok &= (1   <= itsDay)   && (itsDay    <= 31);
        ok &= (1   <= itsMonth) && (itsMonth  <= 12);
        ok &= (500 <= itsYear)  && (itsYear   <= 2500);
        ok &= itsJulianDate == daynumber( itsDay, itsMonth, itsYear );
    }
    if (itsTimeValid) {

        ok &= (0   <= itsHour)  && (itsHour   <= 23);
        ok &= (0   <= itsMinute)&& (itsMinute <= 59);
        ok &= itsJulianMinute == itsHour * 60 + itsMinute;
    }

    return ok;
}
