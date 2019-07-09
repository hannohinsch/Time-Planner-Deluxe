//*****************************************************************************
// set of alarm oblects
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "alarmset.h"
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __notelisd__
#include "notelisd.h"
#endif
#ifndef __noteview__
#include "noteview.h"
#endif
#ifndef __frame__
#include "frame.h"
#endif
#ifndef __alarmd__
#include "alarmd.h"
#endif

// ------------------------------------------------------------------------
// Constructor
// ------------------------------------------------------------------------
TAlarmSet::TAlarmSet( void )
{
    pAlarmArray = new TXArray( 10, 10, TXArray::kNoSort );
}


// ------------------------------------------------------------------------
// Destructor
// ------------------------------------------------------------------------
TAlarmSet:: ~TAlarmSet( void )
{
    delete pAlarmArray;
}


// ------------------------------------------------------------------------
// Detach
// ------------------------------------------------------------------------
void TAlarmSet::detach( RTAlarm alarm )
{
   pAlarmArray->detach( alarm );
}

// ------------------------------------------------------------------------
// Add
// ------------------------------------------------------------------------
void TAlarmSet::add( RTAlarm alarm )
{
    pAlarmArray->add( alarm );
}

// ------------------------------------------------------------------------
// Flush
// ------------------------------------------------------------------------
void TAlarmSet::flush( TXArray::TDeleteStyle dt )
{
    pAlarmArray->flush( dt );
}

// ------------------------------------------------------------------------
// operator []
// ------------------------------------------------------------------------
RTAlarm TAlarmSet::operator []( int index )
{
    return (RTAlarm) (*pAlarmArray)[ index ];
}

// ------------------------------------------------------------------------
// AlarmForNote
// ------------------------------------------------------------------------
PTAlarm TAlarmSet::AlarmForNote( RTNote note )
{
    int numAlarms = pAlarmArray->getItemsInContainer();
    for (int i = 0; i < numAlarms; i++) {

        RTAlarm alarm = (RTAlarm) (*pAlarmArray)[i];
            if (alarm.Match( &note )) {

                return( &alarm );
            }
    }

    return NULL;
}

// ------------------------------------------------------------------------
// CheckAlarms
// ------------------------------------------------------------------------
void TAlarmSet::CheckAlarms( void )
{
    // because this method is called every ten seconds, even if we're
    // already in the middle of it, check to see if we should really
    // do anything.
    static BOOL executingAlready = FALSE;
    if (executingAlready) return;
    executingAlready = TRUE;

    TTime    now;

    now.SetNow();

    for (int i = pAlarmArray->getItemsInContainer() - 1; i >= 0; i--) {

        RTAlarm alarm = (RTAlarm) (*pAlarmArray)[i];

        PTNote noteP = alarm.AlarmRinging( now );
        if (noteP != NULL) {

            // put up the AlarmRings dialog (dialog will reset the
            // alarm time if the user chooses to snooze)
            PTAlarmRingsDialog dialogP = new TAlarmRingsDialog(
                TPimFrame::pFrame, &alarm );
            if (IDCANCEL == GetApplicationObject()->ExecDialog( dialogP )) {

                // check to see that the alarm is still in the list,
                // since the user might have deleted either the alarm
                // or (worse still) the note itself while the dialog
                // was up
                if (AlarmInList( &alarm )) {

                    // the alarm is still in the list, so see if it
                    // should be removed
                    if (alarm.IsDone()) {

                        // user cancelled the alarm: take it out and throw it away
                        detach( alarm );
                        delete( &alarm );
                    }
                }
            }
        }
    }

    executingAlready = FALSE;
}

// ------------------------------------------------------------------------
// AlarmInList
// ------------------------------------------------------------------------
BOOL TAlarmSet::AlarmInList( PTAlarm alarmP )
{
    // look for the alarm in the list of alarms
    for (int i = 0; i < getItemsInContainer(); i++) {

        PTAlarm ithAlarmP = (PTAlarm) &(*pAlarmArray)[i];
        if (alarmP == ithAlarmP) {

            return TRUE;
        }
    }

    return FALSE;
}



// ------------------------------------------------------------------------
// read
// ------------------------------------------------------------------------
TStreamableClass RegTAlarmSet( "TAlarmSet", TAlarmSet::build, __DELTA(TAlarmSet) );
Pvoid TAlarmSet::read( Ripstream is )
{
    int numAlarms;
    int version;

    is >> version;
    is >> numAlarms;

    pAlarmArray = new TXArray( numAlarms, 10, TXArray::kNoSort );

	for( int i = 0; i < numAlarms; i++ ) {

        PTAlarm pAlarm;
        is >> pAlarm;
        pAlarmArray->add( *pAlarm );
	}

    return this;
}

// ------------------------------------------------------------------------
// write
// ------------------------------------------------------------------------
void TAlarmSet::write( Ropstream os )
{
    os << (int) 1;      //version
    os << pAlarmArray->getItemsInContainer();

    int numAlarms = pAlarmArray->getItemsInContainer();
    for (int i = 0; i < numAlarms; i++) {

        RTAlarm alarm = (RTAlarm) (*pAlarmArray)[i];
        if (NOOBJECT != alarm) {

            os << &alarm;
        }
    }
}
