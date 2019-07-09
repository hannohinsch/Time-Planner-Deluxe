//*****************************************************************************
// month calendar w/ navigation buttons; used to select a date
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "datepick.h"

#ifndef __calctl__
#include "calctl.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __fontmap__
#include "fontmap.h"
#endif
#ifndef  __cursor__
#include "cursor.h"
#endif
#ifndef  __setmond__
#include "setmond.h"
#endif
#ifndef  __odbutton__
#include "odbutton.h"
#endif

//*****************************************************************************

//*****************************************************************************
TDatePick::TDatePick( PTWindowsObject AParent, WORD resID, RTTime startDate ):
        TControl( AParent, resID )
{
Attr.Style |= WS_BORDER;

selTime= startDate;

pMinus2= new TODTextButton( this, IDD_CALMINUS2, "<<", 0, 0, 20, 20 );
pMinus= new TODTextButton( this, IDD_CALMINUS, "<", 19, 0, 20, 20 );
pTitle= new TODTextButton( this, IDD_CALTITLE, "", 38, 0, 72, 20 );
pTitle->Attr.Style |= SS_CENTER;

pPlus= new TODTextButton( this, IDD_CALPLUS, ">", 109, 0, 20, 20 );
pPlus2= new TODTextButton( this, IDD_CALPLUS2, ">>", 128, 0, 20, 20 );

pGrid= new TCalCtl( this, IDD_CALMONGRID, 0, 22, 150, 142, startDate );
pGrid->Attr.Style |= WS_TABSTOP;

pGrid->SetAutoClick( TRUE );        //kbd control
}

//*****************************************************************************

//*****************************************************************************
TDatePick::TDatePick( PTWindowsObject AParent, WORD resID, int x, int y, int dx, int dy, RTTime startDate ):
        TControl( AParent, resID, "", x, y, dx, dy )
{

Attr.Style |= WS_BORDER;

selTime= startDate;

pMinus2= new TODTextButton( this, IDD_CALMINUS2, "<<", 0, 0, 20, 20 );
pMinus2->Attr.Style &= ~WS_TABSTOP;
pMinus= new TODTextButton( this, IDD_CALMINUS, "<", 19, 0, 20, 20 );
pTitle= new TODTextButton( this, IDD_CALTITLE, "", 38, 0, 72, 20 );
pTitle->Attr.Style |= SS_CENTER;

pPlus= new TODTextButton( this, IDD_CALPLUS, ">", 109, 0, 20, 20 );
pPlus2= new TODTextButton( this, IDD_CALPLUS2, ">>", 128, 0, 20, 20 );

pGrid= new TCalCtl( this, IDD_CALMONGRID, 0, 22, 150, 142, startDate );

pGrid->SetAutoClick( TRUE );        //kbd control
}

//*****************************************************************************

//*****************************************************************************
TDatePick::~TDatePick( void )
{
}

//*****************************************************************************
//
//*****************************************************************************
void TDatePick::SetupWindow( void )
{
TControl::SetupWindow();

//fonts
HFONT hFont= TFontMap::GetScreenFont( kNormalFont );

SendMessage( pTitle->HWindow, WM_SETFONT, hFont, (DWORD) 0 );
SendMessage( pMinus->HWindow, WM_SETFONT, hFont, (DWORD) 0 );
SendMessage( pMinus2->HWindow, WM_SETFONT, hFont, (DWORD) 0 );
SendMessage( pPlus->HWindow, WM_SETFONT, hFont, (DWORD) 0 );
SendMessage( pPlus2->HWindow, WM_SETFONT, hFont, (DWORD) 0 );

ResetTitle();
}

//*****************************************************************************
//
//*****************************************************************************
void TDatePick::WMCtlColor( RTMessage Msg )
{
WORD childID;
HDC hdc= (HDC) Msg.WParam;

if( Msg.LP.Hi == CTLCOLOR_STATIC )
    {
    childID= GetDlgCtrlID( (HANDLE) Msg.LP.Lo );
    if( childID == IDD_CALTITLE )
        {
        Msg.Result= TGDIMap::SetColorEnvironment( hdc, kEnvBkCtl );
        }
    else
        DefWndProc( Msg );
    }
else if( Msg.LP.Hi == CTLCOLOR_BTN )
    {
    childID= GetDlgCtrlID( (HANDLE) Msg.LP.Lo );
    if( childID == IDD_CALTITLE )
        {
        Msg.Result= TGDIMap::SetColorEnvironment( hdc, kEnvBkCtl );
        }
    else
        DefWndProc( Msg );
    }
else
    DefWndProc( Msg );

}

//*****************************************************************************

//*****************************************************************************
void TDatePick::WMLocalColor(RTMessage)
{
SetClassWord( HWindow, GCW_HBRBACKGROUND, TGDIMap::GetScreenBrush( kHbrBkCtl ) );

//update our grid, as well
SendMessage( pGrid->HWindow, WM_LOCALCOLOR, 0, 0 );

InvalidateRect( HWindow, NULL, TRUE );
}

//*****************************************************************************
// set new date, based on click
//*****************************************************************************
void TDatePick::GridClick( RTMessage Msg )
{
switch( Msg.LP.Hi )
    {
    case CN_CLICKED:
    case CN_DBLCLICKED:
        selTime= pGrid->GetSelTime();
        TellSelChanged();
        break;
    default:
        break;
    }
}

//*****************************************************************************

//*****************************************************************************
void TDatePick::Minus( RTMessage )
{
    selTime.PlusEquals( SCALE_MONTH, -1 );

    pGrid->SetSelTime( selTime );
    ResetTitle();
    TellSelChanged();
}

//*****************************************************************************

//*****************************************************************************
void TDatePick::Minus2( RTMessage )
{
    selTime.PlusEquals( SCALE_YEAR, -1 );

    pGrid->SetSelTime( selTime );
    ResetTitle();
    TellSelChanged();
}

//*****************************************************************************

//*****************************************************************************
void TDatePick::Plus( RTMessage )
{
    selTime.PlusEquals( SCALE_MONTH, 1 );

    pGrid->SetSelTime( selTime );
    ResetTitle();
    TellSelChanged();
}

//*****************************************************************************

//*****************************************************************************
void TDatePick::Plus2( RTMessage )
{
    selTime.PlusEquals( SCALE_YEAR, 1 );

    pGrid->SetSelTime( selTime );
    ResetTitle();
    TellSelChanged();
}

//*****************************************************************************

//*****************************************************************************
void TDatePick::Title( RTMessage )
{
    // put up the month selection dialog
    TTime newDate( selTime );

    // (make dialog child of parent, not us, for proper deactivation)
    PTSetMonthD dialog = new TSetMonthD( Parent, &newDate );
    GetApplication()->ExecDialog( dialog );

    if (newDate != selTime) {

        selTime = newDate;
        pGrid->SetSelTime( selTime );
        ResetTitle();
        TellSelChanged();
    }
}

//*****************************************************************************

//*****************************************************************************
void TDatePick::SetSelTime( RTTime time )
{
if( !selTime.IsSameDay( time ) )
    {
    selTime= time;
    pGrid->SetSelTime( selTime );
    ResetTitle();
    }
}

//*****************************************************************************

//*****************************************************************************
void TDatePick::WMSetFocus( RTMessage )
{
//give focus to grid
SetFocus( pGrid->HWindow );
}

//*****************************************************************************

//*****************************************************************************
void TDatePick::ResetTitle( void )
{
char buf1[50], buf2[50], buf3[50];

TIntlFmt::FormatYear( selTime, buf1 );
TIntlFmt::FormatMonthName( selTime, kThreeLetter, buf2 );
wsprintf( buf3, "%s %s", buf2, buf1 );

pTitle->SetCaption( buf3 );

}

//*****************************************************************************

//*****************************************************************************
void TDatePick::TellSelChanged( void )
{
SendMessage( Parent->HWindow, WM_COMMAND, Attr.Id,
            MAKELONG( HWindow, CN_TIMECHANGED ));
}

//*****************************************************************************

//*****************************************************************************
RTTime TDatePick::GetSelTime( void )
{
return selTime;
}

//*****************************************************************************

//*****************************************************************************
void TDatePick::GetWindowClass( WNDCLASS & AWndClass )
{
TControl::GetWindowClass( AWndClass );

AWndClass.hbrBackground= TGDIMap::GetScreenBrush( kHbrBkCtl );
}

//*****************************************************************************

//*****************************************************************************
void TDatePick::WMDrawItem( RTMessage msg )
{
    DRAWITEMSTRUCT *di = (DRAWITEMSTRUCT*) msg.LParam;

    SendMessage( di->hwndItem,
        WM_FIRST + WM_DRAWITEM, 0, (long) di );
}

