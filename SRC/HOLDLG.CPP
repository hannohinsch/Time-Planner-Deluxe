//*****************************************************************************
// holiday dialog
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "holdlg.h"

#ifndef __cursor__
#include "cursor.h"
#endif
#ifndef __holcal__
#include "holcal.h"
#endif
#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __memnote__
#include "memnote.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __xarray__
#include "xarray.h"
#endif
#ifndef __utility__
#include "utility.h"
#endif

// ########################################################################
//
//                            TAddHoliday
//
// ########################################################################
class _CLASSTYPE TAddHoliday: public TOurDialog {

public:

    TAddHoliday( PTWindowsObject AParent, int AResID, PTHoliday holidayP );
    virtual ~TAddHoliday( void );

    void Ok( RTMessage ) = [ ID_FIRST + IDOK ];

private:

    PTHoliday itsHolidayP;
    PTEdit    itsNameCtl;
    char      itsName[256];

};

// --------------------------------------------------------------------------
// Constructor
// --------------------------------------------------------------------------
THolidayDialog::THolidayDialog( PTWindowsObject AParent, int AResID, RTTime startDate ):
	TOurDialog( AParent, AResID )
{
	int		i;
	char	monthStr[256];
	char	yearStr[256];

    itsHelpTopic= _HelpHolidays;

    TTime   date;

    date = startDate;
	for (i = 0; i < kNumMonths; i++) {

		// create a calendar control and title for this month
		// the startDate for the first month is the start date
		// as specified by the startDate parameter, but subsequent
		// months always start on the first
        itsMonthCtls[i]        = new THolCal( this, IDD_HOLIDAY_MONTH_1 + i, date );
        itsMonthNameCtls[i]    = new TStatic( this, IDD_HOLIDAY_MONTH_1_NAME + i, 256 );


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

    // copy the holidays from the global list
    itsHolidaysP = new TXArray( 10, 10 );
    int n = THolidaySet::GetNumHolidays();
    for (i = 0; i < n; i++) {

        TTime date;
        THoliday holiday;
        THolidaySet::GetHoliday( i, holiday );
        PTHoliday holidayP = new THoliday( holiday );
        itsHolidaysP->add( *holidayP );
        holiday.GetDate( date );
        MarkDate( date, THolCal::kHolidayCell );
    }

	// initialize visible dates
    itsFirstVisibleDate = itsMonthCtls[0]->GetSelTime();
    itsFirstVisibleDate.SetDay( 1 );
    itsFirstVisibleDate.ClearTime();
    itsLastVisibleDate = itsFirstVisibleDate.Plus( SCALE_MONTH, 5 );
    itsLastVisibleDate.SetLastDayOfMonth();

    itsOldCountry = THolidaySet::Country();

	// initialize the prompt text
    itsPromptCtl = new TStatic( this, IDD_HOLIDAY_PROMPT, 256 );
    itsPromptCtl->EnableTransfer();
    strcpy( itsTransferBuffer.prompt,
        "Click on a day to create a holiday." );

	// initialize the list box
    itsCountryCtl = new TListBox( this, IDD_HOLIDAY_COUNTRY );
    itsCountryCtl->DisableTransfer();

    // duration scrollbar
    itsScrollCtl = new TScrollBar( this, IDD_HOLIDAY_SCROLL );
    itsScrollCtl->DisableTransfer();

	// set up transfers
	SetTransferBuffer( &itsTransferBuffer );

}

// --------------------------------------------------------------------------
// Destructor
// --------------------------------------------------------------------------
THolidayDialog::~THolidayDialog( void )
{
    itsHolidaysP->flush( TXArray::kDelete );
    delete itsHolidaysP;
}

// --------------------------------------------------------------------------
// SetupWindow
// --------------------------------------------------------------------------
void THolidayDialog::SetupWindow()
{
    // inherited
    TOurDialog::SetupWindow();

    // set up the list box
    itsCountryCtl->AddString( "none" );
    itsCountryCtl->AddString( "USA" );
    // itsCountryCtl->AddString( "Canada" );
    itsCountryCtl->SetSelIndex( itsOldCountry );

    // set up the scroll bar
    itsScrollCtl->SetRange( 1, 3 );
    itsScrollCtl->SetPosition( 2 );

    ShowDates();

}

// --------------------------------------------------------------------------
// HandleCalendar1 to HandleCalendar6
//
//	Respond to a message from one of the calendar
//	controls. They all just call HandleCalendar.
// --------------------------------------------------------------------------
void THolidayDialog::HandleCalendar1(  RTMessage msg ) { HandleCalendar( msg, 0 );}
void THolidayDialog::HandleCalendar2(  RTMessage msg ) { HandleCalendar( msg, 1 );}
void THolidayDialog::HandleCalendar3(  RTMessage msg ) { HandleCalendar( msg, 2 );}
void THolidayDialog::HandleCalendar4(  RTMessage msg ) { HandleCalendar( msg, 3 );}
void THolidayDialog::HandleCalendar5(  RTMessage msg ) { HandleCalendar( msg, 4 );}
void THolidayDialog::HandleCalendar6(  RTMessage msg ) { HandleCalendar( msg, 5 );}

// --------------------------------------------------------------------------
// HandleCalendar
//
// put up pop-up menu to decide what to do
// --------------------------------------------------------------------------
void THolidayDialog::HandleCalendar( RTMessage msg, int monthIndex )
{
    // mouse moved, update prompt
    if (msg.LP.Hi == MOUSE_MESSAGE) {

        THolCal::TCellType type;
        TTime    date;
        TSafStr  name;

        itsMonthCtls[monthIndex]->GetMouseDate( type, date );
        switch( type ) {

            case THolCal::kNotACell:

                strcpy( itsTransferBuffer.prompt,
                    "Click on a day to create a holiday." );
                break;

            case THolCal::kNormalCell:

                strcpy( itsTransferBuffer.prompt,
                    "Click to create a holiday." );
                break;

            case THolCal::kHolidayCell:

                PTHoliday holidayP = FindHoliday( date );
                if (holidayP == NULL) break;
                holidayP->GetName( name );
                wsprintf( itsTransferBuffer.prompt,
                    "Click to remove %s as a holiday.", name.Addr() );
                break;

            case THolCal::kFixedHolidayCell:

                THoliday holiday;
                THolidaySet::GetFixedHoliday( date, holiday );
                holiday.GetName( name );
                wsprintf( itsTransferBuffer.prompt,
                    "Switch the country to remove %s as a holiday.",
                    name.Addr() );
                break;
        }

        itsPromptCtl->SetText( itsTransferBuffer.prompt );

        return;
    }

    // otherwise, there was an actual click in the calendar
    itsSelectedDate = itsMonthCtls[monthIndex]->GetSelTime();
    itsSelectedDate.ClearTime();

    switch (GetMark( itsSelectedDate )) {

        case THolCal::kNormalCell:

            AddHoliday();
            break;

        case THolCal::kFixedHolidayCell:

            TMessageBox::Ok( this,
                "Switch the country to 'none' in order to delete this holiday.",
                "Can't Delete Holiday" );
            break;

        case THolCal::kHolidayCell:

            DeleteHoliday();
            break;
    }
}

// --------------------------------------------------------------------------
// AddHoliday
//
// add a holiday to the list of holidays
// --------------------------------------------------------------------------
void THolidayDialog::AddHoliday( void )
{
    TSafStr name;
    THoliday holiday( itsSelectedDate, name );

    if (IDOK == GetApplication()->ExecDialog( new TAddHoliday( this, IDD_ADD_HOLIDAY, &holiday ))) {

        PTHoliday holidayP = new THoliday( holiday );
        itsHolidaysP->add( *holidayP );
        MarkDate( itsSelectedDate, THolCal::kHolidayCell );
    }

}

// --------------------------------------------------------------------------
// DeleteHoliday
//
// remove a holiday from the list of holidays (first asking if it's ok to
// do it, of course)
// --------------------------------------------------------------------------
void THolidayDialog::DeleteHoliday( void  )
{
    PTHoliday holidayP = FindHoliday( itsSelectedDate );
    if (holidayP == NULL) return;

    TSafStr name;
    char prompt[256];
    holidayP->GetName( name );
    wsprintf( prompt, "Delete '%s'?", name.Addr() );

    if( IDOK == TMessageBox::OkAndCancel( this,
            prompt, "Confirm Deletion" )) {

        itsHolidaysP->detach( *holidayP, TXArray::kDelete );
        MarkDate( itsSelectedDate, THolCal::kNormalCell );
    }
}

// --------------------------------------------------------------------------
// FindHoliday
//
// look for the date in the temporary list of holidays
// --------------------------------------------------------------------------
PTHoliday THolidayDialog::FindHoliday( RTTime date )
{

    int n = itsHolidaysP->getItemsInContainer();
    for (int i = 0; i < n; i++) {

        RTHoliday h = (RTHoliday) (*itsHolidaysP)[i];

        TTime holidate;
        h.GetDate( holidate );
        if (holidate == date) {

            return &h;
        }
    }

    return NULL;
}

// --------------------------------------------------------------------------
// HandleCountry
//
// handle the country list box
// --------------------------------------------------------------------------
void THolidayDialog::HandleCountry( RTMessage  )
{
    TCountry country = (TCountry) itsCountryCtl->GetSelIndex();

    if ((kNone <= country) && (country <= kCanada)) {

        THolidaySet::SetCountry( country );

        ShowDates();
    }

}


// --------------------------------------------------------------------------
// ShowDates
//
// Redraw the calendars
// --------------------------------------------------------------------------
void THolidayDialog::ShowDates( void )
{
    // clear out old dates
    for (int month = 0; month < kNumMonths; month++) {

        itsMonthCtls[month]->ClearCells();
    }

    // loop through holidays marking calendars
    int n = itsHolidaysP->getItemsInContainer();
    for (int i = 0; i < n; i++) {

        RTHoliday holiday = (RTHoliday) (*itsHolidaysP)[i];

        TTime date;
        holiday.GetDate( date );
        MarkDate( date, THolCal::kHolidayCell );
    }

    // loop through the fixed holidays marking calendars
    TTime holidate;
    BOOL valid = THolidaySet::GetFirstFixedHoliday(
        itsFirstVisibleDate, itsLastVisibleDate, holidate );
    while (valid) {

        MarkDate( holidate, THolCal::kFixedHolidayCell );
        valid = THolidaySet::GetNextFixedHoliday( itsLastVisibleDate, holidate );
    }
}

// --------------------------------------------------------------------------
// MarkDate
//
// set the cell type of the cell with the specified date
// --------------------------------------------------------------------------
void THolidayDialog::MarkDate( RTTime date, THolCal::TCellType cellType )
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
THolCal::TCellType THolidayDialog::GetMark( RTTime date )
{
    // if the date is within kNumMonths of the first day, then
    // mark the appropriate month
    int month = itsFirstVisibleDate.MonthDelta( date );
    if ((0 <= month) && (month < kNumMonths)) {

        return( itsMonthCtls[month]->GetCellType( date ));
    }

    return THolCal::kNormalCell;
}

// --------------------------------------------------------------------------
// ok
// --------------------------------------------------------------------------
void THolidayDialog::Ok( RTMessage Msg)
{
    TCursor::Busy( TRUE );

    //save current note state
    TSysState::pAllNotes->UpdateUnitLength();
    TSysState::pMemNotes->UpdateUnitLength();

    // copy the holidays into the global holiday set
    THolidaySet::Flush();
    int n = itsHolidaysP->getItemsInContainer();
    for (int i = 0; i < n; i++) {

        RTHoliday holiday = (RTHoliday) (*itsHolidaysP)[i];

        PTHoliday holidayP = new THoliday( holiday );
        THolidaySet::Add( *holidayP );
    }

    //update notes
    TSysState::pAllNotes->ModifyForChangedWorkTime();
    TSysState::pMemNotes->ModifyForChangedWorkTime();

    CHKBUG( ERR_GEN, TSysState::pAllNotes->HealthCheck() );
    CHKBUG( ERR_GEN, TSysState::pMemNotes->HealthCheck() );

    TCursor::Busy( FALSE );

    TOurDialog::Ok( Msg );
}

// --------------------------------------------------------------------------
// cancel
// --------------------------------------------------------------------------
void THolidayDialog::Cancel( RTMessage Msg)
{
    THolidaySet::SetCountry( itsOldCountry );

    TOurDialog::Cancel( Msg );
}

// --------------------------------------------------------------------------
// HandleScroll
// --------------------------------------------------------------------------
void THolidayDialog::HandleScroll( RTMessage )
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

    itsFirstVisibleDate = itsMonthCtls[0]->GetSelTime();
    itsFirstVisibleDate.SetDay( 1 );
    itsFirstVisibleDate.ClearTime();
    itsLastVisibleDate = itsFirstVisibleDate.Plus( SCALE_MONTH, 5 );
    itsLastVisibleDate.SetLastDayOfMonth();

    ShowDates();

	TransferData( TF_SETDATA );
    itsScrollCtl->SetPosition( 2 );
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
//
//                      DeleteHoliday Dialog
//
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Constructor
// --------------------------------------------------------------------------
TAddHoliday::TAddHoliday( PTWindowsObject AParent, int AResID, PTHoliday holidayP ):
	TOurDialog( AParent, AResID )
{
    itsHolidayP = holidayP;

    itsNameCtl = new TEdit( this, IDD_NEW_HOLIDAY_NAME, 256 );
    itsNameCtl->EnableTransfer();

    itsName[0] = '\0';

    SetTransferBuffer( itsName );
}


// --------------------------------------------------------------------------
// Destructor
// --------------------------------------------------------------------------
TAddHoliday::~TAddHoliday( void )
{
}

// --------------------------------------------------------------------------
// Ok
// --------------------------------------------------------------------------
void TAddHoliday::Ok( RTMessage msg )
{
    TransferData( TF_GETDATA );

    if( !IsBlank( itsName ) ) {

        TTime   date;
        TSafStr name( itsName );
        itsHolidayP->GetDate( date );

        THoliday holiday( date, name );

        *itsHolidayP = holiday;

        TOurDialog::Ok( msg );
    }
    else {

    TMessageBox::Ok( this, "A holiday must have a name.", "Name Not Specified" );
    }

}

