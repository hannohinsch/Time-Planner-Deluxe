//*****************************************************************************
// pen-aware edit control
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "pen.h"
#include "ink.h"
#include "penedit.h"
#include "sysstate.h"

#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __fontmap__
#include "fontmap.h"
#endif

//*****************************************************************************

//*****************************************************************************
void TPenEdit::TPenEdit( PTWindowsObject aParent, int anID,
    PTInk aPInk, BOOL aBInkMode )

    : TEdit( aParent, anID, 0 )
{
pInk= aPInk;
bInkMode= aBInkMode;
itsText.Set( "" );

// no automatic transfer
DisableTransfer();
}

//*****************************************************************************
// SetText
//
//  Set the text both in itsText and in the control
//*****************************************************************************
void TPenEdit::Set( RTSafStr safStr )
{
    itsText.Set( safStr );
    SetText( (char*) safStr.Addr() );
}

//*****************************************************************************

//*****************************************************************************
void TPenEdit::~TPenEdit()
{
}

//*****************************************************************************

//*****************************************************************************
RTSafStr TPenEdit::GetString( void )
{
    //FIXME as a side-effect, GetString() makes sure the pInk pointer
    // in fact points to the current ink.  This is terrible.  The
    // penEdit object should have a local copy of its ink, as it
    // has its other data!      //hmh
    if( TPen::bPenActive && bInkMode && pInk )
        {
        //store ink
        HPENDATA hPenDataTemp= (WORD) SendMessage( HWindow, WM_HEDITCTL, HE_GETINKHANDLE, 0L );
        if( hPenDataTemp )
            {
            pInk->SetInkHandle( hPenDataTemp );
            }
        }

    return( itsText );
}

//*****************************************************************************

//*****************************************************************************
void TPenEdit::SetupWindow( void )
{
TEdit::SetupWindow();

//set better font
SendMessage( HWindow, WM_SETFONT,
             TFontMap::GetScreenFont( kBoldFont ), (DWORD) 0 );

if( TPen::bPenActive )
	{
	if( bInkMode && pInk )		//load ink
		{
		HPENDATA hPen= (HPENDATA) pInk->GetInkHandle();

		//switch into ink mode, putting possible ink into control
        SendMessage( HWindow, WM_HEDITCTL, HE_SETINKMODE, hPen );
        }
	}

}

//*****************************************************************************

//*****************************************************************************
void TPenEdit::DefNotificationProc( RTMessage Msg )
{

// this is a MS-supplied workaround to keep the Hedit from flashing while
// in ink mode.
if( Msg.LP.Hi == HN_ENDREC && TPen::bPenActive && bInkMode )
    {
    RECT r;
    //first, validate parent's rectangle covering edit control
    GetWindowRect( HWindow, &r );

    //FIXME disgusting use of cast (doesn't windows.h use union?)
    ScreenToClient( Parent->HWindow, (POINT *) &r.left );
    ScreenToClient( Parent->HWindow, (POINT *) &r.right );

    ValidateRect( Parent->HWindow, &r );

    //now do control itself
    ValidateRect( HWindow, NULL );
    }
else if( Msg.LP.Hi == EN_SETFOCUS && TPen::bPenActive && bInkMode)
    {
    HideCaret( HWindow );       //don't show while in ink mode
    }

TEdit::DefNotificationProc( Msg );
}


//*****************************************************************************
// set selection caret at end of control; i.e., don't select all of it
//*****************************************************************************
void TPenEdit::WMSetFocus( RTMessage Msg )
{
DefWndProc( Msg );

//set selection at end of text
SetSelection( 32767, 32767 );
}

//*****************************************************************************
// WMKillFocus
//
//  If the user has just typed something, then transfer it into the local
//  buffer
//*****************************************************************************
void TPenEdit::WMKillFocus( RTMessage msg )
{
    ForceUpdate();

    // now that we got the time, let windows really kill the focus
    DefWndProc( msg );

}


//*****************************************************************************

//*****************************************************************************
void TPenEdit::WMKeyDown( RTMessage Msg )
{
if( TPen::bPenActive && bInkMode )
    SetTextMode();

DefWndProc( Msg );
}


//*****************************************************************************

//*****************************************************************************
void TPenEdit::Clear( void )
{

SetText( "" );

itsText.Set( "" );

//clean up pen data
if( TPen::bPenActive && pInk )
    {
	pInk->Destroy();

	if( bInkMode )		//load ink
		{
		HPENDATA hPen= (HPENDATA) pInk->GetInkHandle();

        //put (empty) ink into control
        SendMessage( HWindow, WM_HEDITCTL, HE_SETINKMODE, hPen );
        }
    }


}

//*****************************************************************************
// requires that we have pointer to ink object
//*****************************************************************************
void TPenEdit::SetInkMode( void )
{
if( TPen::bPenActive && !bInkMode && pInk )  //is text, switch to ink
    {
    //get text from control
    int  length = GetTextLen() + 1;
    char *p = new char[length];
    GetText( p, length );
    itsText.Set( p );
    delete p;

	//switch into ink mode, possibly putting ink into control
    HPENDATA hPen= pInk->GetInkHandle();
	SendMessage( HWindow, WM_HEDITCTL, HE_SETINKMODE, hPen );
    bInkMode= TRUE;
    HideCaret( HWindow );
    }
}

//*****************************************************************************

//*****************************************************************************
void TPenEdit::SetTextMode( void )
{
if( TPen::bPenActive && bInkMode && pInk )	//is ink, switch to text
    {
    //store ink
	HPENDATA hPenDataTemp= (WORD) SendMessage( HWindow, WM_HEDITCTL, HE_GETINKHANDLE, 0L );
    if( hPenDataTemp )
        {
		pInk->SetInkHandle( hPenDataTemp );
        }

    //go into text mode (don't force recognize)
    SendMessage( HWindow, WM_HEDITCTL, HE_STOPINKMODE, 0L );

    ShowCaret( HWindow );
    bInkMode= FALSE;            //must set mode before calling focus
    SetFocus( HWindow );

    SetText( (char *) itsText.Addr() );
    }
}

//*****************************************************************************

//*****************************************************************************
void TPenEdit::Recognize( void )
{
//go into ink mode (unless we're already there)
if( TPen::bPenActive && pInk)
	{
	if( !bInkMode )  //is text, switch to ink
		{
        //switch into ink mode, possibly putting ink into control
        HPENDATA hPen=pInk->GetInkHandle();
		SendMessage( HWindow, WM_HEDITCTL, HE_SETINKMODE, hPen );
        HideCaret( HWindow );
        }
	else  //is ink, store ink before switching to text mode
		{
        HPENDATA hPenDataTemp= (WORD) SendMessage( HWindow, WM_HEDITCTL, HE_GETINKHANDLE, 0L );
		if( hPenDataTemp )
			{
			pInk->SetInkHandle( hPenDataTemp );
			}
		}

	//go into text mode (force recognize)
    SendMessage( HWindow, WM_HEDITCTL, HE_STOPINKMODE, 1L );

	//stay in text mode
    bInkMode= FALSE;        //must set mode before calling focus
    ShowCaret( HWindow );
    SetFocus( HWindow );
    itsText.Set( "" );
	}
}

//*****************************************************************************
// ForceUpdate
//
//  Force the control to parse its value, just to make sure that there's
//  a way to force the control to be up to date without waiting for a
//  KillFocus message.
//*****************************************************************************
void TPenEdit::ForceUpdate( void )
{
    int  length = GetTextLen() + 1;
    char *p = new char[length];
    GetText( p, length );
    itsText.Set( p );
    delete p;
}

