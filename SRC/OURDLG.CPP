//*****************************************************************************
// adds help button, automatic placement to owl dialog
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "ourdlg.h"
#ifndef __btnutil__
#include "btnutil.h"
#endif
#ifndef __fontmap__
#include "fontmap.h"
#endif
#ifndef __frame__
#include "frame.h"
#endif
#ifndef __message__
#include "message.h"
#endif
#ifndef __oem__
#include "oem.h"
#endif


// -----------------------------------------------------
// Constructor
// -----------------------------------------------------
TOurDialog::TOurDialog( PTWindowsObject parent,
    LPSTR name, PTModule module ):
    TDialog( parent, name, module )
{
    itsButtonOn  = FALSE;
    itsMouseDown = FALSE;
    itsHelpTopic= 0;
    itsParentCaptureH= NULL;
    SetRect( &itsButtonR, 0, 0, 0, 0 );
}

// -----------------------------------------------------
// Constructor
// -----------------------------------------------------
TOurDialog::TOurDialog( PTWindowsObject parent,
    int resID, PTModule module ):
    TDialog( parent, resID, module )
{
    itsButtonOn  = FALSE;
    itsMouseDown = FALSE;
    itsHelpTopic= 0;
    itsParentCaptureH= NULL;
    SetRect( &itsButtonR, 0, 0, 0, 0 );
}

//*****************************************************************************

//*****************************************************************************
TOurDialog::~TOurDialog()
{
if( itsParentCaptureH != NULL )
    SetCapture( itsParentCaptureH );
}

// -----------------------------------------------------
// SetupWindow
//
//  Center the window over its parent
// -----------------------------------------------------
void TOurDialog::SetupWindow( void )
{
    SetCursor( NULL );      //don't save current cursor on activation
    itsParentCaptureH= GetCapture();
    ReleaseCapture();

    TDialog::SetupWindow();

    CenterWindow( Parent->HWindow, HWindow );

    //calculate button rect ( window coords )
    if( itsHelpTopic )
        {
        RECT r;
        GetWindowRect( HWindow, &r );

        int frameWidth   = GetSystemMetrics( SM_CXDLGFRAME );
        int frameHeight  = GetSystemMetrics( SM_CYDLGFRAME );
        int captionHeight = GetSystemMetrics( SM_CYCAPTION );

        SetRect( &itsButtonR,
                r.right - r.left - frameWidth - 29,
                frameHeight + 3,
                r.right - r.left - frameWidth - 4,
                frameHeight + captionHeight - 3 );
        }

}


// -----------------------------------------------------
// CenterWindow
//
//  Center the window over its parent
// -----------------------------------------------------
void TOurDialog::CenterWindow( HANDLE parentWindow, HANDLE window )
{
    RECT r,parentR;
    GetWindowRect( parentWindow, &parentR );
    GetWindowRect( window, &r );
    int width        = r.right - r.left;
    int height       = r.bottom - r.top;
    int parentWidth  = parentR.right - parentR.left;
    int parentHeight = parentR.bottom - parentR.top;

    // calculate the position that would be exactly
    // over the center of the parent
    int newX = parentR.left + (parentWidth - width) / 2;
    int newY = parentR.top  + (parentHeight - height) / 2;

    // if centering the window over its parent would
    // mean that the dialog would be partly offscreen
    // then center the dialog on the screen instead
    int screenWidth  = GetSystemMetrics( SM_CXSCREEN );
    int screenHeight = GetSystemMetrics( SM_CYSCREEN );
    if ((newX < 0) || (newX + width > screenWidth) ||
        (newY < 0) || (newY + height > screenHeight))
        {
        newX = (screenWidth - width) / 2;
        newY = (screenHeight - height) / 2;
        }

    SetWindowPos( window, 0, newX, newY, 0, 0,
        SWP_NOZORDER | SWP_NOSIZE );
}

// -----------------------------------------------------
// NCActivate
//
//  The window is activating or deactivating, redraw
// -----------------------------------------------------
void TOurDialog::NCActivate( RTMessage msg )
{
    // post ourselves a message to paint
    if( itsHelpTopic )
        PostMessage( HWindow, WM_PAINTHELP, 0, 0 );

    DefWndProc( msg );
}

// -----------------------------------------------------
// NCPaint
//
//  Paint the help button in the right of the dialog's
//  menu bar
// -----------------------------------------------------
void TOurDialog::NCPaint( RTMessage msg )
{
    // post ourselves a message to paint
    if( itsHelpTopic )
        PostMessage( HWindow, WM_PAINTHELP, 0, 0 );

    DefWndProc( msg );
}

// -----------------------------------------------------
// NCPaintHelp
//
//  Paint the help button in the right of the dialog's
//  menu bar
// -----------------------------------------------------
void TOurDialog::NCPaintHelp( RTMessage )
{
    PaintHelp();
}

// -----------------------------------------------------
// NCLButtonDown
//
//  The left button went down. Is it in the help button?
//
//  NOTE: Because we capture the mouse, no subsequent mouse messages
//  are non-client messages!
// -----------------------------------------------------
void TOurDialog::NCLButtonDown( RTMessage msg )
{
    POINT p = *(POINT*) &msg.LParam;
    //now make p relative to window origin (comes in screen coords)
    RECT r;
    GetWindowRect( HWindow, &r );
    p.x -= r.left;
    p.y -= r.top;

    if( itsHelpTopic && PtInRect( &itsButtonR, p ))
        {
        itsButtonOn = TRUE;
        itsMouseDown = TRUE;
        itsOldCapture= SetCapture( HWindow );
        PaintHelp();
        }
    else
        DefWndProc( msg );
}

// -----------------------------------------------------
// WMMouseMove
//
//  The mouse moved. Has it left the help button area?>
// -----------------------------------------------------
void TOurDialog::WMMouseMove( RTMessage msg )
{
    if (itsMouseDown)
        {
        POINT p = *(POINT*) &msg.LParam;
        //p is relative to upper left corner of client
        //make relative to window
        ClientToScreen( HWindow, &p );
        RECT r;
        GetWindowRect( HWindow, &r );
        p.x -= r.left;
        p.y -= r.top;

        BOOL nowOn= PtInRect( &itsButtonR, p );
        if( itsButtonOn != nowOn )
            {
            itsButtonOn= nowOn;
            PaintHelp();
            }
        }

    DefWndProc( msg );
}


// -----------------------------------------------------
// WMLButtonUp
//
//  The mouse was released. Is it in the help button?
// -----------------------------------------------------
void TOurDialog::WMLButtonUp( RTMessage msg )
{
    if( itsMouseDown )
        {
        ReleaseCapture();
        SetCapture( itsOldCapture );
        }

    itsMouseDown = FALSE;
    itsButtonOn = FALSE;

    if( itsHelpTopic )
        PaintHelp();

    //if mouse was in button area, open help file
    POINT p = *(POINT*) &msg.LParam;
    //p is relative to upper left corner of client
    //make relative to window
    ClientToScreen( HWindow, &p );
    RECT r;
    GetWindowRect( HWindow, &r );
    p.x -= r.left;
    p.y -= r.top;

    if( itsHelpTopic && PtInRect( &itsButtonR, p ))
        {
        DisplayHelp();
        }
    else
        DefWndProc( msg );
}


// -----------------------------------------------------
// PaintHelp
//
//  Paint the help button
// -----------------------------------------------------
void TOurDialog::PaintHelp( void )
{
    HDC dc = GetWindowDC( HWindow );
    HFONT hOldFont= SelectObject( dc, TFontMap::GetScreenFont( kSmallFont ) );

    DrawTextButton( dc, "Help",
        itsButtonR.left,
        itsButtonR.top,
        itsButtonR.right - itsButtonR.left,
        itsButtonR.bottom - itsButtonR.top,
        itsButtonOn );

    SelectObject( dc, hOldFont );
    ReleaseDC( HWindow, dc );
}


// -----------------------------------------------------
// DisplayHelp
//
//  The help button was successfully pushed. Display
//  the help topic for this dialog
// -----------------------------------------------------
void TOurDialog::DisplayHelp( void )
{
    if( itsHelpTopic )
        WinHelp( TPimFrame::pFrame->HWindow, TOEM::helpFileName, HELP_CONTEXT, itsHelpTopic );
}

