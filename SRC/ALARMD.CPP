//*****************************************************************************
// alarm dialogs: alarm set & alarm ringing
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "alarmd.h"
#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __noteview__
#include "noteview.h"
#endif
#ifndef __frame__
#include "frame.h"
#endif
#ifndef __alarmset__
#include "alarmset.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TAlarmDialog::TAlarmDialog( PTWindowsObject AParent, PTAlarm alarmP ):
	TOurDialog( AParent, IDD_ALARM )
{

    itsTextP        = new TStatic(    this, IDD_ALARM_TEXT, 256 );
    itsScrollBarP   = new TScrollBar( this, IDD_ALARM_SCROLL );
    itsUnitP        = new TComboBox(  this, IDD_ALARM_UNITS, 256 );

    itsAlarmP       = alarmP;
    itsHelpTopic    = _HelpAlarm;
}


// -------------------------------------------------
// Destructor
// -------------------------------------------------
TAlarmDialog::~TAlarmDialog( void )
{
}

// -------------------------------------------------
// Setup
// -------------------------------------------------
void TAlarmDialog::SetupWindow( void )
{
    char s[80];

    // inherited
    TOurDialog::SetupWindow();

    // setup the scroll bar and text
    itsScrollBarP->SetRange( 0, 10000 );
    itsScrollBarP->SetPosition( itsAlarmP->itsOffset );
    wsprintf( s, "%d", itsAlarmP->itsOffset );
    itsTextP->SetText( s );

    // initialize the unit box (manually because of Borland combo box bug)
    itsUnitP->AddString( "minutes" );
    itsUnitP->AddString( "hours" );
    itsUnitP->AddString( "days" );
    itsUnitP->AddString( "weeks" );

    switch( itsAlarmP->itsUnit ) {

        case k1Min:
        case k5Min:
        case k6Min:
        case k10Min:
        case k12Min:
        case k15Min:
        case k20Min:
        case k30Min:     itsUnitP->SetSelIndex( 0 );     break;
        case k1Hour:     itsUnitP->SetSelIndex( 1 );     break;
        case k1Day:      itsUnitP->SetSelIndex( 2 );     break;
        case k1Week:     itsUnitP->SetSelIndex( 3 );     break;
    }
}



// -------------------------------------------------
// HandleUnit
// -------------------------------------------------
void TAlarmDialog::HandleUnit( RTMessage )
{
    switch( itsUnitP->GetSelIndex() ) {

        default:
        case 0:      itsAlarmP->SetUnit(  k1Min  );  break;
        case 1:      itsAlarmP->SetUnit(  k1Hour );  break;
        case 2:      itsAlarmP->SetUnit(  k1Day  );  break;
        case 3:      itsAlarmP->SetUnit(  k1Week );  break;
    }
}


// -------------------------------------------------
// WMHScroll
// -------------------------------------------------
void TAlarmDialog::WMHScroll( RTMessage Msg )
{
    char s[80];
    switch( Msg.WParam ) {

        case SB_LINEDOWN:

            itsAlarmP->SetOffset( min( 10000, itsAlarmP->itsOffset + 1 ));
            break;

        case SB_LINEUP:

            itsAlarmP->SetOffset( max( 0, itsAlarmP->itsOffset - 1 ));
            break;

        default:

            return;
    }

    wsprintf( s, "%d", itsAlarmP->itsOffset );
    itsTextP->SetText( s );
}



// ************************************************************************
//
//  ALARM RINGS DIALOG
//
// ************************************************************************

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TAlarmRingsDialog::TAlarmRingsDialog( PTWindowsObject AParent, PTAlarm alarmP ):
	TOurDialog( AParent, IDD_ALARM_RINGS )
{

    itsAlarmP        = alarmP;

    ASSERT_STR( "Constructor: bad alarm pointer", itsAlarmP != NULL );

    itsNoteTextCtl   = new TStatic( this, IDD_ALARM_NOTE,      256 );
    itsNoteTimeCtl   = new TStatic( this, IDD_ALARM_NOTE_TIME, 256 );
    itsTimeNowCtl    = new TStatic( this, IDD_ALARM_TIME_NOW,  256 );
    itsSnoozeCtl     = new TButton( this, IDOK );
    itsShowNoteCtl   = new TButton( this, IDD_ALARM_SHOW_NOTE );

}


// -------------------------------------------------
// Destructor
// -------------------------------------------------
TAlarmRingsDialog::~TAlarmRingsDialog( void )
{
}

// -------------------------------------------------
// Setup
// -------------------------------------------------
void TAlarmRingsDialog::SetupWindow( void )
{
    char buf[80];

    // inherited
    TOurDialog::SetupWindow();

    MessageBeep( MB_ICONEXCLAMATION );

    // get the alarm's note and title
    ASSERT_STR( "SetupWindow: bad alarm pointer", itsAlarmP != NULL );

    PTNote  noteP = itsAlarmP->GetNote();
    TSafStr title;
    noteP->Title( title );
    itsNoteTextCtl->SetText( (char*) title.Addr() );

    // get its start time
    TIntlFmt::FormatDateAndTime( noteP->GetStart(), kShortDate, buf );
    itsNoteTimeCtl->SetText( buf );

    // get the current time
    TTime   timeNow;
    timeNow.SetNow();
    TIntlFmt::FormatDateAndTime( timeNow, kShortDate, buf );
    itsTimeNowCtl->SetText( buf );

    //make it topmost
    SetWindowPos( HWindow, HWND_TOPMOST, 0, 0, 0, 0,
                  SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE );
}

// -------------------------------------------------
// Ok: Snooze
// -------------------------------------------------
void TAlarmRingsDialog::Ok( RTMessage msg )
{
    ASSERT_STR( "Ok: bad alarm pointer", itsAlarmP != NULL );

    itsAlarmP->Snooze();
    TDialog::Ok( msg );
}

// -------------------------------------------------
// Cancel
// -------------------------------------------------
void TAlarmRingsDialog::Cancel( RTMessage msg )
{
    // itsAlarmP would be NULL iff the user deleted the alarm
    // while we were in HandleShowNote
    if (itsAlarmP != NULL) {

        TSysState::bSystemDirty= TRUE; //FIXME move into note sometime
        itsAlarmP->CancelAlarm();
    }

    TDialog::Cancel( msg );
}

// -------------------------------------------------
// HandleShowNote
// -------------------------------------------------
void TAlarmRingsDialog::HandleShowNote( RTMessage )
{
    ASSERT_STR( "HandleShowNote: bad alarm pointer", itsAlarmP != NULL );

    GetApplication()->ExecDialog( new TNoteView(
        this, IDD_NOTE, itsAlarmP->GetNote() ));

    #if 0  //now shuts down alarm dialog after note is visited
    // check to see that the alarm is still in the list, since while
    // the note is open the user might have turned off the alarm or
    // (worse still) deleted the note. If the alarm is now missing
    // then turn off "snooze" and "show note"
    if (!TSysState::pAlarms->AlarmInList( itsAlarmP )) {

        itsAlarmP = NULL;
        EnableWindow( itsSnoozeCtl->HWindow, FALSE );
        EnableWindow( itsShowNoteCtl->HWindow, FALSE );
    }
    #endif

    ShutDownWindow( IDOK );       //end dialog

}

