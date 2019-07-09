//*****************************************************************************
// note list dialog (if user clicks on month calendar view cell, for example )
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "notelisd.h"

#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __note__
#include "note.h"
#endif
#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __paint__
#include "paint.h"
#endif
#ifndef __msg__
#include "msg.h"
#endif
#ifndef __cursor__
#include "cursor.h"
#endif

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TNoteListDialog::TNoteListDialog( PTWindowsObject AParent, int AResID, TNoteSet &noteSet, char *prompt ):
	TOurDialog( AParent, AResID )
{
    TCursor::Busy( TRUE );

    // create the interface objects
    itsCancelButton = new TButton( this, IDCANCEL );
    itsOkButton     = new TButton( this, IDOK );
    itsPromptP      = new TStatic( this, IDD_NOTELIST_PROMPT, 256 );
    itsSelector     = new TNoteListGrid( this, IDD_NOTELIST_SELECTOR, TRUE,
        &noteSet );

    itsPromptString = prompt;
}


// -------------------------------------------------
// Destructor
// -------------------------------------------------
TNoteListDialog::~TNoteListDialog( void )
{

    delete itsSelector;
    delete itsOkButton;
    delete itsCancelButton;
}


// -------------------------------------------------
// Ok
// -------------------------------------------------
void TNoteListDialog::Ok( RTMessage )
{
    CloseWindow( (int) itsSelector->GetSelection() );
}

// -------------------------------------------------
// Cancel
//
//  return an impossible number to indicate that
//  nothing was selected
// -------------------------------------------------
void TNoteListDialog::Cancel( RTMessage )
{
    ShutDownWindow( kCanceledNotelist );
}

// -------------------------------------------------
// SetupWindow
// -------------------------------------------------
void TNoteListDialog::SetupWindow( void )
{                                  
    TOurDialog::SetupWindow();

    itsPromptP->SetText( itsPromptString );

    TCursor::Busy( FALSE );
}

// -------------------------------------------------
// HandlePopup
//
//  Close the dialog on a double click
// -------------------------------------------------
void TNoteListDialog::HandlePopup( RTMessage msg )
{
    if (msg.LP.Hi == CN_DBLCLICKED) {

       Ok( msg );
    }
}


// ----------------------------------------------------------------
// ----------------------------------------------------------------
// ---------------------  TNoteListGrid ---------------------------
// ----------------------------------------------------------------
// ----------------------------------------------------------------


const int kNumCols = 3;
enum { kMarkColumn, kDateColumn, kDescriptionColumn };


//*****************************************************************************
// Constructor
//*****************************************************************************
TNoteListGrid::TNoteListGrid( PTWindowsObject AParent, WORD resID,
    BOOL live, PTNoteSet noteSetP )
    : TRuleGrid( AParent, resID )
{
    scrPainter= new TPaint;
    prtPainter= new TPaintPrt;

    SetLeftMargin( 1 );
    SetTopMargin( 1 );
    SetRowCnt( noteSetP->getItemsInContainer() );
    SetColCnt( kNumCols );

    itsNoteSetP  = noteSetP;
    itsSelection = 0;
    itsLive      = live;
}

//*****************************************************************************
// Destructor
//*****************************************************************************
TNoteListGrid::~TNoteListGrid( void )
{
delete scrPainter;
delete prtPainter;
}


//*****************************************************************************
// PaintCell
//*****************************************************************************
void TNoteListGrid::PaintCell( RTDrawInfo di )
{
    char buf1[200];
    char buf2[200];
    RTNote note = (*itsNoteSetP)[di.row];
    if (note == NOOBJECT) {

        return;
    }

    THilite hilite= (itsSelection == di.row)? kInvertHilite: kNoHilite;

    // fill in the background
    switch( di.col ) {

        case kMarkColumn:

            di.pPainter->DrawMark( di, note );
            break;

        case kDateColumn:

            TIntlFmt::FormatDate( note.GetStart(), kShortDate, buf1 );
            TIntlFmt::FormatTime( note.GetStart(), TRUE, buf2 );

            di.pPainter->TextLR( di, buf1, buf2, hilite );
            break;

        case kDescriptionColumn:

            TSafStr title;
            note.Title( title );
            if( !title.Len() && note.GetInk().HasInk() )
                note.GetInk().Show( di.hdc, &di.cellRect );
            else {

                TFont font= (note.IsDone()) ? kStrikeFont : kNormalFont;
                di.pPainter->Text( di, (char *) title.Addr(), hilite, kSingleLine, font );

            }

            break;

    }
}


// ------------------------------------------------------------------------
//	MsgClick
//
//	Send the parent a notification message
// ------------------------------------------------------------------------
void TNoteListGrid::MousClick( RTMousInfo mi )
{
    if (!itsLive) return;

    if( mi.style != kCellPoint ) return;

    InvalidateCell( mi.row, -1 );
    InvalidateCell( itsSelection, -1 );
    itsSelection = mi.row;
    TRuleGrid::MousClick( mi );
    SendMessage( Parent->HWindow, WM_COMMAND, Attr.Id,
        MAKELONG( HWindow, CN_CLICKED ));
}

// ------------------------------------------------------------------------
//	MsgDblClick
//
//	Send the parent a notification message
// ------------------------------------------------------------------------
void TNoteListGrid::MousDblClick( RTMousInfo mi )
{
    if (!itsLive) return;
    if( mi.style != kCellPoint )
        return;

    InvalidateCell( mi.row, -1 );
    InvalidateCell( itsSelection, -1 );
    itsSelection = mi.row;
    TRuleGrid::MousDblClick( mi );
    SendMessage( Parent->HWindow, WM_COMMAND, Attr.Id,
        MAKELONG( HWindow, CN_DBLCLICKED ));
}


//*****************************************************************************
// CellWidth
//*****************************************************************************
int  TNoteListGrid::CellWidth( PTPaint, int col )
{
   RECT r;
   switch( col ) {

       case kMarkColumn:         return 19;
       case kDateColumn:         return 129;
       case kDescriptionColumn:
                                GetClientRect( HWindow, &r );
                                return (r.right - r.left - 129 - 19 - 5 );

       default:                  return 20;
   }
}

//*****************************************************************************
// CellHeight
//*****************************************************************************
int TNoteListGrid::CellHeight( PTPaint, int )
{
    return 19;
}


//*****************************************************************************
// GetWindowClass
//*****************************************************************************
void TNoteListGrid::GetWindowClass( WNDCLASS & AWndClass )
{
    TRuleGrid::GetWindowClass( AWndClass );
    AWndClass.hbrBackground= TGDIMap::GetScreenBrush( kHbrBkView );
}

//*****************************************************************************
// GetSelection
//*****************************************************************************
int TNoteListGrid::GetSelection( void )
{
    return itsSelection;
}


