 /***************************************************************************
 *                                                                         *
 *  MODULE     : weekgrid.cpp
 *
 ***************************************************************************/
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "weekgrid.h"

#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __frame__
#include "frame.h"
#endif
#ifndef __error__
#include "error.h"
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
#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __notelisd__
#include "notelisd.h"
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

// row information
static const int kNumHeadRows     = 3;
static const int kStripeRowHeight = 5;
static const int kStdRowHeight    = 19;
static const int kLongNoteRow     = 2;


// column information
static const int kNumHeadCols     = 1;
static const int kNumVisibleCols  = 5;
static const int kNumPrintCols    = 7;
static const int kFirstDayColumn  = 2;
static const int kStripeColWidth  = 5;

static const int kMaxSlots        = 3;
static const int kPixelsPerHBar   = 4;
static const int kPixelsPerVBar   = 9;   // including kWhiteSpace
static const int kWhiteSpace      = 1;   // space between bars
static const int kSmallFontHeight = 12;  // height of small font
                                         // FIXME: get the height right

const int WeekviewMinutes[] = { 5, 6, 10, 12, 15, 20, 30, 60, 120 };
const int NumberOfDays[]    = { 24, 30, 50, 60, 80, 100, 150, 200, 200 };

static const long kMinutesPerDay  = 1440;
static const int kScaleFactor     = 8;

static const int kMinDragWidth   = 10;
static const int kHalfMinDragWidth = kMinDragWidth / 2;

//---------------------------------------------------------------------------
//
//                                 Constructors
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Cosntructor
//---------------------------------------------------------------------------
TWeekGrid::TWeekGrid( PTWindowsObject AParent ):
    TDragGrid( AParent, kBoth, 2 )
{
    TCursor::Busy( TRUE );

    // create painters
    scrPainter = new TPaint;
    prtPainter = new TPaintPrt;

    // initialize instance variables
    itsMinutesPerCell    = WeekviewMinutes[TProfile::dayviewIncrement];
    itsNumDays           = NumberOfDays[TProfile::dayviewIncrement];
    itsLiveRows          = 1440 / itsMinutesPerCell;
    itsNumTimes          = itsNumDays * itsLiveRows;
    itsDraggingLongNotes = FALSE;

    // create tables
    itsEntryTableP       = new TEntryTable( itsNumTimes, kMaxSlots );
    itsTitlesP           = new PTNote[itsNumTimes];
    itsLongNoteTableP    = new TXArray( 10, 10, TXArray::kNoSort );

    // initialize grid
    SetRowCnt(    kNumHeadRows + itsLiveRows );
    SetRowFreeze( kNumHeadRows );
    SetColCnt(    kNumHeadCols + 2 * itsNumDays );  // extra col for dragging
    SetColFreeze( kNumHeadCols );

    Setup( TSysState::selTime );

    SetPrtTitle( "Week Grid" );

    TCursor::Busy( FALSE );
}

//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------
TWeekGrid::~TWeekGrid( void )
{
    delete scrPainter;
    delete prtPainter;
    delete itsEntryTableP;
    delete itsTitlesP;

    itsLongNoteTableP->Flush( TXArray::kDelete );
    delete itsLongNoteTableP;
}

//---------------------------------------------------------------------------
//
//                                 Setup
//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Setup
//---------------------------------------------------------------------------
void TWeekGrid::Setup( RTTime startTime )
{
    // clear out tables
    itsEntryTableP->Clear();
    itsNumLongNotes = 0;
    itsLongNoteTableP->flush( TXArray::kDelete );
    for (int hour = 0; hour < itsNumTimes; hour++) {

        itsTitlesP[hour] = NULL;
    }

    // set up times, putting startTime in the middle
    itsStartTime = startTime;
    itsFirstCell = startTime.Plus( SCALE_DAY, -itsNumDays / 2 );
    itsFirstCell.SetToMidnight();
    itsLastCell  = itsFirstCell.Plus( SCALE_DAY, itsNumDays );


    // find all notes that fall within the bounds
    TBoundsPositions positions;
    TBounds bounds( itsFirstCell, itsLastCell );
    int numNotes = (TSysState::pSelNotes)->getItemsInContainer();
    for (int i = 0; i < numNotes; i++ ) {

        RTNote note = (*TSysState::pSelNotes)[i];
        if (NOOBJECT == (RObject) note) continue;

        // regular note
        TTime start( note.GetStart());
        TTime end( note.GetEnd());
        if (start.HasTime() && start.IsSameDay( end )) {

            BOOL overlaps = bounds.NoteOverlaps( note, itsNumTimes, positions );
            do {

                if (overlaps) {

                    AssignEntry( note, positions );
                }
            } while (bounds.NoteOverlapsAgain( note, itsNumTimes, positions, overlaps ));
        }

        // long note
        else {

            BOOL overlaps = bounds.NoteOverlaps( note, itsNumDays, positions );
            do {

                if (overlaps) {

                    AssignLongEntry( note, positions );
                }
            }while (bounds.NoteOverlapsAgain( note, itsNumDays, positions, overlaps ));
         }
    }

    // sort the long notes
    itsLongNoteTableP->Sort();
    itsNumLongNotes = min( kMaxLongNotes,
        RowsForWeek( itsNumDays / 2, FALSE ));

    //has to be set after firstCell is initialized
    SetDefaultPrintRange();

    // go to the startTime
    int row, col;
    TimeToCell( startTime, row, col );
    InvalidateCell( -1, -1 );
    ScrollChange();
    InvalidateFocus();
    GotoRC( row - kNumHeadRows, col - kNumHeadCols);

}

//---------------------------------------------------------------------------
// AssignEntry
//
// Put a note into the note table
//---------------------------------------------------------------------------
void TWeekGrid::AssignEntry( RTNote note, RTBoundsPositions positions )
{
    itsEntryTableP->SetEntry( &note, positions, kScaleFactor );

    // find home for note title
    int h = positions.startCell;
    while ((h < itsNumTimes) && (itsTitlesP[h] != NULL) ) {

        h++;
    }

    if (h < itsNumTimes) {

        itsTitlesP[ h ] = &note;
    }
}

//---------------------------------------------------------------------------
// AssignLongEntry
//
// Put a note into the long note table
//---------------------------------------------------------------------------
void TWeekGrid::AssignLongEntry( RTNote note, RTBoundsPositions positions )
{
    TLongNote *longNote   = new TLongNote;
    longNote->itsNoteP     = &note;
    longNote->itsRow       = itsNumLongNotes++;
    longNote->itsStartCol  = positions.startCell;
    longNote->itsStartPos  = positions.startPos;
    longNote->itsEndPos    = positions.endPos;

    if (positions.endPos == 0) {

        longNote->itsEndCol = positions.endCell - 1;
    }
    else {

        longNote->itsEndCol = positions.endCell;
    }

    itsLongNoteTableP->add( *longNote, TXArray::kNoSort );
}

//---------------------------------------------------------------------------
// CellToTime
//---------------------------------------------------------------------------
void TWeekGrid::CellToTime( int row, int col, RTTime time  )
{
    ASSERT_STR( "CellToTime: illegal column", (col != -1));
    time   = itsFirstCell.Plus( SCALE_DAY, (col - kNumHeadCols) / 2 );

    // a cell in the long-note row is a full day
    if (RowType( row ) == kTimeRow) {

        ASSERT_STR( "CellToTime: illegal row", (row != -1));
        int m  = (row - kNumHeadRows) * itsMinutesPerCell;
        time.SetHour( m / 60 );
        time.SetMinute( m % 60 );
    }
    else {

        time.ClearTime();
    }
}

//---------------------------------------------------------------------------
// CellToEntry
//---------------------------------------------------------------------------
int TWeekGrid::CellToEntry( int row, int col )
{
    if ((row == -1) || (col == -1)) return -1;

    int day = (col - kNumHeadCols) / 2;
    return itsLiveRows * day + (row - kNumHeadRows);
}

//---------------------------------------------------------------------------
// TimeToCell
//---------------------------------------------------------------------------
void TWeekGrid::TimeToCell( RTTime time, int &row, int &col )
{
    int days = itsFirstCell.DayDelta( time );
    col      = kNumHeadCols + 2 * days;

    if (time.HasTime()) {

        int h = time.GetHour();
        int m = time.GetMinute();

        row = kNumHeadRows + (h * 60 + m) / itsMinutesPerCell;
    }
    else {

        row = kLongNoteRow;
    }
}

//---------------------------------------------------------------------------
// GetCellTimes
//---------------------------------------------------------------------------
void TWeekGrid::GetCellTimes( int row, int col, RTTime start, RTTime end  )
{
    CellToTime( row, col, start );

    end = start;
    end.PlusEquals( k1Min, itsMinutesPerCell );
}



//---------------------------------------------------------------------------
//
//                                 Painting
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// CellWidth
//---------------------------------------------------------------------------
int TWeekGrid::CellWidth( PTPaint pPaint, int col )
{

    const int kMinTitleWidth     = 77;
    const int kLeftmostGridWidth = LeftLineWidth( pPaint ) + 2;

    RECT r;
    if( pPaint->IsPrintPainter() ) {

        const int kUnavailableWidth  = kMinTitleWidth + kLeftmostGridWidth;
        const int kSlopPerCellPair   = 3; // include grid lines

        TPrint::GetPrtPixels( &r );

        int totalWidth        = r.right - r.left;
        int availableWidth    = totalWidth - kUnavailableWidth;
        int widthPerCellPair  = availableWidth / kNumPrintCols;
        int stdColWidth       = max( 50, widthPerCellPair - kSlopPerCellPair );

        // titleWidth absorbs all leftovers from std rows
        int usedWidth         = kNumPrintCols * widthPerCellPair +
                                kLeftmostGridWidth;
        int titleWidth        = totalWidth - usedWidth;
        titleWidth            = max( kMinTitleWidth, titleWidth );

        switch (ColType( col )) {

            case kTimeCol:    return titleWidth;
            case kStripeCol:  return 0;
            case kNoteCol:    return stdColWidth;
            default:

               SYSERR_STR( "CellWidth: unexpected case in switch" );
               return 0;
        }
    }
    else {

        const int kScrollBarWidth    = 0;  // GetClientRect cuts scroll bar out
        const int kUnavailableWidth  = kMinTitleWidth + kScrollBarWidth + kLeftmostGridWidth;
        const int kSlopPerCellPair   = kStripeColWidth + 2; // include grid lines

        GetClientRect( HWindow, &r );

        int totalWidth        = r.right - r.left;
        int availableWidth    = totalWidth - kUnavailableWidth;
        int widthPerCellPair  = availableWidth / kNumVisibleCols;
        int stdColWidth       = max( 50, widthPerCellPair - kSlopPerCellPair );

        // titleWidth absorbs all leftovers from std rows
        int usedWidth         = kNumVisibleCols * widthPerCellPair +
                                kScrollBarWidth + kLeftmostGridWidth;
        int titleWidth        = totalWidth - usedWidth;
        titleWidth            = max( kMinTitleWidth, titleWidth );

        switch (ColType( col )) {

            case kTimeCol:    return titleWidth;
            case kStripeCol:  return kStripeColWidth;
            case kNoteCol:    return stdColWidth;
            default:

               SYSERR_STR( "CellWidth: unexpected case in switch" );
               return 0;
        }
    }

}

//---------------------------------------------------------------------------
// CellHeight
//---------------------------------------------------------------------------
int TWeekGrid::CellHeight( PTPaint pPainter, int row )
{

    int titleHeight    = 38;
    int longNoteHeight = max( 19, (kSmallFontHeight + 1) * itsNumLongNotes );

    switch( RowType( row )) {

        case kTitleRow:      return titleHeight;

        case kLongStripeRow:
            if( pPainter->IsPrintPainter() )
                return 0;
            else
                return kStripeRowHeight;

        case kLongNoteRow:   return longNoteHeight;
        case kTimeRow:       return kStdRowHeight;
        default:

            SYSERR_STR( "CellHeight: unexpected case in switch" );
            return 0;
    }
}

//---------------------------------------------------------------------------
// GridWidth
//---------------------------------------------------------------------------
int  TWeekGrid::GridWidth( PTPaint, int col )
{
    return (ColType( col ) == kStripeCol) ? 1 : 2;
}

//---------------------------------------------------------------------------
// GridHeight
//---------------------------------------------------------------------------
int  TWeekGrid::GridHeight( PTPaint pPainter, int row )
{
    TTime time;

    switch( RowType( row )) {

        case kLongNoteRow:   return 2;
        case kTitleRow:      return 2;
        case kLongStripeRow: return 1;
        case kTimeRow:       

            if( pPainter->IsPrintPainter() )
                {
                return 1;
                }
            else
                {
                CellToTime( row + 1, kNumHeadCols, time );
                return (time.GetMinute() == 0) ? 2 : 1;
                }

        case kNotARow:       return 1;
        default:

            SYSERR_STR( "GridHeight: unexpected case in switch" );
            return 1;
    }
}

//---------------------------------------------------------------------------
// PaintCell
//---------------------------------------------------------------------------
void TWeekGrid::PaintCell( RTDrawInfo di )
{
    switch( CellType( di.row, di.col )) {

        case kTimeTitle:         PaintTimeTitle( di );     break;
        case kStripeTitle:                                 break;
        case kNoteTitle:         PaintNoteTitle( di );     break;
        case kLongStripeTime:                              break;
        case kLongStripe:        PaintLongStripe( di );    break;
        case kLongNoteNames:     PaintLongNames( di );     break;
        case kLongNote:          PaintLongNote( di );      break;
        case kTime:              PaintTime( di );          break;
        case kStripe:            PaintStripe( di );        break;
        case kNote:              PaintNote( di );          break;

        default:

            SYSERR_STR( "PaintCell: unexpected case in switch" );
            break;
    }
}


//---------------------------------------------------------------------------
// PaintTimeTitle
// set year to correspond to date of first displayed column
//---------------------------------------------------------------------------
void TWeekGrid::PaintTimeTitle( RTDrawInfo di )
{
    char buf[20];

    TTime leftCellTime;

    CellToTime( kNumHeadRows, hScrollPos + frozenCols, leftCellTime );

    TIntlFmt::FormatYear( leftCellTime, buf );
    di.pPainter->Text( di, buf, kNoHilite, kSingleLine | DT_VCENTER | DT_CENTER, kBoldFont );
}

//---------------------------------------------------------------------------
// PaintNoteTitle
//---------------------------------------------------------------------------
void TWeekGrid::PaintNoteTitle( RTDrawInfo di )
{

    TTime thisDay;
    char buf2[50];
    char dowBuf[10];
    char buf[100];

    CellToTime( di.row, di.col, thisDay );
    TGridUtil::PaintBackground( di, thisDay, FALSE, FALSE );

    TIntlFmt::FormatDayAndMonth( thisDay, buf2 );

    TIntlFmt::DayName( thisDay.GetWeekDay(), kThreeLetter, dowBuf );
    wsprintf( buf, "%s\n%s", dowBuf, buf2 );

    RECT r= di.cellRect;
    r.top += 5; //text looks better when moved down
    di.pPainter->Text( di, buf, kNoHilite, kMultiLine | DT_CENTER, kBoldFont, &r );
}

//---------------------------------------------------------------------------
// PaintTime
//---------------------------------------------------------------------------
void TWeekGrid::PaintTime( RTDrawInfo di )
{
    char buf[20];
    WORD tStyle= DT_SINGLELINE | DT_NOPREFIX | DT_EXTERNALLEADING | DT_RIGHT | DT_VCENTER;

    TTime time;
    CellToTime( di.row, di.col, time );
    TIntlFmt::FormatTime( time, FALSE, buf );
    di.pPainter->Text( di, buf, kNoHilite, tStyle, kBoldFont );
}


//---------------------------------------------------------------------------
// PaintNote
//---------------------------------------------------------------------------
void TWeekGrid::PaintNote( RTDrawInfo di )
{
    TTime start, end;
    GetCellTimes( di.row, di.col, start, end );
    end.PlusEquals( k1Min, -1 );
    TGridUtil::PaintBackground( di, end, FALSE );

    RECT r  = di.cellRect;

    // calculate the rectangle for the first bar
    r.right = r.left + kPixelsPerVBar - kWhiteSpace;


    PTNote  pNote;
    long    startPos, stopPos;
    int     lastFilledSlot = 0;

    // calculate the entry
    int entry = CellToEntry( di.row, di.col );
    if (entry == -1) return;
    for (int slot = 0; slot < kMaxSlots; slot++) {

        // get the entry for this slot and continue if there is none
        if (itsEntryTableP->GetEntry( entry, slot, &pNote, startPos, stopPos )) {

            lastFilledSlot = slot;

            switch( pNote->GetStyle() ) {

                case kMilestoneStyle:

                    PaintMilestone( di, r, pNote->GetColor() );
                    break;

                case kTaskStyle:

                    PaintTask( di, r, pNote->GetColor(), (int) startPos, (int) stopPos );
                    break;
            }
        }


        // push the rectangle right
        r.left  += kPixelsPerVBar;
        r.right += kPixelsPerVBar;
    }

    // print note title
    if (itsTitlesP[entry] != NULL ) {

        PaintNoteTitle( di, r, itsTitlesP[entry], lastFilledSlot );
    }
}

//---------------------------------------------------------------------------
// PaintMilestone
//---------------------------------------------------------------------------
void TWeekGrid::PaintMilestone( RTDrawInfo di, RECT &r, RTColor color )
{
    RECT drawR;

    drawR = r;
    drawR.top    = (di.cellRect.top + di.cellRect.bottom - kPixelsPerVBar) / 2;
    drawR.bottom = drawR.top + kPixelsPerVBar;
    di.pPainter->FillDiamond( di, drawR, color );
}

//---------------------------------------------------------------------------
// PaintTask
//---------------------------------------------------------------------------
void TWeekGrid::PaintTask(      RTDrawInfo di, RECT &r, RTColor color, int startPos, int stopPos )
{
    RECT drawR;
    int height   = di.cellRect.bottom - di.cellRect.top;

    drawR        = r;

    if ((startPos == 0) && (stopPos == 0)) {

        // if startPos and stopPos are both 0, then we're in a 'more'
        // slot: paint a black note all through the slot
        //TColor color;
        //color.Set( kBlack );
        //di.pPainter->FillRectangle( di, drawR, color );
        di.pPainter->Text( di, "+", kNoHilite, DT_LEFT, kBoldFont, &drawR );
    }
    else {

        drawR.top    = di.cellRect.top + height * startPos / kScaleFactor;
        drawR.bottom = di.cellRect.top + height * stopPos  / kScaleFactor;
        di.pPainter->FillRectangle( di, drawR, color );
    }
}

//---------------------------------------------------------------------------
// PaintNoteTitle
//---------------------------------------------------------------------------
void TWeekGrid::PaintNoteTitle( RTDrawInfo di, RECT &, PTNote pNote, int lastFilledSlot )
{
    RECT drawR;

    drawR       = di.cellRect;
    drawR.left += 5 + kPixelsPerVBar * (lastFilledSlot + 1);

    TSafStr title;
    pNote->Title( title );

    TFont font= (pNote->IsDone()) ? kSmallStrikeFont : kSmallFont;
    di.pPainter->Text( di, title.Addr(), kNoHilite, kMultiLine, font, &drawR );

    // draw ink if there was no title
    if( (title.Len() == 0) && pNote->GetInk().HasInk() ) {

        pNote->GetInk().Show( di.hdc, &drawR );
    }
}


//---------------------------------------------------------------------------
// PaintLongNames
//---------------------------------------------------------------------------
void TWeekGrid::PaintLongNames( RTDrawInfo di )
{
    RECT r = di.cellRect;

    for (int i = 0; i < itsNumLongNotes; i++) {

        TSafStr title;
        PTNote  noteP = LongNoteInRow( i ).itsNoteP;

        noteP->Title( title );
        r.top    = di.cellRect.top + kSmallFontHeight * i;
        r.bottom = r.top + kSmallFontHeight;

        TFont font= (noteP->IsDone()) ? kSmallStrikeFont : kSmallFont;

        if (i == kMaxLongNotes - 1) {

            di.pPainter->Text( di, "more", kNoHilite,
                kSingleLine, kSmallFont, &r );
        }
        else {

            di.pPainter->Text( di, (char*) title.Addr(), kNoHilite,
                kSingleLine, font, &r );
        }
    }
}

//---------------------------------------------------------------------------
// PaintLongNote
//---------------------------------------------------------------------------
void TWeekGrid::PaintLongNote( RTDrawInfo di )
{
    int dayIndex  = (di.col - kNumHeadCols) / 2;
    TTime thisDay;

    CellToTime( di.row, di.col, thisDay );

    for (int i = 0; i < itsNumLongNotes; i++) {

        RECT r        = di.cellRect;

        // handle "more" row by using plus signs
        if (i == kMaxLongNotes - 1) {

            r        = di.cellRect;
            r.top    = di.cellRect.top + kSmallFontHeight * i + 5;

            di.pPainter->Text( di, "+ + + +", kNoHilite, DT_CENTER,
                kSmallFont, &r );
            return;
        }

        TLongNote &longNote = LongNoteInRow( i );
        int startCol = longNote.itsStartCol;
        int endCol   = longNote.itsEndCol;
        PTNote noteP = longNote.itsNoteP;
        if ((startCol <= dayIndex) && (dayIndex <= endCol)) {

            r.top    = di.cellRect.top + kSmallFontHeight * i + 5;
            r.bottom = r.top + kPixelsPerHBar;

            if ((noteP->GetStyle() == kMilestoneStyle ) &&
                (r.right - r.left > kStripeColWidth)) {

                const int kHalfBarSize = 2;
                const int kHalfDiamond = kSmallFontHeight / 2 - 1;
                RECT leftR, diamondR, rightR;

                int midX = (di.cellRect.left + di.cellRect.right)  / 2;
                int midY = (r.top + r.bottom) / 2;

                SetRect( &leftR,    di.cellRect.left,        midY - kHalfBarSize,
                                    midX - kHalfDiamond - 5, midY + kHalfBarSize );
                SetRect( &rightR,   midX + kHalfDiamond + 5, midY - kHalfBarSize,
                                    di.cellRect.right,       midY + kHalfBarSize );
                SetRect( &diamondR, midX - kHalfDiamond,     midY - kHalfDiamond,
                                    midX + kHalfDiamond,     midY + kHalfDiamond );

                di.pPainter->FillRectangle( di, leftR,    noteP->GetColor() );
                di.pPainter->FillRectangle( di, rightR,   noteP->GetColor() );
                di.pPainter->FillDiamond(   di, diamondR, noteP->GetColor() );

            }

            // "real" long notes
            else if (!noteP->GetStart().HasTime())
                {
                if (noteP->HollowOnDate( thisDay ))
                    {
                    di.pPainter->FrameRectangle( di, r, noteP->GetColor() );
                    }
                else
                    {
                    di.pPainter->FillRectangle( di, r, noteP->GetColor() );
                    }
                }

            // long hour notes
            else
                PaintLongHourNote( di, *noteP, r );
        }
    }
}

// ------------------------------------------------------------------
// PaintLongHourNote
// ------------------------------------------------------------------
void TWeekGrid::PaintLongHourNote( RTDrawInfo di, RTNote note, RECT &r )
{
    TTime thisDay;
    TBoundsPositions positions;
    CellToTime( di.row, di.col, thisDay );

    // drag row?
    if ((di.col % 2) == 1)
        {
        thisDay.SetToMidnight();
        TBounds bounds( thisDay, thisDay.Plus( k1Min, 1 ) );
        if( bounds.NoteOverlaps( note, 1, positions ))
            {
            if (note.HollowOnDate( thisDay ))
                di.pPainter->FrameRectangle( di, r, note.GetColor() );
            else
                di.pPainter->FillRectangle( di, r, note.GetColor() );
            }
        return;
        }

    TBounds bounds( thisDay, thisDay );

    if( bounds.NoteOverlaps( note, 1, positions ))
        {
        int width = r.right - r.left;
        r.right   = r.left + positions.endPos   * width;
        r.left   += positions.startPos * width;
        r.right   = max( r.right, r.left + 1 );         // (>= 1 pixel wide)

        if (note.HollowOnDate( thisDay ))
            di.pPainter->FrameRectangle( di, r, note.GetColor() );
        else
            di.pPainter->FillRectangle( di, r, note.GetColor() );
    }
}


// ------------------------------------------------------------------
// PaintStripe
// ------------------------------------------------------------------
void TWeekGrid::PaintStripe( RTDrawInfo di )
{
    di.pPainter->FillRectangle( di, di.cellRect, kHbrMultiCell );

    //restrict hiliting to col in which drag started
    int row, col;
    GetRangeAnchor( row, col );
    if( col == di.col && IsCellInRange( di.row, di.col ) && !itsDraggingDays ) {

        di.pPainter->Hilite( di, kInvertHilite );
    }
}

// ------------------------------------------------------------------
// PaintLongStripe
// ------------------------------------------------------------------
void TWeekGrid::PaintLongStripe( RTDrawInfo di )
{
    di.pPainter->FillRectangle( di, di.cellRect, kHbrMultiCell );

    if (IsCellInRange( di.row, di.col ) && itsDraggingDays) {

        di.pPainter->Hilite( di, kInvertHilite );
    }
}

//---------------------------------------------------------------------------
//
//                                 Scrolling
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// WMHScroll
//
// Handles the fact that a single day is represented by 2 separate columns
// which are supposed to move in synch
//---------------------------------------------------------------------------
void TWeekGrid::WMHScroll( RTMessage Msg)
{
    TMessage newMsg   = Msg;
    int      maxRight = (itsNumDays - kNumVisibleCols) * 2;

    switch( Msg.WParam ) {

        case SB_LINEUP:

            newMsg.WParam = SB_THUMBPOSITION;
            newMsg.LParam = max( 0, hScrollPos - 2 );
            break;

        case SB_LINEDOWN:

            newMsg.WParam = SB_THUMBPOSITION;
            newMsg.LParam = min( maxRight, hScrollPos + 2 );
            break;

        case SB_PAGEUP:

            newMsg.WParam = SB_THUMBPOSITION;
            newMsg.LParam = max( 0, hScrollPos - 2 * (kNumVisibleCols - 1));
            break;

        case SB_PAGEDOWN:

            newMsg.WParam = SB_THUMBPOSITION;
            newMsg.LParam = min( maxRight, hScrollPos + 2 * (kNumVisibleCols - 1));
            break;

        case SB_THUMBPOSITION:

            LOWORD( newMsg.LParam ) = LOWORD( Msg.LParam ) & 0xFFFE;
            break;

        case SB_TOP:
        case SB_BOTTOM:

            break;
    }


    // send the corrected version off to basegrid
    TBaseGrid::WMHScroll( newMsg );


    //set global time based on current scroll position
    //this insures that weekgrid, or subsequent daygrid, open
    // to same scroll place; might have been disabled by scale zoom
    CellToTime( vScrollPos + kNumHeadRows, hScrollPos + kNumHeadCols,
        TSysState::selTime );

    InvalidateCell( 0, 0 );     //keep the "year" cell in synch with column

    // since we're moving to a new column, show
    // new long notes (but not within an autoscroll
    // that results from a range or drag being
    // in progress
    if (!bDragInProgress && !bRangeInProgress) {

        int oldFirst = itsFirstVisibleLongNote;
        int rows =RowsForWeek( hScrollPos / 2,FALSE );
        if (itsNumLongNotes != rows) {

            itsNumLongNotes = min( kMaxLongNotes, rows );
            InvalidateCell( -1, -1 );
        }
        else if (oldFirst != itsFirstVisibleLongNote) {

            InvalidateCell( kLongNoteRow, -1 );
        }
    }

    Notify( kTimeChg, NOOBJECT );
}

//---------------------------------------------------------------------------
// WMVScroll
//
// Keeps track of the selected time
//---------------------------------------------------------------------------
void TWeekGrid::WMVScroll( RTMessage Msg)
{
    // send the corrected version off to basegrid
    TBaseGrid::WMVScroll( Msg );


    //set global time based on current scroll position
    //this insures that weekgrid, or subsequent daygrid, open
    // to same scroll place; might have been disabled by scale zoom
    CellToTime( vScrollPos + kNumHeadRows, hScrollPos + kNumHeadCols,
        TSysState::selTime );
    Notify( kTimeChg, NOOBJECT );
}

//---------------------------------------------------------------------------
//
//                                 Mousing
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// MousOver
//---------------------------------------------------------------------------
void TWeekGrid::MousOver( RTMousInfo mi )
{

    switch( CellType( mi.row, mi.col )) {

        case kStripeTitle:
        case kNoteTitle:     TCursor::Set( kZoomCursor );         break;
        case kLongStripe:    TCursor::Set( kLongNoteBarCursor );  break;
        case kLongNoteNames: TCursor::Set( kNoteSelCursor );      break;
        case kStripe:        TCursor::Set( kNoteBarCursor );      break;

        case kLongNote:

            TCursorStyle style;
            if ( CursorForDrag( style, mi ) ) {

                TCursor::Set( style );
            }
            else if (itsTitlesP[CellToEntry( mi.row, mi.col )]) {

                TCursor::Set( kNoteSelCursor );
            }
            else {

                TCursor::Set( kArrowCursor );
            }
            break;

        case kNote:

            if ( CursorForDrag( style, mi ) ) {

                TCursor::Set( style );
            }
            else if (itsTitlesP[CellToEntry( mi.row, mi.col )]) {

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
//---------------------------------------------------------------------------
void TWeekGrid::MousDblClick( RTMousInfo mi )
{
    switch (CellType( mi.row, mi.col )) {

        case kNoteTitle:     GotoDay( mi );                break;
        case kStripeTitle:   GotoDay( mi );                break;
        case kLongNoteNames: ShowAllLongNotes();           break;
        case kLongNote:      ShowTodaysLongNotes( mi );    break;
        case kStripe:        ShowCellsNote( mi );          break;
        case kNote:          ShowCellsNote( mi );          break;
        default:             /* no effect */               break;
    }
}

//---------------------------------------------------------------------------
// MousRtClick
//---------------------------------------------------------------------------
void TWeekGrid::MousRtClick( RTMousInfo mi )
{
    switch (CellType( mi.row, mi.col )) {

        case kNote:

            PTNote pNote = itsTitlesP[ CellToEntry( mi.row, mi.col ) ];

            if( pNote != NULL ) {

                TNoteMenu::PopUp( this, pNote, mi.xy.x, mi.xy.y );
                InvalidateCell( -1, mi.col );
            }
            else {

                TMousHelp::MousHelp( this,
                    "Double-click left button to create a new note starting at this time." );
            }
            break;

        case kLongNote:

            TCursorStyle style;
            if ( CursorForDrag( style, mi ) ) {

                TMousHelp::MousHelp( this,
                    "Click left button and drag mouse to move this note to a different time." );
            }
            else if (itsTitlesP[CellToEntry( mi.row, mi.col )]) {

                PTNote noteP = itsTitlesP[CellToEntry( mi.row, mi.col )];
                TNoteMenu::PopUp( this, noteP, mi.xy.x, mi.xy.y );
                InvalidateCell( -1, mi.col );
            }
            break;

        case kLongNoteNames:

            TMousHelp::MousHelp( this,
                "Double-click left button to see a list of multi-day notes." );
            break;

        case kStripeTitle:
        case kNoteTitle:

            TMousHelp::MousHelp( this,
                "Double-click left button to view this day (zoom in)." );
            break;

        case kStripe:
        case kLongStripe:

            TMousHelp::MousHelp( this,
                "Click left button and drag mouse along shaded stripe to create a new note with start and stop times." );
            break;

        default:

            break;
    }
}

//---------------------------------------------------------------------------
// GotoDay
//---------------------------------------------------------------------------
void TWeekGrid::GotoDay( RTMousInfo mi )
{
    CellToTime( vScrollPos + kNumHeadRows, mi.col, TSysState::selTime );
    Notify( kTimeChg, NOOBJECT );

    //old way -- causes problems
    //TSysState::timeScale = SCALE_HOUR;
    //Notify( kScaleChg, NOOBJECT );

    //emulate menu scale change command
    PostMessage( TPimFrame::pFrame->HWindow, WM_COMMAND, IDM_VIEWDAY, 0 );

}

//---------------------------------------------------------------------------
// ShowAllLongNotes
//---------------------------------------------------------------------------
void TWeekGrid::ShowAllLongNotes( void )
{
    // popup a list if there are any long notes in the set
    int rows = RowsForWeek( hScrollPos / 2, FALSE );
    if (rows > 0) {

        TNoteSet notes;

        for (int i = 0; i < rows; i++) {

            notes.add( *LongNoteInRow( i ).itsNoteP );
        }

        TNoteListDialog *noteListDialog = new TNoteListDialog(
            TPimFrame::pFrame, IDD_NOTELIST_DIALOG,
            notes, " Multi-day Notes" );

        // if we didn't cancel out of the box then show a note
        int selection = GetApplicationObject()->ExecDialog( noteListDialog );
        if (selection != kCanceledNotelist) {

            GetApplication()->ExecDialog(new TNoteView(
                TPimFrame::pFrame, IDD_NOTE,
                LongNoteInRow( selection ).itsNoteP ));
        }
    }
}

//---------------------------------------------------------------------------
// ShowTodaysLongNotes
//---------------------------------------------------------------------------
void TWeekGrid::ShowTodaysLongNotes( RTMousInfo  mi )
{
    TTime start;
    CellToTime( mi.row, mi.col, start );
    TGridUtil::NotePopup( start );
}

//---------------------------------------------------------------------------
// ShowCellsNote
//---------------------------------------------------------------------------
void TWeekGrid::ShowCellsNote( RTMousInfo  mi )
{
    //opens the note in the cell
    PTNote pNote = itsTitlesP[CellToEntry( mi.row, mi.col )];

    if (pNote == NULL) {

        pNote = TSysState::pAllNotes->CreateNote( this );
        if (pNote != NULL) {

            TTime start;
            CellToTime( mi.row, mi.col, start );
            pNote->SetStartAndClearDuration( start );
        }
    }

    //still NULL if user cancelled CreateNote
    if (pNote != NULL) {

        TGridUtil::ShowNote( pNote );
    }
}



//---------------------------------------------------------------------------
// MousRangeInit
//---------------------------------------------------------------------------
BOOL TWeekGrid::MousRangeInit( RTMousInfo mi )
{
    TRuleGrid::MousRangeInit( mi );

    if( mi.bKeyAction ) return FALSE;

    TCellType cellType = CellType( mi.row, mi.col );
    itsDraggingDays    = (cellType == kLongStripe);

    if ((cellType == kLongStripe) || (cellType == kStripe)) {

        OpenDragWindow( FALSE );
        return TRUE;
    }
    else {

        return FALSE;
    }

}

//---------------------------------------------------------------------------
// MousRangeOver
//
// handle autoscrolling if mouse goes outside window
//---------------------------------------------------------------------------
void TWeekGrid::MousRangeOver( RTMousInfo mi )
{
    MousOver( mi );     //maintain proper cursor

    if (itsDraggingDays) {    // long notes

        int colDelta = 0;

        if (XYLeft( mi ) || XYInFrozenCol( mi )) {

            colDelta = (hScrollPos > 0)                ?  2 : 0;
        }
        else if (XYRight( mi )) {

            colDelta = (hScrollPos < hScrollMax - 1)   ? -2 : 0;
        }
        else {

            TRuleGrid::MousRangeOver( mi );
        }


        // autoscroll
        if (colDelta != 0) {

            // include scrolled cell in range
            int height, width;
            GetRangeExtent( height, width );
            SetRangeExtent( height, width - colDelta );

            // scroll to show new position
            Scroll( 0, colDelta );
            UpdateWindow( HWindow );
        }


        // show times
        TTime start, end;
        int   row, col, top, left, bottom, right;
        GetRangeAnchor( row, col );
        GetRangeRect( top, left, bottom, right );
        CellToTime( row, left, start );
        CellToTime( row, right, end );
        itsNewTimes.SetStartAndEnd( start, end, k1Day );
        itsDragWinP->Set( "", itsNewTimes );
    }
    else {                    // regular notes

        int rowDelta = 0;
        if ( XYAbove( mi ) || XYInFrozenRow( mi )) {

            rowDelta = (vScrollPos > 0)                ?  1 : 0;
        }
        else if (XYBelow( mi )) {

            rowDelta = (vScrollPos < vScrollMax - 1)   ? -1 : 0;
        }
        else {

            TRuleGrid::MousRangeOver( mi );
        }

        // autoscroll
        if (rowDelta != 0) {

            // include scrolled cell in range
            int height, width;
            GetRangeExtent( height, width );
            SetRangeExtent( height - rowDelta, width );

            // scroll to show new position
            Scroll( rowDelta, 0 );
            UpdateWindow( HWindow );
        }

        // show times
        TTime start, end;
        int   row, col, top, left, bottom, right;
        GetRangeAnchor( row, col );
        GetRangeRect( top, left, bottom, right );
        CellToTime( top, col, start );
        CellToTime( bottom + 1, col, end );
        itsNewTimes.SetStartAndEnd( start, end,
            (TTimeUnit) (TProfile::dayviewIncrement + 1));
        itsDragWinP->Set( "", itsNewTimes );
    }
}

//---------------------------------------------------------------------------
// MousRangeRelease
//---------------------------------------------------------------------------
void TWeekGrid::MousRangeRelease( RTMousInfo mi )
{
    CloseDragWindow();

    // set range
    TRuleGrid::MousRangeRelease( mi );

    // get range
    int top, left, bottom, right;
    if (!GetRangeRect( top, left, bottom, right )) goto exit;

    // create a long note
    if (itsDraggingDays) {

        // return if the drag ended outside the row it started in
        if (CellType( mi.row, mi.col ) != kLongStripe) goto exit;
        if (top != bottom) goto exit;

        PTNote pNote = TSysState::pAllNotes->CreateNote( this );
        if( pNote == NULL ) goto exit;

        TTime start, end;
        CellToTime( -1, left, start );
        CellToTime( -1, right, end );
        start.ClearTime();
        end.ClearTime();

        pNote->SetStartAndEnd( start, end, k1Day );

        TGridUtil::ShowNote( pNote );
    }

    // create a regular note
    else {

        // return if the drag ended outside the column it started in
        if (CellType( mi.row, mi.col ) != kStripe) goto exit;
        if (left != right) goto exit;

        PTNote pNote = TSysState::pAllNotes->CreateNote( this );
        if( pNote == NULL ) goto exit;

        TTime start, end;
        CellToTime( max( top, kNumHeadRows ), left, start );
        CellToTime( bottom + 1, left + 1, end );

        pNote->SetStartAndEnd( start, end,
            min( k1Hour, (TTimeUnit) (TProfile::dayviewIncrement + 1)));

        TGridUtil::ShowNote( pNote );
    }

    //clear selection
exit:
    SetRangeAnchor( -1, -1 );
}

//---------------------------------------------------------------------------
//
//                                 CellType
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// CellType
//
// Return the type of the cell; handles the case where row or col is -1,
// as it would be if XYWithin were false
//---------------------------------------------------------------------------
TWeekGrid::TCellType TWeekGrid::CellType( int row, int col )
{
    if (row == -1) {

        return kNotACell;
    }
    else if (row == 0) { // titles

        switch( col ) {

            case -1:  return kNotACell;
            case  0:  return kTimeTitle;
            default:  return ((col % 2) == 0) ? kNoteTitle : kStripeTitle;
        }
    }
    else if (row == 1) { // long note Stripe

        switch( col ) {

            case -1:  return kNotACell;
            case  0:  return kLongStripeTime;
            default:  return kLongStripe;
        }
    }
    else if (row == kLongNoteRow) { // long note

        switch( col ) {

            case -1:  return kNotACell;
            case  0:  return kLongNoteNames;
            default:  return kLongNote;
        }
    }
    else {               // regular notes

        switch( col ) {

            case -1:  return kNotACell;
            case  0:  return kTime;
            default:  return ((col % 2) == 0) ? kNote : kStripe;
        }
    }
};



//---------------------------------------------------------------------------
// ColType
//
// Return the type of the col; handles the case where col is -1, as it would
// be if XYWithin were false
//---------------------------------------------------------------------------
TWeekGrid::TColType TWeekGrid::ColType( int col )
{
    switch( col ) {

        case -1:    return kNotACol;
        case  0:    return kTimeCol;
        default:    return ((col % 2) == 0) ? kNoteCol : kStripeCol;
    }
}

//---------------------------------------------------------------------------
// RowType
//
// Return the type of the row; handles the case where row is -1, as it would
// be if XYWithin were false
//---------------------------------------------------------------------------
TWeekGrid::TRowType TWeekGrid::RowType( int row )
{
    switch( row ) {

        case -1:   return kNotARow;
        case  0:   return kTitleRow;
        case  1:   return kLongStripeRow;
        case  2:   return kLongNoteRow;
        default:   return kTimeRow;
    }
}

//---------------------------------------------------------------------------
//
//                                 Listening
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// ResetTime
//---------------------------------------------------------------------------
void TWeekGrid::ResetTime( void )
{
    if (TSysState::selTime != itsStartTime) {

        Setup( TSysState::selTime );
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
void TWeekGrid::Listen( TSnoopMsg aMsg, RObject )
{
switch( aMsg )
    {
    case kNoteSetChg:

        Setup( TSysState::selTime );
        break;

    case kTimeChg:
        ResetTime();
        break;

    default:
        break;
    }
}

//---------------------------------------------------------------------------
//
//                                 Printing
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// SetDefaultPrintRange
//---------------------------------------------------------------------------
void TWeekGrid::SetDefaultPrintRange( void )
{
TTime early= itsFirstCell;
TTime late= itsFirstCell;

early.SetHour( 8 );
late.SetHour( 20 );

int row, col;
TimeToCell( early, row, col );
rFirstLiquid= row;

TimeToCell( late, row, col );
rLastLiquid= row;

cFirstLiquid= frozenCols + hScrollPos;
cLastLiquid= cFirstLiquid + 13;       //one week

}

//---------------------------------------------------------------------------
// HChgCellStartIndex
//---------------------------------------------------------------------------
BOOL TWeekGrid::HChgCellStartIndex( int n, char *buf, int /*size*/ )
{
BOOL rc;

//this view uses two cols per date, so n is 2 per day
// also, range selection is constrained to cover whole weeks
// so smallest increment is 14
n *= 14;

rc= ((cFirstLiquid + n) >= frozenCols &&
     (cFirstLiquid + n) < colCnt &&
     (cFirstLiquid + n ) < cLastLiquid );

if( rc )
    cFirstLiquid+= n;

FormatColDate( cFirstLiquid, buf );

return rc;
}

//---------------------------------------------------------------------------
// HChgCellEndIndex
//---------------------------------------------------------------------------
BOOL TWeekGrid::HChgCellEndIndex( int n, char *buf, int /*size*/ )
{
BOOL rc;

// see above for explanation of 14
n *= 14;

rc= ((cLastLiquid + n) >= frozenCols &&
     (cLastLiquid + n) < colCnt &&
     (cLastLiquid + n) > cFirstLiquid );

if( rc )
    cLastLiquid+= n;

FormatColDate( cLastLiquid, buf );

return rc;
}

//---------------------------------------------------------------------------
// VChgCellStartIndex
//---------------------------------------------------------------------------
BOOL TWeekGrid::VChgCellStartIndex( int n, char *buf, int /*size*/ )
{
BOOL rc;
TTime time;

rc= ((rFirstLiquid + n) >= frozenRows &&
     (rFirstLiquid + n) < rowCnt &&
     (rFirstLiquid + n ) < rLastLiquid );

if( rc )
    rFirstLiquid+= n;

CellToTime( rFirstLiquid, kFirstDayColumn, time );
TIntlFmt::FormatTime( time, FALSE, buf );

return rc;
}

//---------------------------------------------------------------------------
// VChgCellEndIndex
//---------------------------------------------------------------------------
BOOL TWeekGrid::VChgCellEndIndex( int n, char *buf, int /*size*/ )
{
BOOL rc;
TTime time;

rc= ((rLastLiquid + n) >= frozenRows &&
     (rLastLiquid + n) < rowCnt &&
     (rLastLiquid + n) > rFirstLiquid );

if( rc )
    rLastLiquid+= n;

CellToTime( rLastLiquid, kFirstDayColumn, time );
TIntlFmt::FormatTime( time, FALSE, buf );

return rc;
}

//---------------------------------------------------------------------------
// FormatColDate
//
// FormatColDate, used for print setup
//---------------------------------------------------------------------------
void TWeekGrid::FormatColDate( int col, char *buf )
{
    TTime thisDay;
    char buf2[50];
    char dowBuf[10];

    CellToTime( -1, col, thisDay );
    TIntlFmt::FormatDayAndMonth( thisDay, buf2 );

    TIntlFmt::DayName( thisDay.GetWeekDay(), kThreeLetter, dowBuf );
    wsprintf( buf, "%s %s", dowBuf, buf2 );
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
//          Dragging
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

//---------------------------------------------------------------------------
// MousDragOver
//---------------------------------------------------------------------------
void TWeekGrid::MousDragOver( RTMousInfo mi )
{
    if (itsDraggingLongNotes) {

        HMousDragOver( mi );
    }
    else {

        VMousDragOver( mi );
    }
}

//---------------------------------------------------------------------------
// MousDragRelease
//---------------------------------------------------------------------------
void TWeekGrid::MousDragRelease( RTMousInfo mi )
{
    if (itsDraggingLongNotes) {

        HMousDragRelease( mi );
    }
    else {

        VMousDragRelease( mi );
    }

    itsDraggingLongNotes = FALSE;
}

//---------------------------------------------------------------------------
// ValidDragCell
//---------------------------------------------------------------------------
BOOL TWeekGrid::ValidDragCell( TDragMode mode, int row, int col )
{
    if (itsDraggingLongNotes) {

        return HValidDragCell( mode, row, col );
    }
    else {

        return VValidDragCell( mode, row, col );
    }
}

//---------------------------------------------------------------------------
// ScrollStart
//---------------------------------------------------------------------------
void TWeekGrid::ScrollStart( RTMousInfo mi, int deltaRow, int deltaCol )
{
    if (itsDraggingLongNotes) {

        HScrollStart( mi, deltaRow, deltaCol );
    }
    else {

        VScrollStart( mi, deltaRow, deltaCol );
    }
}

//---------------------------------------------------------------------------
// ScrollEnd
//---------------------------------------------------------------------------
void TWeekGrid::ScrollEnd( RTMousInfo mi, int deltaRow, int deltaCol )
{
    if (itsDraggingLongNotes) {

        HScrollEnd( mi, deltaRow, deltaCol );
    }
    else {

        VScrollEnd( mi, deltaRow, deltaCol );
    }
}

//---------------------------------------------------------------------------
// ScrollBoth
//
// scroll and adjust the start and end times to match the new scroll pos
//---------------------------------------------------------------------------
void TWeekGrid::ScrollBoth( RTMousInfo mi, int deltaRow, int deltaCol )
{
    if (itsDraggingLongNotes) {

        HScrollBoth( mi, deltaRow, deltaCol );
    }
    else {

        VScrollBoth( mi, deltaRow, deltaCol );
    }
}


//---------------------------------------------------------------------------
// SetStart
//
// we're dragging the start time
//---------------------------------------------------------------------------
void TWeekGrid::SetStart( RTTime start )
{
    if (itsDraggingLongNotes) {

        if( itsNewTimes.GetTimeUnit() >= k1Day)
            start.ClearTime();
        itsNewTimes.SetStartAndKeepEnd( start, k1Day, 1 );
        itsNewTimes.ConstrainToInterval(
            itsFirstCell, itsLastCell,
            NULL, &itsNewTimes.GetEnd() );
    }
    else {

        itsNewTimes.SetStartAndKeepEnd(
            start, k1Min, itsMinutesPerCell );
        itsNewTimes.ConstrainStartToDayAndEnd(
            k1Min, itsMinutesPerCell );
    }
}

//---------------------------------------------------------------------------
// SetEnd
//
// we're dragging the end
//---------------------------------------------------------------------------
void TWeekGrid::SetEnd( RTTime end )
{

    if (itsDraggingLongNotes) {

        if( itsNewTimes.GetTimeUnit() >= k1Day)
            end.ClearTime();
        itsNewTimes.SetEndAndKeepStart(
            end, k1Day, 1  );
        itsNewTimes.ConstrainToInterval(
            itsFirstCell, itsLastCell,
            &itsNewTimes.GetStart(), NULL );
    }
    else {

        itsNewTimes.SetEndAndKeepStart(
            end, k1Min, itsMinutesPerCell  );
        itsNewTimes.ConstrainEndToDayAndStart(
            k1Min, itsMinutesPerCell );
    }
}

//---------------------------------------------------------------------------
// SetBoth
//---------------------------------------------------------------------------
void TWeekGrid::SetBoth( RTMousInfo mi, RTTime start )
{
    if (itsDraggingLongNotes) {

        HSetBoth( mi, start );
    }
    else {

        VSetBoth( mi, start );
    }
}

//---------------------------------------------------------------------------
// ConstrainMarqueeRect
//
// keep the marquee out of the frozen rows
//---------------------------------------------------------------------------
void TWeekGrid::ConstrainMarqueeRect( TDragMode mode, RECT &r )
{
    if (itsDraggingLongNotes) {

        // long notes can't be dragged into frozen columns
        r.left = AT_LEAST( r.left, FrozenWidth( scrPainter ));
    }
    else {

        // short notes can be DRAGGED into the long note row
        if (mode == kDrag) {

            int maxHeight = FrozenHeight( scrPainter ) -
                CellHeight( scrPainter, kLongNoteRow );
            r.top  = AT_LEAST( r.top,  maxHeight );
        }

        // but they can't be GROWN into the long note row
        else {

            r.top  = AT_LEAST( r.top,  FrozenHeight( scrPainter ));
        }
    }
}

//---------------------------------------------------------------------------
// GetNoteRects
//---------------------------------------------------------------------------
void TWeekGrid::GetNoteRects( RTNote note, RTMousInfo mi,
    RECT &growStartR, RECT &dragR, RECT &growEndR, RECT &marqueeR )

{
    if (itsDraggingLongNotes) {

        HGetNoteRects( note, mi, growStartR, dragR, growEndR, marqueeR );
    }
    else {

        VGetNoteRects( note, mi, growStartR, dragR, growEndR, marqueeR );
    }
}

//---------------------------------------------------------------------------
// PointToTime
//---------------------------------------------------------------------------
void TWeekGrid::PointToTime( RTMousInfo mi, RTTime time )
{
    if (itsDraggingLongNotes) {

        HPointToTime( mi, time );
    }
    else {

        VPointToTime( mi, time );
    }
}

//---------------------------------------------------------------------------
// PointToNote
//---------------------------------------------------------------------------
RTNote TWeekGrid::PointToNote( RTMousInfo mi )
{
    if (CellType( mi.row, mi.col ) == kLongNote) {

        SetScrollDirection( kHorizontal );
        itsGrowStartCursor = kGrowLeftCursor;
        itsGrowEndCursor   = kGrowRightCursor;
        itsDragCursor      = kDragBothCursor;
        itsDraggingLongNotes = TRUE;
        return HPointToNote( mi );
    }
    else {

        SetScrollDirection( kBoth );
        itsGrowStartCursor   = kGrowUpCursor;
        itsGrowEndCursor     = kGrowDownCursor;
        itsDragCursor        = kDragBothCursor;
        itsDraggingLongNotes = FALSE;
        return VPointToNote( mi );
    }
}

//---------------------------------------------------------------------------
// TimeToPoint
//---------------------------------------------------------------------------
BOOL TWeekGrid::TimeToPoint( RTTime time, POINT &p )
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
//---------------------------------------------------------------------------
// Release
//---------------------------------------------------------------------------
void TWeekGrid::Release( void )
{
    if (itsDraggingLongNotes) {

        HRelease();
    }
    else {

        VRelease();
    }
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
//          Vertical Dragging
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

//---------------------------------------------------------------------------
// VValidDragCell
//
// Is this a cell that allows dragging?
//---------------------------------------------------------------------------
BOOL TWeekGrid::VValidDragCell( TDragMode mode, int row, int col )
{
    TCellType cellType = CellType( row, col );

    if (mode == kDrag) {

        return (cellType == kNote) || (cellType == kLongNote);
    }
    else {

        return (cellType == kNote);
    }
}


//---------------------------------------------------------------------------
// VSetBoth
//
// Make sure both the start and the end are within the day
//---------------------------------------------------------------------------
void TWeekGrid::VSetBoth( RTMousInfo mi, RTTime start )
{

    // handle the special case where the user has
    // dragged the note into a long note cell and
    // it really represents a whole day
    if (CellType( mi.row, mi.col ) == kLongNote) {

        itsNewTimes.SetTo1DayOn( start );
    }
    else {

        // we have to reset the duration to the
        // original note duration since passing
        // through the long note row sets the
        // duration to 1 day
        itsNewTimes.SetDurationFromNote( itsNoteP, start );
    }
}

//---------------------------------------------------------------------------
// VRelease
//
// to be done when dragging is finished
//---------------------------------------------------------------------------
void TWeekGrid::VRelease( void )
{
    Setup( TSysState::selTime );
}

//---------------------------------------------------------------------------
// VMousDragOver
//
// Makes sure that the long note row doesn't cause a scroll
//---------------------------------------------------------------------------
void TWeekGrid::VMousDragOver( RTMousInfo mi )
{
    TDragGrid::MousDragOver( mi );
    #ifdef NOT_NOW
    if (RowType( mi.row ) == kLongNoteRow) {

        // don't do anything
    }
    else {

        TDragGrid::MousDragOver( mi );
    }
    #endif
}

//---------------------------------------------------------------------------
// VMousDragRelease
//
// Clear the last drag rectangle
//---------------------------------------------------------------------------
void TWeekGrid::VMousDragRelease( RTMousInfo mi )
{

    // if the mouse is released in a long note, then make the note a long
    // note
    if (CellType( mi.row, mi.col ) == kLongNote) {

        TTime start;
        CellToTime( mi.row, mi.col, start );
        itsNewTimes.SetTo1DayOn( start );
    }

    TDragGrid::MousDragRelease( mi );
}

//---------------------------------------------------------------------------
// VPointToTime
//
// Convert a point WITHIN THE GRID OF LIVE CELLS to a time
//
// NB currently assumes we're using SCREEN grid, not print grid
//---------------------------------------------------------------------------
void TWeekGrid::VPointToTime( RTMousInfo mi, RTTime time )
{
    if (CellType( mi.row, mi.col ) != kNote) {

        CellToTime( kNumHeadRows, mi.col, time );
        return;
    }

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
    double fraction   = (y - mi.cellRect.top) / (mi.cellRect.bottom - mi.cellRect.top);
    int    extraMins  = fraction * itsMinutesPerCell;
    extraMins         = extraMins * kMinutesPerTimeUnit[itsPrecision] / kMinutesPerTimeUnit[itsPrecision];

    time.PlusEquals( k1Min, extraMins );
}

//---------------------------------------------------------------------------
// VPointToNote
//
// Convert a point to a note taking into account the entry and slot;
// Allows some slop for grow cursors beyond the ends of the note
//---------------------------------------------------------------------------
RTNote TWeekGrid::VPointToNote( RTMousInfo mi )
{
    int         row, col, entry, slot;
    long        startPos, stopPos;
    TPointStyle style;
    RECT        cellRect;
    PTNote      noteP;
    POINT       p = mi.xy;

    // point is in a note?
    PointToCell( p, row, col, style, cellRect );
    slot  = (p.x - cellRect.left) / kPixelsPerVBar;
    entry = CellToEntry( row, col );
    if (entry != -1) {

        itsEntryTableP->GetEntry( entry, slot, &noteP, startPos, stopPos );
        if (noteP != NULL) {

            return *noteP;
        }
    }

    // point is a few pixels above a note?
    p.y -= kMinDragWidth - 1;
    PointToCell( p, row, col, style, cellRect );
    entry = CellToEntry( row, col );
    if (entry != -1) {

        itsEntryTableP->GetEntry( entry, slot, &noteP, startPos, stopPos );
        if (noteP != NULL) {

            return *noteP;
        }
    }

    // point is a few pixels below a note?
    p.y += 2 * kMinDragWidth - 2;
    PointToCell( p, row, col, style, cellRect );
    entry = CellToEntry( row, col );
    if (entry != -1) {

        itsEntryTableP->GetEntry( entry, slot, &noteP, startPos, stopPos );
        if (noteP != NULL) {

            return *noteP;
        }
    }


    // nope
    return (RTNote) NOOBJECT;
}


//---------------------------------------------------------------------------
// VGetNoteRects
//
// for Milestones, growStartR and growEndR will be empty
// note: dragR no longer includes growStartR and growEndR
//---------------------------------------------------------------------------
void TWeekGrid::VGetNoteRects( RTNote note, RTMousInfo mi,
    RECT &growStartR, RECT &dragR, RECT &growEndR, RECT &marqueeR )
{
    BOOL multiDay = FALSE;
    TBounds bounds( itsFirstCell, itsLastCell );
    TBoundsPositions positions;

    switch (note.GetStyle()) {

        case kItemStyle:
        case kFloatStyle:

            SetRect( &growStartR,  0, 0, -1, -1 );
            SetRect( &dragR,       0, 0, -1, -1 );
            SetRect( &growEndR,    0, 0, -1, -1 );
            SetRect( &marqueeR,    0, 0, -1, -1 );
            return;

        case kMilestoneStyle:
        case kTaskStyle:

            if (!bounds.NoteOverlaps( note, itsNumTimes, positions )) {

                SetRect( &growStartR,  0, 0, -1, -1 );
                SetRect( &dragR,       0, 0, -1, -1 );
                SetRect( &growEndR,    0, 0, -1, -1 );
                SetRect( &marqueeR,    0, 0, -1, -1 );
                return;
            }
            break;
    }

    // get the slot
    int slot       = (mi.xy.x - mi.cellRect.left) / kPixelsPerVBar;
    double vStart;
    double vEnd;

    if ( note.GetStyle() == kTaskStyle) {

        vStart  = positions.startCell % itsLiveRows + positions.startPos;
        vEnd    = positions.endCell   % itsLiveRows + positions.endPos;

        // adjust vStart and vEnd to handle notes that go over 1 day
        if (!note.GetStart().IsSameDay( note.GetEnd() )) {

            multiDay = TRUE;
            int entry = CellToEntry( mi.row, mi.col );
            if ((entry / itsLiveRows) != (positions.startCell / itsLiveRows)) {

                vStart = 0;
            }
            if ((entry / itsLiveRows) != (positions.endCell / itsLiveRows)) {

                vEnd   = itsLiveRows;
            }
        }

    }
    else {

        // since the diamond is in the middle of the cell, make
        // sure the drag rectangle is also
        vStart  = positions.startCell % itsLiveRows + 0.5;
        vEnd    = positions.endCell   % itsLiveRows + 0.5;
    }


    // FIXME: hack to handle the fact that the bottom row will be
    // exactly divisible by itsLiveRows
    if (vEnd == 0) {

        vEnd = itsLiveRows;
    }

    // get logical positions for rect
    int cellHeight = kStdRowHeight + 1 /* for grid line */;
    int top        = vStart * cellHeight;
    int bottom     = vEnd   * cellHeight;
    int left       = mi.cellRect.left + slot * kPixelsPerVBar;
    int right      = left + kPixelsPerVBar;

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
        SetRect( &growStartR,  0, 0, -1, -1 );
        SetRect( &growEndR,    0, 0, -1, -1 );
        marqueeR = dragR;
    }


    // notes with frequencies or that go over more than one day
    // have no grow rectangles
    if ((note.GetFreq() != NULL) || (multiDay)) {

        SetRect( &growStartR,  0, 0, -1, -1 );
        SetRect( &growEndR,    0, 0, -1, -1 );
        marqueeR = dragR;
    }

}


//---------------------------------------------------------------------------
// VScrollStart
//
// scroll up and down only
//---------------------------------------------------------------------------
void TWeekGrid::VScrollStart( RTMousInfo , int deltaRow, int  )
{
    if (deltaRow > 0) {

        itsNewTimes.SetStartToStartOfDayAndKeepEnd();

        if (vScrollPos > 0) {

            // adjust the marquee
            itsMarqueeRect.bottom += kStdRowHeight;
            itsMarqueeRect.bottom = min( itsMarqueeRect.bottom, winHeight );

            Scroll( deltaRow, 0 );
            UpdateWindow( HWindow );
        }
    }
    else if (deltaRow < 0) {

        itsNewTimes.SetStartToJustBeforeEnd( k1Min, itsMinutesPerCell );

        // scroll down only as far as the end time
        int row, col;
        TimeToCell( itsNewTimes.GetEnd(), row, col );
        if (!IsCellVisible( row, col )) {

            Scroll( deltaRow, 0 );
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
// VScrollEnd
//
// Scrolling is only allowed up and down
//---------------------------------------------------------------------------
void TWeekGrid::VScrollEnd( RTMousInfo , int deltaRow, int  )
{
    if (deltaRow < 0) {

        itsNewTimes.SetEndToEndOfDayAndKeepStart();

        if (vScrollPos < vScrollMax) {

            // adjust the marquee
            itsMarqueeRect.top -= kStdRowHeight;
            itsMarqueeRect.top = max( itsMarqueeRect.top, FrozenHeight( scrPainter ));

            Scroll( deltaRow, 0 );
            UpdateWindow( HWindow );
        }
    }
    else if (deltaRow > 0) {

        itsNewTimes.SetEndToJustAfterStart( k1Min, itsMinutesPerCell );

        // scroll up only as far as the start time
        int row, col;
        TimeToCell( itsNewTimes.GetStart(), row, col );
        if (!IsCellVisible( row - 1, col )) {

            Scroll( deltaRow, 0 );
            UpdateWindow( HWindow );
        }


        // adjust the marquee
        POINT p;
        if (TimeToPoint( itsNewTimes.GetStart(), p ) && (itsMarqueeRect.top < p.y)) {

            itsMarqueeRect.top = p.y;
        }
    }
}

//---------------------------------------------------------------------------
// VScrollBoth
//
// scroll and adjust the start and end times to match the new scroll pos
//---------------------------------------------------------------------------
void TWeekGrid::VScrollBoth( RTMousInfo mi, int deltaRow, int deltaCol )
{
    // force the start to midnight if we're scrolling up,
    // or the end to midnight if we're scrolling down
    if (deltaRow > 0) {

        // since the note was probably turned into
        // a long note on our way to scrolling,
        // we have to make it a short note again,
        // and retrieve the duration from its original note
        TTime start;
        if (mi.col == -1) {

            // we're outside the grid: use whatever
            // day we used before
            start = itsNewTimes.GetStart();
            start.SetToMidnight();
        }
        else {

            CellToTime( kNumHeadRows, mi.col, start );
        }

        itsNewTimes.SetDurationFromNote( itsNoteP, start );
        itsNewTimes.SetStartToStartOfDayAndKeepDuration();
    }
    else if (deltaRow < 0) {

        itsNewTimes.SetEndToEndOfDayAndKeepDuration();
    }


    // scroll left
    if ((deltaCol > 0) && (hScrollPos > 0)) {

        TTime start( itsNewTimes.GetStart());
        start.PlusEquals( k1Day, -1 );
        itsNewTimes.SetStartAndKeepDuration( start );
    }

    // scroll right
    // use hScrollMax - 1 to handle the fact that we scroll by 2
    else if ((deltaCol < 0) && (hScrollPos < hScrollMax - 1)) {

        TTime start( itsNewTimes.GetStart());
        start.PlusEquals( k1Day, 1 );
        itsNewTimes.SetStartAndKeepDuration( start );
    }

    // scroll
    Scroll( deltaRow, deltaCol );
    UpdateWindow( HWindow );
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
//          Horizontal Dragging
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

//---------------------------------------------------------------------------
// HValidDragCell
//
// Is this a cell that allows dragging?
//---------------------------------------------------------------------------
BOOL TWeekGrid::HValidDragCell( TDragMode , int row, int col )
{
    TCellType cellType = CellType( row, col );

    return (cellType == kNote) || (cellType == kLongNote);
}


//---------------------------------------------------------------------------
// HSetBoth
//
//  Sets info for dragging, based on the new start
//  time and the cell that the mouse is in
//---------------------------------------------------------------------------
void TWeekGrid::HSetBoth( RTMousInfo mi, RTTime start )
{
    // handle the special case where the user has
    // dragged the note into a regular note cell
    // and it really represents a whole day
    if (CellType( mi.row, mi.col ) == kNote) {

        TTime realStart;
        CellToTime( mi.row, mi.col, realStart );
        itsNewTimes.SetTo2HoursAt( realStart );
    }
    else {

        // we have to reset the duration to the
        // original note duration since passing
        // through the short note row sets
        // the duration to 2 hours
        itsNewTimes.SetDurationFromNote( itsNoteP, start );
    }
}

//---------------------------------------------------------------------------
// HRelease
//
// to be done when dragging is finished
//---------------------------------------------------------------------------
void TWeekGrid::HRelease( void )
{
    Setup( TSysState::selTime );
}

//---------------------------------------------------------------------------
// HMousDragOver
//
// Makes sure that the long note row doesn't cause a scroll
//---------------------------------------------------------------------------
void TWeekGrid::HMousDragOver( RTMousInfo mi )
{
    TDragGrid::MousDragOver( mi );
}

//---------------------------------------------------------------------------
// HMousDragRelease
//
// Clear the last drag rectangle
//---------------------------------------------------------------------------
void TWeekGrid::HMousDragRelease( RTMousInfo mi )
{

    // if the mouse is released in a note cell, then make the note short
    // note
    if (CellType( mi.row, mi.col ) == kNote) {

        TTime start;
        CellToTime( mi.row, mi.col, start );
        itsNewTimes.SetTo2HoursAt( start );
    }

    TDragGrid::MousDragRelease( mi );
}

//---------------------------------------------------------------------------
// HPointToTime
//
// Convert a point WITHIN THE GRID OF LIVE CELLS to a time
//
// NB currently assumes we're using SCREEN grid, not print grid
//---------------------------------------------------------------------------
void TWeekGrid::HPointToTime( RTMousInfo mi, RTTime time )
{
    // somewhere in the regular note grid
    if (CellType( mi.row, mi.col ) != kLongNote) {

        CellToTime( kNumHeadRows, mi.col, time );
        return;
    }

    // get base time of cell
    CellToTime( mi.row, mi.col, time );

    // calculate the extra minutes within the cell
    // calculations are done based on the 
    // stripe and the main cell being treated as
    // a single cell in order to avoid inaccuracy
    RECT twoCellRect = mi.cellRect;
    if ((mi.col % 2) == 0) {

       // we're in a big cell, grow the rectangle
       // to include the little cell to the left
       twoCellRect.left -= CellWidth( scrPainter, mi.col - 1 );
       twoCellRect.left -= GridWidth( scrPainter, mi.col - 1 );
    }
    else {

       // we're in a little cell, grow the rectangle
       // to include the big cell to the right
       twoCellRect.right += CellWidth( scrPainter, mi.col + 1 );
       twoCellRect.right += GridWidth( scrPainter, mi.col );
    }
    long twoCellWidth = twoCellRect.right - twoCellRect.left;
    long extra        = mi.xy.x - twoCellRect.left;
    long extraMinutes = kMinutesPerDay * extra / twoCellWidth;
    time.PlusEquals( k1Min, (int) extraMinutes );
}

//---------------------------------------------------------------------------
// HPointToNote
//
// Convert a point to a note taking into account the entry and slot;
// Allows some slop for grow cursors beyond the ends of the note
//---------------------------------------------------------------------------
RTNote TWeekGrid::HPointToNote( RTMousInfo mi )
{
    int         row, col, slot;
    TPointStyle style;
    RECT        cellRect;
    POINT       p = mi.xy;
    int         dayIndex;

    // get the note corresponding to this slot
    PointToCell( p, row, col, style, cellRect );
    slot  = (p.y - cellRect.top) / kSmallFontHeight;
    if (slot >= itsNumLongNotes) {

        return (RTNote) NOOBJECT;
    }


    dayIndex  = (col - kNumHeadCols) / 2;
    TLongNote &longNote  = LongNoteInRow( slot );
    if ((longNote.itsStartCol <= dayIndex) &&
        (dayIndex <= longNote.itsEndCol)) {

        return *longNote.itsNoteP;
    }

    // point is a few pixels left of a note?
    p.x -= kMinDragWidth - 1;
    PointToCell( p, row, col, style, cellRect );
    dayIndex  = (col - kNumHeadCols) / 2;
    longNote = LongNoteInRow( slot );
    if ((longNote.itsStartCol <= dayIndex) &&
        (dayIndex <= longNote.itsEndCol)) {

        return *longNote.itsNoteP;
    }

    // point is a few pixels right of a note?
    p.x += 2 * kMinDragWidth - 2;
    PointToCell( p, row, col, style, cellRect );
    dayIndex  = (col - kNumHeadCols) / 2;
    longNote = LongNoteInRow( slot );
    if ((longNote.itsStartCol <= dayIndex) &&
        (dayIndex <= longNote.itsEndCol)) {

        return *longNote.itsNoteP;
    }


    // nope
    return (RTNote) NOOBJECT;
}


//---------------------------------------------------------------------------
// HGetNoteRects
//
// for Milestones, growStartR and growEndR will be empty
// note: dragR no longer includes growStartR and growEndR
//---------------------------------------------------------------------------
void TWeekGrid::HGetNoteRects( RTNote note, RTMousInfo mi,
    RECT &growStartR, RECT &dragR, RECT &growEndR, RECT &marqueeR )
{
    // horizontal notes should never have frequencies anyway, but...
    if (note.GetFreq() != NULL) {

        SetRect( &growStartR,  0, 0, -1, -1 );
        SetRect( &dragR,       0, 0, -1, -1 );
        SetRect( &growEndR,    0, 0, -1, -1 );
        SetRect( &marqueeR,    0, 0, -1, -1 );
        return;
    }

    int slot  = (mi.xy.y - mi.cellRect.top) / kSmallFontHeight;
    if ((slot >= itsNumLongNotes ) ||    // slot is past last note
        (slot == kMaxLongNotes - 1)) {   // slot is in 'more' row

        SetRect( &growStartR, 0, 0, -1, -1 );
        SetRect( &dragR,      0, 0, -1, -1 );
        SetRect( &growEndR,   0, 0, -1, -1 );
        SetRect( &marqueeR,   0, 0, -1, -1 );
        return;
    }

    TLongNote &longNote = LongNoteInRow( slot );
    int startCol        = longNote.itsStartCol;
    int endCol          = longNote.itsEndCol;
    double startPos     = longNote.itsStartPos;
    double endPos       = longNote.itsEndPos;

    // get logical positions for rect
    int cellWidth = CellWidth( scrPainter, itsNumDays )     +
                    CellWidth( scrPainter, itsNumDays + 1)  + 2;
    int top       = mi.cellRect.top + slot * kSmallFontHeight;
    int bottom    = top + kSmallFontHeight;
    int left      = (startCol + startPos) * cellWidth;
    int right     = (endCol   + endPos)   * cellWidth;

    // modify logical positions to allow for the fact that the widths of
    // the frozen cols must be added since they are not part of the
    // logical width, but the width of the columns
    // which are scrolled off to the left must be subtracted, since they
    // don't actually appear on the screen but ARE part of the logical width
    int headWidth = FrozenWidth( scrPainter );
    left         += headWidth - (hScrollPos / 2) * cellWidth;
    right        += headWidth - (hScrollPos / 2) * cellWidth;

    // create the rectangles
    if (note.GetStyle() == kTaskStyle) {

        SetRect( &dragR,       left, top, right, bottom );
        SetRect( &growStartR,  left - kMinDragWidth, top, left, bottom );
        SetRect( &growEndR,    right, top, right + kMinDragWidth + 1, bottom );
        marqueeR = dragR;
    }
    else {

        SetRect( &dragR, left - kHalfMinDragWidth, top, right + kHalfMinDragWidth, bottom );
        SetRect( &growStartR,  0, 0, -1, -1 );
        SetRect( &growEndR,    0, 0, -1, -1 );
        marqueeR = dragR;
    }
}


//---------------------------------------------------------------------------
// HScrollStart
//
// scrolling is only allowed horizontally
//---------------------------------------------------------------------------
void TWeekGrid::HScrollStart( RTMousInfo , int , int deltaCol )
{
    if (deltaCol > 0) {

        // adjust the start by offsetting its time
        if (hScrollPos > 0) {

            TTime start( itsNewTimes.GetStart());
            start.PlusEquals( k1Day, -1 );
            itsNewTimes.SetStartAndKeepEnd( start, k1Day, 1 );
            itsNewTimes.ConstrainToInterval(
                itsFirstCell, itsLastCell );

            // adjust the marquee
            itsMarqueeRect.right += CellWidth( scrPainter, 2 /* arbitrary col */ );
            itsMarqueeRect.right = min( itsMarqueeRect.right, winWidth );
        }

        Scroll( 0, deltaCol );
        UpdateWindow( HWindow );
    }
    else if (deltaCol < 0) {

        // scroll only as far as the end time
        int row, col;
        TimeToCell( itsNewTimes.GetEnd(), row, col );
        if (!IsCellVisible( row, col + 2 )) {

            // adjust the start time to be just before the end time
            itsNewTimes.SetStartToJustBeforeEnd( k1Day, 1 );

            Scroll( 0, deltaCol );
            UpdateWindow( HWindow );
        }

        // adjust the marquee
        POINT p;
        if (TimeToPoint( itsNewTimes.GetEnd().Plus( k1Day, 1 ), p ) &&
            (itsMarqueeRect.right > p.x)) {

            itsMarqueeRect.right = p.x;
        }
    }
}

//---------------------------------------------------------------------------
// HScrollEnd
//
// scrolling is only allowed horizontally
//---------------------------------------------------------------------------
void TWeekGrid::HScrollEnd( RTMousInfo , int , int deltaCol )
{
    if (deltaCol < 0) {

        // adjust the end by offsetting its time (use hScroll - 1 in
        // order to deal with scrolling by 2)
        if (hScrollPos < hScrollMax - 1) {

            TTime end( itsNewTimes.GetEnd());
            end.PlusEquals( k1Day, 1 );
            itsNewTimes.SetEndAndKeepStart( end, k1Day, 1 );
            itsNewTimes.ConstrainToInterval(
                itsFirstCell, itsLastCell );

            // adjust the marquee
            int frozenWidth      = FrozenWidth( scrPainter );
            itsMarqueeRect.left -= CellWidth( scrPainter, 2 /* arbitrary col */ );
            itsMarqueeRect.left  = max( itsMarqueeRect.left, frozenWidth );
        }

        Scroll( 0, deltaCol );
        UpdateWindow( HWindow );
    }
    else if (deltaCol > 0) {


        // scroll only as far as the start time
        int row, col;
        TimeToCell( itsNewTimes.GetStart(), row, col );
        if (!IsCellVisible( row, col - 2)) {

            // adjust the end time to be just after the start time
            itsNewTimes.SetEndToJustAfterStart( k1Day, 1 );

            Scroll( 0, deltaCol );
            UpdateWindow( HWindow );
        }

        // adjust the marquee
        POINT p;
        if (TimeToPoint( itsNewTimes.GetStart(), p ) &&
            (itsMarqueeRect.left < p.x)) {

            itsMarqueeRect.left = p.x;
        }
    }
}

//---------------------------------------------------------------------------
// HScrollBoth
//
// scroll and adjust the start and end times to match the new scroll pos
//---------------------------------------------------------------------------
void TWeekGrid::HScrollBoth( RTMousInfo , int deltaRow, int deltaCol )
{
    // force the start to midnight if we're scrolling up,
    // or the end to midnight if we're scrolling down
    if (deltaRow > 0) {

        itsNewTimes.SetStartToStartOfDayAndKeepDuration();
    }
    else if (deltaRow < 0) {

        itsNewTimes.SetEndToEndOfDayAndKeepDuration();
    }

    // scroll left: decrement the day
    if ((deltaCol > 0) && (hScrollPos > 0)) {

        TTime start( itsNewTimes.GetStart() );
        start.PlusEquals( k1Day, -1 );
        itsNewTimes.SetStartAndKeepDuration( start );
    }

    // scroll right: increment the day
    // uses hScrollMax - 1 to handle the fact that we scroll by
    // 2 in the weekgrid
    else if ((deltaCol < 0) && (hScrollPos < hScrollMax - 1)) {

        TTime start( itsNewTimes.GetStart() );
        start.PlusEquals( k1Day, 1 );
        itsNewTimes.SetStartAndKeepDuration( start );
    }

    // scroll
    Scroll( deltaRow, deltaCol );
    UpdateWindow( HWindow );
}

/////////////////////////////////////////////////////////////////////////////
////////////////////////////// FOCUS ////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//*****************************************************************************
//set focus to first time, first day
//*****************************************************************************
void TWeekGrid::GetDefaultFocusCell( int &row, int &col )
{
row= vScrollPos + frozenRows;
col= hScrollPos + frozenCols + 1;
}

//*****************************************************************************

//*****************************************************************************
void TWeekGrid::MousKey( TMousKey key )
{
//horz scroll is different here; we need to shift 2 cols, not 1
if( key == kScrLeftKey )
    Scroll( 0, 2 );
else if( key == kScrRightKey )
    Scroll( 0, -2 );
else
    TMousGrid::MousKey( key );
}



// -----------------------------------------------------
// RowsForWeek
//
//  Return the number of rows that are necessary
//  for the week that starts in this dayIndex
// -----------------------------------------------------
int TWeekGrid::RowsForWeek( int index, BOOL forPrinter )
{
    int offset = forPrinter ? 6 : 4;

    //assumption: notes are sorted in order of their start date.
    //Find the first one that's in this week, then the last one that
    //is in this week.

    int n= itsLongNoteTableP->GetCount();
    //first and last must work correctly for n=0, and
    //also if no notes are in set
    int first= n;
    int last= -1;

    for (int i = 0; i < n; i++)
        {
        RTLongNote note = (RTLongNote) (*itsLongNoteTableP)[i];

        if( (note.itsEndCol >= index) &&
            ( note.itsStartCol <= index + offset ))
            {
            if( first > i )
                first= i;
            if( last < i )
                last= i;
            }

        }

    itsFirstVisibleLongNote = first;
    return max( 0, (last - first + 1));
}

// -----------------------------------------------------
// LongNoteInRow
// -----------------------------------------------------
RTLongNote TWeekGrid::LongNoteInRow( int row )
{
    return (RTLongNote)
        (*itsLongNoteTableP)[row + itsFirstVisibleLongNote];
}

// -----------------------------------------------------
// WeekGridNewPageHook
//
//  A page of the grid is about to be printed. Make
//  sure the long notes are set up properly
// -----------------------------------------------------
void TWeekGrid::WeekGridNewPageHook( void )
{
    itsNumLongNotes = min( kMaxLongNotes,
        RowsForWeek( hScrollPos / 2, TRUE ));
    InvalidateCell( -1, -1 );
}

//**************************************************************************
//
//                      TLongNote
//
//**************************************************************************
// -----------------------------------------------------
// constructor
// -----------------------------------------------------
TLongNote::TLongNote( void )
{
    itsNoteP     = NULL;
    itsRow       = -1;
    itsStartCol  = -1;
    itsEndCol    = -1;
    itsStartPos  = -1;
    itsEndPos    = -1;
}

// -----------------------------------------------------
// destructor
// -----------------------------------------------------
TLongNote::~TLongNote( void )
{
}

classType TLongNote::isA( void ) const           { return IDCat;           }
char * TLongNote::nameOf( void ) const           { return "TLongNote";          }
hashValueType TLongNote::hashValue( void ) const { return 0;               }
void TLongNote::printOn(ostream far&) const      {                         }

// -----------------------------------------------------
// isEqual
//
//  Two long notes are considered equal if their
// columns and positions are the same
// -----------------------------------------------------
int TLongNote::isEqual(const Object & obj) const
{
    RTLongNote other = (RTLongNote) obj;

    return (itsStartCol == other.itsStartCol) &&
           (itsEndCol   == other.itsEndCol)   &&
           (itsStartPos == other.itsStartPos) &&
           (itsEndPos   == other.itsEndPos);
}

// -----------------------------------------------------
// isLessThan
// -----------------------------------------------------
int TLongNote::isLessThan(const Object & obj) const
{
    RTLongNote other = (RTLongNote) obj;

    if (itsStartCol < other.itsStartCol) return TRUE;
    if (itsStartCol > other.itsStartCol) return FALSE;

    if (itsEndCol   < other.itsEndCol)   return TRUE;
    if (itsEndCol   > other.itsEndCol)   return FALSE;

    if (itsStartPos < other.itsStartPos) return TRUE;
    if (itsStartPos > other.itsStartPos) return FALSE;

    if (itsEndPos   < other.itsEndPos)   return TRUE;
    if (itsEndPos   > other.itsEndPos)   return FALSE;

    return (*itsNoteP) < (*other.itsNoteP);
}
