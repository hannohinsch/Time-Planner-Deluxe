//*****************************************************************************
// worktime dialog
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "workdlg.h"

#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __cursor__
#include "cursor.h"
#endif
#ifndef __fmtedit__
#include "fmtedit.h"
#endif
#ifndef __memnote__
#include "memnote.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __profile__
#include "profile.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TWorkTimeD::TWorkTimeD( PTWindowsObject AParent, int AResID ):
	TOurDialog( AParent, AResID )
{

    itsHelpTopic= _HelpWorkSched;

    // create the starts at controls
    TTime start;
    its9AMButtonCP  = new TBButton( this, IDD_WORK_DAY_STARTS_AT_9 );
    itsStartTimeCP  = new TTimeOnlyEdit( this, IDD_WORK_DAY_STARTS, start, kMustHaveTime );

    // create the HoursPerDay controls
    itsHoursButtonCP = new TBButton(  this, IDD_CONVERSION_DEFAULT_1 );
    itsHoursValueCP  = new TEdit(     this, IDD_CONVERSION_EDIT_1, 256 );

    // create the 7 days of the week
    itsDaysGroupCP   = new TGroupBox( this, IDD_CONVERSION_GROUP_2 );
    for (int i = 0; i < 7; i++) {

        itsDayBoxes[i] = new TCheckBox( this, IDD_SUNDAY + i, itsDaysGroupCP );
        itsTransferBuffer.dayValues[i] = TProfile::IsWorkDay[i];
    }

    SetTransferBuffer( &itsTransferBuffer );

    TProfile::DoubleToString( TProfile::HoursPerDay,
        itsTransferBuffer.hoursPerDay );

}


// -------------------------------------------------
// Destructor
// -------------------------------------------------
TWorkTimeD::~TWorkTimeD( void )
{
}

// -------------------------------------------------
// SetupWindow
// -------------------------------------------------
void TWorkTimeD::SetupWindow( void )
{
    TOurDialog::SetupWindow();

    TTime start;
    start.SetNow();
    start.SetHour(   TProfile::WorkDayStartMinute / 60 );
    start.SetMinute( TProfile::WorkDayStartMinute % 60 );
    itsStartTimeCP->Set( start );

}

// -------------------------------------------------
// HandleDefault1
//
//  Hours/Day default
// -------------------------------------------------
void TWorkTimeD::HandleDefault1( RTMessage  )
{
    TransferData( TF_GETDATA );
    strcpy( itsTransferBuffer.hoursPerDay, "8" );
    TransferData( TF_SETDATA );
}

// -------------------------------------------------
// Handle9AM
//
//  Start
// -------------------------------------------------
void TWorkTimeD::Handle9AM( RTMessage  )
{
    TTime time;
    time.SetNow();
    time.SetHour( 9 );
    time.SetMinute( 0 );
    itsStartTimeCP->Set( time );
}


// -------------------------------------------------
// OK
//
//  figure out what was pressed
// -------------------------------------------------
void TWorkTimeD::Ok( RTMessage msg )
{
    double hoursPerDay;

    TransferData( TF_GETDATA );

    // set work time start
    itsStartTimeCP->HasTime();
    TTime start( itsStartTimeCP->Get() );
    if (start.GetHour() > 23) {

        TMessageBox::Ok( this,
            "Can't be later than 11 PM", "Work Day Start Time" );
        return;
    }

    // hours per day
    TProfile::StringToDouble( itsTransferBuffer.hoursPerDay, hoursPerDay );
    if (hoursPerDay < 1) {

        TMessageBox::Ok( this,
            "Must be at least 1", "Hours Per Day" );
        return;
    }
    else if (hoursPerDay > 23 - start.GetHour()) {

        char buf[100];
        wsprintf( buf, "Can't be more than %d hours",
            23 - start.GetHour());
        TMessageBox::Ok( this, buf, "Hours Per Day" );
        return;
    }

    TCursor::Busy( TRUE );
    //save current note state
    TSysState::pAllNotes->UpdateUnitLength();
    TSysState::pMemNotes->UpdateUnitLength();

    // set up work day minutes
    TProfile::WorkDayStartMinute =
        start.GetHour() * 60 + start.GetMinute();
    TProfile::WorkDayEndMinute =
        TProfile::WorkDayStartMinute + hoursPerDay * 60;

    // set up the work days
    TProfile::DaysPerWeek = 0;
    for (int i = 0; i < 7; i++) {

        if (itsTransferBuffer.dayValues[i] == BF_CHECKED) {

            TProfile::IsWorkDay[i] = TRUE;
            TProfile::DaysPerWeek++;
        }
        else {

            TProfile::IsWorkDay[i] = FALSE;
        }
    }

    TProfile::HoursPerDay = hoursPerDay;
    TProfile::CalculateHours();

    //update notes
    TSysState::pAllNotes->ModifyForChangedWorkTime();
    TSysState::pMemNotes->ModifyForChangedWorkTime();

    CHKBUG( ERR_GEN, TSysState::pAllNotes->HealthCheck() );
    CHKBUG( ERR_GEN, TSysState::pMemNotes->HealthCheck() );

    Notify( kNoteSetChg, NOOBJECT );
    TCursor::Busy( FALSE );

    TDialog::Ok( msg );
}

