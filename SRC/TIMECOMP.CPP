//*****************************************************************************
// time complex: start, end, duration, worktime
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "timecomp.h"
#include "error.h"
#ifndef __note__
#include "note.h"
#endif
#ifndef __classid__
#include "classid.h"
#endif

//---------------------------------------------------------------------------
// constructor
//---------------------------------------------------------------------------
TTimeComplex::TTimeComplex( void )
{
    itsTimeUnit = k15Min;
    itsUseWorkTime = FALSE;
}

//---------------------------------------------------------------------------
// destructor
//---------------------------------------------------------------------------
TTimeComplex::~TTimeComplex( void )
{
}

//---------------------------------------------------------------------------
// operator =
//---------------------------------------------------------------------------
RTTimeComplex TTimeComplex::operator =( const RTTimeComplex other )
{
    itsStart        = other.itsStart;
    itsEnd          = other.itsEnd;
    itsDuration     = other.itsDuration;
    itsUseWorkTime  = other.itsUseWorkTime;
    itsWorkTime     = other.itsWorkTime;
    itsTimeUnit     = other.itsTimeUnit;

    return *this;
}

//---------------------------------------------------------------------------
// ExactlyMatches
//---------------------------------------------------------------------------
BOOL TTimeComplex::ExactlyMatches( const RTTimeComplex other )
{
    return
    (itsStart        == other.itsStart)       &&
    (itsEnd          == other.itsEnd)         &&
    (itsDuration     == other.itsDuration)    &&
    (itsUseWorkTime  == other.itsUseWorkTime) &&
    (itsWorkTime     == other.itsWorkTime)    &&
    (itsTimeUnit     == other.itsTimeUnit);
}

//---------------------------------------------------------------------------
// SetStartAndEnd
//
// Makes sure start and end are worktimes
//---------------------------------------------------------------------------
void TTimeComplex::SetStartAndEnd(
    RTTime start, RTTime end,
    TTimeUnit unit )
{
    itsTimeUnit = unit;

    if (itsUseWorkTime) {

        itsStart = start;
        itsEnd = end;

        // move the times around to work time
        ForwardToWorkTime( itsStart );
        BackwardToWorkTime( itsEnd );

        // make sure the end comes after the start
        if (itsEnd < itsStart) {

            TDuration duration( start, end );
            itsEnd = itsStart.Plus( duration );
            ForwardToWorkTime( itsEnd );
        }

        itsDuration.Set( itsStart, itsEnd );
        itsWorkTime.Set( itsStart, itsEnd );
    }
    else {

        itsStart = start;
        itsEnd   = end;
        itsDuration.Set( itsStart, itsEnd );
        itsWorkTime.Set( itsStart, itsEnd );
    }

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// SetStartAndKeepDuration
//
// For worktime, "KeepDuration" really means
// "KeepWorktime" and the duration will actually
// be modified as necessary to keep the worktime
// consistent
//---------------------------------------------------------------------------
void TTimeComplex::SetStartAndKeepDuration(
    RTTime start )
{

    // no end time? just set the start
    if (itsDuration.IsZero()) {

        itsStart = start;
        ForwardToWorkTime( itsStart );
    }
    else {

        if (itsUseWorkTime) {

            itsStart = start;
            ForwardToWorkTime( itsStart );
            //NB assumes itsWorkTime has already been set...
            itsWorkTime.CalculateEnd( itsStart, itsEnd );
            itsDuration.Set( itsStart, itsEnd );
        }
        else {

            itsStart = start;
            itsEnd = itsStart.Plus( itsDuration );
            itsWorkTime.Set( itsStart, itsEnd );
        }
    }


    CHKBUG( ERR_GEN, HealthCheck());

}

//---------------------------------------------------------------------------
// SetDurationAndKeepStart
//
// Duration always means duration, not worktime
//---------------------------------------------------------------------------
void TTimeComplex::SetDurationAndKeepStart(
    RTDuration duration )
{
    itsDuration = duration;
    itsEnd = itsStart.Plus( itsDuration );
    itsWorkTime.Set( itsStart, itsEnd );

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// SetStartAndKeepEnd
//
// Makes sure start is a worktime
//---------------------------------------------------------------------------
void TTimeComplex::SetStartAndKeepEnd(
    RTTime start, TTimeUnit unit, int number )
{
    itsStart = start;
    ForwardToWorkTime( itsStart );

    // (1/1 to 1/2 is two days...)
    if (unit >= k1Day)
        number--;

    // make sure the start is less than the end
    TTime maxTime( itsEnd.Plus( unit, -number ));
    if (itsStart > maxTime) {

        itsStart = maxTime;
    }

    itsDuration.Set( itsStart, itsEnd );
    itsWorkTime.Set( itsStart, itsEnd );

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// SetEndAndKeepStart
//
// Makes sure end is a worktime
//---------------------------------------------------------------------------
void TTimeComplex::SetEndAndKeepStart(
    RTTime end, TTimeUnit unit, int number )
{
    itsEnd = end;
    BackwardToWorkTime( itsEnd );

    // (1/1 to 1/2 is two days...)
    if (unit >= k1Day)
        number--;

    // make sure the end is after the start
    TTime minTime( itsStart.Plus( unit, number ));
    if (itsEnd < minTime) {

        itsEnd = minTime;
    }

    itsDuration.Set( itsStart, itsEnd );
    itsWorkTime.Set( itsStart, itsEnd );

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// SetStartToStartOfDayAndKeepEnd
//
// Set itsStart to worktime and calculates itsDuration
// based on the new start and old end
//---------------------------------------------------------------------------
void TTimeComplex::SetStartToStartOfDayAndKeepEnd( void )
{
    itsStart.SetToMidnight();
    ForwardToWorkTime( itsStart );
    itsDuration.Set( itsStart, itsEnd );
    itsWorkTime.Set( itsStart, itsEnd );

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// SetEndToEndOfDayAndKeepStart
//
// Set itsEnd to worktime ON THE SAME DAY AS ITSSTART
// and calculates itsDuration based on the old start
// and new end
//---------------------------------------------------------------------------
void TTimeComplex::SetEndToEndOfDayAndKeepStart( void )
{
    itsEnd = itsStart;
    itsEnd.SetToMidnightTomorrow();
    BackwardToWorkTime( itsEnd );
    itsDuration.Set( itsStart, itsEnd );
    itsWorkTime.Set( itsStart, itsEnd );

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// SetStartToStartOfDayAndKeepDuration
//
// Set itsStart to worktime and calculates itsEnd
// based on the new start and old duration
//---------------------------------------------------------------------------
void TTimeComplex::SetStartToStartOfDayAndKeepDuration( void )
{
    //set the start time
    itsStart.SetToMidnight();
    ForwardToWorkTime( itsStart );

    // modify the end only if it's not a milestone
    if (itsEnd.HasDate()) {
        if (itsUseWorkTime) {

            itsWorkTime.CalculateEnd( itsStart, itsEnd );
            itsDuration.Set( itsStart, itsEnd );
        }
        else {

            itsEnd = itsStart.Plus( itsDuration );
            itsWorkTime.Set( itsStart, itsEnd );
        }
    }

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// SetEndToEndOfDayAndKeepDuration
//
// Set itsEnd to worktime ON THE SAME DAY AS ITSSTART
// and calculate itsStart based on the old duration
// and new end
//---------------------------------------------------------------------------
void TTimeComplex::SetEndToEndOfDayAndKeepDuration( void )
{
    // if this is a milestone then don't do anything
    if (!itsEnd.HasDate()) return;

    itsEnd = itsStart;
    itsEnd.SetToMidnightTomorrow();

    if (itsUseWorkTime) {

        BackwardToWorkTime( itsEnd );
        itsWorkTime.CalculateStart( itsStart, itsEnd );
        itsDuration.Set( itsStart, itsEnd );
    }
    else {

        itsStart = itsEnd.Plus( -itsDuration );
        itsWorkTime.Set( itsStart, itsEnd );
    }

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// SetStartToJustBeforeEnd
//
// Sets itsStart to be before itsEnd by the number
// of units specified and calculates the new duration
// from the new start and old end
//
// makes sure itsStart lands on a worktime
//---------------------------------------------------------------------------
void TTimeComplex::SetStartToJustBeforeEnd(
    TTimeUnit unit, int number )
{
    itsStart = itsEnd.Plus( unit, -number );
    BackwardToWorkTime( itsStart );
    itsDuration.Set( itsStart, itsEnd );
    itsWorkTime.Set( itsStart, itsEnd );

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// SetEndToJustAfterStart
//
// Sets itsEnd to be after itsStart by the number
// of units specified and calculates the new duration
// from the old start and new end
//---------------------------------------------------------------------------
void TTimeComplex::SetEndToJustAfterStart(
    TTimeUnit unit, int number )
{
    itsEnd = itsStart.Plus( unit, number );
    ForwardToWorkTime( itsEnd );
    itsDuration.Set( itsStart, itsEnd );
    itsWorkTime.Set( itsStart, itsEnd );

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// SetStartAndClearDuration
//---------------------------------------------------------------------------
void TTimeComplex::SetStartAndClearDuration(
    RTTime start )
{
    itsStart = start;
    ForwardToWorkTime( itsStart );
    itsEnd.Clear();
    itsDuration.Clear();
    itsWorkTime.Clear();

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// ConstrainToDay
//
// ASSUMES THAT ITSSTART AND ITSEND WILL FIT IN ONE DAY.
//
// If itsStart doesn't fall on the day, then move it
// to worktime of that day. If itsEnd doesn't fall on
// the day, then move it to worktime at the end of the
// day
//---------------------------------------------------------------------------
void TTimeComplex::ConstrainToDay(
    RTTime day )
{
    // get itsStart and itsEnd into the proper worktime
    if (itsUseWorkTime) {

        TTime workStarts( day );
        workStarts.SetToMidnight();
        workStarts.ForwardToWorkTime();
        TTime workEnds( workStarts );
        workEnds.SetToMidnightTomorrow();
        workEnds.BackwardToWorkTime();

        if (itsStart < workStarts) {

            itsStart = workStarts;
            itsWorkTime.CalculateEnd( itsStart, itsEnd );
            itsDuration.Set( itsStart, itsEnd );
        }

        if (itsEnd > workEnds) {

            itsEnd = workEnds;
            itsWorkTime.CalculateStart( itsStart, itsEnd );
            itsDuration.Set( itsStart, itsEnd );
        }
    }
    else {

        TTime today( day );
        today.SetToMidnight();
        TTime tomorrow( today );
        tomorrow.SetToMidnightTomorrow();

        if (itsStart < today) {

            itsStart = today;
            itsEnd = itsStart.Plus( itsDuration );
            itsWorkTime.Set( itsStart, itsEnd );
        }

        if (itsEnd > tomorrow) {

            itsEnd = tomorrow;
            itsStart = itsEnd.Plus( -itsDuration );
            itsWorkTime.Set( itsStart, itsEnd );
        }
    }

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// ConstrainStartToDayAndEnd
//
// If itsStart doesn't fall between worktime on the
// day of itsEnd and just before itsEnd, then move
// it so that it does fall there. Calculate the new
// duration.
//---------------------------------------------------------------------------
void TTimeComplex::ConstrainStartToDayAndEnd(
    TTimeUnit unit, int number )
{
    if (!itsStart.IsSameDay( itsEnd, FALSE )) {

        itsStart = itsEnd;
        itsStart.SetToMidnight();
        ForwardToWorkTime( itsStart );
        itsDuration.Set( itsStart, itsEnd );
        itsWorkTime.Set( itsStart, itsEnd );
    }
    else if (itsStart > itsEnd.Plus( unit, -number )) {

        itsStart = itsEnd.Plus( unit, -number );
        itsDuration.Set( itsStart, itsEnd );
        itsWorkTime.Set( itsStart, itsEnd );
    }

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// ConstrainEndToDayAndStart
//
// If itsEnd doesn't fall between just after itsStart
// and worktime at the end of the day, them move it so
// that it does. Calculate the new duration.
//---------------------------------------------------------------------------
void TTimeComplex::ConstrainEndToDayAndStart(
    TTimeUnit unit, int number )
{
    if (itsEnd < itsStart.Plus( unit, number )) {

        itsEnd = itsStart.Plus( unit, number );
        itsDuration.Set( itsStart, itsEnd );
        itsWorkTime.Set( itsStart, itsEnd );
    }
    else if (!itsEnd.IsSameDay( itsStart, FALSE )) {

        itsEnd = itsStart;
        itsEnd.SetToMidnightTomorrow();
        BackwardToWorkTime( itsEnd );
        itsDuration.Set( itsStart, itsEnd );
        itsWorkTime.Set( itsStart, itsEnd );
    }

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// ConstrainStartToTimeAndEnd
//
// if itsStart doesn't fall between time and
// "just before" itsEnd, then set it so that it does
//---------------------------------------------------------------------------
void TTimeComplex::ConstrainStartToTimeAndEnd(
    RTTime time, TTimeUnit unit, int number )
{
    // it's the old off-by-one problem for days
    // and weeks since 1/1/93 to 1/2/93 is 2 days
    if (unit >= k1Day) number--;

    if (itsStart < time) {

        itsStart = time;
        ForwardToWorkTime( itsStart );
        itsDuration.Set( itsStart, itsEnd );
        itsWorkTime.Set( itsStart, itsEnd );
    }
    else if (itsStart > itsEnd.Plus( unit, -number )) {

        itsStart = itsEnd.Plus( unit, -number );
        itsDuration.Set( itsStart, itsEnd );
        itsWorkTime.Set( itsStart, itsEnd );
    }

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// ConstrainEndToTimeAndStart
//
// if itsEnd doesn't fall between "just before"
// itsStart and time then set it so that it does
//---------------------------------------------------------------------------
void TTimeComplex::ConstrainEndToTimeAndStart(
    RTTime time, TTimeUnit unit, int number )
{
    // it's the old off-by-one problem for days
    // and weeks since 1/1/93 to 1/2/93 is 2 days
    if (unit >= k1Day) number--;

    if (itsEnd > time) {

        itsEnd = time;
        BackwardToWorkTime( itsEnd );
        itsDuration.Set( itsStart, itsEnd );
        itsWorkTime.Set( itsStart, itsEnd );
    }
    else if (itsEnd < itsStart.Plus( unit, number )) {

        itsEnd = itsStart.Plus( unit, number );
        itsDuration.Set( itsStart, itsEnd );
        itsWorkTime.Set( itsStart, itsEnd );
    }

    CHKBUG( ERR_GEN, HealthCheck());
}


//---------------------------------------------------------------------------
// ConstrainToInterval
//
// If itsStart doesn't fall after the greater of
// start1 and start2P then move it there. If itsEnd
// doesn't fall before the lesser of end1 and end2P
// then move it there. Calculate the new duration.
//---------------------------------------------------------------------------
void TTimeComplex::ConstrainToInterval(
    RTTime start1, RTTime end1,
    PTTime start2P, PTTime end2P )
{
    // get the greater of the two starting times
    // and then move it forward to worktime
    TTime start( start1 );
    if (start2P && ((*start2P) > start1))
        start = *start2P;
    ForwardToWorkTime( start );

    // get the lesser of the two ending times
    // and then move it backward to worktime
    TTime end( end1 );
    if (end2P && ((*end2P) < end1))
        end = *end2P;
    BackwardToWorkTime( end );

    // move itsStart and itsEnd into proper range
    if (itsStart < start)
        itsStart = start;
    if (itsStart > end)
        itsStart = end;
    if (itsEnd.HasDate() && (itsEnd < start))
        itsEnd = start;
    if (itsEnd.HasDate() && (itsEnd > end))
        itsEnd = end;

    itsDuration.Set( itsStart, itsEnd );
    itsWorkTime.Set( itsStart, itsEnd );

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// SetTo1DayOn
//
//   Converts a note with minutes to one without
//   and sets it to the specified day. Slides the
//   day to first worktime
//---------------------------------------------------------------------------
void TTimeComplex::SetTo1DayOn(
    RTTime day )
{
    itsStart = day;
    itsStart.ClearTime();
    ForwardToWorkTime( itsStart );
    itsEnd = itsStart;
    itsDuration.Set( itsStart, itsEnd );
    itsWorkTime.Set( itsStart, itsEnd );
    itsTimeUnit = k1Day;

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// SetTo2HoursAt
//
//   Converts a note without minutes to one with
//   and sets it to the specified time. Slides the
//   day to first worktime
//---------------------------------------------------------------------------
void TTimeComplex::SetTo2HoursAt(
    RTTime time )
{
    itsStart = time;
    itsEnd   = itsStart.Plus( k1Hour, 2 );
    itsDuration.Set( k1Hour, 2 );

    if (itsUseWorkTime) {

        // slide around to the start of the
        // first work day
        while (!itsStart.IsWorkDay()) {

            itsStart.PlusEquals( k1Day, 1 );
            itsStart.SetToMidnight();
            itsStart.ForwardToWorkTime();
            itsEnd = itsStart.Plus( k1Hour, 2 );
        }
            
        if (itsStart.IsBeforeWorkTime()) {

            itsStart.ForwardToWorkTime();
            itsEnd = itsStart.Plus( k1Hour, 2 );
        }
        if (itsEnd.IsAfterWorkTime()) {

            itsEnd.BackwardToWorkTime();
            itsStart = itsEnd.Plus( k1Hour, -2 );
        }
    }

    itsWorkTime.Set( itsStart, itsEnd );
    itsTimeUnit = k15Min;

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// SetDurationFromNote
//
// Sets itsStart to start, copies the unit and duration from
// the note and calculates everything else
//---------------------------------------------------------------------------
void TTimeComplex::SetDurationFromNote(
    PTNote noteP, RTTime start )
{
    itsTimeUnit = noteP->GetTimeUnit();
    if ((itsTimeUnit >= k1Day) && start.HasTime())
        start.ClearTime();
    else if ((itsTimeUnit < k1Day) && (!start.HasTime()))
        start.SetToMidnight();

    itsStart = start;
    ForwardToWorkTime( itsStart );

    // milestone in note?
    if (noteP->GetDuration().IsZero()) {

        itsDuration.Set( itsTimeUnit, 0 );
        itsWorkTime.Set( itsDuration );
        itsEnd.Clear();
    }

    else {

        if (itsUseWorkTime) {

            itsWorkTime = noteP->GetWorkTime();
            itsWorkTime.CalculateEnd( itsStart, itsEnd );
            itsDuration.Set( itsStart, itsEnd );
        }
        else {

            itsDuration = noteP->GetDuration();
            itsEnd = itsStart.Plus( itsDuration );
            itsWorkTime.Set( itsStart, itsEnd );
        }
    }

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// SetFromRawData
//
//---------------------------------------------------------------------------
void TTimeComplex::SetFromRawData( RTTime start,
    RTDuration duration, TTimeUnit unit,
    BOOL isWorkTime )
{
    TTime end;

    itsStart= start;
    itsEnd.Clear();
    itsTimeUnit= unit;
    itsUseWorkTime = isWorkTime;

    if( start.HasDate() )
        {
        if( isWorkTime )
            {
            //duration is really a work time
            //...which will be kept constant
            itsWorkTime.Set( unit, duration.GetNumUnits( unit ) );

            //calculate real duration
            TTime end;
            itsWorkTime.CalculateEnd( start, end );
            itsDuration.Set( start, end );

            //normalize for work schedule
            SetStartAndKeepDuration( start );
            }
        else
            {
            //elapsed -- keep duration
            itsDuration= duration;
            itsWorkTime.Set( duration );
            SetStartAndKeepDuration( start );
            }
        }
    else
        {
        //no start date, so worktime == duration
        itsDuration= duration;
        itsWorkTime.Set( duration );
        }

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// GetStart
//---------------------------------------------------------------------------
TTime TTimeComplex::GetStart( void ) const
{
    return itsStart;
}

//---------------------------------------------------------------------------
// GetEnd
//---------------------------------------------------------------------------
TTime TTimeComplex::GetEnd( void ) const
{
    return itsEnd;
}

//---------------------------------------------------------------------------
// GetDuration
//---------------------------------------------------------------------------
TDuration TTimeComplex::GetDuration( void ) const
{
    return itsDuration;
}

//---------------------------------------------------------------------------
// SetTimeUnit
//---------------------------------------------------------------------------
void TTimeComplex::SetTimeUnit( TTimeUnit unit )
{
    itsTimeUnit = unit;

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// GetTimeUnit
//---------------------------------------------------------------------------
TTimeUnit TTimeComplex::GetTimeUnit( void ) const
{
    return itsTimeUnit;
}

//---------------------------------------------------------------------------
// GetWorkTime
//---------------------------------------------------------------------------
TWorkTime TTimeComplex::GetWorkTime( void ) const
{
    return itsWorkTime;
}

//---------------------------------------------------------------------------
// IsWorkTime
//---------------------------------------------------------------------------
BOOL TTimeComplex::IsWorkTime( void ) const
{
    return itsUseWorkTime;
}

//---------------------------------------------------------------------------
// SetWorkTime
//---------------------------------------------------------------------------
void TTimeComplex::SetWorkTime( BOOL worktime )
{
    itsUseWorkTime = worktime;
    itsWorkTime.Set( itsStart, itsEnd );

    CHKBUG( ERR_GEN, HealthCheck());
}

//---------------------------------------------------------------------------
// ForwardToWorkTime
//---------------------------------------------------------------------------
void TTimeComplex::ForwardToWorkTime( RTTime time )
{
    if (itsUseWorkTime) {

        if (!(time.IsWorkDay() && time.IsWorkTime()))
            time.ForwardToWorkTime();
    }
}

//---------------------------------------------------------------------------
// BackwardToWorkTime
//---------------------------------------------------------------------------
void TTimeComplex::BackwardToWorkTime( RTTime time )
{
    if (itsUseWorkTime) {

        if (!(time.IsWorkDay() && time.IsWorkTime()))
            time.BackwardToWorkTime();
    }
}


// ====================================================
//
//                  Basic Object Functions
//
// ====================================================
classType TTimeComplex::isA( void ) const           { return IDTimeComplex; }
char *TTimeComplex::nameOf( void ) const            { return "TTimeComplex"; }
hashValueType TTimeComplex::hashValue( void ) const { return 0; }
void TTimeComplex::printOn(ostream far&) const      { }
PTStreamable TTimeComplex::build( void )            { return( new TTimeComplex( streamableInit ) ); }
TStreamableClass RegTTimeComplex( "TTimeComplex", TTimeComplex::build, __DELTA(TTimeComplex) );

// ====================================================
//
//                  Input/Output
//
// ====================================================

// ---------------------------------------------------
// Streamable Constructor
// ---------------------------------------------------
void TTimeComplex::TTimeComplex( StreamableInit s ):
    itsStart( s ),
    itsEnd( s ),
    itsDuration( s ),
    itsWorkTime( s )
{
    itsUseWorkTime = FALSE;
    itsTimeUnit = k15Min;
}


// ---------------------------------------------------
// Read
// ---------------------------------------------------
Pvoid TTimeComplex::read( Ripstream is )
{
    int version, i;

    is >> version;

    if (version == 3) {             //added unit length

        itsStart.read( is );
        itsEnd.read( is );
        itsDuration.read( is );

        is >> itsUseWorkTime;

        itsWorkTime.read( is );

        is >> i;
        itsTimeUnit = (TTimeUnit) i;

        is >> itsUnitLength;
    }
    else if (version == 2) {         //direct writes

        itsStart.read( is );
        itsEnd.read( is );
        itsDuration.read( is );

        is >> itsUseWorkTime;

        itsWorkTime.read( is );

        is >> i;
        itsTimeUnit = (TTimeUnit) i;

        //fortunately, no released version created TTimeComp files
        // without itsUnitLength, because this worktime calc is
        // wrong and can't really be fixed...
        if( itsUseWorkTime )
            itsUnitLength= itsWorkTime.GetNumUnits( itsTimeUnit );
        else
            itsUnitLength= itsDuration.GetNumUnits( itsTimeUnit );

    }
    else if (version == 1) {

        is >> itsStart;
        is >> itsEnd;
        is >> itsDuration;
        is >> itsUseWorkTime;
        is >> itsWorkTime;
        is >> i;
        itsTimeUnit = (TTimeUnit) i;

        //fortunately, no released version created TTimeComp files
        // without itsUnitLength, because this worktime calc is
        // wrong and can't really be fixed...
        if( itsUseWorkTime )
            itsUnitLength= itsWorkTime.GetNumUnits( itsTimeUnit );
        else
            itsUnitLength= itsDuration.GetNumUnits( itsTimeUnit );
    }
    else
	    SYSERR( ERR_VER );

    ModifyForChangedWorkTime();

    return this;
}

// ---------------------------------------------------
// Write
// ---------------------------------------------------
void TTimeComplex::write( Ropstream os )
{
    UpdateUnitLength();

    // version
    os << 3;

    itsStart.write( os );
    itsEnd.write( os );
    itsDuration.write( os );

    os << itsUseWorkTime;

    itsWorkTime.write( os );

    os << itsTimeUnit;
    os << itsUnitLength;
}

//*****************************************************************************
// itsUnitLength is not explicitly maintained.  Call this function before
// writing to file and before modifying work schedule.  itsUnitLength
// is used by ModifyForChangedWorkTime() to update worktime correctly.
//*****************************************************************************
void TTimeComplex::UpdateUnitLength()
{
//figure out the length, in itsTimeUnits, given the current
// work schedule, and store it in the object
if( itsUseWorkTime )
    {
    itsUnitLength= itsWorkTime.GetNumUnits( itsTimeUnit );
    }
else
    {
    itsUnitLength= itsDuration.GetNumUnits( itsTimeUnit );
    }
}


// ---------------------------------------------------
// ModifyForChangedWorkTime
//
//  The user redefined work time in the work schedule
//  or holiday dialog; also called when note is first read in from
//  file, as work schedule now may be different from when
//  that file was written..
//
//  Calculations are base on the stored itsUnitLength value.
//
//  Returns true if the times are modified
// ---------------------------------------------------
BOOL TTimeComplex::ModifyForChangedWorkTime( void )
{
TWorkTime oldWorkTime= itsWorkTime;
TTime oldStart = itsStart;
TTime oldEnd   = itsEnd;

//critical assumption: itsUnitLength is correct.

    if (itsUseWorkTime)
        {
        if( itsStart.HasDate() )
            ForwardToWorkTime( itsStart );

        //Assumes worktime units should be kept constant.
        itsWorkTime.Set( itsTimeUnit, itsUnitLength );

        if( itsEnd.HasDate() )
            {
            itsWorkTime.CalculateEnd( itsStart, itsEnd );

            //may have changed duration, due to work length change
            itsDuration.Set( itsStart, itsEnd );
            }
        }
    else
        {
        //Assumes start, end dates are correct
        if( itsStart.HasDate() && itsEnd.HasDate() )
            itsWorkTime.Set( itsStart, itsEnd );
        else
            {
            //if a note has a duration, but no start date
            //then its worktime is set from its duration
            itsWorkTime.Set( itsDuration );
            }
        }


    CHKBUG( ERR_GEN, HealthCheck());

    if( itsStart == oldStart && itsEnd == oldEnd && itsWorkTime == oldWorkTime )
        return FALSE;
    else
        return TRUE;
}


// ---------------------------------------------------
// HealthCheck
//
// Returns TRUE if it's ok
// ---------------------------------------------------
BOOL TTimeComplex::HealthCheck( void )
{
    BOOL ok = TRUE;

    // check that each element is ok individually
    ok = itsStart.HealthCheck()   &&
        itsEnd.HealthCheck()      &&
        itsDuration.HealthCheck() &&
        itsWorkTime.HealthCheck() &&
        (k1Min <= itsTimeUnit)    &&
        (itsTimeUnit <= k1Week)   &&
        (FALSE <= itsUseWorkTime) &&
        (itsUseWorkTime <= TRUE);

    // check tasks
    if (itsStart.HasDate() && itsEnd.HasDate()) {

        // make sure the start comes before the end
        ok &= itsStart <= itsEnd;

        // make sure the time units are consistent
        if (itsStart.HasTime()) {

            // the end must have a time also;
            // the units must be in hours
            ok &= itsEnd.HasTime();
            ok &= itsTimeUnit < k1Day;
        }
        else {

            // the end mustn't have a time also;
            // the units must be in days or weeks
            ok &= !itsEnd.HasTime();
            ok &= itsTimeUnit >= k1Day;
        }

        // make sure the duration and worktime are right
        TDuration duration( itsStart, itsEnd );
        ok &= itsDuration == duration;
        TWorkTime worktime( itsStart, itsEnd );
        ok &= itsWorkTime == worktime;

        // make sure the start and end are worktimes
        if (itsUseWorkTime) {

            if (itsStart.HasTime())
                ok &= itsStart.IsWorkTime() && itsEnd.IsWorkTime();
            else
                ok &= itsStart.IsWorkDay() && itsEnd.IsWorkDay();
        }
    }

    // check milestones
    else if (itsStart.HasDate() && !itsEnd.HasDate()) {

        // milestones have no duration or worktime
        ok &= itsDuration.IsZero() && itsWorkTime.IsZero();

        // if the start has a time, the units should
        // be in hours, otherwise the units should be
        // be in days or weeks
        if (itsStart.HasTime()) {

            ok &= (itsTimeUnit < k1Day);
        }
        else {

            ok &= (itsTimeUnit >= k1Day);
        }

        // make sure the start is a worktime
        if (itsUseWorkTime) {

            if (itsStart.HasTime())
                ok &= itsStart.IsWorkTime();
            else
                ok &= itsStart.IsWorkDay();
        }
    }

    // check floats
    else if (!(itsStart.HasDate() || itsEnd.HasDate())) {

        // nothing (right?)
    }

    // not a milestone, float or task
    // (end has date, but start doesn't)
    else {

        return FALSE;
    }


    return ok;
}    
