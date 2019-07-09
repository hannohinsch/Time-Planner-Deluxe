//*****************************************************************************
// new file dialog
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "newdlg.h"
#include "oem.h"

static BOOL CALLBACK EnumChildFn( HWND, long );
#define ENABLE_MORE  0x80000000L
#define DISABLE_MORE 0x00000000L

// -----------------------------------------------------------------------
// Constructor
//
//     Standard file dialog, except that the radio button is also allocated
// -----------------------------------------------------------------------
TNewDialog::TNewDialog( PTWindowsObject AParent,
        LPSTR AFilePath, BOOL *shouldCopyP, PTModule AModule )
     : TOurFileDialog( AParent, IDD_NEW_DIALOG, AFilePath, AModule )
{

    strcpy( Extension, TOEM::fileExt );

    itsHelpTopic= _HelpNew;

    // get the group that will show or hide
    itsMoreGroupCP   = new TGroupBox( this, IDD_NEW_DIALOG_MORE_GROUP );

    // create the radio buttons
    itsMainGroupCP   = new TGroupBox( this, IDD_NEW_DIALOG_GROUP );
    itsEmptyButtonCP = new TRadioButton( this, IDD_NEW_DIALOG_EMPTY, itsMainGroupCP );
    itsCopyButtonCP  = new TRadioButton( this, IDD_NEW_DIALOG_COPY,  itsMainGroupCP );
    itsShouldCopyP   = shouldCopyP;

    itsTransferBuffer[0] = BF_CHECKED;
    itsTransferBuffer[1] = BF_UNCHECKED;
    SetTransferBuffer( itsTransferBuffer );
}


// -----------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------
TNewDialog::~TNewDialog( void )
{
}

// -----------------------------------------------------------------------
// SetupWindow
// -----------------------------------------------------------------------
void TNewDialog::SetupWindow( void )
{
    RECT r;

    TOurFileDialog::SetupWindow();
    GetWindowRect( HWindow, &r );
    itsFullWindowWidth = r.right - r.left;
    itsFullWindowHeight = r.bottom - r.top;
    SetDlgSize( FALSE );
}


// -----------------------------------------------------------------------
// CanClose
//
//  If "empty" is selected, then we can close, otherwise,
//  let the inherited method decide
// -----------------------------------------------------------------------
BOOL TNewDialog::CanClose( void )
{
    TransferData( TF_GETDATA );

    if (itsTransferBuffer[0] == BF_CHECKED)
        return TRUE;
    else
        return TOurFileDialog::CanClose();
}

// -----------------------------------------------------------------------
// HandleEmpty
// -----------------------------------------------------------------------
void TNewDialog::HandleEmpty( RTMessage  )
{
    SetDlgSize( FALSE );
}

// -----------------------------------------------------------------------
// HandleCopy
// -----------------------------------------------------------------------
void TNewDialog::HandleCopy( RTMessage  )
{
    SetDlgSize( TRUE );
}

// -----------------------------------------------------------------------
// Ok
// -----------------------------------------------------------------------
void TNewDialog::Ok( RTMessage Msg )
{
    TransferData( TF_GETDATA );
    *itsShouldCopyP = itsTransferBuffer[1] == BF_CHECKED;
    TOurFileDialog::Ok( Msg );
}

// -----------------------------------------------------------------------
// SetDlgSize
//
//  Show or hide the controls for selecting a file
// -----------------------------------------------------------------------
void TNewDialog::SetDlgSize( BOOL show )
{
RECT moreGroupR;
GetWindowRect( itsMoreGroupCP->HWindow, &moreGroupR );

if( show )
    {
    long parm= ENABLE_MORE | moreGroupR.top;

    // enable/disable windows in moregroup
    EnumChildWindows( HWindow, (WNDENUMPROC) EnumChildFn, parm );

    SetWindowPos( HWindow, NULL, 0, 0,
        itsFullWindowWidth, itsFullWindowHeight,
        SWP_NOZORDER | SWP_NOMOVE );
    }
else
    {
    int  moreGroupHeight = moreGroupR.bottom - moreGroupR.top + 12;

    //resize dlg window to cover moregroup
    SetWindowPos( HWindow, NULL, 0, 0,
        itsFullWindowWidth,
        itsFullWindowHeight - moreGroupHeight,
        SWP_NOZORDER | SWP_NOMOVE );

    long parm= DISABLE_MORE | moreGroupR.top;

    // enable/disable windows in moregroup
    EnumChildWindows( HWindow, (WNDENUMPROC) EnumChildFn, parm );
    }
}

//*****************************************************************************
// EnumChildFn
//*****************************************************************************
BOOL CALLBACK EnumChildFn( HWND h, long parm )
{
RECT rc;
BOOL bEnable= (parm & ENABLE_MORE) != 0;
parm &= ~ENABLE_MORE;

GetWindowRect( h, &rc );

if( rc.top >= parm )     //in moregroup
    {
    ShowWindow( h, (bEnable ? SW_SHOW : SW_HIDE ) );
    EnableWindow( h, bEnable );
    }

return 1;
}

