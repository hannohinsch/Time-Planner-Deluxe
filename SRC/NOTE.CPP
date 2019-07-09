//*****************************************************************************
// note object
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "note.h"
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __classid__
#include "classid.h"
#endif
#ifndef __cat__
#include "cat.h"
#endif
#ifndef __catlist__
#include "catlist.h"
#endif
#ifndef __cattree__
#include "cattree.h"
#endif
#ifndef __pen__
#include "pen.h"
#endif
#ifndef __parse__
#include "parse.h"
#endif
#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __alarmset__
#include "alarmset.h"
#endif
#ifndef __freq__
#include "freq.h"
#endif
#ifndef __worktime__
#include "worktime.h"
#endif

//*****************************************************************************
// new note object
//*****************************************************************************
TNote::TNote( void )
{
    itsCatListP    = new TCatList;
    itsInkMode    = TPen::bPenActive;     //start in itsInk if on pen system
    itsDone        = FALSE;
    itsNoteID      = TSysState::nextNoteID++;
    itsRateFlag = FALSE;
    itsANewNote = TRUE;
    itsFreqP    = NULL;

    itsRate.Clear();
    itsValue.Clear();

    itsColor.Set( kGray );
}

//*****************************************************************************
// Copy Constructor
//  FIXME use member's copy constructors, once we have them all defined
//*****************************************************************************
TNote::TNote( RTNote oldNote )
{
    itsCatListP      = new TCatList( *oldNote.itsCatListP );
    itsInkMode      = oldNote.itsInkMode;
    itsInk           = oldNote.itsInk;
    itsDone          = oldNote.itsDone;
    itsRateFlag   = oldNote.itsRateFlag;
    itsFreqP      = NULL;     //must start as NULL, even if SetFreq is called!

    // carefully copy the old frequency
    if (oldNote.itsFreqP) {

        SetFreq( *oldNote.itsFreqP );
    }

    //NOTE that the newly copied note is marked as NEW, even though
    // it is a copy of an existing note.
    itsANewNote   = TRUE;

    itsText.Set(     oldNote.itsText );
    itsColor.Set(    oldNote.itsColor.Get() );

    itsTimes = oldNote.itsTimes;
    SetRate(     oldNote.itsRate     );
    SetValue(    oldNote.itsValue    );

    //new note has separate ID
    itsNoteID= TSysState::nextNoteID++;

}


//*****************************************************************************

//*****************************************************************************
TNote::~TNote( void )
{
if( itsCatListP != NULL )
    delete itsCatListP;

if( TSysState::pAlarms != NULL)
    {
    PTAlarm alarmP = TSysState::pAlarms->AlarmForNote( *this );
    if (alarmP != NULL) 
        {
        TSysState::pAlarms->detach( *alarmP );
        delete alarmP;
        }
    }

if (itsFreqP != NULL)
    {
    delete itsFreqP;
    itsFreqP = NULL;
    }

}

//*****************************************************************************
// Title
//
//  Make up a title for the note from its itsText data
//*****************************************************************************
void TNote::Title( RTSafStr title )
{
    char buf[50];

    // copy the first 40 characters of itsText into buf
    int n = min( 40, itsText.Len() );
    strncpy( buf, itsText.Addr(), n );
    buf[ n ]= '\0';

    // if there's a carriage return in there somewhere, then
    // cut the first line down even further
    char *p= strchr( buf, '\r' );
    if( p != NULL ) {

        *p = '\0';
    }

    // set the title
    title.Set( buf );
}

//*****************************************************************************

//*****************************************************************************
BOOL TNote::HealthCheck( void )
{
BOOL ok= TRUE;

ok &= itsCatListP->HealthCheck();
ok &= itsText.HealthCheck();
ok &= itsTimes.HealthCheck();

return ok;
}

//*****************************************************************************
// overload basic Object functions
//*****************************************************************************
classType TNote::isA( void ) const
{
return IDNote;
}

char * TNote::nameOf( void ) const
{
return "TNote";
}

hashValueType TNote::hashValue( void ) const
{
return 0; //FIXME
}

int TNote::isEqual(const Object & obj) const
{
return( this == & obj );
}

// -----------------------------------------------------
// ExactlyMatches
//
// Compare each of the fields of this with the fields of
// note and return TRUE only if they all match.
// -----------------------------------------------------
BOOL TNote::ExactlyMatches( RTNote note )
{
return(
     itsTimes.ExactlyMatches( note.itsTimes )    &&
    (itsText        == note.itsText )       &&
    (itsValue    == note.itsValue)    &&
    (itsRate     == note.itsRate));
}

void TNote::printOn(ostream far&) const
{
//later
}

//*****************************************************************************
// isLessThan
//
// Sort according to the NoteOrder options in the profile
//*****************************************************************************
int TNote::isLessThan( const Object & other ) const
{
    RTNote otherNote= (RTNote) other;

    // compare up to three times
    int comparison = Compare( otherNote, TProfile::NoteOrder[0] );
    if (comparison == 0) {

        comparison = Compare( otherNote, TProfile::NoteOrder[1] );
        if (comparison == 0) {

            comparison = Compare( otherNote, TProfile::NoteOrder[2] );
        }
    }


    if (comparison < 0) {

        return( TRUE );
    }
    else {

        return( FALSE );
    }
}

//*****************************************************************************
// compare
//
// compare two notes according to the criteria specified
//
//                  value
//   condition      returned
//   -------------  --------
//   note == other  0
//   note < other  -1
//   note > other   1
//*****************************************************************************
int TNote::Compare( RTNote other, TNoteOrder criterion, BOOL useTime ) const
{
    switch( criterion ) {

        case kNoteOrder_Color:

            TVirtColor itsColor1 = itsColor.Get();
            TVirtColor itsColor2 = other.itsColor.Get();

            if      (itsColor1 == itsColor2)            return(  0 );
            else if (itsColor1 <  itsColor2)            return( -1 );
            else                                  return(  1 );

        case kNoteOrder_Date:

            TTime start, otherStart;
            start      = itsTimes.GetStart();
            otherStart = other.itsTimes.GetStart();

            if (!useTime)
                {
                start.ClearTime();
                otherStart.ClearTime();
                }

            if      (start == otherStart) return(  0 );
            else if (start <  otherStart) return( -1 );
            else                          return(  1 );

        case kNoteOrder_Duration:

            TDuration duration( itsTimes.GetDuration() );
            TDuration otherDuration( other.itsTimes.GetDuration() );

            if      (duration == otherDuration)  return(  0 );
            else if (duration <  otherDuration)  return( -1 );
            else                                 return(  1 );


        case kNoteOrder_Type:

            BOOL style      = GetStyle();
            BOOL otherStyle = other.GetStyle();
            if      (style == otherStyle)         return(  0 );
            else if (style <  otherStyle)         return( -1 );
            else                                  return(  1 );

        case kNoteOrder_Alpha:

            if      (itsText == other.itsText)          return(  0 );
            else if (itsText <  other.itsText)          return( -1 );
            else                                  return(  1 );


    }

    return( 0 );
}

//*****************************************************************************
// stream support
//*****************************************************************************
PTStreamable TNote::build( void )
{
return( new TNote( streamableInit ) );
}

TStreamableClass RegTNote( "TNote", TNote::build, __DELTA(TNote) );

//*****************************************************************************
// note assumption that catlist array has not been allocated!
// i.e. we used streamableinit constructor
//*****************************************************************************
Pvoid TNote::read( Ripstream is )
{
int       version, i;
TTime     start, end;
TDuration duration;
BOOL      isWorkTime  = FALSE;
BOOL      updateTimes = TRUE;
TTimeUnit unit;


is >> version;

itsFreqP = NULL;             // (frequency is read in iff version >= 8)

if( version == 12 )           // replaces itsStart, itsEnd, itsDuration
    {
    itsCatListP= new TCatList( streamableInit );
    itsCatListP->read( is );

    itsText.read( is );
    itsColor.read( is );

    itsValue.read( is );

    itsRate.read( is );

    is >> itsInkMode;

    itsInk.read( is );
    is >> itsDone;

    itsFreqP= NULL;
    BOOL hasFreq;
    is >> hasFreq;
    if( hasFreq )
        {
        itsFreqP= new TFreq( streamableInit );
        itsFreqP->read( is );
        }

    // direct
    itsTimes.read( is );
    updateTimes = FALSE;

    //id is not stored with note
    itsNoteID= TSysState::nextNoteID++;

    }
else if( version == 11 )           // replaces itsStart, itsEnd, itsDuration
    {                         // with itsTimes
    is >> itsCatListP;

    itsText.read( is );
    itsColor.read( is );

    itsValue.read( is );

    itsRate.read( is );

    is >> itsInkMode;

    itsInk.read( is );
    is >> itsDone;

    is >> itsFreqP;

    // direct
    itsTimes.read( is );
    updateTimes = FALSE;

    //id is not stored with note
    itsNoteID= TSysState::nextNoteID++;

    }
else if( version == 10 )           // adds itsWorkTime
    {
    is >> itsCatListP;

    itsText.read( is );
    itsColor.read( is );

    start.read( is );
    duration.read( is );
    itsValue.read( is );

    itsRate.read( is );

    is >> i;
    unit = (TTimeUnit) i;

    is >> itsInkMode;

    itsInk.read( is );
    is >> itsDone;
    is >> itsFreqP;
    is >> isWorkTime;

    //id is not stored with note
    itsNoteID= TSysState::nextNoteID++;

    }
else if( version == 9 )           // direct writes
    {
    is >> itsCatListP;

    itsText.read( is );
    itsColor.read( is );

    start.read( is );
    duration.read( is );
    itsValue.read( is );

    itsRate.read( is );

    is >> i;
    unit = (TTimeUnit) i;

    is >> itsInkMode;

    itsInk.read( is );
    is >> itsDone;
    is >> itsFreqP;

    //id is not stored with note
    itsNoteID= TSysState::nextNoteID++;
    }
else if( version == 8 )           // added frequency  (version 1.00)
    {
    is >> itsCatListP;
    is >> itsText;
    is >> itsColor;

    is >> start;
    is >> duration;
    is >> itsValue;

    is >> itsRate;
    is >> i;
    unit = (TTimeUnit) i;

    //itsInkMode
    is >> itsInkMode;

    //pendata
    is >> itsInk;

    is >> itsDone;

    is >> itsFreqP;

    //id is not stored with note
    itsNoteID= TSysState::nextNoteID++;
    }

else
	SYSERR( ERR_VER );

// calculate the dependent fields (new with version 7)
if (updateTimes) {

    // if the start has a date and no time, then
    // make sure the unit is at least k1Day
    if (start.HasDate() && (!start.HasTime()) &&
        (unit < k1Day)) {

        unit = k1Day;
    }
    // if the start has a date and a time, then
    // make sure the unit is at most k1Hour
    else if ((start.HasDate() && start.HasTime()) &&
        (unit >= k1Day)) {

        unit = k1Hour;
    }

    itsTimes.SetFromRawData( start, duration,
        unit, isWorkTime );
}
SetAmount();

itsRateFlag= FALSE;
itsANewNote= FALSE;

return this;
}

//*****************************************************************************
//
//*****************************************************************************
void TNote::write( Ropstream os )
{
os << (int) 12;      //current version #
                    // 3->4: added itsDuration
                    // 4->5: changed rateitsText to itsRate
                    //       deleted freqitsText, screenSize, title
                    //       changed unititsText to itsTimeUnit
                    // 6:    added itsDone field
                    // 7:    no longer writing out end or style fields
                    // 8:    added frequency
                    // 9:    added direct writes
                    // 10:   added itsWorkTime
                    // 11:   replace itsStart, itsDuration, itsUnit
                    //       with itsTimes
                    // 12:   direct writes for catlist, freq

itsCatListP->write(os);

itsText.write( os );
itsColor.write( os );

itsValue.write( os );

itsRate.write( os );

os << itsInkMode;
itsInk.write( os );

os << itsDone;

if( itsFreqP )
    {
    os << TRUE;
    itsFreqP->write( os );
    }
else
    {
    os << FALSE;
    }

itsTimes.write( os );
}

//*****************************************************************************
// NOTE WELL :gotta call right constructor for member objects
//*****************************************************************************
void TNote::TNote( StreamableInit s ) :
	itsColor(s ),
	itsText(s),
    itsTimes( s ),
    itsValue(s),
	itsRate(s),
	itsInk(s)
{
    itsFreqP    = NULL;
};





// -------------------------------------------------------------------------
// Data operators
// -------------------------------------------------------------------------
TTime      TNote::GetStart(    void ) const   { return itsTimes.GetStart();    }
TTime      TNote::GetEnd(      void ) const    { return itsTimes.GetEnd();      }
TValue     TNote::GetRate(     void ) const    { return itsRate;     }
TValue     TNote::GetValue(    void ) const    { return itsValue;    }
TDuration  TNote::GetDuration( void ) const    { return itsTimes.GetDuration(); }
TTimeUnit  TNote::GetTimeUnit( void ) const    { return itsTimes.GetTimeUnit();    }
BOOL       TNote::IsWorkTime(  void ) const    { return itsTimes.IsWorkTime(); }


// -------------------------------------------------------------------------
// GetWorkTime
// -------------------------------------------------------------------------
TWorkTime TNote::GetWorkTime( void ) const
{
    return itsTimes.GetWorkTime();
}

// -------------------------------------------------------------------------
// GetFreq
// -------------------------------------------------------------------------
PTFreq  TNote::GetFreq(     void ) const
{
    if (itsFreqP == NULL) {

        return NULL;
    }
    else if (!itsFreqP->IsRealFreq()) {

        return NULL;
    }
    else {

        return itsFreqP;
    }
}

//*****************************************************************************

//*****************************************************************************
void TNote::SetTimes( RTTimeComplex times )
{
    itsTimes = times;
    UpdateFieldsAfterTimeChange();
};

// -------------------------------------------------------------------------
// SetStartAndKeepDuration
// -------------------------------------------------------------------------
void TNote::SetStartAndKeepDuration( RTTime start )
{
    itsTimes.SetStartAndKeepDuration( start );
    UpdateFieldsAfterTimeChange();
}

// -------------------------------------------------------------------------
// SetDurationAndKeepStart
// -------------------------------------------------------------------------
void TNote::SetDurationAndKeepStart( RTDuration duration )
{
    itsTimes.SetDurationAndKeepStart( duration );
    UpdateFieldsAfterTimeChange();
}


// -------------------------------------------------------------------------
// SetStartAndClearDuration
// -------------------------------------------------------------------------
void TNote::SetStartAndClearDuration( RTTime start )
{
    itsTimes.SetStartAndClearDuration( start );
    UpdateFieldsAfterTimeChange();
}

// -------------------------------------------------------------------------
// KeepStartAndClearDuration
// -------------------------------------------------------------------------
void TNote::KeepStartAndClearDuration( void )
{
    itsTimes.SetStartAndClearDuration( itsTimes.GetStart() );
    UpdateFieldsAfterTimeChange();
}

// -------------------------------------------------------------------------
// SetStartAndEnd
// -------------------------------------------------------------------------
void TNote::SetStartAndEnd( RTTime start, RTTime end, TTimeUnit unit )
{
    itsTimes.SetStartAndEnd( start, end, unit );
    UpdateFieldsAfterTimeChange();
}

// -------------------------------------------------------------------------
// SetFromRawData
// -------------------------------------------------------------------------
void TNote::SetFromRawData( RTTime start, RTDuration duration,
    TTimeUnit unit, BOOL isWorkTime )
{
    itsTimes.SetFromRawData( start, duration, unit, isWorkTime );
    UpdateFieldsAfterTimeChange();
}


// -------------------------------------------------------------------------
// UpdateFieldsAfterTimeChange
// -------------------------------------------------------------------------
void TNote::UpdateFieldsAfterTimeChange( void )
{
    // if the note has an alarm, then reset its time
    if( TSysState::pAlarms != NULL ) {

        PTAlarm alarmP = TSysState::pAlarms->AlarmForNote( *this );
        if (alarmP != NULL) {

            alarmP->SetBaseTime( GetStart(), itsFreqP );
        }
    }

    TSysState::bSystemDirty = TRUE;
    SetAmount();
}

// -------------------------------------------------------------------------
// SetInkMode
// -------------------------------------------------------------------------
void TNote::SetInkMode( BOOL inkMode )
{
    itsInkMode = inkMode;
}

// -------------------------------------------------------------------------
// SetText
// -------------------------------------------------------------------------
void TNote::SetText( RTSafStr text )
{
    itsText = text;
}

// -------------------------------------------------------------------------
// SetWorkTime
// -------------------------------------------------------------------------
void TNote::SetWorkTime( BOOL workTime )
{
    itsTimes.SetWorkTime( workTime );
}

// -------------------------------------------------------------------------
// SetCatList
// -------------------------------------------------------------------------
void TNote::SetCatList( PTCatList listP )
{
    delete itsCatListP;
    itsCatListP = listP;
}

// -------------------------------------------------------------------------
// SetRate
// -------------------------------------------------------------------------
void TNote::SetRate( RTValue rate )
{
    itsRate     = rate;
    TSysState::bSystemDirty  = TRUE;

    SetAmount();
}

// -------------------------------------------------------------------------
// SetValue
// -------------------------------------------------------------------------
void       TNote::SetValue(    RTValue    value )
{
    itsValue    = value;
    TSysState::bSystemDirty  = TRUE;
}

// -------------------------------------------------------------------------
// SetTimeUnit
// -------------------------------------------------------------------------
void TNote::SetTimeUnit( TTimeUnit unit )
{
    TTimeUnit oldUnit = itsTimes.GetTimeUnit();
    itsTimes.SetTimeUnit( unit );

    // If note has rate, then mark the time unit change.
    // This forces message notification next time note is opened
    if (oldUnit != unit) {

        itsRateFlag = itsRate.HasValue();
        TSysState::bSystemDirty  = TRUE;
    }
}

// -------------------------------------------------------------------------
// SetFreq
// -------------------------------------------------------------------------
void TNote::SetFreq( RTFreq freq )
{
    //don't bother saving if freq == kOnce ...
    if( !freq.IsRealFreq() ) {

        if( itsFreqP ) {

            delete itsFreqP;
            itsFreqP = NULL;
        }

        return;
    }

    // create a new frequency if there is none yet
    if (itsFreqP == NULL) {

        itsFreqP = new TFreq;
    }

    // copy the frequency
    (*itsFreqP) = freq;

    // if the note has an alarm, then reset its time
    if( TSysState::pAlarms != NULL ) {
        PTAlarm alarmP = TSysState::pAlarms->AlarmForNote( *this );
        if (alarmP != NULL) {

            alarmP->SetBaseTime( GetStart(), itsFreqP );
        }
    }

    TSysState::bSystemDirty  = TRUE;
}


// -------------------------------------------------------------------------
// GetStyle
// -------------------------------------------------------------------------
TNoteStyle TNote::GetStyle( void ) const
{
    if (GetStart().HasDate()) {

        if (GetDuration().IsZero()) {

            return kMilestoneStyle;
        }
        else {

            return kTaskStyle;
        }
    }
    else if (GetDuration().IsZero()) {

        return kItemStyle;
    }
    else {

        return kFloatStyle;
    }
}

// -------------------------------------------------------------------------
// SetAmount
// -------------------------------------------------------------------------
void TNote::SetAmount( void )
{
    if (itsRate.HasValue()) {

        if( GetDuration().IsZero() ) {
            itsValue= 0L;
        }
        else {
            long factor= 1;
            TTimeUnit units= GetTimeUnit();

            // rate is hourly, even if units are sub-hourly
            switch( units ) {

                case k1Min:     factor = 60;    break;
                case k5Min:     factor = 12;    break;
                case k6Min:     factor = 10;    break;
                case k10Min:    factor =  6;    break;
                case k12Min:    factor =  5;    break;
                case k15Min:    factor =  4;    break;
                case k20Min:    factor =  3;    break;
                case k30Min:    factor =  2;    break;
                case k1Hour:    factor =  1;    break;
                case k1Day:     factor =  1;    break;
                case k1Week:    factor =  1;    break;
            }

            long length;
            if (IsWorkTime()) {

                length = GetWorkTime().GetNumUnits( units );
            }
            else {

                length = GetDuration().GetNumUnits( units );
            }
            itsValue    = itsRate * length;
            itsValue   /= factor;
        }
    }
}



// -------------------------------------------------------------------------
// CheckRateFlag
// -------------------------------------------------------------------------
BOOL TNote::CheckRateFlag( void )
{
    return itsRateFlag;
}


// -------------------------------------------------------------------------
// ClearRateFlag
// -------------------------------------------------------------------------
void TNote::ClearRateFlag( void )
{
    itsRateFlag = FALSE;
}

// -------------------------------------------------------------------------
// HollowOnDate
//
//  Should the note show up as hollow in the timeline
//  and calendar views on this date? Only work time
//  notes on holidays and weekends should be hollow
// -------------------------------------------------------------------------
BOOL TNote::HollowOnDate( RTTime date )
{
    // elapsed time notes are never hollow
    if (!itsTimes.IsWorkTime()) return FALSE;

    // work time notes on non-work days are hollow
    return !date.IsWorkDay();
}

// -------------------------------------------------------------------------
// ModifyForChangedWorkTime
//
//  The user changed the definition of worktime in the
//  worktime schedule dialog
// -------------------------------------------------------------------------
BOOL TNote::ModifyForChangedWorkTime( void )
{
    BOOL rc= itsTimes.ModifyForChangedWorkTime();
    if ( rc ) {

        //FIXME what happens if alarmset hasn't yet been read in?
        UpdateFieldsAfterTimeChange();

        #if 0
        TSafStr prompt;
        TSafStr title;
        Title( title );
        prompt.Printf( "Times for '%s' have been changed to accommodate the new work schedule.",
           SAF2CP( title ) );
        TMessageBox::Ok( TPimFrame::pFrame, prompt, "Times Modified" );
        #endif

        TSysState::bSystemDirty = TRUE;

    }

    return rc;
}

//*****************************************************************************

//*****************************************************************************
void TNote::UpdateUnitLength( void )
{
    itsTimes.UpdateUnitLength();
}
