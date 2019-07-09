//*****************************************************************************
// right mouse button pop-up window
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "moushelp.h"

// ---------------------------------------------------
// PopUpWindow
//
// Creates a static pop up window
// ---------------------------------------------------
void TMousHelp::MousHelp( PTWindowsObject parent, char *text )
{

    PTMousHelp helpP = new TMousHelp( parent, text );
    PTWindowsObject windowP = parent->GetApplication()->MakeWindow( helpP );

    //make window top-most, and position it
    // slightly above cursor (if possible)
    POINT xy;
    GetCursorPos( &xy );

    RECT r;
    GetWindowRect( windowP->HWindow, &r );
    int width   = r.right  - r.left;
    int height  = r.bottom - r.top;

    int offset  = 7;

    //move it diagonal to cursor position
    xy.y -= offset + height;
    if( xy.y <= 0 )
        xy.y+= 2 * offset + height;

    xy.x -= offset + width;
    if( xy.x <= 0 )
        xy.x+= 2 * offset + width;

    SetWindowPos( windowP->HWindow, HWND_TOPMOST, xy.x, xy.y, 0, 0, SWP_NOSIZE );

    windowP->Show( SW_SHOW );
}

// ---------------------------------------------------
// Constructor -- (sized by caller)
// ---------------------------------------------------
TMousHelp::TMousHelp( PTWindowsObject parent, char *text ):
    TDialog( parent, IDD_POP_UP_WINDOW )
{
SetCaption( "Mouse Help" );

itsTextCP= new TStatic( this, IDD_POP_UP_TEXT, 300 );
itsText.Set( text );
}

// ---------------------------------------------------
// Destructor
// ---------------------------------------------------
TMousHelp::~TMousHelp( void )
{
}

// ---------------------------------------------------
// SetupWindow
//
//  Make sure that the mouse buttons all go to this
//  window, since we want to close the window
//  as soon as the right mouse button is released
// ---------------------------------------------------
void TMousHelp::SetupWindow( void )
{
    TDialog::SetupWindow();
    itsOldCaptureH= SetCapture( HWindow );
    itsTextCP->SetCaption( SAF2CP( itsText ) );
}

// ---------------------------------------------------
// HandleRelease
//
//  If the right mouse button went up, then close
//  the box.
// ---------------------------------------------------
void TMousHelp::HandleRelease( RTMessage )
{
    ReleaseCapture();

    if( itsOldCaptureH )
        SetCapture( itsOldCaptureH );

    ShutDownWindow();
}
