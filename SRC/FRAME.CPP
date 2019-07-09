//*****************************************************************************
//  main window and command processing //
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "frame.h"

#ifndef __cat__
#include "cat.h"
#endif
#ifndef __catlist__
#include "catlist.h"
#endif
#ifndef __cattree__
#include "cattree.h"
#endif
#ifndef __catbox__
#include "catbox.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __profile__
#include "profile.h"
#endif
#ifndef __filter__
#include "filter.h"
#endif
#ifndef __datepick__
#include "datepick.h"
#endif
#ifndef __error__
#include "error.h"
#endif
#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __holiday__
#include "holiday.h"
#endif
#ifndef __fontmap__
#include "fontmap.h"
#endif
#ifndef __utility__
#include "utility.h"
#endif
#ifndef __odbutton__
#include "odbutton.h"
#endif
#ifndef __tipwin__
#include "tipwin.h"
#endif
#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef  __cursor__
#include "cursor.h"
#endif
#ifndef __pen__
#include "pen.h"
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
#ifndef __memview__
#include "memview.h"
#endif
#ifndef __memnote__
#include "memnote.h"
#endif
#ifndef __rptgrid__
#include "rptgrid.h"
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
#ifndef __alarmset__
#include "alarmset.h"
#endif
#ifndef __print__
#include "print.h"
#endif
#ifndef __oem__
#include "oem.h"
#endif
#ifndef __parse__
#include "parse.h"
#endif
#ifndef __addressd__
#include "addressd.h"
#endif
#ifndef __fieldmap__
#include "fieldmap.h"
#endif
#ifndef __dialdlg__
#include "dialdlg.h"
#endif

#define MAX_TITLE_LEN 120

const kTimerID= 1;

//publicly available pointer to the frame object
PTPimFrame  TPimFrame::pFrame= NULL;

//handle to startup window and font (called from htp.cpp)
HANDLE      TPimFrame::itsHStartMsg= NULL;
HFONT       TPimFrame::itsHStartFont= NULL;

//public pointer to tip window
PTTipWin  TPimFrame::pTipWin= NULL;

//*****************************************************************************
// startup message handling
//*****************************************************************************
void TPimFrame::ShowStartMsg( HINSTANCE hInstance, int scrHeight, int scrWidth )
{

itsHStartMsg= CreateWindow( "STATIC",
                           "",
                           WS_DLGFRAME | SS_CENTER | WS_VISIBLE | WS_POPUP,
                           scrWidth/2 - 165,
                           scrHeight/2 - 40,
                           330,
                           80,
                           HWND_DESKTOP,
                           NULL,
                           hInstance,
                           NULL );

if( itsHStartMsg )
    {
    //can't use system font here, because it doesn't have the (TM) character
    itsHStartFont= CreateFont( -15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Arial" );
    SendMessage( itsHStartMsg, WM_SETFONT, itsHStartFont, (DWORD) 0 );

    SendMessage( itsHStartMsg, WM_SETTEXT, 0, (DWORD) TOEM::signOnMsg );

    SetWindowPos( itsHStartMsg, HWND_TOPMOST, 0, 0, 0, 0,
                  SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE );
    }
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::KillStartMsg( void )
{
if( itsHStartMsg )
    {
    DestroyWindow( itsHStartMsg );
    DeleteObject( itsHStartFont );
    itsHStartMsg= NULL;         //this is needed by timer routine!
    }
}

//*****************************************************************************
// create frame
//*****************************************************************************
TPimFrame::TPimFrame( LPSTR ATitle ): TWindow( NULL, ATitle)
{
RECT r;

//come up disabled, let timer tick enable window when ready
Attr.Style |= WS_DISABLED;

// initialize shared public vars
pFrame= this;
TSysState::timeScale= SCALE_DAY;
TSysState::selTime.SetNow();

// private vars
bMemMessage= FALSE; 	//haven't seen low mem message
bMaxFrame= FALSE;       //don't maximize view

EnableKBHandler();		//enable tabbing between child windows

//
// set up all the "life-of-application" objects
//
pHolidays=  new THolidaySet;  // has to precede profile
pIntlFmt= new TIntlFmt;       // has to precede profile
pProfile = new TProfile;

TCatBoxData::AppInit();
TReportGrid::AppInit();
TParse::AppInit();
TFieldMap::AppInit();

pCursor=  new TCursor;
pGDIMap=  new TGDIMap;
pFontMap= new TFontMap;
pPen=     new TPen;
pPrint=   new TPrint();

TSysState::CreateContainers();

//system menu
AssignMenu( IDM_FRAMEMENU );

//steal the whole desktop, more or less
GetClientRect( GetDesktopWindow() , &r );
Attr.X= r.left;
Attr.Y= r.top;
Attr.H= min( ((r.bottom - r.top) - 20), 580 );
Attr.W= min( ((r.right - r.left) - 20), 780 );

//set up cat box data (data is prepared when a file
// is read in)
pCatBoxData= new TCatBoxData( TRUE );
pCatBoxData->PrepareNone();


//
//open the top bar of button controls...
//
#define BUTTONROW 8
#define SCALEROW 80
#define BARPOS(n) (4 + 24*n)

pNoteButton= new TODBitMapButton( this, IDD_BARNOTE, BARPOS(1), BUTTONROW+5, 42, 52 );
pNoteButton->Attr.Style &= ~WS_TABSTOP;

// set up rectangles for current date and time
pDOWTitle= new TStatic( this, IDD_TIMETITLE, "DOW", 0, 0, 0, 0, FALSE );
pDOWTitle->Attr.Style |= SS_CENTER;
pDateTitle= new TStatic( this, IDD_TIMETITLE, "Time", 0, 0, 0, 0, FALSE );
pDateTitle->Attr.Style |= SS_CENTER;
pTimeTitle= new TStatic( this, IDD_TIMETITLE, "Time", 0, 0, 0, 0, FALSE );
pTimeTitle->Attr.Style |= SS_CENTER;
SetTime();

pCalViewButton= new TODBitMapButton( this, IDD_BARCALENDAR, BARPOS(7), BUTTONROW+5, 42, 52 );
pCalViewButton->Attr.Style &= ~WS_TABSTOP;

pTabViewButton= new TODBitMapButton( this, IDD_BARTABLE, BARPOS(9), BUTTONROW+5, 42, 52 );
pTabViewButton->Attr.Style &= ~WS_TABSTOP;

pTimeViewButton= new TODBitMapButton( this, IDD_BARTIMELINE, BARPOS(11), BUTTONROW+5, 42, 52 );
pTimeViewButton->Attr.Style &= ~WS_TABSTOP;

pGrafViewButton= new TODBitMapButton( this, IDD_BARGRAPH, BARPOS(13), BUTTONROW+5, 42, 52 );
pGrafViewButton->Attr.Style &= ~WS_TABSTOP;

pBrowViewButton= new TODBitMapButton( this, IDD_BARBROWSE, BARPOS(15), BUTTONROW+5, 42, 52 );
pBrowViewButton->Attr.Style &= ~WS_TABSTOP;

//
// NB the sizes and positions of controls here is fictitious
// they're actually set in layout()
//

pMaxButton= new TBButton( this, IDD_FRAMEMAX, "^", 0, 0, 0, 0, FALSE );
pMaxButton->Attr.Style &= ~WS_TABSTOP;

pFullTextButton= new TCheckBox( this, IDD_FULLTEXT, "", 0, 0, 0, 0, NULL );
pFullTextButton->Attr.Style &= ~WS_TABSTOP;

//
//timescale buttons
//
pGroupBox= new TGroupBox( this, IDD_SCALEGROUP, "", 0, 0, 0, 0 );
pGroupBox->Attr.Style &= ~WS_TABSTOP;

pDayButton= new TRadioButton( this, IDD_SCALEHOUR, "day", 0, 0, 0, 0, pGroupBox);
pDayButton->Attr.Style |= BS_AUTORADIOBUTTON;
pDayButton->Attr.Style &= ~WS_TABSTOP;

pWeekButton= new TRadioButton( this, IDD_SCALEDAY, "week", 0, 0, 0, 0, pGroupBox  );
pWeekButton->Attr.Style |= BS_AUTORADIOBUTTON;
pWeekButton->Attr.Style &= ~WS_TABSTOP;

pMonthButton= new TRadioButton( this, IDD_SCALEWEEK, "month", 0, 0, 0, 0, pGroupBox );
pMonthButton->Attr.Style |= BS_AUTORADIOBUTTON;
pMonthButton->Attr.Style &= ~WS_TABSTOP;

pYearButton= new TRadioButton( this, IDD_SCALEMONTH, "year", 0, 0, 0, 0, pGroupBox );
pYearButton->Attr.Style |= BS_AUTORADIOBUTTON;
pYearButton->Attr.Style &= ~WS_TABSTOP;

pTitle= new TStatic( this, IDD_BARTITLE, "Notes", 0, 0, 0, 0, MAX_TITLE_LEN );
pTitle->Attr.Style |= SS_LEFTNOWORDWRAP;

pCatTitle= new TStatic( this, IDD_CATTITLE, "Cats", 0, 0, 0, 0, MAX_TITLE_LEN );
pCatTitle->Attr.Style |= SS_CENTER | WS_BORDER;

//address button
pAddrButton= new TODTextButton( this, IDD_ADDRESS_BUTTON, "Addresses...", 0, 0, 0, 0, FALSE );
pAddrButton->Attr.Style &= ~WS_TABSTOP;

//cat button
pCatButton=new TODTextButton( this, IDD_BARCAT, "Folders...", 0, 0, 0, 0, FALSE );
pCatButton->Attr.Style &= ~WS_TABSTOP;

// catbox
pBox= new TCatBox( this, IDD_NOTELIST, 0, 0, 0, 0 );

pCatAll= new TODTextButton( this, IDD_CATSELALL, "All", 0, 0, 0, 0, NULL);
pCatAll->Attr.Style &= ~WS_TABSTOP;

pCatNone= new TODTextButton( this, IDD_CATSELNONE, "None", 0, 0, 0, 0, NULL);
pCatNone->Attr.Style &= ~WS_TABSTOP;

pFilterButton=new TODTextButton( this, IDD_BARFILTER, "Filter...", 0, 0, 0, 0, FALSE );
pFilterButton->Attr.Style &= ~WS_TABSTOP;

pCalendar= new TDatePick( this, IDD_CALMONTH, 0, 0, 0, 0, TSysState::selTime );

pNowButton= new TODTextButton( this, IDD_DATENOW, "Today", 0, 0, 0, 0, FALSE );
pNowButton->Attr.Style &= ~WS_TABSTOP;

//
//create tip msg window (modeless dialog)
//
pTipWin= new TTipWin( this, IDD_TIPMSGWIN );
pTipWin->Show( SW_HIDE );

//
// start with calendar view (may be changed by auto-loaded file)
//
pView= new TWeekGrid( this );
TSysState::activeView= kCalView;

// we're the top application
itsActive = TRUE;

CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::GetWindowClass( WNDCLASS & AWndClass )
{
TWindow::GetWindowClass( AWndClass );

AWndClass.hIcon= LoadIcon( GetApplication()->hInstance, MAKEINTRESOURCE( ICN_PIMSTER ));
}

//*****************************************************************************
//
//*****************************************************************************
void TPimFrame::SetupWindow( void )
{
TWindow::SetupWindow();

//address book is optional
if( !TOEM::bAddressBook )
    {
    pAddrButton->Show( SW_HIDE );
    HMENU hMenu= GetMenu( HWindow );
    DeleteMenu( hMenu, IDM_CATADDR, MF_BYCOMMAND );
    }

//point to our data
pBox->Transfer( &pCatBoxData, TF_SETDATA );

//change buttons' font
HFONT hFont= TFontMap::GetScreenFont( kNormalFont );

SendMessage( pCatTitle->HWindow, WM_SETFONT,
            (WORD) TFontMap::GetScreenFont( kBigFont ), (DWORD) 0 );
SendMessage( pDOWTitle->HWindow, WM_SETFONT,
            (WORD) TFontMap::GetScreenFont( kBigFont ), (DWORD) 0 );
SendMessage( pDateTitle->HWindow, WM_SETFONT,
            (WORD) TFontMap::GetScreenFont( kNormalFont ), (DWORD) 0 );
SendMessage( pTimeTitle->HWindow, WM_SETFONT,
            (WORD) TFontMap::GetScreenFont( kNormalFont ), (DWORD) 0 );

SendMessage( pCatButton->HWindow, WM_SETFONT, hFont, (DWORD) 0 );
SendMessage( pAddrButton->HWindow, WM_SETFONT, hFont, (DWORD) 0 );
SendMessage( pFilterButton->HWindow, WM_SETFONT, hFont, (DWORD) 0 );
SendMessage( pCatAll->HWindow, WM_SETFONT, hFont, (DWORD) 0 );
SendMessage( pCatNone->HWindow, WM_SETFONT, hFont, (DWORD) 0 );
SendMessage( pNowButton->HWindow, WM_SETFONT, hFont, (DWORD) 0 );
SendMessage( pTitle->HWindow, WM_SETFONT, hFont, (DWORD) 0 );

SendMessage( pDayButton->HWindow, WM_SETFONT, hFont, (DWORD) 0 );
SendMessage( pWeekButton->HWindow, WM_SETFONT, hFont, (DWORD) 0 );
SendMessage( pMonthButton->HWindow, WM_SETFONT, hFont, (DWORD) 0 );
SendMessage( pYearButton->HWindow, WM_SETFONT, hFont, (DWORD) 0 );

SendMessage( pFullTextButton->HWindow, WM_SETFONT, hFont, (DWORD) 0 );
SendMessage( pMaxButton->HWindow, WM_SETFONT, hFont, (DWORD) 0 );

pCalViewButton->SetCheck( BF_CHECKED );

//get timer ticks for life of frame window
// approx every 1 second (shortened, was 10 seconds - hmh )
if( 0 == SetTimer( HWindow, kTimerID, 1000, NULL ) )
    {
    SYSERR( ERR_TIMER );
    }

//load command line file (if specified)
char *p= GetApplication()->lpCmdLine;
if( *p )
    DoFileOpen( p, FALSE );
else if( TProfile::bAutoLoad )
    ReadMRUFile( 1, FALSE );

UpdateButtons();

SetTitle();

GetApplicationObject()->MakeWindow( pTipWin );
pTipWin->Update();

CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
BOOL TPimFrame::CanClose( void )
{
BOOL rc= FALSE;

if( ConfirmClean() )
    {
    //close the help engine (even though it might not be open)
    WinHelp( HWindow, TOEM::helpFileName, HELP_QUIT, (DWORD) 0 );

    rc= TRUE;
    }

return rc;
}

//*****************************************************************************

//*****************************************************************************
TPimFrame::~TPimFrame()
{

//FIXME check that this isn't too late!
// i.e., that window still exists
KillTimer( HWindow, kTimerID);

TSysState::DeleteContainers();

delete pBox;
delete pCatBoxData;
delete pProfile;
delete pIntlFmt;
delete pCursor;
delete pGDIMap;
delete pHolidays;
delete pFontMap;
delete pPen;
delete pPrint;

TCatBoxData::AppCleanup();
TParse::AppCleanup();
TFieldMap::AppCleanup();
}


//*****************************************************************************
// Every few seconds....
// 1. check system resources
// 2. check alarms
//*****************************************************************************
void TPimFrame::WMTimer( RTMessage )
{
const kTickInterval= 1;     //seconds between ticks
static BOOL toldCnt= 0;
static inProgress= FALSE;
static alarmCheck= 0;
static startTime= 0;

//cancel startup msg
startTime+= kTickInterval;
if( itsHStartMsg )   //depose immediately
    {
    KillStartMsg();
    EnableWindow( HWindow, TRUE );
    pTipWin->MakeActive( TRUE );
    SetFocus( pCalendar->HWindow );
    }

BOOL low= FALSE;

if( IsWin31() )
    {
    // insist on 6% free GDI
    WORD freeGDI= GetFreeSystemResources( GFSR_GDIRESOURCES );
    if( freeGDI < 6 )
        low= TRUE;

    // insist on 16% free USER
    WORD freeUser= GetFreeSystemResources( GFSR_USERRESOURCES );
    if( freeUser < 16 )
        low= TRUE;
    }

//need 100K free mem
DWORD freeGlobal= GetFreeSpace( 0 );
if( freeGlobal < 100000L )
    low= TRUE;

if( low )
    {
    if( toldCnt== 0 )
        {
        if( !IsIconic( HWindow ) && !inProgress )
            {
            //timer ticks are asynchronous, so we have to explicitly prevent
            //serial calls to the MessageBox while it's up
            inProgress= TRUE;

            MessageBox( HWindow, "To avoid losing information,\nclose other active Windows applications,\nthen press the OK button.",
                        "Low Memory and/or System Resources", MB_ICONSTOP );
            toldCnt= 30 / kTickInterval;        //give user thirty secs before next warning

            inProgress= FALSE;
            }
        }
    else
        {
        if( toldCnt > 0 )       //keep quiet until cnt = 0
            toldCnt--;
        }
    }
else
    toldCnt= 0;

//
// check the alarms:
//
// if the application is active then only handle alarms if the
// the frame or tip window is top-most;
// if the application is minimized or inactive (meaning some other
// application is on top) then always show the alarms
//
alarmCheck --;
HWND activeWindow = GetActiveWindow();
if ( alarmCheck < 0 &&
    TSysState::pAlarms != NULL &&
    ( HWindow == activeWindow ||
      pTipWin->HWindow == activeWindow ||
      IsIconic( HWindow ) || !itsActive ) )
    {
    TSysState::pAlarms->CheckAlarms();
    alarmCheck= 10 / kTickInterval;     //check every ten seconds
    }

// update the clock
SetTime();

if (TDialDialog::CurrentDialDialog != NULL)
    TDialDialog::CurrentDialDialog->UpdateTime();
}

//*****************************************************************************
// SetTime
//*****************************************************************************
void TPimFrame::SetTime( void )
{
TTime now;
now.SetNow();
// change the title of the dow/date/time controls
if (now != currentTime)
    {
    currentTime = now;
    char  s[256];
    TIntlFmt::FormatDayName( currentTime, kFullName, s );
    pDOWTitle->SetCaption( s );
    TIntlFmt::FormatDate( currentTime, kLongDateNoDOW, s );
    pDateTitle->SetCaption( s );
    TIntlFmt::FormatTime( currentTime, FALSE, s );
    pTimeTitle->SetCaption( s );
    }
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::WMCtlColor( RTMessage Msg )
{
HDC hdc= (HDC) Msg.WParam;
WORD childID= GetDlgCtrlID( (HANDLE) Msg.LP.Lo );

if( Msg.LP.Hi == CTLCOLOR_BTN && childID == IDD_CATSELALL )
     {
     Msg.Result= TGDIMap::SetColorEnvironment( hdc, kEnvBkCtl );
     }
else if( Msg.LP.Hi == CTLCOLOR_STATIC &&
        ( childID == IDD_CATTITLE ||
          childID == IDD_TIMETITLE ))
    {
    Msg.Result= TGDIMap::SetColorEnvironment( hdc, kEnvBkCtl );
    }
else
    DefWndProc( Msg );

}

//*****************************************************************************
// don't erase background underneath view window, it's about to be
// repainted by view anyway; this avoids the yellow flash...
//*****************************************************************************
void TPimFrame::WMEraseBkgnd( RTMessage Msg )
{
Msg.Result= (DWORD) 1;  //we do it ourselves, in WMPaint
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::WMSysColorChange( RTMessage )
{
//for now, only gdimap cares about system colors
TGDIMap::SysColorChange();
}

//*****************************************************************************
// prepare menubar
//*****************************************************************************
void TPimFrame::WMInitMenu( RTMessage Msg )
{
HMENU hMenu= (HMENU) Msg.WParam;

if( hMenu == GetMenu( HWindow ) )
	{
    // lru filenames
    PrepareMRUMenu( hMenu );

    //memorized views
    PrepareViewMenu( hMenu );

    //remove phone item if address book is disabled
    if( !TOEM::bAddressBook )
        {
        DeleteMenu( hMenu, IDM_PHONEMENU, MF_BYCOMMAND );
        }

    //help context --
    TSafStr buf;
    switch( TSysState::activeView )
        {
        case kCalView:      buf.KSET( "&View help (Calendar)" );    break;
        case kReportView:   buf.KSET( "&View help (Report)" );      break;
        case kTimeView:     buf.KSET( "&View help (Timeline)" );    break;
        case kGrafView:     buf.KSET( "&View help (Graph)" );       break;
        case kBrowView:     buf.KSET( "&View help (Browser)" );     break;
        default:
            break;
        }

    ModifyMenu( hMenu, IDM_HELPVIEW,
                        MF_BYCOMMAND | MF_STRING, IDM_HELPVIEW,
                        buf );

    }

// clipboard paste
if( IsClipboardFormatAvailable( CF_TEXT ) )
    EnableMenuItem( hMenu, IDM_EDITPASTE,  MF_BYCOMMAND | MF_ENABLED );
else
    EnableMenuItem( hMenu, IDM_EDITPASTE,  MF_BYCOMMAND | MF_GRAYED );

}


//*****************************************************************************
// eat mouse message if activating window
//FIXME not sure why this doesn't work
//*****************************************************************************
void TPimFrame::WMMouseActivate( RTMessage Msg )
{
//Msg.Result= MA_ACTIVATEANDEAT;
DefWndProc( Msg );
}

//*****************************************************************************
// close tipwin if modal dialog deactivates frame
//*****************************************************************************
void TPimFrame::WMEnable( RTMessage Msg )
{
// show/hide tipwin (which is "topmost", hence ill-behaved)
// check for address dialog or noteview dialog
// still up, since the dialog seems to get confused
// and enable this window if two modal dialogs are up
// at the same time and one closes
if( Msg.WParam == 1 && !IsIconic( HWindow ))
    {
    // if either the address window or the noteview
    // window is up, then make sure it gets its
    // deserved enable call. Windows sends the enable
    // to the parent of a just-closed dialog, which
    // means that if both modal dialogs are up,
    // the frame is enabled at the expense of the
    // other modal dialog
    HANDLE addressWin = TAddressDialog::GetWindowHandle();
    if (addressWin)
        SendMessage( addressWin, WM_ENABLE, TRUE, 0 );
    else if (TNoteView::CurrentNoteViewP)
        SendMessage( TNoteView::CurrentNoteViewP->HWindow,
            WM_ENABLE, TRUE, 0 );
    else 
        pTipWin->MakeActive( TRUE );
    }
else
    pTipWin->MakeActive( FALSE );

DefWndProc( Msg );
}


//*****************************************************************************
// handle tipwin
//*****************************************************************************
void TPimFrame::WMActivateApp( RTMessage Msg )
{
// show/hide tipwin (which is "topmost", hence ill-behaved)
if( Msg.WParam == 1 && !IsIconic( HWindow ) && IsWindowEnabled( HWindow) )
    pTipWin->MakeActive( TRUE );
else
    pTipWin->MakeActive( FALSE );

itsActive = Msg.WParam;
DefWndProc( Msg );
}

//*****************************************************************************
// handle tipwin in icon state transitions
//*****************************************************************************
void TPimFrame::WMSysCommand( RTMessage Msg )
{
// show tipwin if restoring icon
if( (Msg.WParam == SC_RESTORE || Msg.WParam == SC_ZOOM ) && IsIconic( HWindow ) )
    pTipWin->MakeActive( TRUE );
else if( Msg.WParam == SC_ICON )
    pTipWin->MakeActive( FALSE );

DefWndProc( Msg );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::Paint( HDC hdc, PAINTSTRUCT & )
{

//
// do our own background erase
//
SaveDC( hdc );

//
//  This could be made faster, I suspect, by explictly figuring
// out which rectangles needed to be redrawn, and doing just those
//

//exclude completely repainted areas; reduces flashing
ExcludeClipRect( hdc, viewRect.left, viewRect.top, viewRect.right, viewRect.bottom );
ExcludeClipRect( hdc, barRect.left, barRect.top, barRect.right, barRect.bottom );
ExcludeClipRect( hdc, catRect.left, catRect.top, catRect.right, catRect.bottom );
ExcludeClipRect( hdc, calRect.left, calRect.top, calRect.right, calRect.bottom );

RECT r;
GetClipBox( hdc, &r );

HBRUSH hBrush= TGDIMap::GetScreenBrush( kHbrBkFrame);
FillRect( hdc, &r, hBrush );

//don't want to exclude stuff from paint msg
RestoreDC( hdc, -1 );

// now paint our own stuff
RECT newRect= viewRect;
newRect.top --;
FrameRect( hdc, &newRect, TGDIMap::GetScreenBrush( kHbrBorder ) );

//white background for view title
newRect= viewRect;
newRect.bottom= newRect.top + 20;
newRect.left++;
FillRect( hdc, &newRect, TGDIMap::GetScreenBrush( kHbrBkView ) );

//draw bar rectangle
if( !bMaxFrame )
    {
    FillRect( hdc, &barRect, TGDIMap::GetScreenBrush( kHbrBkBar ) );
    FrameRect( hdc, &barRect, TGDIMap::GetScreenBrush( kHbrBorder ) );

    //use white pen top and left, to give 3D look to barRect
    HANDLE hOldPen= SelectObject( hdc, TGDIMap::GetScreenPen( kHi3DPen ) );
    MoveTo( hdc, barRect.left+1, barRect.bottom-2 );
    LineTo( hdc, barRect.left+1, barRect.top+1 );
    LineTo( hdc, barRect.right-1, barRect.top+1 );
    SelectObject( hdc, hOldPen );
    }

//now add drop shadows
DrawDropShadow( hdc, viewRect );

if( !bMaxFrame )
    {
    DrawDropShadow( hdc, catRect );
    DrawDropShadow( hdc, calRect );
    DrawDropShadow( hdc, barRect );
    }
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::WMSize(RTMessage Msg)
{
TWindow::WMSize( Msg );

// adjust child window sizes
Layout();
}

//*****************************************************************************
// move child windows into proper position
//*****************************************************************************
void TPimFrame::Layout( void )
{
WORD maxFlag;
RECT r;
RECT catButtonRect, addButtonRect;

GetClientRect( HWindow, &r );

//if r is less than minimum reasonable dimensions, do layout based
// on reasonable dimensions
if( (r.bottom - r.top) < 290 )
    r.bottom= r.top + 290;

if( (r.right - r.left) < 580 )
    r.right= r.left + 580;

const vDrop= 8;     //space for vertical dropshadow and margin
const hDrop= 8;     // horizontal dropshadow

const catWidth= 150;
const calHeight= 142 + 20;

//calculate bounding rectangles, each includes size of associated buttons
// but not shadow

if( bMaxFrame )
    {
    viewRect.top= BUTTONROW;
    viewRect.left= 8;
    viewRect.right= r.right - hDrop;
    viewRect.bottom= r.bottom - vDrop;
    maxFlag= SWP_HIDEWINDOW;
    }
else
    {
    viewRect.top= SCALEROW;
    viewRect.left= 8;
    viewRect.right= r.right - catWidth - 2*hDrop;
    viewRect.bottom= r.bottom - vDrop;
    maxFlag= SWP_SHOWWINDOW;
    }

barRect.top= BUTTONROW;
barRect.left= 8;
barRect.right= r.right - catWidth - 2*hDrop;
barRect.bottom= SCALEROW - vDrop -1;    //-1 because it just looks better!

if (TOEM::bAddressBook)
    {
    catRect.top          = BUTTONROW;
    catRect.left         = r.right - catWidth - hDrop;
    catRect.right        = catRect.left + catWidth;
    catRect.bottom       = r.bottom - calHeight - 2*vDrop;

    catButtonRect.top    = BUTTONROW;
    catButtonRect.left   = r.right - catWidth - hDrop;
    catButtonRect.right  = catRect.left + catWidth / 2;
    catButtonRect.bottom = r.bottom - calHeight - 2*vDrop;

    addButtonRect.top    = BUTTONROW;
    addButtonRect.left   = r.right - catWidth / 2 - hDrop;
    addButtonRect.right  = catRect.left + catWidth;
    addButtonRect.bottom = r.bottom - calHeight - 2*vDrop;
    }
else
    {
    catRect.top          = BUTTONROW;
    catRect.left         = r.right - catWidth - hDrop;
    catRect.right        = catRect.left + catWidth;
    catRect.bottom       = r.bottom - calHeight - 2*vDrop;

    catButtonRect        = catRect;

    addButtonRect.top    = 0;
    addButtonRect.left   = 0;
    addButtonRect.right  = 0;
    addButtonRect.bottom = 0;
    }

calRect.top= catRect.bottom + vDrop;
calRect.left= r.right - catWidth - hDrop;
calRect.right= calRect.left + catWidth;
calRect.bottom= r.bottom - vDrop;

HANDLE hMove= BeginDeferWindowPos( 22 );
//FIXME check hMove for NULL after each operation

//view group
int left= viewRect.right;
if( pMaxButton != NULL )
    {
    int size= 19;
    hMove= DeferWindowPos( hMove, pMaxButton->HWindow, NULL,
                max( 0, left - size), viewRect.top - 2,
                size,
                20, SWP_NOZORDER );
    left-= size;
    }

if( pFullTextButton != NULL )
    {
    int size= 70;
    hMove= DeferWindowPos( hMove, pFullTextButton->HWindow, NULL,
                max( 0, left - size), viewRect.top,
                size,
                20, SWP_NOZORDER );
    left-= size;
    }

if( pYearButton != NULL )
    {
    int size= 50;
    hMove= DeferWindowPos( hMove, pYearButton->HWindow, NULL,
                max( 0, left - size), viewRect.top,
                size,
                20, SWP_NOZORDER );
    left-= size;
    }


if( pMonthButton != NULL )
    {
    int size= 48+5;
    hMove= DeferWindowPos( hMove, pMonthButton->HWindow, NULL,
                max( 0, left - size), viewRect.top,
                size,
                20, SWP_NOZORDER );
    left-= size;
    }

if( pWeekButton != NULL )
    {
    int size= 48+3;
    hMove= DeferWindowPos( hMove, pWeekButton->HWindow, NULL,
                max( 0, left-size), viewRect.top,
                size,
                20, SWP_NOZORDER );
    left-= size;
    }

if( pDayButton != NULL )
    {
    int size= 48-5;
    hMove= DeferWindowPos( hMove, pDayButton->HWindow, NULL,
                max( 0, left- size), viewRect.top,
                size,
                20, SWP_NOZORDER );
    left-= size;
    }


// shifted to align text with other controls
// that have text at top of view window
if( pTitle != NULL )
    {
    hMove= DeferWindowPos( hMove, pTitle->HWindow, NULL,
                viewRect.left + 5, viewRect.top + 4,
                max( 0, left - viewRect.left - 5),
                15, SWP_NOZORDER );
    }

if( pView != NULL )
    {
    hMove= DeferWindowPos( hMove, pView->HWindow, NULL,
                viewRect.left, viewRect.top + 20,
                viewRect.right - viewRect.left,
                viewRect.bottom - viewRect.top - 20, SWP_NOZORDER );
    }


// catbox group
// pin catbox to right of main window
int catTop= catRect.top;
if( pCatButton != NULL )
    {
    int size= 20;
    hMove= DeferWindowPos( hMove, pCatButton->HWindow, NULL,
                catButtonRect.left, catButtonRect.top,
                catButtonRect.right - catButtonRect.left,
                (bMaxFrame)?0:size, SWP_NOZORDER | maxFlag );
    catTop+= size - 1;      //overlap borders
    }


// address button
if( pAddrButton != NULL && TOEM::bAddressBook )
    {
    int size= 20;
    hMove= DeferWindowPos( hMove, pAddrButton->HWindow, NULL,
                addButtonRect.left, addButtonRect.top,
                addButtonRect.right - addButtonRect.left,
                (bMaxFrame)?0:size, SWP_NOZORDER | maxFlag );
    }

if( pCatTitle != NULL )
    {
    int size= 20;
    hMove= DeferWindowPos( hMove, pCatTitle->HWindow, NULL,
                catRect.left, catTop,
                catRect.right - catRect.left,
                (bMaxFrame)?0:size, SWP_NOZORDER | maxFlag );
    catTop+= size - 1;   // -1 to allow black lines to overlap
    }

if( pBox != NULL )
    {
    hMove= DeferWindowPos( hMove, pBox->HWindow, NULL,
                catRect.left, catTop,
                catRect.right - catRect.left,
                (bMaxFrame)?0:(catRect.bottom - catTop - 19),
                SWP_NOZORDER | maxFlag );
    }

if( pCatAll != NULL )
    {
    hMove= DeferWindowPos( hMove, pCatAll->HWindow, NULL,
        catRect.left, catRect.bottom - 20,
        51, (bMaxFrame)?0:20, SWP_NOZORDER | maxFlag );
    }

if( pCatNone != NULL )
    {
    hMove= DeferWindowPos( hMove, pCatNone->HWindow, NULL,
        catRect.left + 50, catRect.bottom - 20,
        50, (bMaxFrame)?0:20, SWP_NOZORDER | maxFlag );
    }

if( pFilterButton != NULL )
    {
    hMove= DeferWindowPos( hMove, pFilterButton->HWindow, NULL,
        catRect.left + 99, catRect.bottom - 20,
        51, (bMaxFrame)?0:20, SWP_NOZORDER | maxFlag );
    }

// calendar group
// pin calendar below catbox
if( pCalendar != NULL )
    {
    hMove= DeferWindowPos( hMove, pCalendar->HWindow, NULL,
                calRect.left, calRect.top,
                150,
                (bMaxFrame)?0:(142 + 1), SWP_NOZORDER | maxFlag );
    }


if( pNowButton != NULL )
    {
    hMove= DeferWindowPos( hMove, pNowButton->HWindow, NULL,
                calRect.left, calRect.bottom - 20,
                150,
                (bMaxFrame)?0:20, SWP_NOZORDER | maxFlag );
    }

if( pDOWTitle != NULL )
    {
    hMove= DeferWindowPos( hMove, pDOWTitle->HWindow, NULL,
                BARPOS( 3 ) - 4, BUTTONROW + 6,
                BARPOS( 7 ) - BARPOS( 3 ), 18, SWP_NOZORDER | maxFlag );
    }
if( pDateTitle != NULL )
    {
    hMove= DeferWindowPos( hMove, pDateTitle->HWindow, NULL,
                BARPOS( 3 ) - 4, BUTTONROW + 24,
                BARPOS( 7 ) - BARPOS( 3 ), 16, SWP_NOZORDER | maxFlag );
    }
if( pTimeTitle != NULL )
    {
    hMove= DeferWindowPos( hMove, pTimeTitle->HWindow, NULL,
                BARPOS( 3 ) - 4, BUTTONROW + 42,
                BARPOS( 7 ) - BARPOS( 3 ), 16, SWP_NOZORDER | maxFlag );
    }

//button bar buttons just have show/hide flag changed
if( pCalViewButton != NULL )
    hMove= DeferWindowPos( hMove, pCalViewButton->HWindow, NULL, 0, 0, 0, 0,
           SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | maxFlag );

if( pTabViewButton != NULL )
    hMove= DeferWindowPos( hMove, pTabViewButton->HWindow, NULL, 0, 0, 0, 0,
           SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | maxFlag );

if( pTimeViewButton != NULL )
    hMove= DeferWindowPos( hMove, pTimeViewButton->HWindow, NULL, 0, 0, 0, 0,
           SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | maxFlag );

if( pGrafViewButton != NULL )
    hMove= DeferWindowPos( hMove, pGrafViewButton->HWindow, NULL, 0, 0, 0, 0,
           SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | maxFlag );

if( pBrowViewButton != NULL )
    hMove= DeferWindowPos( hMove, pBrowViewButton->HWindow, NULL, 0, 0, 0, 0,
           SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | maxFlag );

if( pNoteButton != NULL )
    hMove= DeferWindowPos( hMove, pNoteButton->HWindow, NULL, 0, 0, 0, 0,
           SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | maxFlag );

//do the move
EndDeferWindowPos( hMove );
}

//*****************************************************************************
// surprise! also updates menu items (checkmarks)
//*****************************************************************************
void TPimFrame::UpdateButtons( BOOL changeTime )
{
WORD d,w,m,y,f;
WORD cd,cw,cm,cy;
WORD cReport, cCal, cCross, cTime, cGraf;

HMENU hMenu= GetMenu( HWindow );

cCal= cReport= cCross= cTime= cGraf= BF_UNCHECKED;
d=w=m=y=f= TRUE;

switch( TSysState::activeView )
    {
    case kCalView:
        y= FALSE;
        f= (TSysState::timeScale == SCALE_WEEK);
        cCal= BF_CHECKED;
        break;

    case kReportView:
        d=w=m=y= FALSE;
        cReport= BF_CHECKED;
        break;

    case kBrowView:
        d=w=m=y= FALSE;
        cCross= BF_CHECKED;
        break;

    case kTimeView:
        cTime= BF_CHECKED;
        break;

    case kGrafView:
        d= FALSE;
        f= FALSE;
        cGraf= BF_CHECKED;
        break;

    default: ;
    }

// set up the "Headlines" or "Show Bars" check box
// (two booleans share a single CheckBox)
if (TSysState::activeView == kCalView)
    {
    KSTR kShowBars( "Time Bars" );

    WORD bCheck = (TSysState::bShowBars) ? BF_CHECKED : BF_UNCHECKED;
    WORD mCheck = (TSysState::bShowBars) ? MF_CHECKED : MF_UNCHECKED;
    mCheck      |= (f) ? 0 : MF_GRAYED;

    // handle the menu items
    EnableMenuItem( GetMenu( HWindow ), IDM_SHOWBARS,
        MF_BYCOMMAND | mCheck );
    EnableMenuItem( GetMenu( HWindow ), IDM_VIEWHEADLINES,
        MF_BYCOMMAND | MF_GRAYED );
    CheckMenuItem( hMenu, IDM_SHOWBARS,
        MF_BYCOMMAND | mCheck );

    // handle the check box
    EnableWindow( pFullTextButton->HWindow, f );
    pFullTextButton->SetCaption( SAF2CP( kShowBars ) );
    pFullTextButton->SetCheck( bCheck );
    }
else
    {
    KSTR kHeadlines( "One Line" );

    WORD bCheck = (!TSysState::bShowFullText) ? BF_CHECKED : BF_UNCHECKED;
    WORD mCheck = (!TSysState::bShowFullText) ? MF_CHECKED : MF_UNCHECKED;
    mCheck      |= (f) ? 0 : MF_GRAYED;

    // handle the menu items
    EnableMenuItem( GetMenu( HWindow ), IDM_SHOWBARS,
        MF_BYCOMMAND | MF_GRAYED );
    EnableMenuItem( GetMenu( HWindow ), IDM_VIEWHEADLINES,
        MF_BYCOMMAND | mCheck );
    CheckMenuItem( hMenu, IDM_VIEWHEADLINES,
        MF_BYCOMMAND | mCheck );

    // handle the check box
    pFullTextButton->SetCaption( SAF2CP( kHeadlines ) );
    pFullTextButton->SetCheck( bCheck );
    EnableWindow( pFullTextButton->HWindow, f );
    }

EnableWindow( pDayButton->HWindow, d );
EnableMenuItem( hMenu, IDM_VIEWDAY, 
				MF_BYCOMMAND | (d)? MF_ENABLED: MF_GRAYED );

EnableWindow( pWeekButton->HWindow, w );
EnableMenuItem( hMenu, IDM_VIEWWEEK, 
				MF_BYCOMMAND | (w)? MF_ENABLED: MF_GRAYED );

EnableWindow( pMonthButton->HWindow, m );
EnableMenuItem( hMenu, IDM_VIEWMONTH, 
				MF_BYCOMMAND | (m)? MF_ENABLED: MF_GRAYED );

EnableWindow( pYearButton->HWindow, y );
EnableMenuItem( hMenu, IDM_VIEWYEAR, 
				MF_BYCOMMAND | (y)? MF_ENABLED: MF_GRAYED );

//set check state ( but only if enabled)
cd=cw=cm=cy= BF_UNCHECKED;
switch( TSysState::timeScale )
    {
    case SCALE_HOUR:
        if( d ) cd= BF_CHECKED;
        break;

    case SCALE_DAY:
        if( w ) cw= BF_CHECKED;
        break;

    case SCALE_WEEK:
        if( m ) cm= BF_CHECKED;
        break;

    case SCALE_MONTH:
        if( y ) cy= BF_CHECKED;
        break;

    default:;
    }

if (changeTime)
    {
    pDayButton->SetCheck( cd );
    pWeekButton->SetCheck( cw );
    pMonthButton->SetCheck( cm );
    pYearButton->SetCheck( cy );
    }

CheckMenuItem( hMenu, IDM_VIEWDAY, MF_BYCOMMAND |
    (cd == BF_CHECKED )? MF_CHECKED: MF_UNCHECKED);
CheckMenuItem( hMenu, IDM_VIEWWEEK, MF_BYCOMMAND |
    (cw == BF_CHECKED )? MF_CHECKED: MF_UNCHECKED);
CheckMenuItem( hMenu, IDM_VIEWMONTH, MF_BYCOMMAND |
    (cm == BF_CHECKED )? MF_CHECKED: MF_UNCHECKED);
CheckMenuItem( hMenu, IDM_VIEWYEAR, MF_BYCOMMAND |
    (cy == BF_CHECKED )? MF_CHECKED: MF_UNCHECKED);


//set check state of view buttons
pCalViewButton->SetCheck( cCal );
pTabViewButton->SetCheck( cReport );
pTimeViewButton->SetCheck( cTime );
pGrafViewButton->SetCheck( cGraf );
pBrowViewButton->SetCheck( cCross );

}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::UpdateCalendars( void )
{
	pView->CloseWindow();

    switch( TSysState::timeScale ) {

        case SCALE_HOUR:

            pView= new TDayGrid( this );
            break;

        case SCALE_DAY:

            pView= new TWeekGrid( this );
            break;

        case SCALE_MONTH:
            TSysState::timeScale= SCALE_WEEK;
            // fall through
        case SCALE_WEEK:

            pView= new TMonGrid( this );
            break;
    }

	GetApplication()->MakeWindow( pView );
    Layout();
    TSysState::activeView= kCalView;
    UpdateButtons();
    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::SetTitle( void )
{
TTime now;
char buf[200];
char filterBuf[50];

wsprintf( buf, "%d of %d notes selected",
            TSysState::pSelNotes->getItemsInContainer(),
            TSysState::pAllNotes->getItemsInContainer() );

pTitle->SetCaption( buf );


filterBuf[0]= '\0';
if( TSysState::pFilter->IsActive() )
    {
    strcpy( filterBuf, "[filtered]" );
    }

if( TSysState::pFilter->IsAllSelected() )
    {
    wsprintf( buf, "ALL selected %s",
            filterBuf
            );
    }
else
    {
    wsprintf( buf, "%d selected %s",
            TSysState::pFilter->NumCats(),
            filterBuf
            );
    }

pCatTitle->SetCaption( buf );

}


//*****************************************************************************
// note or filter changed -- rebuild noteset, then notify world
//*****************************************************************************
void TPimFrame::NoteReset( void )
{
TCursor::Busy( TRUE );

delete TSysState::pSelNotes;
TSysState::pSelNotes= new TNoteSet( *(TSysState::pFilter) );

Notify( kNoteSetChg, NOOBJECT );
SetTitle();
pTipWin->Update();

TCursor::Busy( FALSE );
}

//*****************************************************************************
// call current view's reset function
//*****************************************************************************
void TPimFrame::ResetView( void )
{
TCursor::Busy( TRUE );

if( TSysState::activeView == kCalView )
    UpdateCalendars();
else
    pView->Reset();

TCursor::Busy( FALSE );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::Listen( TSnoopMsg aMsg, RObject /*aObject*/ )
{
switch( aMsg )
    {
    case kCatNew:
    case kCatDel:
        TSysState::bSystemDirty= TRUE;
        SetTitle();
        break;

    //kludge: kCatVis is sent whenever the cat mgr dialog closes,
    // and frame needs to update the title after a cat is deleted,
    // AND kCatDel notification isn't enough because it may come
    // before the cat is removed from the filter, SO this is how I
    // fixed it. /hmh
    case kCatVis:
        SetTitle();
        break;

    case kNoteChg:
        TSysState::bSystemDirty= TRUE;
        NoteReset();
        break;

    case kTimeChg:
        pCalendar->SetSelTime( TSysState::selTime );
        break;

    case kScaleChg:
        UpdateButtons();

        break;

    default:
        break;
    }
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::CalendarHandler(RTMessage Msg)
{
switch( Msg.LP.Hi )
    {
    case CN_TIMECHANGED:
        TSysState::selTime= pCalendar->GetSelTime();
        Notify( kTimeChg, NOOBJECT );
        break;
    default:
        break;
    }

CHKBUG( ERR_GEN, HealthCheck() );
}


//*****************************************************************************
// on selection change, reset the grid window
//*****************************************************************************
void TPimFrame::ListHandler( RTMessage )
{
    TCursor::Busy( TRUE );
    pBox->Transfer( &pCatBoxData, TF_GETDATA );

    if( TSysState::pFilter->itsAllSelected )
        {
        TSysState::pFilter->itsAllSelected= FALSE;   //undo select all
        }

    pCatBoxData->Store( TSysState::pFilter->itsSelCatsP );

    CHKBUG( ERR_GEN, HealthCheck() );

    //rebuild note list
    NoteReset();

    CHKBUG( ERR_GEN, HealthCheck() );
    TCursor::Busy( FALSE );
}

//*****************************************************************************
// check system for problems
//*****************************************************************************
BOOL TPimFrame::HealthCheck( void )
{
TError::MemOK( TRUE );

return ( TSysState::pAllNotes->HealthCheck() &&
        TSysState::pCategories->HealthCheck() &&
        TSysState::pMemNotes->HealthCheck() &&
        TSysState::pMemViews->HealthCheck() &&
        TSysState::GroupCatListP->HealthCheck()
        );
}


//*****************************************************************************
//  TimeDateUpdate
//
//  Somebody messed with WIN.INI, so the conventions for time and date
//  representations may have changed. Read the new conventions and
//  redraw the screen
//*****************************************************************************
void TPimFrame::TimeDateUpdate( RTMessage  )
{
    TIntlFmt::ReadTimeAndDate();

    InvalidateRect( HWindow, NULL, FALSE );

}

//*****************************************************************************
// update window background brush
//*****************************************************************************
void TPimFrame::WMLocalColor( RTMessage )
{
InvalidateRect( HWindow, NULL, TRUE );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::DrawDropShadow( HDC /* hdc */, RECT &/* r */)
{
#if 0
int width= 2;
int indent= 5;

HANDLE hOldBrush= SelectObject( hdc, TGDIMap::GetScreenBrush( kHbrShadow ) );

PatBlt( hdc, r.right, r.top + indent, width, r.bottom - r.top - indent + width, PATCOPY );
PatBlt( hdc, r.left + indent, r.bottom, r.right - r.left - indent, width, PATCOPY );

SelectObject( hdc, hOldBrush );
#endif
}

//************************************************************************
//	put filename in window caption
//************************************************************************
void TPimFrame::ShowCurrentFileName( void )
{
char base[ 100 + MAXPATH ];

wsprintf( base, "%s -  ", TOEM::frameTitle );
strcat( base, currentFile );		//FIXME parse; show only base file name
if( !currentFile[0] )
	strcat( base, "Untitled" );

SetCaption( base );
}


//*****************************************************************************
// global filter changed (i.e., from file read ), so reset frame's catbox
//*****************************************************************************
void TPimFrame::UpdateCatBox( void )
{
    TCursor::Busy( TRUE );

    // reset the catbox and tell it about the new filter
    if (TSysState::pFilter->IsAllSelected())
       pCatBoxData->PrepareAll();
    else
       pCatBoxData->Prepare( TSysState::pFilter->itsSelCatsP );
    pBox->Transfer( &pCatBoxData, TF_SETDATA );

    TCursor::Busy( FALSE );
}

//*****************************************************************************
// file op changed "current view", so complete update is necessary
//*****************************************************************************
void TPimFrame::UpdateView( TTimeScale aTimeScale, RTTime aSelTime, TActiveView aView  )
{
//first, close active window
pView->CloseWindow();

//now update global state variables
TSysState::timeScale= aTimeScale;
TSysState::selTime= aSelTime;

//switch to new view
switch( aView )
    {
    case kTimeView:
        pView= new TTimeLineGrid( this );
        break;

    case kReportView:
        pView= new TReportGrid( this );
        break;

    case kBrowView:
        pView= new TBrowGrid( this );
        break;

    case kGrafView:
        pView= new TGrafGrid( this );
        break;

    case kCalView:
    default:

        if( TSysState::timeScale == SCALE_MONTH )
            TSysState::timeScale= SCALE_WEEK;

        switch( TSysState::timeScale )
            {
            case SCALE_HOUR:
                pView= new TDayGrid( this );
                break;

            case SCALE_DAY:
                pView= new TWeekGrid( this );
                break;

            case SCALE_WEEK:
            default:
                pView= new TMonGrid( this );
                break;
            }

        break;

    }



GetApplication()->MakeWindow( pView );
TSysState::activeView= aView;


///update those who need to know
//NB there is no "Notify()", here, or after globals are reset,
// because there is no view.  Later on, we may decide that it's
// needed(?)
Layout();
pCalendar->SetSelTime( TSysState::selTime );

SetTitle();
pTipWin->Update();


UpdateButtons( /*FALSE*/ );     //FIXME why was this FALSE? /hmh
UpdateCatBox();
}
