//*****************************************************************************
// duration-like object used to calculate and format worktime
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "worktime.h"
#ifndef __profile__
#include "profile.h"
#endif
#ifndef __note__
#include "note.h"
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

#define CONSTRAIN( X ) \
    (max( TProfile::WorkDayStartMinute, \
     min( TProfile::WorkDayEndMinute, X )))

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
TWorkTime::TWorkTime( void ): TDuration()
{
}

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
TWorkTime::TWorkTime( RTWorkTime workTime )
{
    itsMinutes = workTime.itsMinutes;
}

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
TWorkTime::TWorkTime( RTTime start, RTTime end )
{
    Set( start, end );
}

// -------------------------------------------------------------------------
// Constructor
//
//  Convert a raw duration into work time.
//  Durations are in terms of 24-hour days, but
//  worktime is usually in fewer than 24-hours days.
// -------------------------------------------------------------------------
TWorkTime::TWorkTime( RTDuration duration )
{
    Set( duration );
}

// -------------------------------------------------------------------------
// Constructor
//
// fill it in directly
// -------------------------------------------------------------------------
TWorkTime::TWorkTime( TTimeUnit unit, long numUnits )
{
    Set( unit, numUnits );
}

// -------------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------------
TWorkTime::~TWorkTime( void )
{
}

// -------------------------------------------------------------------------
// GetNumUnits
//
//  Calculate the number of units in the worktime
// -------------------------------------------------------------------------
long TWorkTime::GetNumUnits( TTimeUnit unit )
{
    if (unit == k1Day) {

        long workMinsPerDay  = DoubleToLong( TProfile::HoursPerDay  * kMinutesPerHour );
        return itsMinutes / workMinsPerDay;
    }
    else if (unit == k1Week) {

        long workMinsPerWeek = DoubleToLong( TProfile::HoursPerWeek * kMinutesPerHour );
        return itsMinutes / workMinsPerWeek;
    }
    else {      //some sort of hourly units...

        return TDuration::GetNumUnits( unit );
    }
}

// -------------------------------------------------------------------------
// CalculateStart
//
//  Figure out when the start of an interval is given
//  the end
// -------------------------------------------------------------------------
void TWorkTime::CalculateStart( RTTime start, RTTime end )
{
   if ( ! start.HasTime()) {

        CalculateLongStart( start, end );
   }
   else {

        CalculateShortStart( start, end );
   }
}

// -------------------------------------------------------------------------
// CalculateEnd
//
//  Figure out when the end of an interval is given
//  the start
// -------------------------------------------------------------------------
void TWorkTime::CalculateEnd( RTTime start, RTTime end )
{
   if( itsMinutes == 0 || !start.HasDate() )   //no interval, no end
        end.Clear();
   else if ( !start.HasTime())
        CalculateLongEnd( start, end );
   else
        CalculateShortEnd( start, end );
}

// -------------------------------------------------------------------------
// CalculateShortEnd
//
//  Figure out when the end of an interval in hours is,
//  given the start
// -------------------------------------------------------------------------
void TWorkTime::CalculateShortEnd( RTTime start, RTTime end )
{
    long workMinsPerDay  = DoubleToLong( TProfile::HoursPerDay  * kMinutesPerHour );
    long workDays        = itsMinutes / workMinsPerDay;
    long extraMinutes    = itsMinutes % workMinsPerDay;

    ASSERT( ERR_GEN, start.IsWorkTime() );

    // assume end is workDays after start
    if (workDays > 0) {

        // add an extra workDay because CalculateLongEnd
        // assumes that 1/1/93 to 1/1/93 is one day,
        // but we want 1/1/93 9am to 1/2/93 9am to
        // be one day
        TWorkTime workTimeForDays;
        workTimeForDays.itsMinutes = (workDays + 1) * workMinsPerDay;
        workTimeForDays.CalculateLongEnd( start, end );
    }
    else {

        end = start;
    }

    // if the extra minutes still fit on that day,
    // then just add them
    if (end.itsJulianMinute + extraMinutes < TProfile::WorkDayEndMinute) {

        end.itsJulianMinute += extraMinutes;
        end.itsHour   = (int) end.itsJulianMinute / kMinutesPerHour;
        end.itsMinute = (int) end.itsJulianMinute % kMinutesPerHour;
    }
    // otherwise move to the next work day
    else {

        // figure out how many of the extra minutes
        // carry over to the next day
        long minsOn1stDay = TProfile::WorkDayEndMinute -
            CONSTRAIN( start.itsJulianMinute );
        long minsOn2ndDay = extraMinutes - minsOn1stDay;
        end.itsJulianMinute = TProfile::WorkDayStartMinute + minsOn2ndDay;
        end.itsHour   = (int) end.itsJulianMinute / kMinutesPerHour;
        end.itsMinute = (int) end.itsJulianMinute % kMinutesPerHour;

        // increment the end to the start of the next
        // work day
        do {

            end.PlusEquals( k1Day, 1 );
        } while(!end.IsWorkDay());
    }

    // if the end came out to be at the start of a day
    // then back up to the end of the last work day
    if (end.itsJulianMinute == TProfile::WorkDayStartMinute) {

        end.itsJulianMinute = TProfile::WorkDayEndMinute;
        end.itsHour         = TProfile::WorkDayEndMinute / kMinutesPerHour;
        end.itsMinute       = TProfile::WorkDayEndMinute % kMinutesPerHour;
        do {

            end.PlusEquals( k1Day, -1 );
        } while(!end.IsWorkDay());
    }
}

// -------------------------------------------------------------------------
// CalculateLongEnd
//
//  Figure out when the end of an interval in days or
//  weeks is, given the start
// -------------------------------------------------------------------------
void TWorkTime::CalculateLongEnd( RTTime start, RTTime end )
{
    long workMinsPerDay  = DoubleToLong( TProfile::HoursPerDay  * kMinutesPerHour );
    long workMinsPerWeek = DoubleToLong( TProfile::HoursPerWeek * kMinutesPerHour );

    long workWeeks       = itsMinutes / workMinsPerWeek;
    long workDays        = (itsMinutes % workMinsPerWeek) / workMinsPerDay;
    workDays            += workWeeks * TProfile::DaysPerWeek;

    end = start;
    while (workDays > 1) {

        if (end.IsWorkDay()) {

            workDays--;
        }

        end.PlusEquals( k1Day, 1 );
    }

    // if the end falls on a non-workday, move forward
    while (!end.IsWorkDay()) {

        end.PlusEquals( k1Day, 1 );
    }
}

// -------------------------------------------------------------------------
// CalculateShortStart
//
//  Figure out when the start of an interval in hours is,
//  given the end
// -------------------------------------------------------------------------
void TWorkTime::CalculateShortStart( RTTime start, RTTime end )
{
    long workMinsPerDay  = DoubleToLong( TProfile::HoursPerDay  * kMinutesPerHour );
    long workDays        = itsMinutes / workMinsPerDay;
    long extraMinutes    = itsMinutes % workMinsPerDay;

    // assume start is workDays before end
    if (workDays > 0) {

        // add an extra workDay because CalculateLongStart
        // assumes that 1/1/93 to 1/1/93 is one day,
        // but we want 1/1/93 9am to 1/2/93 9am to
        // be one day
        TWorkTime workTimeForDays;
        workTimeForDays.itsMinutes = (workDays + 1) * workMinsPerDay;
        workTimeForDays.CalculateLongStart( start, end );
    }
    else {

        start = end;
    }

    // if the extra minutes still fit on that day,
    // then just add them
    if (start.itsJulianMinute - extraMinutes >= TProfile::WorkDayStartMinute) {

        start.itsJulianMinute -= extraMinutes;
        start.itsHour   = (int) start.itsJulianMinute / kMinutesPerHour;
        start.itsMinute = (int) start.itsJulianMinute % kMinutesPerHour;
    }
    // otherwise move to the last work day
    else {

        // figure out how many of the extra minutes
        // carry over to the previous day
        long minsOn2ndDay =
            CONSTRAIN( start.itsJulianMinute ) -
            TProfile::WorkDayStartMinute;
        long minsOn1stDay = extraMinutes - minsOn2ndDay;
        start.itsJulianMinute = TProfile::WorkDayEndMinute - minsOn1stDay;
        start.itsHour   = (int) start.itsJulianMinute / kMinutesPerHour;
        start.itsMinute = (int) start.itsJulianMinute % kMinutesPerHour;

        // decrement the start to the end of the last
        // work day
        do {

            start.PlusEquals( k1Day, -1 );
        } while(!start.IsWorkDay());
    }

    // if the start came out to be at the end of a day
    // then move to the start of the next work day
    if (start.itsJulianMinute == TProfile::WorkDayEndMinute) {

        start.itsJulianMinute = TProfile::WorkDayStartMinute;
        start.itsHour         = TProfile::WorkDayStartMinute / kMinutesPerHour;
        start.itsMinute       = TProfile::WorkDayStartMinute % kMinutesPerHour;
        do {

            start.PlusEquals( k1Day, 1 );
        } while(!start.IsWorkDay());
    }
}

// -------------------------------------------------------------------------
// CalculateLongStart
//
//  Figure out when the start of an interval in days or
//  weeks is, given the end
// -------------------------------------------------------------------------
void TWorkTime::CalculateLongStart( RTTime start, RTTime end )
{
    long workMinsPerDay  = DoubleToLong( TProfile::HoursPerDay  * kMinutesPerHour );
    long workMinsPerWeek = DoubleToLong( TProfile::HoursPerWeek * kMinutesPerHour );

    long workWeeks       = itsMinutes / workMinsPerWeek;
    long workDays        = (itsMinutes % workMinsPerWeek) / workMinsPerDay;
    workDays            += workWeeks * TProfile::DaysPerWeek;

    start = end;
    while (workDays > 1) {

        if (start.IsWorkDay()) {

            workDays--;
        }

        start.PlusEquals( k1Day, -1 );
    }

    // if the start falls on a workday, move backward
    while (!start.IsWorkDay()) {

        start.PlusEquals( k1Day, -1 );
    }
}

// -------------------------------------------------------------------------
// Set
// -------------------------------------------------------------------------
void TWorkTime::Set( RTDuration duration )
{
    TTimeUnit naturalUnit;
    if (duration.itsMinutes % kMinutesPerWeek == 0) {

        naturalUnit = k1Week;
    }
    else if (duration.itsMinutes % kMinutesPerDay == 0) {

        naturalUnit = k1Day;
    }
    else {

        naturalUnit = k1Hour;
    }

    long workMinsPerDay  = DoubleToLong( TProfile::HoursPerDay  * kMinutesPerHour );
    long workMinsPerWeek = DoubleToLong( TProfile::HoursPerWeek * kMinutesPerHour );

    if (naturalUnit <= k1Hour) {

        itsMinutes = duration.itsMinutes;
    }
    else if (naturalUnit <= k1Day) {

        itsMinutes = duration.itsMinutes * workMinsPerDay / kMinutesPerDay;
    }
    else {

        itsMinutes = duration.itsMinutes * workMinsPerWeek / kMinutesPerWeek;
    }
}

// -------------------------------------------------------------------------
// Set
// -------------------------------------------------------------------------
void TWorkTime::Set( RTTime start, RTTime end )
{
    // is there an interval at all?
    if (!(start.HasDate() && end.HasDate())) {

        itsMinutes = 0;
        return;
    }

    long workMinsPerDay  = DoubleToLong( TProfile::HoursPerDay  * kMinutesPerHour );
    long workMinsPerWeek = DoubleToLong( TProfile::HoursPerWeek * kMinutesPerHour );

    if (start.HasTime()) {

        long firstMinute = CONSTRAIN( start.itsJulianMinute );
        long lastMinute = CONSTRAIN( end.itsJulianMinute );

        // first and last day match
        itsMinutes = 0;
        if( start.IsSameDay( end, FALSE )) {

            if (start.IsWorkDay()) {

                itsMinutes = lastMinute - firstMinute;
            }
        }
        else {

            if (start.IsWorkDay()) {

                itsMinutes = TProfile::WorkDayEndMinute - firstMinute;
            }
            if (end.IsWorkDay()) {

                itsMinutes += lastMinute - TProfile::WorkDayStartMinute;
            }

            // intervening days count only if they're workdays
            TTime day( start.Plus( k1Day, 1 ));
            TTime endDay( end );
            day.ClearTime();
            endDay.ClearTime();
            for (day; day < endDay; day.PlusEquals( k1Day, 1 )) {

                if (day.IsWorkDay()) {

                    itsMinutes += workMinsPerDay;
                }
            }
        }
    }
    else {
        #if 0    //old code
        long fullWeeks = (end.itsJulianDate - start.itsJulianDate + 1) / 7;
        long extraDays = (end.itsJulianDate - start.itsJulianDate + 1) % 7;

        itsMinutes = fullWeeks * workMinsPerWeek;
        TTime lastDay( start, k1Day, (int) extraDays );
        for (TTime day( start ); day < lastDay; day.PlusEquals( k1Day, 1 )) {

            if (day.IsWorkDay()) {

                itsMinutes += workMinsPerDay;
            }
        }
        #else

        itsMinutes = 0;
        for (TTime day( start ); day <= end; day.PlusEquals( k1Day, 1 )) {

            if (day.IsWorkDay()) {

                itsMinutes += workMinsPerDay;
            }
        }
        #endif
    }
}

// -------------------------------------------------------------------------
// Set
//
// fill it in directly
// -------------------------------------------------------------------------
void TWorkTime::Set( TTimeUnit unit, long numUnits )
{
    if (unit <= k1Hour) {

        itsMinutes = kMinutesPerTimeUnit[unit] * numUnits;
    }
    else if (unit == k1Day) {

        long workMinsPerDay  = DoubleToLong( TProfile::HoursPerDay  * kMinutesPerHour );
        itsMinutes = workMinsPerDay * numUnits;
    }
    else if (unit == k1Week) {

        long workMinsPerWeek = DoubleToLong( TProfile::HoursPerWeek * kMinutesPerHour );
        itsMinutes = workMinsPerWeek * numUnits;
    }
}


// -------------------------------------------------------------------------
// Format
// -------------------------------------------------------------------------
void TWorkTime::Format( TTimeUnit unit, BOOL mixed, char *buf )
{
    if (unit == k1Week) {

        Format( SCALE_WEEK, mixed, buf );
    }
    else if (unit == k1Day) {

        Format( SCALE_DAY, mixed, buf );
    }
    else {

        Format( SCALE_HOUR, mixed, buf );
    }
}
// -------------------------------------------------------------------------
// Format
//
// A workTime of 2 weeks (5 days per week) will become a workTime of
// 10 days. Rather than
// -------------------------------------------------------------------------
void TWorkTime::Format( TTimeScale scale, BOOL mixed, char *buf )
{
    long workMinsPerDay  = DoubleToLong( TProfile::HoursPerDay  * kMinutesPerHour );
    long workMinsPerWeek = DoubleToLong( TProfile::HoursPerWeek * kMinutesPerHour );
    long minutes         = itsMinutes;

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

                d =  minutes / workMinsPerDay;
                h = (minutes % workMinsPerDay) / kMinutesPerHour;
                m = (minutes % workMinsPerDay) % kMinutesPerHour;

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

                w =  minutes / workMinsPerWeek;
                d = (minutes % workMinsPerWeek) / workMinsPerDay;
                h = (minutes % workMinsPerDay)  / kMinutesPerHour;
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

                length = ((double) minutes) / workMinsPerDay;
                break;

            case SCALE_WEEK:

                length = ((double) minutes) / workMinsPerWeek;
                break;
        }

        static const char abbreviations[3] = { 'h', 'd', 'w' };
        char rawBuf[80];

        FormatDouble( length, rawBuf );
        wsprintf( buf, "%s %c", rawBuf, abbreviations[scale] );
    }
}

