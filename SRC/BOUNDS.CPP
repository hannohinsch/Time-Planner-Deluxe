//*****************************************************************************

//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "bounds.h"
#include <math.h>

#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __utility__
#include "utility.h"
#endif
#ifndef __error__
#include "error.h"
#endif
#ifndef __note__
#include "note.h"
#endif
#ifndef __freq__
#include "freq.h"
#endif

// -----------------------------------------------------------------------
// Bounds
//
// Every TBounds has a start and end date, each with a time. A 'day'
// within a TBounds goes from midnight to midnight
// -----------------------------------------------------------------------

const long kMinutesPerDay = 1440;

// ***********************************************************************
// ******************                                   ******************
// ******************          constructors             ******************
// ******************                                   ******************
// ***********************************************************************

// -----------------------------------------------------------------------
// constructor
//
// guarantees that both itsStart and itsEnd have times
// -----------------------------------------------------------------------
TBounds::TBounds( RTTime start, RTTime end )
{
	itsStart 	= start;
	itsEnd		= end;

    if (!start.HasTime()) {

        itsStart.SetToMidnight();
    }

    if (!end.HasTime()) {

        itsEnd.SetToMidnightTomorrow();
    }
}


// -----------------------------------------------------------------------
// destructor
// -----------------------------------------------------------------------
TBounds::~TBounds( void )
{
}



// ***********************************************************************
// ******************                                   ******************
// ******************       relationship functions      ******************
// ******************                                   ******************
// ***********************************************************************

// -----------------------------------------------------------------------
// TimeIsWithin
//
// Is (any part of) the time within this bounds?
// -----------------------------------------------------------------------
BOOL TBounds::TimeIsWithin( RTTime time )
{
    if (time.HasTime()) {

        return ((itsStart <= time) && (time < itsEnd));
    }


    // assign times
    time.SetToMidnight();
    TTime end( time );
    end.SetToMidnightTomorrow();

    // NOW both time and end must have real times. Test if:
    //
    // - time starts within bounds
    // - time ends within bounds
    // - time starts before bounds and ends after bounds
    if ((itsStart <= time )   && (time   < itsEnd)) return TRUE;
    if ((itsStart <  end)     && (end    < itsEnd)) return TRUE;
    if ((time     < itsStart) && (itsEnd <= end))   return TRUE;

    return FALSE;
}



// -----------------------------------------------------------------------
// RangeOverlaps
//
// Does the time between start and end overlap the bounds?
// -----------------------------------------------------------------------
BOOL TBounds::RangeOverlaps( TTime start, TTime end )
{
    // check to make sure that there are dates and exactly 0 or 2 times
    if (!start.HasDate()) return FALSE;
    if (!end.HasDate())   return TimeIsWithin( start );
    if ((start.HasTime() && !end.HasTime()) ||
        (end.HasTime()   && !start.HasTime())) {

        SYSERR_STR( "RangeOverlaps: start or end has no time." );
        return FALSE;
    }


    // if there are no times, assign times
    if (!start.HasTime()) {

        start.SetToMidnight();
        end.SetToMidnightTomorrow();
    }


    // NOW both start and end must have real times. Test if:
    //
    // - time starts within bounds
    // - time ends within bounds
    // - time starts before bounds and ends after bounds
    if ((itsStart <= start)   && (start  < itsEnd)) return TRUE;
    if ((itsStart <  end)     && (end    < itsEnd)) return TRUE;
    if ((start    < itsStart) && (itsEnd <= end))   return TRUE;

    return FALSE;
}


// ***********************************************************************
// ******************                                   ******************
// ******************       position functions          ******************
// ******************                                   ******************
// ***********************************************************************

// -----------------------------------------------------------------------
// Position
//
// Calculate the position of the time relative to the bounds.
// -----------------------------------------------------------------------
void TBounds::Position( TTime time, int numCells, RTBoundsPositions positions )
{
    double cellPosition;
    BOOL   useObscureMonthCalculations;

    // if there's no time, then use DoublePosition to calculate values
    // for both ends of the time day
    if (!time.HasTime()) {

        DoublePosition( time, time, numCells, positions );
        return;
    }


    // because months are not of equal lengths, the calculations for
    // position have to be done differently if there timescale is months
    // and the number of months in the bounds is greater than 1
    if (TSysState::timeScale == SCALE_MONTH) {

        long boundsMonths = itsStart.MonthDelta( itsEnd );
        useObscureMonthCalculations = (boundsMonths != 1);
    }
    else {

        useObscureMonthCalculations = FALSE;
    }



    // when the scale is months, then the position is not strictly linear:
    // if the bounds has 200 months, then the first day of the 50th month
    // should have a cellPosition of .25, no matter how many actual days
    // are involved.
    if (useObscureMonthCalculations) {

        double boundsMonths, timeMonths, timeDays, timeMins;

        boundsMonths = itsStart.MonthDelta( itsEnd );

        timeMins     = (time.itsJulianMinute - itsStart.itsJulianMinute);
        timeDays     = (time.itsDay - 1);
        timeDays    += (timeMins / kMinutesPerDay);
        timeMonths   = itsStart.MonthDelta( time );
        timeMonths  += timeDays / time.LastDayOfMonth();

        cellPosition = timeMonths / boundsMonths;
        cellPosition = max( 0, cellPosition );
    }

    // if obscure month calculations are not involved, the position
    // can be easily determined by dividing the number of minutes
    // from the start of the bounds to the requested time
    // by the total number of minutes in the bounds itself
    else {

        double boundsMinutes =
            (itsEnd.itsJulianDate   - itsStart.itsJulianDate) * kMinutesPerDay +
            (itsEnd.itsJulianMinute - itsStart.itsJulianMinute);
        double timeMinutes   =
            (time.itsJulianDate     - itsStart.itsJulianDate) * kMinutesPerDay +
            (time.itsJulianMinute   - itsStart.itsJulianMinute);

        timeMinutes = max( 0, timeMinutes );
        cellPosition = (timeMinutes / boundsMinutes);
    }

    cellPosition        *= numCells;
    long cell;
    SplitDouble( cellPosition, cell, positions.startPos );
    positions.startCell = (int) cell;

    if (positions.startCell >= numCells) {

        positions.startCell  = numCells - 1;
        positions.startPos   = 1;
    }


    // fill in the ending values as well
    positions.endCell   = positions.startCell;
    positions.endPos    = positions.startPos;
}



// -----------------------------------------------------------------------
// DoublePosition
//
// Calculate the position of the two times relative to the bounds.
// If the end has no time, then set it's position to 1
// -----------------------------------------------------------------------
void TBounds::DoublePosition( TTime start, TTime end, int numCells,
    RTBoundsPositions positions )
{
    TBoundsPositions pos;


    // calculate the start positions
    if (!start.HasTime()) {

        start.SetToMidnight();
    }
    Position( start, numCells, pos );
    positions.startCell = pos.startCell;
    positions.startPos  = pos.startPos;


    // calculate the end positions
    if (end.HasTime()) {

        Position( end, numCells, pos );
        positions.endCell   = pos.startCell;
        positions.endPos    = pos.startPos;
    }
    else {

        // if the end has no time, then first add one day to the end
        // and calculate it position
        end.SetToMidnightTomorrow();
        Position( end, numCells, pos );

        // actually, we really wanted to calculate the position of
        // the end as as the end's time were 23:59:59.999999..., rather
        // than midnight of the next day, but since we can't represent
        // this time accurately, we use midnight of the next day, and
        // then move back 1 cell and increment the position to 1, if
        // this won't push the position over 1.
        if (pos.startPos < 0.00001) {

            positions.endCell   = max( pos.startCell - 1, positions.startCell );
            positions.endPos    = 1;
        }
        else {

            positions.endCell   = pos.startCell;
            positions.endPos    = pos.startPos;
        }
    }



}


// -----------------------------------------------------------------------
// NoteOverlaps
// -----------------------------------------------------------------------
BOOL TBounds::NoteOverlaps( RTNote note )
{
    // if the note has a frequency, then initialize it for NoteOverlapAgain
    TTime dummy;
    PTFreq freqP = note.GetFreq();
    if (freqP != NULL) {

        freqP->First( dummy );
    }

    switch (note.GetStyle()) {

        case kMilestoneStyle:

            return TimeIsWithin( note.GetStart() );

        case kTaskStyle:

            return RangeOverlaps( note.GetStart(), note.GetEnd() );

        default:

            return FALSE;
    }
}

// -----------------------------------------------------------------------
// NoteOverlaps
// -----------------------------------------------------------------------
BOOL TBounds::NoteOverlaps( RTNote note, int numCells, RTBoundsPositions positions )
{
    // if the note has a frequency, then initialize it for NoteOverlapAgain
    TTime dummy;
    PTFreq freqP = note.GetFreq();
    if (freqP != NULL) {

        freqP->First( dummy );
    }

    switch (note.GetStyle()) {

        case kMilestoneStyle:

            if (TimeIsWithin( note.GetStart() )) {

                Position( note.GetStart(), numCells, positions );
                return TRUE;
            }
            break;

        case kTaskStyle:

            if (RangeOverlaps( note.GetStart(), note.GetEnd() )) {

                DoublePosition( note.GetStart(), note.GetEnd(), numCells, positions );
                return TRUE;
                }
            break;
    }

    return FALSE;
}


// -----------------------------------------------------------------------
// NoteOverlapsAgain
//
// If the note has a frequency, then check to see if it overlaps the bounds
// again and update the time being checked so that NoteOverlapsAgain
// can be called repeatedly until it DOESN'T overlap
// -----------------------------------------------------------------------
BOOL TBounds::NoteOverlapsAgain( RTNote note, BOOL &overlaps )
{
    if (note.GetFreq() == NULL) {

        // don't continue iterating if there is no frequency
        overlaps = FALSE;
        return FALSE;
    }

    // get the next times based on the frequency (return false if
    // the frequency stops)
    TTime freqStart;
    if (!note.GetFreq()->Next( freqStart )) {

        // don't continue iterating if the frequency ended
        overlaps = FALSE;
        return FALSE;
    }


    // make sure the iterated time is still within the bounds
    if (freqStart >= itsEnd) {

        // don't continue iterating if it isn't
        overlaps = FALSE;
        return FALSE;
    }

    switch (note.GetStyle()) {

        case kMilestoneStyle:

            overlaps = TimeIsWithin( freqStart );
            return TRUE;

        case kTaskStyle:

            // move the end to the same day as freqStart, but with
            // the time of the note
            // Remember that we restrict notes with frequencies so
            // that they must start and end on the same day
            // (although they are allowed to end at midnight)
            TTime freqEnd = freqStart;
            if (note.GetEnd().IsMidnight()) {

                freqEnd.SetToMidnightTomorrow();
            }
            else {

                freqEnd.CopyTime( note.GetEnd() );
            }

            overlaps = RangeOverlaps( freqStart, freqEnd);
            return TRUE;

        default:

            overlaps = FALSE;
            return TRUE;
    }
}

// -----------------------------------------------------------------------
// NoteOverlapsAgain
//
// If the note has a frequency, then check to see if it overlaps the bounds
// again and update the time being checked so that NoteOverlapsAgain
// can be called repeatedly until it DOESN'T overlap
// -----------------------------------------------------------------------
BOOL TBounds::NoteOverlapsAgain( RTNote note, int numCells, RTBoundsPositions positions, BOOL &overlaps )
{
    // assume that it doesn't overlap until proven otherwise
    overlaps = FALSE;


    if (note.GetFreq() == NULL) {

        // don't continue iterating if there is no frequency
        return FALSE;
    }

    // get the next times based on the frequency (return false if
    // the frequency stops)
    TTime freqStart;
    if (!note.GetFreq()->Next( freqStart )) {

        // don't continue iterating if the frequency ended
        return FALSE;
    }


    // make sure the iterated time is still within the bounds
    if (freqStart >= itsEnd) {

        // don't continue iterating if it isn't
        return FALSE;
    }

    switch (note.GetStyle()) {

        case kMilestoneStyle:

            if (TimeIsWithin( freqStart )) {

                Position( freqStart, numCells, positions );
                overlaps = TRUE;
            }
            break;

        case kTaskStyle:

            // move the end to the same day as freqStart, but with
            // the time of the note
            // Remember that we restrict notes with frequencies so
            // that they must start and end on the same day
            // (although we do allow it to end at midnight)
            TTime freqEnd = freqStart;
            if (note.GetEnd().IsMidnight()) {

                freqEnd.SetToMidnightTomorrow();
            }
            else {

                freqEnd.CopyTime( note.GetEnd() );
            }

            if (RangeOverlaps( freqStart, freqEnd)) {

                DoublePosition( freqStart, freqEnd, numCells, positions );
                overlaps = TRUE;
            }
            break;
    }

    // continue iterating
    return TRUE;


}



