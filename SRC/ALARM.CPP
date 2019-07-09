//*****************************************************************************
// alarm objects
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "alarm.h"

// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------
TAlarm::TAlarm( RTAlarm alarm ):
    itsBaseTime( alarm.itsBaseTime ),
    itsAlarmTime( alarm.itsAlarmTime )
{
    itsNoteP  = alarm.itsNoteP;
    itsOffset = alarm.itsOffset;
    itsUnit   = alarm.itsUnit;
    itsFreqP  = (alarm.itsFreqP) ? new TFreq( *alarm.itsFreqP ) : NULL;
    itsDone   = FALSE;
}

// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------
TAlarm::TAlarm( PTNote noteP, RTTime time, PTFreq freqP ):
    itsBaseTime( time ),
    itsAlarmTime( time )
{
    itsNoteP  = noteP;
    itsOffset = 0;
    itsUnit   = k1Min;
    itsFreqP  = (freqP) ? new TFreq( *freqP ) : NULL;
    itsDone   = FALSE;
}

// -----------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------
TAlarm::~TAlarm( void )
{
    if (itsFreqP) {

        delete itsFreqP;
    }
}

// -----------------------------------------------------------------------
// SetBaseTime
// -----------------------------------------------------------------------
void TAlarm::SetBaseTime( RTTime time, PTFreq freqP )
{
    // throw away the old frequency
    if (itsFreqP) delete itsFreqP;

    itsBaseTime = time;
    itsFreqP    = (freqP) ? new TFreq( *freqP ) : NULL;

    SetAlarmTime();
}
    
// -----------------------------------------------------------------------
// SetUnit
// -----------------------------------------------------------------------
void TAlarm::SetUnit( TTimeUnit unit )
{
    itsUnit = unit;

    SetAlarmTime();
}
    
// -----------------------------------------------------------------------
// SetOffset
// -----------------------------------------------------------------------
void TAlarm::SetOffset( int offset )
{
    itsOffset = offset;

    SetAlarmTime();
}
    

// -----------------------------------------------------------------------
// operator =
// -----------------------------------------------------------------------
RTAlarm TAlarm::operator =( RTAlarm alarm )
{
    // throw away the old frequency
    if (itsFreqP) delete itsFreqP;

    itsNoteP     = alarm.itsNoteP;
    itsOffset    = alarm.itsOffset;
    itsUnit      = alarm.itsUnit;
    itsBaseTime  = alarm.itsBaseTime;
    itsAlarmTime = alarm.itsAlarmTime;
    itsDone      = alarm.itsDone;
    itsFreqP     = (alarm.itsFreqP) ? new TFreq( *alarm.itsFreqP ) : NULL;

    return *this;
}

// -----------------------------------------------------------------------
// SetAlarmTime
// -----------------------------------------------------------------------
void TAlarm::SetAlarmTime( void )
{
    if (itsFreqP) {

        BOOL    found = TRUE;
        TTime   now;

        now.SetNow();
        itsFreqP->First( itsAlarmTime );
        while ((itsAlarmTime <= now) && found) {

            found = itsFreqP->Next( itsAlarmTime );
        }

        if (found) {

            itsAlarmTime.PlusEquals( itsUnit, -itsOffset );
        }
        else {

            itsDone = TRUE;
        }
    }

    // no frequency
    else {

        itsAlarmTime = itsBaseTime.Plus( itsUnit, -itsOffset );
    }
}

// -----------------------------------------------------------------------
// Match
//
// Does this alarm belong to the specified note?
// -----------------------------------------------------------------------
BOOL TAlarm::Match( PTNote noteP )
{
    return( noteP == itsNoteP );
}

// -----------------------------------------------------------------------
// AlarmRinging
//
// Is the alarm ringing at the specified time?
// -----------------------------------------------------------------------
PTNote TAlarm::AlarmRinging( RTTime time )
{
    return (time >= itsAlarmTime) ? itsNoteP : NULL;
}
   
// -----------------------------------------------------------------------
// Snooze
//
// Put the alarm to sleep for a while
// -----------------------------------------------------------------------
void TAlarm::Snooze( void )
{
    itsAlarmTime.SetNow();
    itsAlarmTime.PlusEquals( k1Min, 10 );
}

// -----------------------------------------------------------------------
// CancelAlarm
//
// Kill the current alarm
// -----------------------------------------------------------------------
void TAlarm::CancelAlarm( void )
{
    if (itsFreqP) {

        // set up the next iteration
        SetAlarmTime();
    }
    else {

        itsDone = TRUE;
    }
}

// -----------------------------------------------------------------------
// IsDone
// -----------------------------------------------------------------------
BOOL TAlarm::IsDone( void )
{
    return itsDone;
}

// -------------------------------------------------------------------------
// Read
// -------------------------------------------------------------------------
TStreamableClass RegTAlarm( "TAlarm", TAlarm::build, __DELTA(TAlarm) );
Pvoid TAlarm::read( Ripstream is )
{
    int version, i;
    is >> version;

    if (version == 3) {               // version 3: added freq (htp 1.00)

        is >> itsNoteP;
        is >> itsBaseTime;
        is >> itsAlarmTime;
        is >> itsOffset;
        is >> i;
        itsUnit = (TTimeUnit) i;
        is >> itsFreqP;
    }
    else if (version == 2) {           // version 2: changed scale to unit

        is >> itsNoteP;
        is >> itsBaseTime;
        is >> itsAlarmTime;
        is >> itsOffset;
        is >> i;
        itsUnit = (TTimeUnit) i;
        itsFreqP = NULL;
    }
    else if (version == 1) {

        is >> itsNoteP;
        is >> itsBaseTime;
        is >> itsAlarmTime;
        is >> itsOffset;
        is >> i;
        switch( i ) {

            case 0: itsUnit = k1Min;       break;
            case 1: itsUnit = k1Hour;      break;
            case 2: itsUnit = k1Day;       break;
            case 3: itsUnit = k1Week;      break;
        }
        itsFreqP = NULL;
    }

    itsDone = FALSE;
    return this;
}

// -------------------------------------------------------------------------
// Write
// -------------------------------------------------------------------------
void TAlarm::write( Ropstream os )
{
    os << (int) 3;  //object version
    os << itsNoteP;
    os << itsBaseTime;
    os << itsAlarmTime;
    os << itsOffset;
    os << itsUnit;
    os << itsFreqP;
}


