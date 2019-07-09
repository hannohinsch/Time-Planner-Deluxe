//*****************************************************************************
// part of TFrame object -- menu and button handlers
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "frame.h"

#ifndef __noteview__
#include "noteview.h"
#endif
#ifndef __cattree__
#include "cattree.h"
#endif
#ifndef __catdlg__
#include "catdlg.h"
#endif
#ifndef __datepick__
#include "datepick.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __rptgrid__
#include "rptgrid.h"
#endif
#ifndef __daygrid__
#include "daygrid.h"
#endif
#ifndef __weekgrid__
#include "weekgrid.h"
#endif
#ifndef __mongrid__
#include "mongrid.h"
#endif
#ifndef __grafgrid__
#include "grafgrid.h"
#endif
#ifndef __tmlinegr__
#include "tmlinegr.h"
#endif
#ifndef __browgrid__
#include "browgrid.h"
#endif
#ifndef __filter__
#include "filter.h"
#endif
#ifndef __error__
#include "error.h"
#endif
#ifndef __memview__
#include "memview.h"
#endif
#ifndef __memviewd__
#include "memviewd.h"
#endif
#ifndef __memnote__
#include "memnote.h"
#endif
#ifndef __addressd__
#include "addressd.h"
#endif
#ifndef __reportd__
#include "reportd.h"
#endif
#ifndef __sortd__
#include "sortd.h"
#endif
#ifndef __dayviewd__
#include "dayviewd.h"
#endif
#ifndef __print__
#include "print.h"
#endif
#ifndef __workdlg__
#include "workdlg.h"
#endif
#ifndef __durfmtd__
#include "durfmtd.h"
#endif
#ifndef __preferd__
#include "preferd.h"
#endif
#ifndef __filterd__
#include "filterd.h"
#endif
#ifndef __odbutton__
#include "odbutton.h"
#endif
#ifndef __tipwin__
#include "tipwin.h"
#endif
#ifndef __holdlg__
#include "holdlg.h"
#endif
#ifndef __oem__
#include "oem.h"
#endif
#ifndef __utility__
#include "utility.h"
#endif
#ifndef  __ourinpd__
#include "ourinpd.h"
#endif
#ifndef  __grpcatd__
#include "grpcatd.h"
#endif
#ifndef  __catbox__
#include "catbox.h"
#endif
#ifndef __dialsetd__
#include "dialsetd.h"
#endif
#ifndef __dialdlg__
#include "dialdlg.h"
#endif

//*****************************************************************************

//*****************************************************************************
void TPimFrame::ExitCmd( RTMessage )
{
CloseWindow();  //system exit
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::AddressCmd(RTMessage)
{

    GetApplicationObject()->ExecDialog(
        new TAddressDialog( this, IDD_ADDRESS ));
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::SelectNoneCmd(RTMessage)
{
    pBox->SelectNone();

    // deselect everything in the filter
    TSysState::pFilter->SelectNone();

    NoteReset();
    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::SelectAllCmd(RTMessage)
{
    pBox->SelectAll();

    // tell the filter to select everything
    TSysState::pFilter->SelectAll();

    NoteReset();
    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::FilterCmd(RTMessage)
{
    PTFilterDlg pDlg= new TFilterDlg(this, IDD_FILTER, TSysState::pFilter);
    if( IDOK == GetApplication()->ExecDialog( pDlg ) )
        {
        NoteReset();
        }

    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************
// create new note; paste clipboard text into note; open noteview
//*****************************************************************************
void TPimFrame::EditPasteCmd( RTMessage )
{
PTNote newNoteP = NULL;

// look first for the NoteID format in the clipboard, so we
// can try to copy an entire note
WORD privateFormat = RegisterClipboardFormat( "NoteID" );
if( IsClipboardFormatAvailable( privateFormat ) )
    {
    OpenClipboard( HWindow );

    HANDLE hId = GetClipboardData( privateFormat );
    if( hId != NULL )
        {
        // get the NoteID from the clipboard
        int noteID = *(int*) GlobalLock( hId );
        GlobalUnlock( hId );

        // see if we can find the original note to copy in the
        // global note set
        for( int i = 0; i < TSysState::pAllNotes->getItemsInContainer(); i++)
            {
            RTNote oldNote = (*TSysState::pAllNotes)[i];
            if (oldNote.GetNoteID() == noteID)
                {
                newNoteP = TSysState::pAllNotes->CopyNote( oldNote );
                break;
                }
            }
        }

    CloseClipboard();
    }

// if we didn't find a NoteID, or didn't find the note with
// that ID (because it was deleted), then try pasting just the text
if (!newNoteP)
    {
    newNoteP = TSysState::pAllNotes->CreateNote( this );
    if( newNoteP != NULL )
        {
        // copy text from the clipboard if there is any
        if( IsClipboardFormatAvailable( CF_TEXT ) )
            {
            OpenClipboard( HWindow );

            HANDLE hText= GetClipboardData( CF_TEXT );
            if( hText != NULL )
                {
                char *p= GlobalLock( hText );
                newNoteP->GetText().Set( p );
                GlobalUnlock( hText );
                }
            CloseClipboard();
            }

        }
    }

// now, no matter what we managed to copy from the clipboard, show the note
if (newNoteP)
    GetApplication()->ExecDialog(new TNoteView(this, IDD_NOTE, newNoteP ));
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::HeadlineCmd(RTMessage)
{
TSysState::bShowFullText = !TSysState::bShowFullText;

UpdateButtons();

//change in row size screws up scroll thumb, so tell view to recalc
pView->ScrollChange();
InvalidateRect( pView->HWindow, NULL, TRUE );

// notify the window that the button was clicked
SendMessage( pView->HWindow, WM_HEADLINE_CHG, 0, (DWORD) 0 ); // tell the noteview about it

}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::ShowBarsCmd(RTMessage)
{
TSysState::bShowBars = !TSysState::bShowBars;
UpdateButtons();

// notify the window that the button was clicked
SendMessage( pView->HWindow, WM_HEADLINE_CHG, 0, (DWORD) 0 ); // tell the noteview about it

}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::ViewMaxCmd(RTMessage)
{
bMaxFrame = !bMaxFrame;

// update menu
if( bMaxFrame )
    {
	CheckMenuItem( GetMenu( HWindow), IDM_VIEWMAX, MF_BYCOMMAND | MF_CHECKED );
    }
else
    {
	CheckMenuItem( GetMenu( HWindow), IDM_VIEWMAX, MF_BYCOMMAND | MF_UNCHECKED );
    }

InvalidateRect( HWindow, NULL, TRUE );
Layout();
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::SelectMemorizedViewCmd(RTMessage)
{
    // pick memorized view
    TMemorizedViewDialog *dialog =
        new TMemorizedViewDialog( TPimFrame::pFrame,
        IDD_MEMORIZED_DIALOG );

    //a weird bug: if a timescale radio button has the focus
    //when a memorized view is loaded, then that button gets a
    //click when the dialog is closed.  Causes weird behavior if
    //the dialog changed the timescale.
    //Easy fix -- kill the focus before opening this dialog.
    SetFocus( NULL );

    GetApplicationObject()->ExecDialog( dialog );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::MemorizeViewCmd(RTMessage)
{
    TSafStr name;

    // get a name for this memorized view
    KSTR banner( "Memorizing Current View" );
    KSTR message( "Enter name for memorized view:" );
    TOurInputDialog *aDialog = new TOurInputDialog(
        this, banner, message, name, _HelpMemView );

    // cancelled?
    if (GetApplication()->ExecDialog( aDialog ) != IDOK) {

        return;
    }

    // blank name?
    if (name.IsBlank()) {

        TMessageBox::Ok( this,
            "Blank names are not allowed.",
            "Bad Memorized View Name" );
        return;
    }

    // create a memorized view
    PTMemorizedView  newViewP = new TMemorizedView( name );

    // check to see if any other memorized view has
    // the same name
    RTMemorizedView oldView = TSysState::pMemViews->FindView( *newViewP );

    // if no other memorized view with this name
    // exists, add it to the memorized list
    if (oldView == NOOBJECT) {

        TSysState::pMemViews->add( *newViewP );
        TSysState::bSystemDirty = TRUE;
    }
    else {

        // view should be replaced?
        if( IDOK == TMessageBox::OkAndCancel( this,
            "There is already a memorized view with this name.\n\n Replace it?",
            "Replace Memorized View" )) {

            // YES, replace old view with new
            TSysState::pMemViews->detach( oldView, TXArray::kDelete );
            TSysState::pMemViews->add( *newViewP );
            TSysState::bSystemDirty = TRUE;
        }
        else {

            // NO, delete new view
            delete newViewP;
        }
    }
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::DayCmd(RTMessage)
{
    TSysState::timeScale= SCALE_HOUR;
    UpdateButtons();

    if( TSysState::activeView == kCalView )
        UpdateCalendars();

    Notify( kScaleChg, NOOBJECT );
    CHKBUG( ERR_GEN, HealthCheck() );
}


//*****************************************************************************

//*****************************************************************************
void TPimFrame::WeekCmd(RTMessage)
{
    TSysState::timeScale= SCALE_DAY;
    UpdateButtons();

    if( TSysState::activeView == kCalView )
        UpdateCalendars();

    Notify( kScaleChg, NOOBJECT );
    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::MonthCmd(RTMessage)
{
    TSysState::timeScale= SCALE_WEEK;
    UpdateButtons();

    if( TSysState::activeView == kCalView )
        UpdateCalendars();

    Notify( kScaleChg, NOOBJECT );
    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::YearCmd(RTMessage)
{
    TSysState::timeScale= SCALE_MONTH;
    UpdateButtons();

    if( TSysState::activeView == kCalView )
        UpdateCalendars();

    Notify( kScaleChg, NOOBJECT );
    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::TodayCmd(RTMessage)
{
    TSysState::selTime.SetNow();

    pCalendar->SetSelTime( TSysState::selTime );
    Notify( kTimeChg, NOOBJECT );

    CHKBUG( ERR_GEN, HealthCheck() );
}


//*****************************************************************************

//*****************************************************************************
void TPimFrame::NewNoteButton(RTMessage)
{
PTNote pNote;

    pNote= TSysState::pAllNotes->CreateNote( this );
    if( pNote != NULL )
        {
        GetApplication()->ExecDialog(new TNoteView(this, IDD_NOTE, pNote ));
        }

    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************
//
//*****************************************************************************
void TPimFrame::TableButton(RTMessage)
{
// don't do anything if we're already in the tabgrid
if (TSysState::activeView == kReportView) return;

TCursor::Busy( TRUE );

	pView->CloseWindow();

    pView= new TReportGrid( this );
	GetApplication()->MakeWindow( pView );
	Layout();
    TSysState::activeView= kReportView;
    UpdateButtons();
    CHKBUG( ERR_GEN, HealthCheck() );

TCursor::Busy( FALSE );
}

//*****************************************************************************
//
//*****************************************************************************
void TPimFrame::CalendarButton(RTMessage)
{
// don't do anything if we're already in the calendar
if (TSysState::activeView == kCalView) return;

TCursor::Busy( TRUE );
UpdateCalendars();
TCursor::Busy( FALSE );
}

//*****************************************************************************
//
//*****************************************************************************
void TPimFrame::TimelineButton(RTMessage Msg)
{
// don't do anything if we're already in the timeline
if (TSysState::activeView == kTimeView) return;

if ((Msg.LP.Hi == BN_CLICKED))
	{
    TCursor::Busy( TRUE );

    pView->CloseWindow();

    pView= new TTimeLineGrid( this );
	GetApplication()->MakeWindow( pView );
    Layout();
    TSysState::activeView= kTimeView;
    UpdateButtons();
    CHKBUG( ERR_GEN, HealthCheck() );

    TCursor::Busy( FALSE );
    }
}

//*****************************************************************************
//
//*****************************************************************************
void TPimFrame::GraphButton(RTMessage)
{
// don't do anything if we're already in the graph
if (TSysState::activeView == kGrafView) return;

    TCursor::Busy( TRUE );

    pView->CloseWindow();

	pView= new TGrafGrid( this );
	GetApplication()->MakeWindow( pView );
	Layout();
    TSysState::activeView= kGrafView;
    UpdateButtons();
    CHKBUG( ERR_GEN, HealthCheck() );

    TCursor::Busy( FALSE );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::BrowseButton(RTMessage)
{
// don't do anything if we're already in the browser
if (TSysState::activeView == kBrowView) return;

    TCursor::Busy( TRUE );

    pView->CloseWindow();

	pView= new TBrowGrid( this );
	GetApplication()->MakeWindow( pView );
	Layout();
    TSysState::activeView= kBrowView;
    UpdateButtons();
    CHKBUG( ERR_GEN, HealthCheck() );

    TCursor::Busy( FALSE );

}

//*****************************************************************************
// AddressButton
//*****************************************************************************
void TPimFrame::AddressButton(RTMessage)
{
    GetApplicationObject()->ExecDialog(
        new TAddressDialog( this, IDD_ADDRESS ));
}

//*****************************************************************************
//
//*****************************************************************************
void TPimFrame::FilterButton(RTMessage)
{
    if( IDOK == GetApplication()->ExecDialog(new TFilterDlg(this, IDD_FILTER, TSysState::pFilter)))
        {
        NoteReset();
        }

    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::CatMgrButton(RTMessage)
{
	GetApplication()->ExecDialog(new TCatMgr(this, IDD_CATMGR));
    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::HelpAboutCmd(RTMessage)
{
    GetApplication()->ExecDialog(new TDialog( this, TOEM::aboutBoxID ));
    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::HelpOnHelpCmd( RTMessage )
{
WinHelp( HWindow, TOEM::helpFileName, HELP_HELPONHELP, (DWORD) 0 );
}


//*****************************************************************************

//*****************************************************************************
void TPimFrame::HelpViewCmd( RTMessage )
{
DWORD id=0;

switch( TSysState::activeView )
    {
    case kCalView:
        switch( TSysState::timeScale )
            {
            case SCALE_WEEK:
                    id= _HelpCalMonth;
                    break;
            case SCALE_DAY:
                    id= _HelpCalWeek;
                    break;
            case SCALE_HOUR:
            default:
                    id= _HelpCalDay;
                    break;
            }
        break;
    case kReportView:   id= _HelpReportOv;      break;
    case kTimeView:     id= _HelpTimelineOv;    break;
    case kGrafView:     id= _HelpGraphOv;       break;
    case kBrowView:     id= _HelpBrowserOv;     break;
    default:
        break;
    }

WinHelp( HWindow, TOEM::helpFileName, HELP_CONTEXT, id );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::HelpIndexCmd( RTMessage )
{
WinHelp( HWindow, TOEM::helpFileName, HELP_INDEX, (DWORD) 0 );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::ScaleHourButton(RTMessage msg)
{

msg;

// don't move if we're already in hours
if (TSysState::timeScale == SCALE_HOUR) return;

TMessage Msg;
DayCmd( Msg );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::ScaleDayButton(RTMessage)
{
// don't move if we're already in days
if (TSysState::timeScale == SCALE_DAY) return;

TMessage Msg;
WeekCmd( Msg );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::ScaleWeekButton(RTMessage)
{
// don't move if we're already in weeks
if (TSysState::timeScale == SCALE_WEEK) return;

TMessage Msg;
MonthCmd( Msg );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::ScaleMonthButton(RTMessage)
{
// don't move if we're already in months
if (TSysState::timeScale == SCALE_MONTH) return;

TMessage Msg;
YearCmd( Msg );
}



//*****************************************************************************

//*****************************************************************************
void TPimFrame::DateNowButton(RTMessage)
{
TMessage Msg;
TodayCmd( Msg );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::AllCatsButton(RTMessage)
{
TMessage Msg;
SelectAllCmd( Msg );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::NoCatsButton(RTMessage)
{
TMessage Msg;
SelectNoneCmd( Msg );
}


//*****************************************************************************

//*****************************************************************************
void TPimFrame::FrameMaxButton(RTMessage)
{
TMessage Msg;
ViewMaxCmd( Msg );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::HeadlineButton(RTMessage)
{
TMessage Msg;
if (TSysState::activeView == kCalView)
    ShowBarsCmd( Msg );
else
    HeadlineCmd( Msg );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::OptionSortCmd( RTMessage )
{
    if (IDOK == GetApplication()->ExecDialog(new TSortD( this, IDD_ORDER ))) {

        TSysState::pAllNotes->Sort();
        NoteReset();
    }
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::OptionSortNowCmd( RTMessage )
{
    TSysState::pAllNotes->Sort();
    NoteReset();
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::OptionFontCmd( RTMessage )
{
}

//*****************************************************************************
// Put up the holidays dialog
//*****************************************************************************
void TPimFrame::OptionHolidaysCmd( RTMessage )
{
    PTHolidayDialog dialogP = new THolidayDialog(
        this, IDD_HOLIDAY, TSysState::selTime );
    if( IDOK == GetApplication()->ExecDialog( dialogP ) )
        {
        //change in holiday set may require
        //view and date selector to change, so give 'em a nudge
        InvalidateRect( pView->HWindow, NULL, TRUE );
        InvalidateRect( pCalendar->HWindow, NULL, TRUE );
        }
}

//*****************************************************************************
// see OptionDisplayCmd()
//*****************************************************************************
void TPimFrame::DisplayUpdateAction( Pvoid pWin, Pvoid )
{
SendMessage( ((PTWindow)pWin)->HWindow, WM_LOCALCOLOR, 0, 0 );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::OptionPreferCmd( RTMessage )
{
    PTPreferD pDlg= new TPreferD( this, IDD_PREFERENCE );

    if( IDOK == GetApplication()->ExecDialog( pDlg ) )
        {
        //update ourselves
        SendMessage( HWindow, WM_LOCALCOLOR, 0, 0 );

        //tell all child windows to update themselves
        ForEach( DisplayUpdateAction, NULL );
        }
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::OptionGroupCats( RTMessage )
{
    PTGroupCatDialog pDlg= new TGroupCatDialog( this );
    if( IDOK == GetApplication()->ExecDialog( pDlg ) )
        {
        ResetView();
        }
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::OptionReportCmd( RTMessage )
{
    PTReportD pDlg= new TReportD( this, IDD_REPORT_OPTIONS );

    if( IDOK == GetApplication()->ExecDialog( pDlg ) )
        {
        //if report view is active, reset it
        // (cols, rows may have changed)
        if( TSysState::activeView == kReportView )
            ResetView();
        }


};

//*****************************************************************************

//*****************************************************************************
void TPimFrame::PhoneSetup( RTMessage )
{
    PTDialog pDlg= new TDialSetupDialog( this );

    GetApplication()->ExecDialog( pDlg );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::PhoneHangup( RTMessage )
{
    TDialDialog::Hangup( this );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::PrintCmd( RTMessage )
{
// prints current view

TPrintView printObj( this );
printObj.PrepareToPrint( pView, IDD_HV_PRTDLG );

}

//*****************************************************************************
//  DayviewOptions
//*****************************************************************************
void TPimFrame::OptionDayviewCmd( RTMessage  )
{
    if( IDOK == GetApplication()->ExecDialog(new TDayviewD( this, IDD_DAYVIEW )))
        {
        // update the dayview if necessary
        if ((TSysState::activeView == kCalView) && (TSysState::timeScale <= SCALE_DAY))
            {
            UpdateCalendars();
            }
        }

}

//*****************************************************************************
//  ConversionOptions (includes duration units and formatting options)
//*****************************************************************************
void TPimFrame::OptionDurationFmtCmd( RTMessage  )
{
    if( IDOK == GetApplication()->ExecDialog(new TDurFmtD( this, IDD_CONVERSION )))
        {
        // update the tabview if necessary
        if ((TSysState::activeView == kReportView) || (TSysState::activeView == kGrafView))
            {
            Notify( kNoteSetChg, NOOBJECT );
            }
        }
}

//*****************************************************************************
// restore default settings
//*****************************************************************************
void TPimFrame::OptionRestoreCmd( RTMessage )
{
TProfile::RestoreSort();
TProfile::RestoreInterval();

TSysState::UseDefaultGroups();
TReportGrid::ReadProfile();

ResetView();
}

//*****************************************************************************
// restore default settings
//*****************************************************************************
void TPimFrame::OptionRestoreIntervalCmd( RTMessage )
{
TProfile::RestoreInterval();
ResetView();
}

//*****************************************************************************
// restore default settings
//*****************************************************************************
void TPimFrame::OptionRestoreColumnCmd( RTMessage )
{
TReportGrid::ReadProfile();
ResetView();
}

//*****************************************************************************
// restore default settings
//*****************************************************************************
void TPimFrame::OptionRestoreTabsCmd( RTMessage )
{
TSysState::UseDefaultGroups();
ResetView();
}

//*****************************************************************************
// restore default settings
//*****************************************************************************
void TPimFrame::OptionRestoreSortCmd( RTMessage )
{
TProfile::RestoreSort();
ResetView();
}

//*****************************************************************************
//  WorkTimeOptions
//*****************************************************************************
void TPimFrame::OptionWorkTimeCmd( RTMessage  )
{
    if( IDOK == GetApplication()->ExecDialog(new TWorkTimeD( this, IDD_WORK_TIME)))
        {
        // redraw the window
        InvalidateRect( pView->HWindow, NULL, TRUE );
        // always redraw the calendar in the lower-right
        InvalidateRect( pCalendar->HWindow, NULL, TRUE );
        }
}

//-----------------------------------------------------------------
// PrepareViewMenu
//
//  Add the memorized views to the Select View menu
//  expects handle of menubar
//
//-----------------------------------------------------------------
const int kNumStockItems = 5;
void TPimFrame::PrepareViewMenu( HMENU hMenuBar )
{
HMENU hMenu;

    hMenu= GetSubMenu( hMenuBar, 4 );   // the view menu
    hMenu= GetSubMenu( hMenu, 0 );      // the view popup

    // delete the old ones
    int n = GetMenuItemCount( hMenu ) - kNumStockItems;
    for (int i = 0; i < n; i++)
        {
        DeleteMenu( hMenu, 0, MF_BYPOSITION );
        }

    // insert up to 10 new ones
    n = TSysState::pMemViews->getItemsInContainer();
    n = min( n, 10 );

    for (i = 0; i < n; i++)
        {
        TSafStr name;

        RTMemorizedView view = (RTMemorizedView) (*TSysState::pMemViews)[i];
        view.GetName( name );

        //limit text to 40 chars to avoid huge menus
        char buf[ 41 ];
        name.Get( buf, 40 );

        InsertMenu( hMenu, i,
            MF_BYPOSITION | MF_STRING,
            IDM_FILE_VIEW_0 + i, buf);
        }

    // add separator
    if (n > 0)
        {
        InsertMenu( hMenu, n,
            MF_BYPOSITION | MF_SEPARATOR,
            IDM_FILE_VIEW_0, NULL );
        }

}


//-----------------------------------------------------------------
// ViewCmd (and its minions)
//
//  Select a memorized view based on the menu item
//  selected
//-----------------------------------------------------------------
void TPimFrame::View0Cmd( RTMessage ) { ViewCmd( 0 ); }
void TPimFrame::View1Cmd( RTMessage ) { ViewCmd( 1 ); }
void TPimFrame::View2Cmd( RTMessage ) { ViewCmd( 2 ); }
void TPimFrame::View3Cmd( RTMessage ) { ViewCmd( 3 ); }
void TPimFrame::View4Cmd( RTMessage ) { ViewCmd( 4 ); }
void TPimFrame::View5Cmd( RTMessage ) { ViewCmd( 5 ); }
void TPimFrame::View6Cmd( RTMessage ) { ViewCmd( 6 ); }
void TPimFrame::View7Cmd( RTMessage ) { ViewCmd( 7 ); }
void TPimFrame::View8Cmd( RTMessage ) { ViewCmd( 8 ); }
void TPimFrame::View9Cmd( RTMessage ) { ViewCmd( 9 ); }
void TPimFrame::ViewCmd( int n )
{
    RTMemorizedView memorizedView =
        ((RTMemorizedView) (*TSysState::pMemViews)[n]);
    memorizedView.Select();
}
