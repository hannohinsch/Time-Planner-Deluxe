//*****************************************************************************
// manage the drag window ( status window exists only during drag op)
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "dragwin.h"
#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __profile__
#include "profile.h"
#endif
#ifndef __tipwin__
#include "tipwin.h"
#endif
#ifndef __frame__
#include "frame.h"
#endif
#ifndef __worktime__
#include "worktime.h"
#endif

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TDragWin::TDragWin( PTWindowsObject AParent, int resourceId ):
    TDialog( AParent, resourceId )
{

    //get rid of TipWindow
    itsTipWinState= TPimFrame::pTipWin->MakeActive( FALSE );

    itsGotMessage = (resourceId == IDD_DRAGWIN_WITH_MESSAGE);
    if (itsGotMessage) {

        itsMessageCP = new TStatic( this, IDD_DRAGWIN_MESSAGE, 256 );
    }

    itsStartCP    = new TStatic( this, IDD_DRAGWIN_START,    256 );
    itsEndCP      = new TStatic( this, IDD_DRAGWIN_END,      256 );
    itsDurationCP = new TStatic( this, IDD_DRAGWIN_DURATION, 256 );
}


// -------------------------------------------------
// Destructor
// -------------------------------------------------
TDragWin::~TDragWin( void )
{
TPimFrame::pTipWin->MakeActive( itsTipWinState );
}

//*****************************************************************************

//*****************************************************************************
void TDragWin::SetupWindow( void )
{
TDialog::SetupWindow();
EnableWindow( HWindow, FALSE );
}

//*****************************************************************************
// bounce focus back to parent
//*****************************************************************************
void TDragWin::WMSetFocus( RTMessage Msg )
{
#if 0
EnableWindow( HWindow, FALSE );     //we never want msgs
SetFocus( Msg.WParam );
#endif
DefWndProc( Msg );
}

// -------------------------------------------------
// Set
// -------------------------------------------------
void TDragWin::Set( char *message, RTTimeComplex times )
{
    char buf1[100];
    char buf2[100];
    char buf3[100];
    TTime start( times.GetStart());
    TTime end( times.GetEnd() );

    // don't redraw the same information
    if ((start == itsStart) && (end == itsEnd)) {

        return;
    }

    // milestone with time
    if ((!end.HasDate()) && start.HasTime()) {

        // put the date at the top
        TIntlFmt::FormatDate( start, kLongDate, buf1 );
        itsStartCP->SetText( buf1 );

        // put the time in the middle
        TIntlFmt::FormatTime( start, FALSE, buf1 );
        itsEndCP->SetText( buf1 );

        // put the duration at the bottom
        itsDurationCP->SetText( "" );
    }

    // milestone without time
    else if (!end.HasDate()) {

        // put the date at the top
        TIntlFmt::FormatDate( start, kLongDate, buf1 );
        itsStartCP->SetText( buf1 );

        // clear out the other fields
        itsEndCP->SetText( "" );
        itsDurationCP->SetText( "" );
    }


    // task on one day with both times
    else if (start.IsSameDay( end ) && start.HasTime() && end.HasTime()) {

        // put the date at the top
        TIntlFmt::FormatDate( start, kLongDate, buf1 );
        itsStartCP->SetText( buf1 );

        // put the times in the middle
        TIntlFmt::FormatTime( start, FALSE, buf1 );
        TIntlFmt::FormatTime( end,   FALSE, buf2 );
        wsprintf( buf3, "%s to %s", buf1, buf2 );
        itsEndCP->SetText( buf3 );

        // put the duration at the bottom
        if (times.IsWorkTime()) {

            times.GetWorkTime().Format( SCALE_HOUR, TRUE, buf1 );
            wsprintf( buf2, "work time: %s", buf1 );
        }
        else {

            times.GetDuration().Format( SCALE_HOUR, TRUE, buf1 );
            wsprintf( buf2, "elapsed time: %s", buf1 );
        }
        itsDurationCP->SetText( buf2 );
    }

    // task on one day with 0 or 1 time
    else if (start.IsSameDay( end )) {

        // put the date at the top
        TIntlFmt::FormatDate( start, kLongDate, buf1 );
        itsStartCP->SetText( buf1 );

        // put the date at the top
        TIntlFmt::FormatDate( end, kLongDate, buf1 );
        itsEndCP->SetText( buf1 );

        // put the duration at the bottom
        if (times.IsWorkTime()) {

            times.GetWorkTime().Format( SCALE_DAY, TRUE, buf1 );
            wsprintf( buf2, "work time: %s", buf1 );
        }
        else {

            times.GetDuration().Format( SCALE_DAY, TRUE, buf1 );
            wsprintf( buf2, "elapsed time: %s", buf1 );
        }
        itsDurationCP->SetText( buf2 );
    }

    // task on more than one day with both times
    else if (start.HasTime() && end.HasTime()) {

        // put the start date and time at the top
        TIntlFmt::FormatDate( start, kLongDate,  buf1 );
        TIntlFmt::FormatTime( start, FALSE, buf2 );
        wsprintf( buf3, "%s %s", buf1, buf2 );
        itsStartCP->SetText( buf3 );

        // put the end date and time in the middle
        TIntlFmt::FormatDate( end, kLongDate,  buf1 );
        TIntlFmt::FormatTime( end, FALSE, buf2 );
        wsprintf( buf3, "%s %s", buf1, buf2 );
        itsEndCP->SetText( buf3 );

        // put the duration at the bottom
        if (times.IsWorkTime()) {

            times.GetWorkTime().Format( SCALE_HOUR, TRUE, buf1 );
            wsprintf( buf2, "work time: %s", buf1 );
        }
        else {

            times.GetDuration().Format( SCALE_HOUR, TRUE, buf1 );
            wsprintf( buf2, "elapsed time: %s", buf1 );
        }
        itsDurationCP->SetText( buf2 );
    }

    // task on more than one day with 0 or 1 times
    else {

        // put the start date at the top
        TIntlFmt::FormatDate( start, kLongDate,  buf1 );
        itsStartCP->SetText( buf1 );

        // put the end date in the middle
        TIntlFmt::FormatDate( end, kLongDate,  buf1 );
        itsEndCP->SetText( buf1 );

        // put the duration at the bottom
        if (times.IsWorkTime()) {

            times.GetWorkTime().Format( SCALE_DAY, TRUE, buf1 );
            wsprintf( buf2, "work time: %s", buf1 );
        }
        else {

            times.GetDuration().Format( SCALE_DAY, TRUE, buf1 );
            wsprintf( buf2, "elapsed time: %s", buf1 );
        }
        itsDurationCP->SetText( buf2 );
    }

    itsStart    = start;
    itsEnd      = end;

    if (itsGotMessage) {

        itsMessageCP->SetText( message );
    }

    UpdateWindow( HWindow );
}

