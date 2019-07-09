//*****************************************************************************
// timepicker dialog
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "timepick.h"

#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __ticker__
#include "ticker.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __markctl__
#include "markctl.h"
#endif
#ifndef __worktime__
#include "worktime.h"
#endif

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TTimePicker::TTimePicker( PTWindowsObject AParent, int AResID,
    char *, PTTime startTimeP, PTTime endTimeP, PTFreq freqP,
    PTDuration durationP,
    TTimeUnit *unitP, RTColor color ):
	TOurDialog( AParent, AResID )
{

//    itsHelpTopic       = _HelpTime;

    itsStartTimeP      = startTimeP;
    itsEndTimeP        = endTimeP;
    itsFreqP           = freqP;
    itsDurationP       = durationP;
    itsTimeUnitP       = unitP;

    // initialize controls
    itsMonthPickerCP    = new TDatePick( this, IDD_MONTH_PICKER, *startTimeP );
    itsTickerCP         = new TTicker( this, IDD_TIMEPICKER_TICKER,
                           *startTimeP, *durationP, unitP );
    itsTimeUnitBoxCP    = new TComboBox( this, IDD_TIMEPICKER_UNITS, 256 );
    itsDurationTextCP   = new TStatic( this, IDD_TIMEPICKER_DURATION_TEXT, 256 );
    itsWorkTimeTextCP   = new TStatic( this, IDD_TIMEPICKER_WORK_TIME, 256 );
    itsStartTextCP      = new TStatic( this, IDD_TIMEPICKER_START_TEXT, 256 );
    itsEndTextCP        = new TStatic( this, IDD_TIMEPICKER_END_TEXT, 256 );
    itsRadioGroupCP     = new TGroupBox( this, IDD_TIMEPICKER_GROUP );
    itsUseDurationCP    = new TRadioButton( this, IDD_TIMEPICKER_USE_DURATION, itsRadioGroupCP );
    itsUseTimesCP       = new TRadioButton( this, IDD_TIMEPICKER_USE_TIMES, itsRadioGroupCP );
    itsUseMilestoneCP   = new TRadioButton( this, IDD_TIMEPICKER_MILESTONE, itsRadioGroupCP );
    itsDurationTitleCP  = new TStatic( this, IDD_TIMEPICKER_DURATION_TITLE, 256 );
    itsWorkTimeTitleCP  = new TStatic( this, IDD_TIMEPICKER_WORK_TIME_TITLE, 256 );
    itsStartTitleCP     = new TStatic( this, IDD_TIMEPICKER_START_TITLE, 256 );
    itsEndTitleCP       = new TStatic( this, IDD_TIMEPICKER_END_TITLE, 256 );
    itsNowButtonCP      = new TBButton( this, IDD_TIMEPICKER_NOW );
    itsNoTimeButtonCP   = new TBButton( this, IDD_TIMEPICKER_NO_TIME );

    itsFloatCP=         new TMarkCtl( this, IDD_TIMEPICKER_FLOATCTL );
    itsFloatCP->Set( color, kFloatStyle );
    itsTaskCP=          new TMarkCtl( this, IDD_TIMEPICKER_TASKCTL );
    itsTaskCP->Set( color, kTaskStyle );
    itsItemCP=          new TMarkCtl( this, IDD_TIMEPICKER_ITEMCTL );
    itsItemCP->Set( color, kItemStyle );
    itsMilestoneCP=     new TMarkCtl( this, IDD_TIMEPICKER_MILESTONECTL );
    itsMilestoneCP->Set( color, kMilestoneStyle );

    // I hate the transfer mechanism, and the combo box transfer
    // doesn't work anyway
    DisableTransfer();


    // initialize the data
    itsDelta     = 1.0;
}

// -------------------------------------------------
// Destructor
// -------------------------------------------------
TTimePicker::~TTimePicker( void )
{
}

// -------------------------------------------------
// SetupWindow
// -------------------------------------------------
void TTimePicker::SetupWindow( void )
{

    // inherited
    TOurDialog::SetupWindow();

    // initialize the units box (manually because of Borland combo box bug)
    TSafStr s;
    TTime::GetUnitName( k1Min, s );
    itsTimeUnitBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k5Min, s );
    itsTimeUnitBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k6Min, s );
    itsTimeUnitBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k10Min, s );
    itsTimeUnitBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k12Min, s );
    itsTimeUnitBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k15Min, s );
    itsTimeUnitBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k20Min, s );
    itsTimeUnitBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k30Min, s );
    itsTimeUnitBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k1Hour, s );
    itsTimeUnitBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k1Day, s );
    itsTimeUnitBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k1Week, s );
    itsTimeUnitBoxCP->AddString( SAF2CP( s ) );

    // figure out what which time unit to use
    itsTimeUnitBoxCP->SetSelIndex( *itsTimeUnitP );


    // show/hide time controls based on whether start time is valid
    if (itsStartTimeP->HasDate()) {

        if (!itsDurationP->IsZero()) {

            itsUseTimesCP->Check();
            ShowDateControls( kStartEnd );
        }
        else {

            itsUseMilestoneCP->Check();
            ShowDateControls( kMilestoneOnly );
        }
    }
    else {

        itsUseDurationCP->Check();
        ShowDateControls( kDurationOnly );
    }

    // initialize the values for the various fields
    UpdateDialog();
}


// -------------------------------------------------
// UpdateDialog
//
//  Update the fields of the dialog from the internal
//  data structures
// -------------------------------------------------
void TTimePicker::UpdateDialog( void )
{
    char buf[256];

    itsTickerCP->GetTimes( *itsStartTimeP, *itsEndTimeP, *itsDurationP );
    TIntlFmt::FormatDateAndTime( *itsStartTimeP, kShortDate, buf );
    itsStartTextCP->SetText( buf );
    TIntlFmt::FormatDateAndTime( *itsEndTimeP, kShortDate, buf );
    itsEndTextCP->SetText( buf );
    itsDurationP->Format( *itsTimeUnitP, TRUE, buf );
    itsDurationTextCP->SetText( buf );

    TWorkTime workTime( *itsStartTimeP, *itsEndTimeP );
    workTime.Format( *itsTimeUnitP, TRUE, buf );
    itsWorkTimeTextCP->SetText( buf );

}


// -------------------------------------------------
// HandleUnit
//
//  Modifying the unit changes the unit of the
//  duration and the accuracy of the ticker
// -------------------------------------------------
void TTimePicker::HandleUnit( RTMessage Msg )
{
    if (Msg.LP.Hi == CBN_SELCHANGE) {

        *itsTimeUnitP = (TTimeUnit) itsTimeUnitBoxCP->GetSelIndex();
        itsTickerCP->SetTimeUnit( *itsTimeUnitP );
        UpdateDialog();
    }
}

// -------------------------------------------------
// HandleUseDuration
//
//  Use only the duration part of the time picker,
//  not the start and end times
// -------------------------------------------------
void TTimePicker::HandleUseDuration( RTMessage  )
{
    itsTickerCP->SwitchModes( kDurationOnly );
    ShowDateControls( kDurationOnly );
    UpdateDialog();
}

// -------------------------------------------------
// HandleUseTimes
//
//  Use the start and end times of the timepicker
//  not just the duration
// -------------------------------------------------
void TTimePicker::HandleUseTimes( RTMessage  )
{
    itsTickerCP->SwitchModes( kStartEnd );
    ShowDateControls( kStartEnd );
    UpdateDialog();
    itsMonthPickerCP->SetSelTime( *itsStartTimeP );
}

// -------------------------------------------------
// HandleUseMilestone
//
//  Select a milestone
// -------------------------------------------------
void TTimePicker::HandleUseMilestone( RTMessage  )
{
    itsTickerCP->SwitchModes( kMilestoneOnly );
    ShowDateControls( kMilestoneOnly );
    UpdateDialog();
    itsMonthPickerCP->SetSelTime( *itsStartTimeP );
}

// -------------------------------------------------
// HandleCalendar
//
//  The calendar changed: set the new start date
// -------------------------------------------------
void TTimePicker::HandleCalendar( RTMessage  )
{
    itsTickerCP->SetStart( itsMonthPickerCP->GetSelTime() );
    UpdateDialog();
}

// -------------------------------------------------
// HandleNow
// -------------------------------------------------
void TTimePicker::HandleNow( RTMessage  )
{
    TTime time;
    time.SetNow();

    itsTickerCP->SetStart( time );
    itsMonthPickerCP->SetSelTime( time );
    UpdateDialog();

}


// -------------------------------------------------
// HandleTicker
// -------------------------------------------------
void TTimePicker::HandleTicker( RTMessage  )
{
    UpdateDialog();
    itsMonthPickerCP->SetSelTime( *itsStartTimeP );

}


// -------------------------------------------------
// ShowDateControls
//
//  Show/Hide date controls
// -------------------------------------------------
void TTimePicker::ShowDateControls( TTickerMode mode )
{
    switch( mode ) {

        case kStartEnd:

            itsMonthPickerCP->Show( SW_SHOW );
            itsNowButtonCP->Show( SW_SHOW );
            itsStartTextCP->Show( SW_SHOW );
            itsStartTitleCP->Show( SW_SHOW );
            itsEndTextCP->Show( SW_SHOW );
            itsEndTitleCP->Show( SW_SHOW );
            itsDurationTextCP->Show( SW_SHOW );
            itsWorkTimeTextCP->Show( SW_SHOW );
            itsDurationTitleCP->Show( SW_SHOW );
            itsWorkTimeTitleCP->Show( SW_SHOW );
            break;

        case kMilestoneOnly:

            itsMonthPickerCP->Show( SW_SHOW );
            itsNowButtonCP->Show( SW_SHOW );
            itsStartTextCP->Show( SW_SHOW );
            itsStartTitleCP->Show( SW_SHOW );

            itsEndTextCP->Show( SW_HIDE );
            itsEndTitleCP->Show( SW_HIDE );
            itsDurationTextCP->Show( SW_HIDE );
            itsWorkTimeTextCP->Show( SW_HIDE );
            itsDurationTitleCP->Show( SW_HIDE );
            itsWorkTimeTitleCP->Show( SW_HIDE );
            break;

        case kDurationOnly:

            itsMonthPickerCP->Show( SW_HIDE );
            itsNowButtonCP->Show( SW_HIDE );
            itsStartTextCP->Show( SW_HIDE );
            itsStartTitleCP->Show( SW_HIDE );
            itsEndTextCP->Show( SW_HIDE );
            itsEndTitleCP->Show( SW_HIDE );
            itsWorkTimeTextCP->Show( SW_HIDE );
            itsWorkTimeTitleCP->Show( SW_HIDE );

            itsDurationTextCP->Show( SW_SHOW );
            itsDurationTitleCP->Show( SW_SHOW );
            break;
    }
}

// -------------------------------------------------
// Ok
//
//  The ticker has the real times
// -------------------------------------------------
void TTimePicker::Ok( RTMessage Msg )
{
    TTime     start, end;
    TDuration duration;

    itsTickerCP->GetTimes( start, end, duration );


    if ((itsFreqP != NULL) && itsFreqP->IsRealFreq()) {

       if (!start.HasTime()) {

           TMessageBox::Ok( this,
               "Notes with frequencies cannot be in days or weeks.",
               "Frequency Restriction" );
           return;
       }
       else if (!start.IsSameDay( end )) {

           TMessageBox::Ok( this,
               "Notes with frequencies must start and end on the same day.",
               "Frequency Restriction" );
           return;
       }
    }

    *itsStartTimeP = start;
    *itsEndTimeP   = end;
    *itsDurationP  = duration;
    TDialog::Ok( Msg );
}

// -------------------------------------------------
// HandleNoTime
//
//  The no time button
// -------------------------------------------------
void TTimePicker::HandleNoTime( RTMessage Msg )
{
    if ((itsFreqP != NULL) && itsFreqP->IsRealFreq()) {

       TMessageBox::Ok( this,
           "Notes with frequencies must have start times.",
           "Frequency Restriction" );
       return;
    }

    itsStartTimeP->Clear();
    itsEndTimeP->Clear();
    itsDurationP->Clear();
    TDialog::Ok( Msg );
}

void TTimePicker::Cancel( RTMessage Msg )
{
    TDialog::Cancel( Msg );
}
