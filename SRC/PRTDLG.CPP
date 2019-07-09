//*****************************************************************************
// print dialogs: set up PrintInfo structure
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "addrbook.h"
#include "basegrid.h"
#include "cursor.h"
#include "gdimap.h"
#include "print.h"
#include "prtdlg.h"
#include "sysstate.h"

//report title length
const kMaxTitleLen= 100;

BOOL    TPrtViewDlg::itsShowDate= TRUE;
BOOL    TPrtViewDlg::itsShowPage= TRUE;

//*****************************************************************************

//*****************************************************************************
void TPrtViewDlg::TPrtViewDlg( PTWindowsObject parent, WORD id, PTRawGrid gridP ): TOurDialog( parent, id )
{
TCursor::Busy( TRUE );

itsGridP= gridP;
itsMark= FALSE;

//help id
if( id == IDD_ADDR_PRTDLG )
    itsHelpTopic= _HelpAddrPrint;
else
    itsHelpTopic= _HelpPrinting;

//choose the right set of controls...
itsHSpinEnabled= FALSE;
itsVSpinEnabled= FALSE;
itsMarkEnabled= FALSE;

if( id == IDD_V_PRTDLG || id == IDD_HV_PRTDLG )
    itsVSpinEnabled= TRUE;

if( id == IDD_H_PRTDLG || id == IDD_HV_PRTDLG )
    itsHSpinEnabled= TRUE;

if( id == IDD_ADDR_PRTDLG )
    itsMarkEnabled= TRUE;


//
//now set up dialog controls
//
pTitle= new TEdit( this, IDD_PRTTITLE, kMaxTitleLen );

itsPageCP= new TCheckBox( this, IDD_PRTSHOWPAGE, NULL );
itsDateCP= new TCheckBox( this, IDD_PRTSHOWDATE, NULL );

if( itsMarkEnabled )
    {
    itsMarkCP= new TCheckBox( this, IDD_PRTMARKEDONLY, NULL );
    }

if( itsHSpinEnabled )
    {
    pHStart= new TStatic( this, IDD_PRTSTART_H, 100 );
    pHStartSpin= new TScrollBar( this, IDD_PRTSTARTSPIN_H );
    pHEnd= new TStatic( this, IDD_PRTEND_H, 100 );
    pHEndSpin= new TScrollBar( this, IDD_PRTENDSPIN_H );
    }
else
    {
    pHStart= NULL;
    pHStartSpin= NULL;
    pHEnd= NULL;
    pHEndSpin= NULL;
    }

if( itsVSpinEnabled )
    {
    pVStart= new TStatic( this, IDD_PRTSTART_V, 100 );
    pVStartSpin= new TScrollBar( this, IDD_PRTSTARTSPIN_V );
    pVEnd= new TStatic( this, IDD_PRTEND_V, 100 );
    pVEndSpin= new TScrollBar( this, IDD_PRTENDSPIN_V );
    }
else
    {
    pVStart= NULL;
    pVStartSpin= NULL;
    pVEnd= NULL;
    pVEndSpin= NULL;
    }

pHorzPageNum= new TStatic( this, IDD_PRTHORZPAGENUM, 10 );
pVertPageNum= new TStatic( this, IDD_PRTVERTPAGENUM, 10 );
pOrientation= new TStatic( this, IDD_PRTORIENT, 10 );
pPrtName= new TStatic( this, IDD_PRTNAME, 40 );

itsGridP->SetDefaultPrintRange();
}

//*****************************************************************************

//*****************************************************************************
void TPrtViewDlg::SetupWindow( )
{
char buf[100];

TOurDialog::SetupWindow();

itsDateCP->SetCheck( (itsShowDate)? BF_CHECKED: BF_UNCHECKED );
itsPageCP->SetCheck( (itsShowPage)? BF_CHECKED: BF_UNCHECKED );

//scroll controls
if( itsHSpinEnabled )
    {
    pHStartSpin->SetRange( 0, 2 );
    pHStartSpin->SetPosition( 1 );

    pHEndSpin->SetRange( 0, 2 );
    pHEndSpin->SetPosition( 1 );
    }

if( itsVSpinEnabled )
    {
    pVStartSpin->SetRange( 0, 2 );
    pVStartSpin->SetPosition( 1 );

    pVEndSpin->SetRange( 0, 2 );
    pVEndSpin->SetPosition( 1 );
    }

//put text into controls
TSafStr safBuf;
itsGridP->GetPrtTitle( safBuf );
pTitle->SetText( (char *) safBuf.Addr() );

//range spin controls
if( itsHSpinEnabled )
    {
    if( itsGridP->HChgCellStartIndex( 0, buf, sizeof(buf) ) )
        pHStart->SetCaption( buf );
    else
        EnableWindow( pHStartSpin->HWindow, FALSE );

    if( itsGridP->HChgCellEndIndex( 0, buf, sizeof(buf) ) )
        pHEnd->SetCaption( buf );
    else
        EnableWindow( pHEndSpin->HWindow, FALSE );
    }

if( itsVSpinEnabled )
    {
    if( itsGridP->VChgCellStartIndex( 0, buf, sizeof(buf) ) )
        pVStart->SetCaption( buf );
    else
        EnableWindow( pVStartSpin->HWindow, FALSE );

    if( itsGridP->VChgCellEndIndex( 0, buf, sizeof(buf) ) )
        pVEnd->SetCaption( buf );
    else
        EnableWindow( pVEndSpin->HWindow, FALSE );
    }

//
// page sizes
//
WORD hPages, vPages;
itsGridP->GetPrtPages( hPages, vPages );

wsprintf( buf, " %d", hPages );
pHorzPageNum->SetText( buf );

wsprintf( buf, " %d", vPages );
pVertPageNum->SetText( buf );

TPrint::GetPageOrientation( safBuf );
pOrientation->SetText( (char *) safBuf.Addr() );

TPrint::GetPrtName( safBuf );
pPrtName->SetText( (char *) safBuf.Addr() );

TCursor::Busy( FALSE );
};

//*****************************************************************************
// invoke printer setup common dialog
//*****************************************************************************
void TPrtViewDlg::SetupPrinter( RTMessage )
{

BOOL rc= TPrint::SetupPrinter( this );

if( rc )
    {
    Redisplay();
    }

}

//*****************************************************************************

//*****************************************************************************
void TPrtViewDlg::Ok( RTMessage msg )
{
char buf[ kMaxTitleLen + 1 ];

pTitle->GetText( buf, kMaxTitleLen );
itsGridP->SetPrtTitle( buf );

itsShowDate= itsDateCP->GetCheck() == BF_CHECKED;
itsShowPage= itsPageCP->GetCheck() == BF_CHECKED;

TDialog::Ok( msg );
}


//*****************************************************************************
// redirects scrollbar msgs
//*****************************************************************************
void TPrtViewDlg::WMHScroll( RTMessage Msg )
{
HWND hwnd= Msg.LP.Hi;

if( itsHSpinEnabled && hwnd == pHStartSpin->HWindow )
    HStartSpinHandler( Msg );
else if( itsHSpinEnabled && hwnd == pHEndSpin->HWindow )
    HEndSpinHandler( Msg );
else if( itsVSpinEnabled && hwnd == pVStartSpin->HWindow )
    VStartSpinHandler( Msg );
else if( itsVSpinEnabled && hwnd == pVEndSpin->HWindow )
    VEndSpinHandler( Msg );

}


//*****************************************************************************
// changes itsGridP->leftCell
//*****************************************************************************
void TPrtViewDlg::HStartSpinHandler( RTMessage Msg )
{
char buf[100];

switch( Msg.WParam )
    {
    case SB_LINEUP:
        if( itsGridP->HChgCellStartIndex( -1, buf, sizeof(buf) ) )
            {
            pHStart->SetCaption( buf );
            Redisplay();
            }
        break;

    case SB_LINEDOWN:
        if( itsGridP->HChgCellStartIndex( +1, buf, sizeof(buf) ) )
            {
            pHStart->SetCaption( buf );
            Redisplay();
            }
        break;

    default:
        break;
    }

}

//*****************************************************************************
// changes itsPiP->rightCell
//*****************************************************************************
void TPrtViewDlg::HEndSpinHandler( RTMessage Msg )
{
char buf[100];

switch( Msg.WParam )
    {
    case SB_LINEUP:
        if( itsGridP->HChgCellEndIndex( -1, buf, sizeof(buf) ) )
            {
            pHEnd->SetCaption( buf );
            Redisplay();
            }
        break;

    case SB_LINEDOWN:
        if( itsGridP->HChgCellEndIndex( +1, buf, sizeof(buf) ) )
            {
            pHEnd->SetCaption( buf );
            Redisplay();
            }
        break;

    default:
        break;
    }

};

//*****************************************************************************
// changes itsPiP->leftCell
//*****************************************************************************
void TPrtViewDlg::VStartSpinHandler( RTMessage Msg )
{
char buf[100];

switch( Msg.WParam )
    {
    case SB_LINEUP:
        if( itsGridP->VChgCellStartIndex( -1, buf, sizeof(buf) ) )
            {
            pVStart->SetCaption( buf );
            Redisplay();
            }
        break;

    case SB_LINEDOWN:
        if( itsGridP->VChgCellStartIndex( +1, buf, sizeof(buf) ) )
            {
            pVStart->SetCaption( buf );
            Redisplay();
            }
        break;

    default:
        break;
    }

};

//*****************************************************************************
// changes itsPiP->rightCell
//*****************************************************************************
void TPrtViewDlg::VEndSpinHandler( RTMessage Msg )
{
char buf[100];

switch( Msg.WParam )
    {
    case SB_LINEUP:
        if( itsGridP->VChgCellEndIndex( -1, buf, sizeof(buf) ) )
            {
            pVEnd->SetCaption( buf );
            Redisplay();
            }
        break;

    case SB_LINEDOWN:
        if( itsGridP->VChgCellEndIndex( +1, buf, sizeof(buf) ) )
            {
            pVEnd->SetCaption( buf );
            Redisplay();
            }
        break;

    default:
        break;
    }

};

//*****************************************************************************
// inform address grid if mark changes
//*****************************************************************************
void TPrtViewDlg::HandleMarkedOnly( RTMessage )
{
BOOL m= itsMarkCP->GetCheck() == BF_CHECKED;

if( m != itsMark )
    {
    itsMark= m;
    ((PTAddrGrid) itsGridP)->PrintPrep( itsMark );
    Redisplay();
    }
}

//*****************************************************************************
// update controls
//*****************************************************************************
void TPrtViewDlg::Redisplay( void )
{
char buf[100];

//GetPrtPages() can be reaal slow, so we need busy cursor....
TCursor::Busy( TRUE );

if( itsHSpinEnabled )
    {
    itsGridP->HChgCellStartIndex( 0, buf, sizeof(buf) );
    pHStart->SetCaption( buf );

    itsGridP->HChgCellEndIndex( 0,  buf, sizeof(buf) );
    pHEnd->SetCaption( buf );
    }

if( itsVSpinEnabled )
    {
    itsGridP->VChgCellStartIndex( 0, buf, sizeof(buf) );
    pVStart->SetCaption( buf );

    itsGridP->VChgCellEndIndex( 0,  buf, sizeof(buf) );
    pVEnd->SetCaption( buf );
    }

// printable pages
WORD hPages, vPages;
itsGridP->GetPrtPages( hPages, vPages );
wsprintf( buf, " %d", hPages );
pHorzPageNum->SetText( buf );

wsprintf( buf, " %d", vPages );
pVertPageNum->SetText( buf );

TSafStr safBuf;
TPrint::GetPageOrientation( safBuf );
pOrientation->SetText( (char *) safBuf.Addr() );

TPrint::GetPrtName( safBuf );
pPrtName->SetText( (char *) safBuf.Addr() );

TCursor::Busy( FALSE );
}

