/***************************************************************************
 *                                                                         *
 *  MODULE     : DayGrid.cpp
 *               day painter for calendar grid
 *
 *
 ***************************************************************************/
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "daygrid.h"

#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __note__
#include "note.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __filter__
#include "filter.h"
#endif
#ifndef __paint__
#include "paint.h"
#endif
#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __profile__
#include "profile.h"
#endif
#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __gridutil__
#include "gridutil.h"
#endif
#ifndef  __cursor__
#include "cursor.h"
#endif
#ifndef __notemenu__
#include "notemenu.h"
#endif
#ifndef __print__
#include "print.h"
#endif
#ifndef __moushelp__
#include "moushelp.h"
#endif

static const int kMaxSlots           = 5;
static const int DayviewMinutes[]    = { 5, 6, 10, 12, 15, 20, 30, 60, 120 };
static const int kNumHeadRows        = 2;
static const int kNumHeadCols        = 1;
static const int kLongNoteRow        = 1;


static const int kBarSize            = 12;  // including whitespace
static const int kWhiteSpace         = 2;
static const int kScaleFactor        = 10;
static const int kMinDragWidth       = 10;
static const int kHalfMinDragWidth   = kMinDragWidth / 2;

static const int kPixelsPerHBar      = 3;   // including kWhiteSpace
static const int kSmallFontHeight    = 12;  // height of small font

static const int kStdRowHeight       = 19;

//***************************************************************************
//
//                                 Constructors
//
//***************************************************************************
// ------------------------------------------------------------------
// Constructor
// ------------------------------------------------------------------
TDayGrid::TDayGrid( PTWindowsObject AParent ):
    TDragGrid( AParent, kVertical, 1 )
{
    TCursor::Busy( TRUE );

    // create painters
    scrPainter = new TPaint;
    prtPainter = new TPaintPrt;

    // initialize instance variables
    itsMinutesPerCell   = DayviewMinutes[TProfile::dayviewIncrement];
    itsNumTimes         = 1440 / itsMinutesPerCell;

    // create arrays
    itsEntryTableP      = new TEntryTable( itsNumTimes, kMaxSlots );
    itsTitlesP          = new TTitle[itsNumTimes];
    itsLongNoteSetP     = new TNoteSet;

    // initialize the grid
    SetRowCnt( itsNumTimes + kNumHeadRows );
    SetRowFreeze( kNumHeadRows );
    SetColCnt( 3 );
    SetColFreeze( kNumHeadCols );

    Setup( TSysState::selTime );

    SetPrtTitle( "Day Grid" );
    SetDefaultPrintRange();
    TCursor::Busy( FALSE );
}

// ------------------------------------------------------------------
// Destructor
// ------------------------------------------------------------------
TDayGrid::~TDayGrid( void )
{
    delete scrPainter;
    delete prtPainter;
    delete itsEntryTableP;
    delete itsTitlesP;
    delete itsLongNoteSetP;
}

//***************************************************************************
//
//                                 Setup
//
//***************************************************************************

//---------------------------------------------------------------------------
// Setup
//---------------------------------------------------------------------------
void TDayGrid::Setup( RTTime startTime )
{
    // clear out tables
    itsEntryTableP->Clear();
    itsNumLongNotes = 0;
    itsLongNoteSetP->flush();
    for (int hour = 0; hour < itsNumTimes; hour++) {

        itsTitlesP[hour].noteP = NULL;
        itsTitlesP[hour].entry = -1;
    }

    // initialize time
    itsStartTime = startTime;
    itsFirstCell = startTime;
    itsLastCell  = startTime;
    itsFirstCell.SetToMidnight();
    itsLastCell.SetToMidnightTomorrow();

    // assign notes to tables
    TBoundsPositions positions;
    TBounds bounds( itsFirstCell, itsLastCell );

    int numNotes = TSysState::pSelNotes->getItemsInContainer();
    for (int i = 0; i < numNotes; i++ ) {

        RTNote note = (*TSysState::pSelNotes)[i];
        if (NOOBJECT == (RObject) note) continue;

        BOOL overlaps = bounds.NoteOverlaps( note, itsNumTimes, positions );
        do {

            if (overlaps) {

                if (note.GetStart().HasTime()) {

                    // regular note
                    AssignEntry( note, positions );
                }
                else {

                    // long note
                    AssignLongEntry( note, positions );
                }
            }

        } while (bounds.NoteOverlapsAgain( note, itsNumTimes, positions, overlaps ));
    }

    // go to the startTime
    int row, col;
    TimeToCell( startTime, row, col );
    GotoRC( row - kNumHeadRows, col - kNumHeadCols );
    ScrollChange();
    InvalidateFocus();
}

//---------------------------------------------------------------------------
// CellToTime
//---------------------------------------------------------------------------
void TDayGrid::CellToTime( int row, int col, RTTime time )
{
    int minutes;

    if (row == kLongNoteRow) {

        time    = itsFirstCell;
        time.ClearTime();
    }
    else {

        minutes = CellToEntry( row, col ) * itsMinutesPerCell;
        time    = itsFirstCell;
        time.PlusEquals( k1Hour, minutes / 60 );
        time.PlusEquals( k1Min,  minutes % 60 );
    }
}

//---------------------------------------------------------------------------
// TimeToCell
//
// Convert a time to a row number, bearing in mind that the amount
// of time represented by a row varies depending on itsMinutesPerCell
//---------------------------------------------------------------------------
void TDayGrid::TimeToCell( RTTime time, int &row, int &col )
{
    int m;

    // if the time is not this day, then return -1
    if (!itsFirstCell.IsSameDay( time )) {

        row = -1;
        col = -1;
        return;
    }

    // calculate the row based on the time
    m = time.GetHour() * 60 + time.GetMinute();

    row = kNumHeadRows + m / itsMinutesPerCell;
    col = kNoteCol;
}

//---------------------------------------------------------------------------
// GetCellTimes
//---------------------------------------------------------------------------
void TDayGrid::GetCellTimes( int row, int col, RTTime start, RTTime end )
{
    CellToTime( row, col, start );
    end = start.Plus( k1Min, itsMinutesPerCell );
}

//---------------------------------------------------------------------------
// CellToEntry
//---------------------------------------------------------------------------
int TDayGrid::CellToEntry( int row, int, BOOL canFail )
{
    if (row < kNumHeadRows) {

        if (canFail) {

            return -1;
        }
        else {

            SYSERR_STR( "CellToEntry: bad row number" );
        }
    }

    return row - kNumHeadRows;
}

//---------------------------------------------------------------------------
// AssignEntry
//
// Handle the note titles too
//---------------------------------------------------------------------------
void TDayGrid::AssignEntry( RTNote note, RTBoundsPositions positions )
{
    int    h;

    itsEntryTableP->SetEntry( &note, positions, kScaleFactor );


    //find home for note title
    h = positions.startCell;
    while ((h < itsNumTimes) && (itsTitlesP[h].noteP != NULL) ) {

        h++;
    }

    if (h < itsNumTimes) {

        itsTitlesP[ h ].noteP = &note;
        itsTitlesP[ h ].entry = positions.startCell;
    }
}

//---------------------------------------------------------------------------
// AssignLongEntry
//
// Put a note into the long note table
//---------------------------------------------------------------------------
void TDayGrid::AssignLongEntry( RTNote note, RTBoundsPositions  )
{
    if (itsNumLongNotes < kMaxLongDayNotes) {

        TLongNote &longNote = itsLongNotes[itsNumLongNotes];
        longNote.noteP      = &note;
        longNote.row        = itsNumLongNotes++;
    }

    itsLongNoteSetP->add( note );
}


//***************************************************************************
//
//                                 Painting
//
//***************************************************************************

// ------------------------------------------------------------------
// CellWidth
// ------------------------------------------------------------------
int TDayGrid::CellWidth( PTPaint pPaint, int col )
{

    switch (ColType( col )) {

        case kTimeCol:   return 82;
        case kStripeCol:
            if( pPaint->IsPrintPainter() )
                return 0;
            else
                return 12;
        case kNoteCol:

            RECT r;
            if( pPaint->IsPrintPainter() )
                {
                TPrint::GetPrtPixels( &r );
                // -6 for gridlines
                return (r.right - r.left - 6 - 82 );    //no stripe!
                }
            else
                {
                GetClientRect( HWindow, &r );
                // -6 for gridlines
                return (r.right - r.left - 6 - 82 - 12);
                }

        default:

           SYSERR_STR( "CellWidth: unexpected case in switch" );
           return 0;
    }
}

// ------------------------------------------------------------------
// CellHeight
// ------------------------------------------------------------------
int TDayGrid::CellHeight( PTPaint, int row )
{
    int longNoteHeight = max( 19, (kSmallFontHeight + 1) * itsNumLongNotes );

    switch (RowType( row )) {

        case kTitleRow:    return 36;
        case kLongNoteRow: return longNoteHeight;
        case kTimeRow:     return kStdRowHeight;
        default:

           SYSERR_STR( "CellHeight: unexpected case in switch" );
           return 0;
    }
}

//---------------------------------------------------------------------------
// GridWidth
//---------------------------------------------------------------------------
int  TDayGrid::GridWidth( PTPaint, int col )
{
    switch( ColType( col )) {

        case kTimeCol:   return 2;
        case kStripeCol: return 1;
        case kNoteCol:   return 1;
        default:

           SYSERR_STR( "GridWidth: unexpected case in switch" );
           return 0;
    }
}

//---------------------------------------------------------------------------
// GridHeight
//---------------------------------------------------------------------------
int  TDayGrid::GridHeight( PTPaint pPaint, int row )
{
    TTime time;

    switch (RowType( row )) {

        case kTitleRow:
        case kLongNoteRow:

            return 2;

        case kTimeRow:

            if( pPaint->IsPrintPainter() )
                {
                return 1;
                }
            else
                {
                CellToTime( row + 1, -1, time );
                return (time.GetMinute() == 0) ? 2 : 1;
                }

        default:

            SYSERR_STR( "GridHeight: unexpected case in switch" );
            return 1;
    }
}

// ------------------------------------------------------------------
// PaintCell
// ------------------------------------------------------------------
void TDayGrid::PaintCell( RTDrawInfo di )
{
    switch( CellType( di.row, di.col )) {

        case kTimeTitle:                                break;
        case kStripeTitle:                              break;
        case kNoteTitle:         PaintTitle( di );      break;
        case kTime:              PaintTime( di );       break;
        case kStripe:            PaintDrag( di );       break;
        case kNote:              PaintNote( di );       break;
        case kLongNoteNames:                            break;
        case kLongNoteStripe:                           break;
        case kLongNote:          PaintLongNote( di );   break;
        default:

            SYSERR_STR( "PaintCell: unexpected case in switch" );
            break;
    }
}

// ------------------------------------------------------------------
// PaintTime
// ------------------------------------------------------------------
void TDayGrid::PaintTime( RTDrawInfo di )
{
    char buf[30];
    TTime time;

    CellToTime( di.row, -1, time );
    TIntlFmt::FormatTime( time, FALSE, buf );
    WORD tStyle= DT_SINGLELINE | DT_NOPREFIX | DT_EXTERNALLEADING | DT_RIGHT | DT_VCENTER;
    di.pPainter->Text( di, buf, kNoHilite, tStyle, kBoldFont );
}

// ------------------------------------------------------------------
// PaintDrag
// ------------------------------------------------------------------
void TDayGrid::PaintDrag( RTDrawInfo di )
{
    di.pPainter->FillRectangle( di, di.cellRect, kHbrMultiCell );

    if( IsCellInRange( di.row, di.col ) )
        di.pPainter->Hilite( di, kInvertHilite );
}

// ------------------------------------------------------------------
// PaintTitle
// ------------------------------------------------------------------
void TDayGrid::PaintTitle( RTDrawInfo di )
{
    char buf[30];

    TGridUtil::PaintBackground( di, itsFirstCell, TRUE, FALSE );

    TIntlFmt::FormatDate( itsFirstCell, kLongDate, buf );
    WORD tStyle= DT_SINGLELINE | DT_NOPREFIX | DT_EXTERNALLEADING | DT_CENTER | DT_VCENTER;
    di.pPainter->Text( di, buf, kNoHilite, tStyle, kBoldFont );
}

//---------------------------------------------------------------------------
// PaintNote
//---------------------------------------------------------------------------
void TDayGrid::PaintNote( RTDrawInfo di )
{
    RECT   r;
    TTime  start,end;
    GetCellTimes( di.row, di.col, start, end );
    end.PlusEquals( k1Min, -1 );
    TGridUtil::PaintBackground( di, end, FALSE );

    RECT *pRect= &di.cellRect;

    // calculate the rectangle for the first bar
    r         = *pRect;
    r.right   = pRect->left + kBarSize - kWhiteSpace;

    PTNote  pNote;
    long    startPos, stopPos;

    int entry = CellToEntry( di.row, di.col );
    for (int slot = 0; slot < kMaxSlots - 1; slot++) {

        // get the entry for this slot and continue if there is none
        if (itsEntryTableP->GetEntry( entry, slot, &pNote, startPos, stopPos )) {

            if (pNote->GetStyle() == kMilestoneStyle) {

                PaintMilestone( di, r, pNote->GetColor() );
            }
            else if (pNote->GetStyle() == kTaskStyle) {

                PaintTask( di, r, pNote->GetColor(), (int) startPos, (int) stopPos );
            }
        }

        // push the rectangle across
        r.left  += kBarSize;
        r.right += kBarSize;
    }

    if (itsEntryTableP->GetEntry( entry, kMaxSlots - 1,
        &pNote, startPos, stopPos )) {

        di.pPainter->Text( di, "+", kNoHilite, DT_LEFT,
            kBoldFont, &r );
    }




    // print note title
    PaintNoteTitle( di, *pRect, entry );
}

//---------------------------------------------------------------------------
// PaintMilestone
//---------------------------------------------------------------------------
void TDayGrid::PaintMilestone( RTDrawInfo di, RECT &r, RTColor color )
{
    const kDiamondSize = kBarSize - kWhiteSpace;
    RECT  rDiamond, rTopBar, rBottomBar;

    // calculate diamond rectangle
    rDiamond          = r;
    rDiamond.top      = r.top + (r.bottom - r.top - kDiamondSize) / 2;
    rDiamond.bottom   = rDiamond.top + kDiamondSize;

    // calculate top rectangle
    rTopBar           = r;
    rTopBar.top       = r.top;
    rTopBar.bottom    = rDiamond.top - 1;
    rTopBar.left     += (rTopBar.right - rTopBar.left) / 3;
    rTopBar.right    -= (rTopBar.right - rTopBar.left) / 3;

    // calculate bottom rectangle
    rBottomBar        = rTopBar;
    rBottomBar.top    = rDiamond.bottom + 1;
    rBottomBar.bottom = r.bottom;

    // draw them all
    di.pPainter->FillRectangle( di, rTopBar,    color );
    di.pPainter->FillRectangle( di, rBottomBar, color );
    di.pPainter->FillDiamond(   di, rDiamond,   color );
}

//---------------------------------------------------------------------------
// PaintTask
//---------------------------------------------------------------------------
void TDayGrid::PaintTask( RTDrawInfo di, RECT &r, RTColor color, int startPos, int stopPos )
{
    int height   = r.bottom - r.top;

    RECT taskR;
    taskR.left   = r.left;
    taskR.right  = r.right;
    taskR.top    = r.top + height * (int) startPos / kScaleFactor;
    taskR.bottom = r.top + height * (int) stopPos  / kScaleFactor;
    di.pPainter->FillRectangle( di, taskR, color );
}

//---------------------------------------------------------------------------
// PaintNoteTitle
//---------------------------------------------------------------------------
void TDayGrid::PaintNoteTitle( RTDrawInfo di, RECT &r, int entry )
{
    PTNote pNote = itsTitlesP[entry].noteP;
    if (pNote == NULL ) {

        return;
    }

    // get the title
    TSafStr title;
    pNote->Title( title );


    // make the tag
    char tag[150];
    TBounds bounds( itsFirstCell, itsLastCell );
    if (bounds.NoteOverlaps( *pNote )) {

        if( pNote->GetStart().HasTime() )  {

            char timeBuf[30];
            TIntlFmt::FormatTime( pNote->GetStart(), FALSE, timeBuf );
            wsprintf( tag, "%s %s", timeBuf, title.Addr() );
        }
        else {

            wsprintf( tag, "All day -> %s", title.Addr() );
        }
    }
    else {

        if (pNote->GetFreq() != NULL) {

            wsprintf( tag, "Repeated! -> %s", title.Addr() );
        }
        else {

            wsprintf( tag, "Cont'd! -> %s", title.Addr() );
        }
    }


    // paint the tag
    RECT titleR  = r;
    titleR.left += 5 + kBarSize * kMaxSlots;
    TFont font   = (pNote->IsDone()) ? kStrikeFont : kNormalFont;
    di.pPainter->Text( di, tag, kNoHilite, kSingleLine, font, &titleR );

    // paint the bar connecting the title
    int slot  = itsEntryTableP->GetSlot( pNote, entry );
    int dogLeg = 0;
    while ((entry > 0) && (slot == -1)) {

        entry--;
        dogLeg++;
        slot  = itsEntryTableP->GetSlot( pNote, entry );
    }
    if (entry > 0) {

        int left   = r.left + 5 + kBarSize * slot;
        int right  = r.left     + kBarSize * kMaxSlots;
        int bottom = (r.top + r.bottom) / 2;
        RECT littleR;

        SetRect( &littleR, right - 2, bottom, right + 2, bottom + 4 );
        di.pPainter->FillRectangle( di, littleR, pNote->GetColor() );

        int top = bottom - dogLeg * (r.bottom - r.top);
        MoveTo( di.hdc, left, top + 2 );
        LineTo( di.hdc, right, bottom + 2);
    }

    // draw ink if there was no title
    if( (title.Len() == 0) && pNote->GetInk().HasInk() ) {

        //get size of drawn string (start time)
        int width= di.pPainter->CalcWidth( tag, kNormalFont );

        titleR.left += width + width/5;
        pNote->GetInk().Show( di.hdc, &titleR );
    }
}

//---------------------------------------------------------------------------
// PaintLongNames
//---------------------------------------------------------------------------
void TDayGrid::PaintLongNames( RTDrawInfo  )
{
}

//---------------------------------------------------------------------------
// PaintLongNote
//---------------------------------------------------------------------------
void TDayGrid::PaintLongNote( RTDrawInfo di )
{

    for (int i = 0; i < itsNumLongNotes; i++) {

        TSafStr title;
        PTNote  noteP = itsLongNotes[i].noteP;

        noteP->Title( title );

        // draw the text
        RECT r   = di.cellRect;
        r.top    = di.cellRect.top + kSmallFontHeight * i;
        r.bottom = r.top + kSmallFontHeight;

        TFont font= (noteP->IsDone()) ? kSmallStrikeFont : kSmallFont;
        di.pPainter->Text( di, (char*) title.Addr(), kNoHilite,
            kSingleLine | DT_CENTER, font, &r );


        // calculate the width of the text
        // so that the rectangles don't overwrite it
        // (so it prints nicely)
        int halfTextWidth = di.pPainter->CalcWidth(
            (char*) title.Addr(), font ) / 2;

        r.top    = di.cellRect.top + kSmallFontHeight * i + 5;
        r.bottom = r.top + kPixelsPerHBar;


        // draw the rect to the left of the text
        int midX = (di.cellRect.left + di.cellRect.right) / 2;
        r.left   = di.cellRect.left;
        r.right  = midX - halfTextWidth - 5;

        if (noteP->HollowOnDate( itsStartTime ))
            {
            di.pPainter->FrameRectangle( di, r, noteP->GetColor() );
            }
        else
            {
            di.pPainter->FillRectangle( di, r, noteP->GetColor() );
            }

        // draw the rect to the right of the text
        r.right = di.cellRect.right;
        r.left  = midX + halfTextWidth + 5;

        if (noteP->HollowOnDate( itsStartTime ))
            {
            di.pPainter->FrameRectangle( di, r, noteP->GetColor() );
            }
        else
            {
            di.pPainter->FillRectangle( di, r, noteP->GetColor() );
            }
    }
}

//***************************************************************************
//
//                                 Mousing
//
//***************************************************************************

//---------------------------------------------------------------------------
// MousOver
//---------------------------------------------------------------------------
void TDayGrid::MousOver( RTMousInfo mi )
{
    switch( CellType( mi.row, mi.col )) {

        case kStripe:

            TCursor::Set( kNoteBarCursor );
            break;

        case kLongNote:

            int slot  = (mi.xy.y - mi.cellRect.top) / kSmallFontHeight;
            if ((0 <= slot) && (slot < itsNumLongNotes)) {

                PTNote noteP = itsLongNotes[slot].noteP;
                if( noteP == NULL ) {

                   TCursor::Set( kArrowCursor );
                }
                else {

                   TCursor::Set( kNoteSelCursor );
                }
            }
            else {

                TCursor::Set( kArrowCursor );
            }
            break;

        case kNote:

            TCursorStyle style;
            if ( CursorForDrag( style, mi ) ) {

                TCursor::Set( style );
            }
            else if (itsTitlesP[CellToEntry( mi.row, mi.col )].noteP) {

                TCursor::Set( kNoteSelCursor );
            }
            else {

                TCursor::Set( kNewNoteCursor );
            }
            break;

        default:

            TCursor::Set( kArrowCursor );
            break;
    }
}

//---------------------------------------------------------------------------
// MousDblClick
//
// If the cell has a note title already, then open that note.
//---------------------------------------------------------------------------
void TDayGrid::MousDblClick( RTMousInfo mi )
{
    switch( CellType( mi.row, mi.col )) {

        case kNote:          ShowNote( mi );               break;
        case kLongNote:      ShowLongNote( mi );           break;
    }
}

//---------------------------------------------------------------------------
// MousRtClick
//
// If the cell has a note title, then show local menu
//---------------------------------------------------------------------------
void TDayGrid::MousRtClick( RTMousInfo mi )
{
    PTNote noteP;

    switch( CellType( mi.row, mi.col )) {

        case kNote:

            noteP = itsTitlesP[CellToEntry( mi.row, mi.col )].noteP;
            TCursorStyle style;
            if ( CursorForDrag( style, mi ) ) {

                TMousHelp::MousHelp( this,
                   "Click left button and drag mouse to move this note to a different time." );
            }
            else if (noteP) {

                TNoteMenu::PopUp( this, noteP, mi.xy.x, mi.xy.y );
                InvalidateCell( -1, mi.col );
            }
            else {

                TMousHelp::MousHelp( this,
                   "Double-click left button to create a new note starting at this time." );
            }
            break;

        case kLongNote:

            int slot  = (mi.xy.y - mi.cellRect.top) / kSmallFontHeight;
            if ((0 <= slot) && (slot < itsNumLongNotes)) {

                noteP = itsLongNotes[slot].noteP;
                if( noteP != NULL ) {

                    TNoteMenu::PopUp( this, noteP, mi.xy.x, mi.xy.y );
                    InvalidateCell( mi.row, mi.col );
                }
            }
            break;

        case kStripe:

            TMousHelp::MousHelp( this,
                "Click left button and drag mouse along shaded stripe to create a new note with start and stop times." );
            break;

        default:
            break;
    }
}

//---------------------------------------------------------------------------
// ShowNote
//---------------------------------------------------------------------------
void TDayGrid::ShowNote( RTMousInfo mi  )
{
    PTNote pNote;
    pNote = itsTitlesP[CellToEntry( mi.row, mi.col )].noteP;

    if (pNote == NULL) {

        pNote = TSysState::pAllNotes->CreateNote( this );
        if (pNote != NULL) {

            TTime start;
            CellToTime( mi.row, -1, start );
            pNote->SetStartAndClearDuration( start );
        }
    }

    if( pNote != NULL ) {    //still NULL if CreateNote was cancelled

        TGridUtil::ShowNote( pNote );
    }
}

//---------------------------------------------------------------------------
// ShowLongNote
//---------------------------------------------------------------------------
void TDayGrid::ShowLongNote( RTMousInfo mi  )
{
    int slot  = (mi.xy.y - mi.cellRect.top) / kSmallFontHeight;
    if ((0 <= slot) && (slot < itsNumLongNotes)) {

        PTNote pNote = itsLongNotes[slot].noteP;
        if( pNote != NULL ) {

            TGridUtil::ShowNote( pNote );
        }
    }
}

//---------------------------------------------------------------------------
// MousRangeInit
//
// In column 1 we allow ranges...
//---------------------------------------------------------------------------
BOOL TDayGrid::MousRangeInit( RTMousInfo mi  )
{
    TDragGrid::MousRangeInit( mi );

    if( mi.bKeyAction ) return FALSE;

    if (CellType( mi.row, mi.col ) == kStripe) {

        OpenDragWindow( FALSE );
        return TRUE;
    }
    else {

        return FALSE;
    }
}

//---------------------------------------------------------------------------
// MousRangeOver
//---------------------------------------------------------------------------
void TDayGrid::MousRangeOver( RTMousInfo mi )
{
    TDragGrid::MousRangeOver( mi );

    // set cursor
    MousOver( mi );

    // autoscroll
    if (XYAbove( mi ) || XYInFrozenRow( mi )) {

        if (vScrollPos > 0) {

            Scroll( 1, 0 );

            int height, width;
            GetRangeExtent( height, width );        //include scrolled cell in range
            SetRangeExtent( height - 1, width );

            UpdateWindow( HWindow );
        }
    }
    else if( XYBelow( mi ) ) {

        if (vScrollPos < vScrollMax) {

            Scroll( -1, 0 );

            int height, width;
            GetRangeExtent( height, width );        //include scrolled cell in range
            SetRangeExtent( height + 1, width );

            UpdateWindow( HWindow );
        }
    }


    // show times
    int top, left, bottom, right;
    TTime start, end;
    GetRangeRect( top, left, bottom, right );
    CellToTime( top, left, start );
    CellToTime( bottom + 1, right, end );
    itsNewTimes.SetStartAndEnd( start, end,
        (TTimeUnit) (TProfile::dayviewIncrement + 1));
    itsDragWinP->Set( "", itsNewTimes );
}


//---------------------------------------------------------------------------
// MousRangeRelease
//---------------------------------------------------------------------------
void TDayGrid::MousRangeRelease( RTMousInfo mi )
{
    CloseDragWindow();

    // set range
    TDragGrid::MousRangeRelease( mi );

    // create a new note
    int top, left, bottom, right;
    if (GetRangeRect( top, left, bottom, right ) &&
       (CellType( mi.row, mi.col ) == kStripe)) {

        PTNote pNote= TSysState::pAllNotes->CreateNote( this );
        if( pNote != NULL ) {

            TTime start, end;
            CellToTime( top, -1, start );
            CellToTime( bottom + 1, -1, end );

            pNote->SetStartAndEnd( start, end,
                min( k1Hour, (TTimeUnit) (TProfile::dayviewIncrement + 1)));

            TGridUtil::ShowNote( pNote );
        }
    }

    //clear selection
    SetRangeAnchor( -1, -1 );
}


//***************************************************************************
//
//                                 CellType
//
//***************************************************************************

//---------------------------------------------------------------------------
// CellType
//
// Return the type of the cell; handles the case where row or col is -1,
// as it would be if XYWithin were false
//---------------------------------------------------------------------------
TDayGrid::TCellType TDayGrid::CellType( int row, int col )
{
    // titles
    if (row == -1) {

        return kNotACell;
    }
    if (row == 0) {

        switch( col ) {

            case 0:   return kTimeTitle;
            case 1:   return kStripeTitle;
            case 2:   return kNoteTitle;
            default:  return kNotACell;
        }
    }
    // long notes
    else if (row == 1) {

        switch( col ) {

            case 0:   return kLongNoteNames;
            case 1:   return kLongNoteStripe;
            case 2:   return kLongNote;
            default:  return kNotACell;
        }
    }
    else {

        switch( col ) {

            case 0:   return kTime;
            case 1:   return kStripe;
            case 2:   return kNote;
            default:  return kNotACell;
        }
    }
};



//---------------------------------------------------------------------------
// ColType
//
// Return the type of the col; handles the case where col is -1, as it would
// be if XYWithin were false
//---------------------------------------------------------------------------
TDayGrid::TColType TDayGrid::ColType( int col )
{
    switch( col ) {

        case 0:    return kTimeCol;
        case 1:    return kStripeCol;
        case 2:    return kNoteCol;
        default:   return kNotACol;
    }
}

//---------------------------------------------------------------------------
// RowType
//
// Return the type of the row; handles the case where row is -1, as it would
// be if XYWithin were false
//---------------------------------------------------------------------------
TDayGrid::TRowType TDayGrid::RowType( int row )
{
    switch( row ) {

        case -1:   return kNotARow;
        case 0:    return kTitleRow;
        case 1:    return kLongNoteRow;
        default:   return kTimeRow;
    }
}

//***************************************************************************
//
//                                 Listening
//
//***************************************************************************

//---------------------------------------------------------------------------
// ResetNotes
//
// New notes: setup again
//---------------------------------------------------------------------------
void TDayGrid::ResetNotes( void )
{
    Setup( TSysState::selTime );
    InvalidateCell( -1, -1 );
}

//---------------------------------------------------------------------------
// ResetTime
//---------------------------------------------------------------------------
void TDayGrid::ResetTime( void )
{
    if (TSysState::selTime != itsStartTime) {

        Setup( TSysState::selTime );
        InvalidateCell( -1, -1 );
    }

    // go to the startTime
    else {

        int row, col;
        TimeToCell( itsStartTime, row, col );
        InvalidateCell( -1, -1 );
        GotoRC( row - kNumHeadRows, col - kNumHeadCols);
    }
}

//---------------------------------------------------------------------------
// Listen
//---------------------------------------------------------------------------
void TDayGrid::Listen( TSnoopMsg aMsg, RObject )
{
switch( aMsg )
    {
    case kNoteSetChg:
        ResetNotes();
        break;

    case kTimeChg:
        ResetTime();
        break;

    default:
        break;
    }
}

//***************************************************************************
//
//                                 Printing
//
//***************************************************************************


//---------------------------------------------------------------------------
// SetDefaultPrintRange
//---------------------------------------------------------------------------
void TDayGrid::SetDefaultPrintRange( void )
{
int row, col;

TTime early= itsFirstCell;
TTime late= itsFirstCell;

early.SetHour( 8 );
late.SetHour( 20 );

TimeToCell( early, row, col );
rFirstLiquid= row;

TimeToCell( late, row, col );
rLastLiquid= row;

cFirstLiquid= frozenCols;
cLastLiquid= colCnt - kNumHeadCols;
}

//---------------------------------------------------------------------------
// modify index by n, the convert into meaningful representation, such as
// the time and date of that column
// return FALSE if index would be out of range
//---------------------------------------------------------------------------
BOOL TDayGrid::VChgCellStartIndex( int n, char *buf, int /*size*/ )
{
BOOL rc;
TTime time;

rc= ((rFirstLiquid + n) >= frozenRows &&
     (rFirstLiquid + n) < rowCnt &&
     (rFirstLiquid + n ) < rLastLiquid );

if( rc )
    rFirstLiquid+= n;

CellToTime( rFirstLiquid, -1, time );
TIntlFmt::FormatTime( time, FALSE, buf );

return rc;
}

//---------------------------------------------------------------------------
// modify index by n, the convert into meaningful representation, such as
// the time and date of that column
// return FALSE if index would be out of range
//---------------------------------------------------------------------------
BOOL TDayGrid::VChgCellEndIndex( int n, char *buf, int /*size*/ )
{
BOOL rc;
TTime time;

rc= ((rLastLiquid + n) >= frozenRows &&
     (rLastLiquid + n) < rowCnt &&
     (rLastLiquid + n) > rFirstLiquid );

if( rc )
    rLastLiquid+= n;

CellToTime( rLastLiquid, -1, time );
TIntlFmt::FormatTime( time, FALSE, buf );

return rc;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
//          Dragging
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

//*****************************************************************************
// ValidDragCell
//
// Is this a cell that allows dragging?
//*****************************************************************************
BOOL TDayGrid::ValidDragCell( TDragMode , int row, int col )
{
    return CellType( row, col ) == kNote;
}

//---------------------------------------------------------------------------
// ScrollStart
//
// For the daygrid scrolling is only allowed up and down
//---------------------------------------------------------------------------
void TDayGrid::ScrollStart( RTMousInfo , int deltaRow, int deltaCol )
{
    if (deltaRow > 0) {

        itsNewTimes.SetStartToStartOfDayAndKeepEnd();

        if (vScrollPos > 0) {

            // adjust the marquee
            itsMarqueeRect.bottom += kStdRowHeight;
            itsMarqueeRect.bottom = min( itsMarqueeRect.bottom, winHeight );

            Scroll( deltaRow, deltaCol );
            UpdateWindow( HWindow );
        }
    }
    else if (deltaRow < 0) {

        itsNewTimes.SetStartToJustBeforeEnd( k1Min, itsMinutesPerCell );

        // scroll down only as far as the end time
        int row, col;
        TimeToCell( itsNewTimes.GetEnd(), row, col );
        if (!IsCellVisible( row + 1, col )) {

            Scroll( deltaRow, deltaCol );
            UpdateWindow( HWindow );
        }

        // adjust the marquee
        POINT p;
        if (TimeToPoint( itsNewTimes.GetEnd(), p ) &&
            (itsMarqueeRect.bottom > p.y)) {

            itsMarqueeRect.bottom = p.y;
        }
    }
}

//---------------------------------------------------------------------------
// ScrollEnd
//
// For the daygrid scrolling is only allowed up and down
//---------------------------------------------------------------------------
void TDayGrid::ScrollEnd( RTMousInfo , int deltaRow, int deltaCol )
{
    if (deltaRow < 0) {

        itsNewTimes.SetEndToEndOfDayAndKeepStart();

        if (vScrollPos < vScrollMax) {

            // adjust the marquee
            itsMarqueeRect.top -= kStdRowHeight;
            itsMarqueeRect.top = max( itsMarqueeRect.top, FrozenHeight( scrPainter ));

            Scroll( deltaRow, deltaCol );
            UpdateWindow( HWindow );
        }
    }
    else if (deltaRow > 0) {

        itsNewTimes.SetEndToJustAfterStart( k1Min, itsMinutesPerCell );

        // scroll up only as far as the start time
        int row, col;
        TimeToCell( itsNewTimes.GetStart(), row, col );
        if (!IsCellVisible( row, col )) {

            Scroll( deltaRow, deltaCol );
            UpdateWindow( HWindow );
        }


        // adjust the marquee
        POINT p;
        if (TimeToPoint( itsNewTimes.GetStart(), p ) &&
            (itsMarqueeRect.top < p.y)) {

            itsMarqueeRect.top = p.y;
        }
    }
}

//---------------------------------------------------------------------------
// ScrollBoth
//
// scroll and adjust the start and end times to match the new scroll pos
//---------------------------------------------------------------------------
void TDayGrid::ScrollBoth( RTMousInfo , int deltaRow, int deltaCol )
{
    // force the start to midnight if we're scrolling up,
    // or the end to midnight if we're scrolling down
    if (deltaRow > 0) {

        itsNewTimes.SetStartToStartOfDayAndKeepDuration();
    }
    else if (deltaRow < 0) {

        itsNewTimes.SetEndToEndOfDayAndKeepDuration();
    }

    // scroll
    Scroll( deltaRow, deltaCol );
    UpdateWindow( HWindow );
}

//*****************************************************************************
// SetStart
//
// We're dragging the start time
//*****************************************************************************
void TDayGrid::SetStart( RTTime start )
{
    itsNewTimes.SetStartAndKeepEnd( start, k1Min, itsMinutesPerCell );
    itsNewTimes.ConstrainStartToDayAndEnd( k1Min, itsMinutesPerCell );
}

//*****************************************************************************
// SetEnd
//
// We're dragging the end time
//*****************************************************************************
void TDayGrid::SetEnd( RTTime end )
{
    itsNewTimes.SetEndAndKeepStart( end, k1Min, itsMinutesPerCell );
    itsNewTimes.ConstrainEndToDayAndStart( k1Min, itsMinutesPerCell );
}

//*****************************************************************************
// SetBoth
//
// Make sure both the start and the end are within the day
//*****************************************************************************
void TDayGrid::SetBoth( RTMousInfo, RTTime start )
{
    itsNewTimes.SetStartAndKeepDuration( start );
    itsNewTimes.ConstrainToDay( itsFirstCell );
}

//---------------------------------------------------------------------------
// ConstrainMarqueeRect
//
// keep the marquee out of the frozen rows
//---------------------------------------------------------------------------
void TDayGrid::ConstrainMarqueeRect( TDragMode , RECT &r )
{
    r.top  = AT_LEAST( r.top,  FrozenHeight( scrPainter ));
}

//*****************************************************************************
// Release
//
// to be done when dragging is finished
//*****************************************************************************
void TDayGrid::Release( void )
{
    ResetNotes();
}

//*****************************************************************************
// PointToTime
//
// Convert a point WITHIN THE GRID OF LIVE CELLS to a time
//
// NB currently assumes we're using SCREEN grid, not print grid
//*****************************************************************************
void TDayGrid::PointToTime( RTMousInfo mi, RTTime time )
{
    if (CellType( mi.row, mi.col ) != kNote) return;

    // get base time of cell
    if (mi.style == kRowPoint) {

        // basegrid thinks that a gridline is associated with the row
        // above it, and we think that it's associated
        // with the row below it, so add 1 row to get the
        // proper result for the grid line
        CellToTime( mi.row + 1, mi.col, time );
    }
    else {

        CellToTime( mi.row, mi.col, time );
    }

    double y          = mi.xy.y;
    double fraction   = (y- mi.cellRect.top) / (mi.cellRect.bottom - mi.cellRect.top);
    int    extraMins  = fraction * itsMinutesPerCell;
    extraMins = extraMins * kMinutesPerTimeUnit[itsPrecision] / kMinutesPerTimeUnit[itsPrecision];

    time.PlusEquals( k1Min, extraMins );
}

//*****************************************************************************
// PointToNote
//
// Convert a point to a note taking into account the entry and slot;
// Allows some slop for grow cursors beyond the ends of the note
//*****************************************************************************
RTNote TDayGrid::PointToNote( RTMousInfo mi )
{
    int    entry, slot;
    PTNote noteAtP, noteAboveP, noteBelowP;
    RECT   r1, r2, r3, r4;
    POINT  p = mi.xy;

    slot  = (p.x - mi.cellRect.left) / kBarSize;
    if (slot >= kMaxSlots) {

        return (RTNote) NOOBJECT;
    }

    entry = CellToEntry( mi.row, mi.col );
    itsEntryTableP->GetNotesNear( entry, slot, &noteAtP, &noteAboveP, &noteBelowP );

    if (noteAtP != NULL) {

        GetNoteRects( *noteAtP, mi, r1, r2, r3, r4 );
        if (PtInRect( &r1, p ) || PtInRect( &r2, p ) || PtInRect( &r3, p )) {

            return *noteAtP;
        }
    }

    if (noteAboveP != NULL) {

        GetNoteRects( *noteAboveP, mi, r1, r2, r3, r4 );
        if (PtInRect( &r1, p ) || PtInRect( &r2, p ) || PtInRect( &r3, p )) {

            return *noteAboveP;
        }
    }

    if (noteBelowP != NULL) {

        GetNoteRects( *noteBelowP, mi, r1, r2, r3, r4 );
        if (PtInRect( &r1, p ) || PtInRect( &r2, p ) || PtInRect( &r3, p )) {

            return *noteBelowP;
        }
    }

    return (RTNote) NOOBJECT;
}

//---------------------------------------------------------------------------
// TimeToPoint
//---------------------------------------------------------------------------
BOOL TDayGrid::TimeToPoint( RTTime time, POINT &p )
{
    RECT frameR, cellR;
    int  row, col;

    // if the cell isn't on screen, then there is no point
    TimeToCell( time, row, col );
    if (!CellToRect( row, col, frameR, cellR )) {

        return FALSE;
    }


    TTime start, end;
    GetCellTimes( row, col, start, end );
    TDuration deltaDuration( start, time );
    TDuration cellDuration( start, end );

    long deltaMinutes = deltaDuration.GetNumUnits( k1Min );
    long cellMinutes  = cellDuration.GetNumUnits( k1Min );

    p.x = cellR.left;
    p.y = (int) (cellR.top + kStdRowHeight * deltaMinutes / cellMinutes);

    return TRUE;
}


//*****************************************************************************
// GetNoteRects
//
// for Milestones, growStartR and growEndR will be empty
// note: dragR no longer includes growStartR and growEndR
//*****************************************************************************
void TDayGrid::GetNoteRects( RTNote note, RTMousInfo mi,
    RECT &growStartR, RECT &dragR, RECT &growEndR, RECT &marqueeR )
{
    TBounds bounds( itsFirstCell, itsLastCell );
    TBoundsPositions positions;

    SetRect( &growStartR,  0, 0, -1, -1 );
    SetRect( &dragR,       0, 0, -1, -1 );
    SetRect( &growEndR,    0, 0, -1, -1 );

    switch (note.GetStyle()) {

        case kItemStyle:
        case kFloatStyle:

            return;

        case kMilestoneStyle:
        case kTaskStyle:

            if (!bounds.NoteOverlaps( note, itsNumTimes, positions )) {

                return;
            }
            break;
    }

    // get the slot
    int slot  = (mi.xy.x - mi.cellRect.left) / kBarSize;
    if (slot >= kMaxSlots) {

        return;
    }

    double vStart;
    double vEnd;

    if ( note.GetStyle() == kTaskStyle) {

        vStart  = positions.startCell + positions.startPos;
        vEnd    = positions.endCell   + positions.endPos;
    }
    else {

        // drag rectangle for a milestone takes up the whole height
        vStart  = positions.startCell;
        vEnd    = positions.startCell + 1;
    }

    // get logical positions for rect
    int cellHeight = kStdRowHeight + 1; /* for grid line */;
    int top        = vStart * cellHeight;
    int bottom     = vEnd   * cellHeight;
    int left       = mi.cellRect.left + slot * kBarSize;
    int right      = left + kBarSize;

    // modify logical positions to allow for the fact that the heights of
    // the frozen columns must be added since they are not part of the
    // logical height, but the heights of the columns
    // which are scrolled off to the top must be subtracted, since they
    // don't actually appear on the screen but ARE part of the logical height
    int headHeight = FrozenHeight( scrPainter );
    top           += headHeight - vScrollPos * cellHeight;
    bottom        += headHeight - vScrollPos * cellHeight;


    // create the rectangles
    if (note.GetStyle() == kTaskStyle) {

        SetRect( &dragR,       left, top, right, bottom );
        SetRect( &growStartR,  left, top - kMinDragWidth, right, top );
        SetRect( &growEndR,    left, bottom, right, bottom + kMinDragWidth + 1 );
        marqueeR = dragR;
    }
    else {

        SetRect( &dragR, left, top - kHalfMinDragWidth, right, bottom + kHalfMinDragWidth );
        marqueeR = dragR;
    }

    // notes with frequencies don't get grow rects
    if (note.GetFreq() != NULL) {

        SetRect( &growStartR,  0, 0, -1, -1 );
        SetRect( &growEndR,    0, 0, -1, -1 );
    }

}



//---------------------------------------------------------------------------
// WMVScroll
//
// Keeps track of the selected time
//---------------------------------------------------------------------------
void TDayGrid::WMVScroll( RTMessage Msg )
{
    // send the corrected version off to basegrid
    TBaseGrid::WMVScroll( Msg );


    // set global time based on current vScrollPos
    // this insures that weekgrid, or subsequent daygrid, open
    // to same scrollPos
    CellToTime( vScrollPos + kNumHeadRows, kNumHeadCols, TSysState::selTime );
    Notify( kTimeChg, NOOBJECT );

}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////// FOCUS /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
//set focus to first time in day
//*****************************************************************************
void TDayGrid::GetDefaultFocusCell( int &row, int &col )
{
row= vScrollPos + frozenRows;
col= 2;
}

//*****************************************************************************

//*****************************************************************************
void TDayGrid::MousKey( TMousKey key )
{
TMousGrid::MousKey( key );
}

//---------------------------------------------------------------------------
// IsCellSelectable
//
//---------------------------------------------------------------------------
BOOL TDayGrid::IsCellSelectable( int row, int col )
{
    TCellType cellType = CellType( row, col );

    return (cellType == kStripe) || (cellType == kNote);
}

//---------------------------------------------------------------------------
// IsAllowedToDrag
//
//---------------------------------------------------------------------------
BOOL TDayGrid::IsAllowedToDrag( RTNote note, char *message )
{
    BOOL hasTime      = note.GetStart().HasTime();
    BOOL moreThan1Day = !note.GetStart().IsSameDay( note.GetEnd());

    if (hasTime && moreThan1Day) {

        strcpy( message, "Note too long to drag" );
        return FALSE;
    }
    else if (note.GetFreq() != NULL) {

        strcpy( message, "Can't drag notes with frequencies" );
        return FALSE;
    }
    else {

        message[0] = '\0';
        return TRUE;
    }
}
