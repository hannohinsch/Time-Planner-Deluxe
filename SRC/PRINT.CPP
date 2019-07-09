//*****************************************************************************
// print module
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "apptime.h"
#include "fontmap.h"
#include "intlfmt.h"
#include "paint.h"
#include "print.h"
#include "prtdlg.h"
#include "rawgrid.h"
#include "oem.h"

//FIXME put this into standard.h !!!
// note: same name as our own include file!!
#include <c:\borlandc\include\print.h>

//
//static variables
//
WORD            TPrint::vDPI;
WORD            TPrint::hDPI;

RECT            TPrint::rHeader;
RECT            TPrint::rBody;
RECT            TPrint::rFooter;

BOOL            TPrint::bUserAbort= FALSE;
HWND            TPrint::hStatusDlg= NULL;
PTPrtStatusDlg  TPrint::pStatusDlg= NULL;
PTWindowsObject TPrint::pParent= NULL;

PRINTDLG        TPrint::pd;
BOOL            TPrint::bValidPD= FALSE;
HDC             TPrint::hPrintDC= NULL;
int             TPrint::objectCount= 0;

//
//Logical Pixels Per Inch (desired standard scaling)
//
const kLPPI= 100;


/////////////////////////////////////////////////////////////////////////////
////////////////////////// Print VIEW ///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//*****************************************************************************

//*****************************************************************************
void TPrintView::TPrintView( PTWindowsObject aParent ): TPrint( aParent )
{
}

//*****************************************************************************

//*****************************************************************************
void TPrintView::~TPrintView( void )
{
}

//*****************************************************************************
// put up print dialog, then print if user sez so
//*****************************************************************************
void TPrintView::PrepareToPrint( PTRawGrid gridP, WORD id )
{
itsGridP= gridP;

//get print specifics
PTPrtViewDlg pDlg= new TPrtViewDlg( pParent, id, itsGridP );
BOOL rcDlg= GetApplicationObject()->ExecDialog( pDlg );

if( rcDlg == IDOK )
    {
    WORD hPages, vPages;

    PTPaint pPainter= gridP->prtPainter;
    gridP->GetPrtPages( hPages, vPages );

    Print( pPainter, hPages, vPages);
    }

}

//*****************************************************************************
// called for each band of page (i.e., could be called several times per page)
// this fn defines the page layout and scaling
//*****************************************************************************
void TPrintView::PrintPage( PTPaint pPainter, WORD hPage, WORD vPage )
{
TDrawInfo di;
di.hdc= hPrintDC;
di.pPainter= pPainter;

//set mapping mode, origin, etc.
SetupDC();

//set normal (default) font
// painters expect normal font to be selected!
HFONT hOldFont= SelectObject( di.hdc, TFontMap::GetPrintFont( kNormalFont ) );

///////////////////////////////
// print stuff at top of page
///////////////////////////////
pPainter->Text( di, itsGridP->prtTitle.Addr(), kNoHilite,
                kSingleLine, kBigBoldFont, &rHeader );

/////////////////////////
//print grid section
////////////////////////
SaveDC( di.hdc );

// map grid to print in rBody rectangle
POINT orgBody;
orgBody.x= rBody.left;
orgBody.y= rBody.top;
LPtoDP( di.hdc, (LPPOINT) &orgBody, 1 );
SetViewportOrg( di.hdc, orgBody.x, orgBody.y );

// print only what's in updRect, not the whole page
TPrint::GetPrtPixels( &di.updRect );
IntersectClipRect( di.hdc, di.updRect.left, di.updRect.top,
                   di.updRect.right, di.updRect.bottom );

itsGridP->PrintSection( di, hPage, vPage );

RestoreDC( di.hdc, -1 );

///////////////////////////////////
//print stuff at bottom of page....
///////////////////////////////////

//print page #
char numBuf[100];
WORD hPages, vPages;

itsGridP->GetPrtPages( hPages, vPages );
int n= vPage * hPages + hPage + 1;
wsprintf( numBuf, "Page %d of %d", n, (hPages * vPages) );

// right side of page
if( TPrtViewDlg::itsShowPage )
    pPainter->Text( di, numBuf, kNoHilite, kSingleLine | DT_RIGHT,
                    kNormalFont, &rFooter );

//print date
char dateBuf[100];
TTime now;
now.SetNow();
TIntlFmt::FormatDateAndTime( now, kLongDate, dateBuf );

//left side of page
if( TPrtViewDlg::itsShowDate )
    pPainter->Text( di, dateBuf, kNoHilite, kSingleLine, kNormalFont, &rFooter );

// restore previous font
SelectObject( di.hdc, hOldFont );
}

//////////////////////////////////////////////////////////////////////////////
////////////////////////  TPrint  ///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
// call this constructor ONLY for the initial life-of-app instance!
//*****************************************************************************
TPrint::TPrint( void )
{
ASSERT_STR( "Wrong TPrint() constructor", objectCount == 0 )

//keep track of the number of active print objects.
//The idea is that one instance exists for life of app, others
//come and go as needed.  The last one destroyed removes all trace
//of the print settings
objectCount++;

hPrintDC= NULL;
}

//*****************************************************************************
//
//*****************************************************************************
TPrint::TPrint( PTWindowsObject aParent )
{
pParent= aParent;

objectCount++;

//get default printer setup, if necessary
if( !bValidPD )
    {
    pd.lStructSize= sizeof( PRINTDLG );
    pd.hwndOwner= (HANDLE) aParent->HWindow;
    pd.hDevMode= (HANDLE) NULL;
    pd.hDevNames= (HANDLE) NULL;
    pd.Flags= PD_RETURNDEFAULT;
    pd.nFromPage= 0;
    pd.nToPage= 0;
    pd.nMinPage= 0;
    pd.nMaxPage= 0;
    pd.nCopies= 0;
    pd.hInstance= (HANDLE) GetApplicationObject()->hInstance;

    if( PrintDlg( &pd ) )       //get setup (won't display)
        {
        bValidPD= TRUE;

        //get new DC
        if( hPrintDC )
            DeleteDC( hPrintDC );

        DEVNAMES *pDevNames= (DEVNAMES *) GlobalLock( pd.hDevNames );
        DEVMODE *pDevMode= (DEVMODE *) GlobalLock( pd.hDevMode );

        char *p= (char *) pDevNames;

        hPrintDC= CreateDC( (char *) (p + pDevNames->wDriverOffset),
                            (char *) (p + pDevNames->wDeviceOffset),
                            (char *) (p + pDevNames->wOutputOffset),
                            pDevMode);

        GlobalUnlock( pd.hDevNames );
        GlobalUnlock( pd.hDevMode );
        }
    }

SetupDC();   //set map mode, origin

}

//*****************************************************************************
// release pd
//*****************************************************************************
TPrint::~TPrint()
{

objectCount--;

//release if we're deleting the last print object
if( bValidPD && objectCount == 0 )
    {
    if( hPrintDC )
        DeleteDC( hPrintDC );
    if( pd.hDevMode )
        GlobalFree( pd.hDevMode );
    if( pd.hDevNames )
        GlobalFree( pd.hDevNames );

    bValidPD= FALSE;
    }
}

//*****************************************************************************
// set scaling, and rHeader, rBody, rFooter
//*****************************************************************************
void TPrint::SetupDC( void )
{
//get printer/paper characteristics
hDPI= GetDeviceCaps( hPrintDC, LOGPIXELSX );
vDPI= GetDeviceCaps( hPrintDC, LOGPIXELSY );

// our default scaling (i.e., 100%) is 100 log pix per inch
// However, we never scale below hardware resolution, i.e., if
// the printer has only 75 device pixels per inch, we don't cram in
// 100 logical pixels per inch (the loss of resolution is disastrous).

SetMapMode( hPrintDC, MM_ISOTROPIC );
SetWindowExt( hPrintDC, 1, 1 );
SetWindowOrg( hPrintDC, 0, 0 );

//scale to standard 100 log pixels per inch
// round up to more device pixels per logical pixel, because
// low resolution printers should probably print bigger, anyhow
int hDevPerLog= max( 1, (hDPI+ (kLPPI - 5)) / kLPPI);
int vDevPerLog= max( 1, (vDPI+ (kLPPI - 5)) / kLPPI);

SetViewportExt( hPrintDC, hDevPerLog, vDevPerLog );
SetViewportOrg( hPrintDC, 0, 0 );

//
//now set rHeader, rBody, rFooter ( in device coordinates )
//
rBody.top= rBody.left= 0;

// AT SOME POINT, this should be modified to take into account the
// non-printing area of the page.  Currently, we're guaranteed to
// have margins that are too large.
#if 0
//get page size
POINT physSize;
physSize.x= 0;
physSize.y= 0;
Escape( hPrintDC, GETPHYSPAGESIZE, NULL, NULL, &physSize );

//get non-printing area (part of margin)
POINT offset;
offset.x= 0;
offset.y= 0;
Escape( hPrintDC, GETPRINTINGOFFSET, NULL, NULL, &offset );
#endif

const float kLeftMargin= 0.5;
const float kRightMargin= 0.5;

//margins left and right
rBody.left= hDPI * kLeftMargin;
rBody.right= GetDeviceCaps( hPrintDC, HORZRES ) - hDPI*kRightMargin;

rHeader= rBody;
rFooter= rBody;

//
//handle vertical boundaries...
//
//header
rHeader.top= 0.25 * vDPI;
rHeader.bottom= rHeader.top + 0.50 * vDPI;

//footer
rFooter.bottom= GetDeviceCaps( hPrintDC, VERTRES ) - 0.25 * vDPI;
rFooter.top= rFooter.bottom - 0.50 * vDPI;

//body
rBody.top= rHeader.bottom + 0.125 * vDPI;
rBody.bottom= rFooter.top - 0.125 * vDPI;

//convert rects to logical coords
DPtoLP( hPrintDC, (LPPOINT) &rHeader, 2 );
DPtoLP( hPrintDC, (LPPOINT) &rBody, 2 );
DPtoLP( hPrintDC, (LPPOINT) &rFooter, 2 );
}

//*****************************************************************************
// call printer setup common dialog
//*****************************************************************************
BOOL TPrint::SetupPrinter( PTWindowsObject aParent )
{

//reinitialize, as necessary
if( !bValidPD )                   //first time
    {
    pd.lStructSize= sizeof( PRINTDLG );
    pd.hwndOwner= aParent->HWindow;
    pd.hDevMode= (HANDLE) NULL;
    pd.hDevNames= (HANDLE) NULL;
    pd.Flags= PD_PRINTSETUP;
    pd.nFromPage= 0;
    pd.nToPage= 0;
    pd.nMinPage= 0;
    pd.nMaxPage= 0;
    pd.nCopies= 0;
    pd.hInstance= (HANDLE) GetApplicationObject()->hInstance;
    }
else    //we already have valid PD struct, so reuse it
    {
    pd.hwndOwner= aParent->HWindow;
    pd.hInstance= (HANDLE) GetApplicationObject()->hInstance;
    pd.Flags= PD_PRINTSETUP;
    }

BOOL rc= PrintDlg( &pd );

if( rc )
    {
    bValidPD= TRUE;

    //get new DC
    if( hPrintDC )
        DeleteDC( hPrintDC );

    DEVNAMES *pDevNames= (DEVNAMES *) GlobalLock( pd.hDevNames );
    DEVMODE *pDevMode= (DEVMODE *) GlobalLock( pd.hDevMode );

    char *p= (char *) pDevNames;

    hPrintDC= CreateDC( (char *) (p + pDevNames->wDriverOffset),
                        (char *) (p + pDevNames->wDeviceOffset),
                        (char *) (p + pDevNames->wOutputOffset),
                        pDevMode);

    GlobalUnlock( pd.hDevNames );
    GlobalUnlock( pd.hDevMode );

    SetupDC();
    }

return rc;
}

//*****************************************************************************

//*****************************************************************************
void TPrint::GetPrtPixels( RECT *r )
{
r->top= r->left= 0;
r->right= rBody.right - rBody.left;
r->bottom= rBody.bottom - rBody.top;
}


//*****************************************************************************

//*****************************************************************************
void TPrint::GetPageOrientation( RTSafStr str )
{
if( rBody.bottom - rBody.top > rBody.right - rBody.left )
    str.Set( " portrait" );
else
    str.Set( " landscape" );
}


//*****************************************************************************

//*****************************************************************************
void TPrint::GetPrtName( RTSafStr name )
{
char buf[ 200 ];

if( pd.hDevNames != NULL )
    {
    DEVNAMES *pDev= (DEVNAMES *) GlobalLock( pd.hDevNames );

    char *p= (char *) pDev;
    wsprintf( buf, "%s on %s",
                    (char *) (p + pDev->wDeviceOffset),
                    (char *) (p + pDev->wOutputOffset) );

    name.Set( buf );

    GlobalUnlock( pd.hDevNames );
    }
else
    name.Set( "No printer specified" );
}

//*****************************************************************************
// print
//*****************************************************************************
BOOL TPrint::Print( PTPaint pPainter, WORD hPages, WORD vPages )
{
WORD rc= 1;

if( hPrintDC != NULL )
    {
    EnableWindow( pParent->HWindow, FALSE );

    bUserAbort= FALSE;

    // note status dialog is child of parent, not ours
    pStatusDlg= new TPrtStatusDlg( pParent, IDD_PRTSTATUS );
    pParent->GetApplication()->MakeWindow( pStatusDlg );
    hStatusDlg= pStatusDlg->HWindow;
    pStatusDlg->Show( SW_SHOW );

    rc= PrintSub( pPainter, hPages, vPages );

    if( !bUserAbort )
        {
        EnableWindow( pParent->HWindow, TRUE );
        pStatusDlg->CloseWindow();
        }
    }
else
    {
    // error or user pressed cancel
    }

return (rc <= 0 ) || bUserAbort;
}

//*****************************************************************************
// print subfunction -- loop through pages, call PrintPage() for each one
// return TRUE if error
//*****************************************************************************
BOOL TPrint::PrintSub( PTPaint pPainter, WORD hPages, WORD vPages )
{
WORD rc= 1;
char *jobName= TOEM::frameTitle;

HDC hdc= hPrintDC;

Escape( hdc, SETABORTPROC, 0, (LPSTR) AbortProc, NULL );

rc= Escape( hdc, STARTDOC, sizeof( jobName ) -1, jobName, NULL );

for( int vPage= 0; vPage < vPages && rc > 0; vPage++  )
    for( int hPage= 0; hPage < hPages && rc > 0; hPage++ )
        {
        RECT rect;

        rc= Escape( hdc, NEXTBAND, 0, NULL, (LPSTR) &rect );
        while( rc > 0 && !bUserAbort && !IsRectEmpty( &rect ) )
            {
            //update page # in dlg
            char numBuf[100];
            int n= vPage * hPages + hPage + 1;
            wsprintf( numBuf, "Printing page %d", n );
            pStatusDlg->SetMsg( numBuf );

            //print this page
            PrintPage( pPainter, hPage, vPage );

            //check for user abort
            AbortProc( hdc, 0 );

            //next band
            rc= Escape( hdc, NEXTBAND, 0, NULL, (LPSTR) &rect );
            }
        }

if( rc > 0 )
    {
    if( bUserAbort )
        rc= Escape( hdc, ABORTDOC, 0, NULL, NULL );
    else
        rc= Escape( hdc, ENDDOC, 0, NULL, NULL );
    }


return (rc <= 0 ) || bUserAbort;
}

//*****************************************************************************
//
//*****************************************************************************
BOOL PASCAL TPrint::AbortProc( HDC, short )
{
MSG msg;

while( !TPrint::bUserAbort && PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))
    {
    if( !hStatusDlg || !IsDialogMessage( hStatusDlg, &msg ) )
        {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
        }
    }

return !bUserAbort;
}


//////////////////////////////////////////////////////////////////////////////
// TPrtStatusDlg
/////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
// show status of print job; allow cancel button
//*****************************************************************************
TPrtStatusDlg::TPrtStatusDlg( PTWindowsObject aParent, WORD anID ): TOurDialog( aParent, anID )
{
itsStatusMsg= new TStatic( this, IDD_PRTSTATUSMSG, 100 );
}

//*****************************************************************************
// update the status msg
//*****************************************************************************
void TPrtStatusDlg::SetMsg( char * msg )
{
itsStatusMsg->SetText( msg );
}

//*****************************************************************************

//*****************************************************************************
void TPrtStatusDlg::SetupWindow( void )
{
TOurDialog::SetupWindow();

EnableMenuItem( GetSystemMenu( HWindow, FALSE ), SC_CLOSE, MF_GRAYED );
}

//*****************************************************************************

//*****************************************************************************
void TPrtStatusDlg::Cancel( RTMessage Msg )
{
TPrint::bUserAbort= TRUE;
EnableWindow( TPrint::pParent->HWindow, TRUE );
TPrint::hStatusDlg= NULL;

TOurDialog::Cancel( Msg );
}
