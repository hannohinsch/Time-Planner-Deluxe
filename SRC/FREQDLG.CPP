//*****************************************************************************
// frequency dialog
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "freqdlg.h"

#ifndef __freqcal__
#include "freqcal.h"
#endif
#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __cursor__
#include "cursor.h"
#endif
#ifndef __gridutil__
#include "gridutil.h"
#endif

// --------------------------------------------------------------------------
// Constructor
// --------------------------------------------------------------------------
TFrequencyDialog::TFrequencyDialog( PTWindowsObject AParent, int AResID,
    PTFreq aFreqP, PTTime noteStartP ):
	TOurDialog( AParent, AResID )
{
	int		i;
	char	monthStr[256];
	char	yearStr[256];

    TTime   date;
	TTime	nextDate;

    itsFreqP      = aFreqP;
    itsFreq       = *aFreqP;
    itsNoteStartP = noteStartP;
    itsNoteStart  = *noteStartP;

    itsHelpTopic= _HelpFrequency;

    // set up dates
    TTime startDate;
    TTime endDate    = itsFreq.itsEndDate;
    itsFreq.First( startDate );
    itsFreq.Next( itsSecondDate );

    date = startDate;
	for (i = 0; i < kNumMonths; i++) {

		// create a calendar control and title for this month
		// the startDate for the first month is the start date
		// as specified by the startDate parameter, but subsequent
		// months always start on the first
        itsMonthCtls[i]        = new TFreqCal( this, IDD_MONTH_1 + i, date );
        itsMonthNameCtls[i]    = new TStatic( this, IDD_MONTH_1_NAME + i, 256 );

        // tell mousgrid not to turn the timer on for the mouse, so
        // that we avoid flickering that would happen otherwise, because
        // the popup-menu eats the WMLButtonUp message that the month
        // controls are expecting
        itsMonthCtls[i]->UseTimerWithLButton( FALSE );

		// put the month on the screen
        itsMonthNameCtls[i]->EnableTransfer();
        TIntlFmt::FormatMonthName( date, kFullName, monthStr );
        TIntlFmt::FormatYear( date, yearStr );
		wsprintf( itsTransferBuffer.monthName[i], "%s %s",
			monthStr, yearStr  );

		// get the next month
        date.SetDay( 1 );
        date.PlusEquals( SCALE_MONTH, 1 );
	}

    // initialize visible dates
    itsFirstVisibleDate = itsMonthCtls[0]->GetSelTime();
    itsLastVisibleDate  = itsMonthCtls[kNumMonths - 1]->GetSelTime();
    itsFirstVisibleDate.SetDay( 1 );
    itsLastVisibleDate.SetLastDayOfMonth();

	// initialize the frequency text
    itsTextCtl= new TStatic( this, IDD_FREQUENCY_VALUE, 256 );
    itsTextCtl->EnableTransfer();
	strcpy( itsTransferBuffer.frequency, "" );

	// initialize the prompt text
    itsPromptCtl= new TStatic( this, IDD_FREQ_PROMPT, 256 );
    itsPromptCtl->EnableTransfer();

	// initialize the start text
    itsStartCtl= new TStatic( this, IDD_FREQ_START, 256 );
    itsStartCtl->EnableTransfer();

	// initialize the end text
    itsEndCtl= new TStatic( this, IDD_FREQ_END, 256 );
    itsEndCtl->EnableTransfer();


    // duration scrollbar
    itsScrollCtl = new TScrollBar( this, IDD_FREQ_SCROLL );
    itsScrollCtl->DisableTransfer();

	// set up transfers
	SetTransferBuffer( &itsTransferBuffer );

    ShowDates();

}

// --------------------------------------------------------------------------
// Destructor
// --------------------------------------------------------------------------
TFrequencyDialog::~TFrequencyDialog( void )
{
}

// --------------------------------------------------------------------------
// SetupWindow
// --------------------------------------------------------------------------
void TFrequencyDialog::SetupWindow()
{
    // inherited
    TOurDialog::SetupWindow();

    itsScrollCtl->SetRange( 1, 3 );
    itsScrollCtl->SetPosition( 2 );

}

// --------------------------------------------------------------------------
// HandleCalendar1 to HandleCalendar6
//
//	Respond to a message from one of the calendar
//	controls. They all just call HandleCalendar.
// --------------------------------------------------------------------------
void TFrequencyDialog::HandleCalendar1(  RTMessage  ) { HandleCalendar( 0 );}
void TFrequencyDialog::HandleCalendar2(  RTMessage  ) { HandleCalendar( 1 );}
void TFrequencyDialog::HandleCalendar3(  RTMessage  ) { HandleCalendar( 2 );}
void TFrequencyDialog::HandleCalendar4(  RTMessage  ) { HandleCalendar( 3 );}
void TFrequencyDialog::HandleCalendar5(  RTMessage  ) { HandleCalendar( 4 );}
void TFrequencyDialog::HandleCalendar6(  RTMessage  ) { HandleCalendar( 5 );}

// --------------------------------------------------------------------------
// HandleCalendar
//
// put up pop-up menu to decide what to do
// --------------------------------------------------------------------------
void TFrequencyDialog::HandleCalendar( int monthIndex )
{
    // change to the arrow cursor
    TCursor::Set( kArrowCursor );

    itsSelectedDate = itsMonthCtls[monthIndex]->GetSelTime();

    // put up popup menu
    POINT p;
    HMENU hMenu = LoadMenu( GetApplicationObject()->hInstance,
        MAKEINTRESOURCE( IDM_FREQUENCY ));

    // remove either "set last date" or "remove last date"
    // from the menu (only removes "set last date"
    // if we're on the last date)
    if (itsFreq.itsEndDate.HasDate() &&
       (itsSelectedDate == itsFreq.itsEndDate)) {

        DeleteMenu( hMenu, IDM_LASTDATE, MF_BYCOMMAND );
    }
    else {

        DeleteMenu( hMenu, IDM_REMOVE_LAST_DATE, MF_BYCOMMAND );
    }

    // if the selected date is before the start date, then the
    // only allowable choice is to set a new start date
    if (itsSelectedDate < itsFreq.itsStartDate ) {

        EnableMenuItem( hMenu, IDM_SECONDDATE,   MF_BYCOMMAND | MF_GRAYED );
        EnableMenuItem( hMenu, IDM_LASTDATE,     MF_BYCOMMAND | MF_GRAYED );
    }

    // "once": don't allow last date
    else if (itsFreq.itsFrequencyType == kOnce) {

        EnableMenuItem( hMenu, IDM_LASTDATE,     MF_BYCOMMAND | MF_GRAYED );
    }

    // if the selected date is before the second date, then the
    // only allowable choices are first and second date
    else if (itsSelectedDate < itsSecondDate ) {

        EnableMenuItem( hMenu, IDM_LASTDATE,     MF_BYCOMMAND | MF_GRAYED );
    }

    switch( itsMonthCtls[monthIndex]->GetCellType( itsSelectedDate )) {

        case TFreq::kStartCell:
        case TFreq::kSecondCell:
        case TFreq::kEndCell:

            // gray-out things you can't do
            EnableMenuItem( hMenu, IDM_ADDDATE,          MF_BYCOMMAND | MF_GRAYED );
            EnableMenuItem( hMenu, IDM_EXCLUDEDATE,      MF_BYCOMMAND | MF_GRAYED );

            // get rid of their synonyms entirely
            DeleteMenu( hMenu, IDM_DELETE_ADDED,     MF_BYCOMMAND );
            DeleteMenu( hMenu, IDM_RESTORE_EXCLUDED, MF_BYCOMMAND );
            break;

        case TFreq::kFrequencyCell:

            // "Exclude Date" remains
            EnableMenuItem( hMenu, IDM_ADDDATE,          MF_BYCOMMAND | MF_GRAYED  );

            DeleteMenu( hMenu, IDM_DELETE_ADDED,     MF_BYCOMMAND );
            DeleteMenu( hMenu, IDM_RESTORE_EXCLUDED, MF_BYCOMMAND );
            break;

        case TFreq::kAdditionCell:

            // "Delete Addition" remains
            EnableMenuItem( hMenu, IDM_EXCLUDEDATE,      MF_BYCOMMAND | MF_GRAYED  );

            DeleteMenu( hMenu, IDM_ADDDATE,          MF_BYCOMMAND );
            DeleteMenu( hMenu, IDM_RESTORE_EXCLUDED, MF_BYCOMMAND );
            break;

        case TFreq::kExcludeCell:

            // "Restore Excluded" remains
            EnableMenuItem( hMenu, IDM_ADDDATE,          MF_BYCOMMAND | MF_GRAYED  );

            DeleteMenu( hMenu, IDM_DELETE_ADDED,     MF_BYCOMMAND );
            DeleteMenu( hMenu, IDM_EXCLUDEDATE,      MF_BYCOMMAND );
            break;

        case TFreq::kNormalCell:

            // "Add Date" is removed if:
            //   the date is before the second
            //   the date is after the end
            //   the frequency type is "once"
            if ((itsSelectedDate < itsSecondDate ) ||
                (itsFreq.itsEndDate.HasDate() &&
                    (itsFreq.itsEndDate < itsSelectedDate)) ||
                (itsFreq.itsFrequencyType == kOnce)) {

                EnableMenuItem( hMenu, IDM_ADDDATE,     MF_BYCOMMAND | MF_GRAYED );
            }

            EnableMenuItem( hMenu, IDM_EXCLUDEDATE,      MF_BYCOMMAND | MF_GRAYED );

            DeleteMenu( hMenu, IDM_DELETE_ADDED,     MF_BYCOMMAND );
            DeleteMenu( hMenu, IDM_RESTORE_EXCLUDED, MF_BYCOMMAND );
            break;
    }

    itsMonthCtls[ monthIndex ]->GetClickPoint( p );
    HMENU hSubMenu = GetSubMenu( hMenu, 0 );
    TGridUtil::OurTrackPopupMenu( hSubMenu, TPM_LEFTALIGN, p.x, p.y, 0, HWindow, NULL );
    DestroyMenu( hMenu );
}


// --------------------------------------------------------------------------
// SetstartDate
// --------------------------------------------------------------------------
void TFrequencyDialog::SetStartDate( RTMessage )
{
    itsNoteStart.CopyDate( itsSelectedDate );

    itsFreq.Clear();
    itsFreq.itsStartDate = itsSelectedDate;
    itsFreq.First( itsSecondDate );
    itsFreq.Next( itsSecondDate );
    ShowDates();
}

// --------------------------------------------------------------------------
// SetSecondDate
// --------------------------------------------------------------------------
void TFrequencyDialog::SetSecondDate( RTMessage )
{
	TTime startDate = itsFreq.itsStartDate;

    itsFreq.Clear();
    itsFreq.itsStartDate = startDate;
    itsFreq.InferFrequency( this, itsFreq.itsStartDate, itsSelectedDate );

    // clear the second date if the freq is once
    if (itsFreq.itsFrequencyType == kOnce) {

        itsSecondDate.Clear();
    }
    else {

        itsSecondDate = itsSelectedDate;
    }

    ShowDates();
}

// --------------------------------------------------------------------------
// SetEndDate
// --------------------------------------------------------------------------
void TFrequencyDialog::SetEndDate( RTMessage )
{
    itsFreq.itsEndDate = itsSelectedDate;

    // set the time to the last minute of the day
    itsFreq.itsEndDate.SetToMidnightTomorrow();
    itsFreq.itsEndDate.PlusEquals( k1Min, -1 );

    ShowDates();
}

// --------------------------------------------------------------------------
// RemoveEndDate
// --------------------------------------------------------------------------
void TFrequencyDialog::RemoveEndDate( RTMessage )
{
    itsFreq.itsEndDate.Clear();
    ShowDates();
}

// --------------------------------------------------------------------------
// AddDate
// --------------------------------------------------------------------------
void TFrequencyDialog::AddDate( RTMessage )
{
    itsFreq.AddDate( itsSelectedDate );
    MarkDate( itsSelectedDate, TFreq::kAdditionCell );
}

// --------------------------------------------------------------------------
// ExcludeDate
// --------------------------------------------------------------------------
void TFrequencyDialog::ExcludeDate( RTMessage )
{
    itsFreq.ExcludeDate( itsSelectedDate );
    MarkDate( itsSelectedDate, TFreq::kExcludeCell );
}

// --------------------------------------------------------------------------
// RemoveAddedDate
// --------------------------------------------------------------------------
void TFrequencyDialog::RemoveAddedDate( RTMessage )
{
    itsFreq.ExcludeDate( itsSelectedDate );
    MarkDate( itsSelectedDate, TFreq::kNormalCell );
}

// --------------------------------------------------------------------------
// RestoreDate
// --------------------------------------------------------------------------
void TFrequencyDialog::RestoreDate( RTMessage )
{
    itsFreq.AddDate( itsSelectedDate );
    MarkDate( itsSelectedDate, TFreq::kFrequencyCell );
}

// --------------------------------------------------------------------------
// ShowDates
//
// Redraw the calendars to show all instances of the frequency
// --------------------------------------------------------------------------
void TFrequencyDialog::ShowDates( void )
{
    ShowPrompts();

    // clear out old dates
    for (int month = 0; month < kNumMonths; month++) {

        itsMonthCtls[month]->ClearCells();
    }

    // loop through frequencies marking calendars
    TTime date;
    TTime end(  itsLastVisibleDate );
    end.ClearTime();

    BOOL             valid = TRUE;
    TFreq::TCellType type  = TFreq::kFrequencyCell;

    itsFreq.First( date );
    while (valid) {

        MarkDate( date, type );
        valid = itsFreq.NextDisplay( date, type );
        date.ClearTime();
        valid &= (date <= end);
    }

    // mark the start, second and end cells
    MarkDate( itsSecondDate,          TFreq::kSecondCell );
    if (itsFreq.itsEndDate.HasDate()) {

        MarkDate( itsFreq.itsEndDate, TFreq::kEndCell );
    }
    MarkDate( itsFreq.itsStartDate, TFreq::kStartCell );
}

// --------------------------------------------------------------------------
// ShowPrompts
// --------------------------------------------------------------------------
void TFrequencyDialog::ShowPrompts( void )
{
	// show the user the inferred frequency
    itsFreq.GetFreqText( itsTransferBuffer.frequency );

    // show the start and end times
    TIntlFmt::FormatDate( itsFreq.itsStartDate, kShortDate, itsTransferBuffer.start );
    TIntlFmt::FormatDate( itsFreq.itsEndDate, kShortDate, itsTransferBuffer.end );

    // show the prompt
    if (itsFreq.itsFrequencyType == kOnce) {

        strcpy( itsTransferBuffer.prompt,
            "Click on a day and choose 'Second Date' to assign a frequency." );
    }
    else {

        strcpy( itsTransferBuffer.prompt,
            "Click on a day to modify the frequency." );
    }

    TransferData( TF_SETDATA );

}

// --------------------------------------------------------------------------
// MarkDate
//
// set the cell type of the cell with the specified date
// --------------------------------------------------------------------------
void TFrequencyDialog::MarkDate( RTTime date, TFreq::TCellType cellType )
{
    // if the date is within kNumMonths of the first day, then
    // mark the appropriate month
    int month = itsFirstVisibleDate.MonthDelta( date );
    if ((0 <= month) && (month < kNumMonths)) {

        itsMonthCtls[month]->SetCellType( date, cellType );
    }
}

// --------------------------------------------------------------------------
// GetMark
//
// get the cell type of the cell with the specified date
// --------------------------------------------------------------------------
TFreq::TCellType TFrequencyDialog::GetMark( RTTime date )
{
    // if the date is within kNumMonths of the first day, then
    // mark the appropriate month
    int month = itsFirstVisibleDate.MonthDelta( date );
    if ((0 <= month) && (month < kNumMonths)) {

        return( itsMonthCtls[month]->GetCellType( date ));
    }

    return TFreq::kNormalCell;
}

// --------------------------------------------------------------------------
// ok
// --------------------------------------------------------------------------
void TFrequencyDialog::Ok( RTMessage Msg)
{
*itsFreqP      = itsFreq;
*itsNoteStartP = itsNoteStart;

TDialog::Ok( Msg );
}

// --------------------------------------------------------------------------
// cancel
// --------------------------------------------------------------------------
void TFrequencyDialog::Cancel( RTMessage Msg)
{
TDialog::Cancel( Msg );
}


// --------------------------------------------------------------------------
// HandleScroll
// --------------------------------------------------------------------------
void TFrequencyDialog::HandleScroll( RTMessage )
{
	char	monthStr[256];
	char	yearStr[256];

    int delta = 3 * (itsScrollCtl->GetPosition() - 2);

    for (int i = 0; i < kNumMonths; i++) {

		// move delta months
        TTime date = itsMonthCtls[i]->GetSelTime();
        date.PlusEquals( SCALE_MONTH, delta );
        itsMonthCtls[i]->SetSelTime( date );

        TIntlFmt::FormatMonthName( date, kFullName, monthStr );
        TIntlFmt::FormatYear( date, yearStr );
		wsprintf( itsTransferBuffer.monthName[i], "%s %s",
			monthStr, yearStr  );
    }

    // initialize visible dates
    itsFirstVisibleDate = itsMonthCtls[0]->GetSelTime();
    itsLastVisibleDate  = itsMonthCtls[kNumMonths - 1]->GetSelTime();
    itsFirstVisibleDate.SetDay( 1 );
    itsLastVisibleDate.SetLastDayOfMonth();

    ShowDates();

	TransferData( TF_SETDATA );
    itsScrollCtl->SetPosition( 2 );
}

