//*****************************************************************************
// timeline view
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "tmlinegr.h"

#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __frame__
#include "frame.h"
#endif
#ifndef __noteview__
#include "noteview.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __error__
#include "error.h"
#endif
#ifndef __pen__                       
#include "pen.h"
#endif
#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __paint__
#include "paint.h"
#endif
#ifndef __print__
#include "print.h"
#endif
#ifndef __bounds__
#include "bounds.h"
#endif
#ifndef __gridutil__
#include "gridutil.h"
#endif
#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef  __cursor__
#include "cursor.h"
#endif
#ifndef __notemenu__
#include "notemenu.h"
#endif
#ifndef __notetab__
#include "notetab.h"
#endif
#ifndef __catlist__
#include "catlist.h"
#endif
#ifndef __filter__
#include "filter.h"
#endif
#ifndef __moushelp__
#include "moushelp.h"
#endif
#ifndef __profile__
#include "profile.h"
#endif

static const int kNumHeadRows      = 1;
static const int kNumHeadCols      = 2;
static const int kNumLiveCols      = 200;
static const int kHeadlineCol      = 1;
static const int kMarkSize         = 5;   // mark is really twice markSize
static const int kTaskHeight       = 9;
static const int kMinDragWidth     = 10;
static const int kHalfMinDragWidth = kMinDragWidth / 2;
static const int kClearEdgePixels  = 30;

static const kColorColWidth        = 19;
static const kMinNoteColWidth      = 153;
static const kTimeColWidth         = 47;

int TTimeLineGrid::itsTimeUnitsPerCell[] = { 4, 1, 1, 1 };

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
//          Constructors
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------


//*****************************************************************************
//  Constructor
//*****************************************************************************
TTimeLineGrid::TTimeLineGrid( PTWindowsObject AParent ):
    TDragGrid( AParent, kHorizontal, 1 )
{
    // create painters
    scrPainter = new TPaint;
    prtPainter = new TPaintPrt;

    // initialize the grid
    SetRowFreeze( kNumHeadRows );
    SetColFreeze( kNumHeadCols );

    SetPrtTitle( "Timeline" );
    SetDefaultPrintRange();

    itsNoteTableP = new TNoteTable;
    Reset();
}

//*****************************************************************************
// Destructor
//*****************************************************************************
TTimeLineGrid::~TTimeLineGrid( void )
{
    CloseDragWindow();

    delete scrPainter;
    delete prtPainter;

    itsNoteTableP->Flush( TXArray::kDelete );
    delete itsNoteTableP;
}


// ----------------------------------------------------------------------
// Reset
// ----------------------------------------------------------------------
void TTimeLineGrid::Reset( void )
{
    // clear out old info
    itsNoteTableP->Flush( TXArray::kDelete );
    if (TSysState::CatGrouping == kNoCatGrouping)
        ResetNotesNoGrouping();
    else
        ResetNotesWithGrouping();

    // normalize itsStartTime; make sure it has a time;
    // fit the right number of time units into the cell
    int timesPerCell = itsTimeUnitsPerCell[TSysState::timeScale];
    itsStartTime= TSysState::selTime;
    itsStartTime.Normalize( TSysState::timeScale );
    itsStartTime.SetHour( (itsStartTime.GetHour() / timesPerCell) * timesPerCell );
    itsStartTime.SetMinute( 0 );

    // set up itsFirstCell and itsLastCell
    int midCol   = kNumLiveCols / 2;
    itsLastCell  = itsStartTime.Plus( TSysState::timeScale,  midCol * timesPerCell );
    itsFirstCell = itsStartTime.Plus( TSysState::timeScale, -midCol * timesPerCell );

    SetColCnt(    kNumLiveCols + kNumHeadCols );

    // goto the midCol
    int initialRow = max( 0, min( TSysState::currentNoteIndex, itsRowCount ));
    GotoRC( initialRow, midCol );

    InvalidateFocus();
    InvalidateCell( -1, -1 );

}

//*****************************************************************************
// ResetNotesNoGrouping
//*****************************************************************************
void TTimeLineGrid::ResetNotesNoGrouping( void )
{
    // header plus 1 row per note
    itsRowCount  = itsNoteTableP->Add( "" );
    itsRowCount += itsNoteTableP->Add(
        TSysState::pSelNotes, NULL, FALSE, FALSE );

    SetRowCnt( itsRowCount );
}


//*****************************************************************************
// ResetNotesWithgrouping
//*****************************************************************************
void TTimeLineGrid::ResetNotesWithGrouping( void )
{
//first, get rid of old info
itsNoteTableP->Flush( TXArray::kDelete );

// put in a dummy header row for the column headings
itsRowCount = itsNoteTableP->Add( "" );

// fill itsNoteTableP with group notes
itsRowCount += itsNoteTableP->AddWithGrouping( FALSE );

SetRowCnt( itsRowCount );
}


// ----------------------------------------------------------------------
// GetCellStart
//
// Default is to ignore row
// ----------------------------------------------------------------------
TTime TTimeLineGrid::GetCellStart( int /* row */, int col )
{
    return itsFirstCell.Plus( TSysState::timeScale,
        (col - frozenCols) * itsTimeUnitsPerCell[TSysState::timeScale] );
}

// ----------------------------------------------------------------------
// GetCellEnd
//
// Default is to ignore row
// ----------------------------------------------------------------------
TTime TTimeLineGrid::GetCellEnd( int /* row */, int col )
{
    return itsFirstCell.Plus( TSysState::timeScale,
        (col - frozenCols + 1) * itsTimeUnitsPerCell[TSysState::timeScale] );
}

// ----------------------------------------------------------------------
// GetCellTimes
//
// Default is to ignore row
// ----------------------------------------------------------------------
void TTimeLineGrid::GetCellTimes( int /* row */, int col, RTTime start, RTTime end )
{
    start = itsFirstCell.Plus( TSysState::timeScale,
        (col - frozenCols)     * itsTimeUnitsPerCell[TSysState::timeScale] );
    end   = itsFirstCell.Plus( TSysState::timeScale,
        (col - frozenCols + 1) * itsTimeUnitsPerCell[TSysState::timeScale] );

}

// ----------------------------------------------------------------------
// TimeOffset
// ----------------------------------------------------------------------
TTime TTimeLineGrid::TimeOffset( RTTime time, int offset )
{
    return time.Plus( TSysState::timeScale,
        offset * itsTimeUnitsPerCell[TSysState::timeScale] );
}

//---------------------------------------------------------------------------
// TimeToCell
//---------------------------------------------------------------------------
void TTimeLineGrid::TimeToCell( RTTime time, int &row, int &col )
{
    row  = itsDragRow;

    TDuration duration( itsFirstCell, time );
    switch (TSysState::timeScale) {

        case SCALE_HOUR:  col = (int) duration.GetNumUnits( k1Hour ); break;
        case SCALE_DAY:   col = (int) duration.GetNumUnits( k1Day  ); break;
        case SCALE_WEEK:  col = (int) duration.GetNumUnits( k1Week ); break;
        case SCALE_MONTH: col = itsFirstCell.MonthDelta( time );
    }

    col /= itsTimeUnitsPerCell[TSysState::timeScale];
    col += frozenCols;
}

//---------------------------------------------------------------------------
// TimeToPoint
//---------------------------------------------------------------------------
BOOL TTimeLineGrid::TimeToPoint( RTTime time, POINT &p )
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

    double deltaMinutes = deltaDuration.GetNumUnits( k1Min );
    double cellMinutes  = cellDuration.GetNumUnits( k1Min );
    double fraction     = deltaMinutes / cellMinutes;

    p.y = cellR.bottom;
    p.x = cellR.left + kTimeColWidth * fraction;

    return TRUE;
}

// ----------------------------------------------------------------------
// FormatTime
// ----------------------------------------------------------------------
void TTimeLineGrid::FormatTime( RTTime time, char *buf )
{
char s0[50];
char s1[50];

switch( (TSysState::timeScale) )
    {
    case SCALE_HOUR:
        TIntlFmt::FormatTime( time, TRUE, s0 );
        TIntlFmt::FormatDayAndMonth( time, s1 );
        wsprintf( buf, "%s\n%s", s0, s1 );
        break;
    case SCALE_DAY:
        TIntlFmt::FormatDayName( time, kThreeLetter, s0 );
        TIntlFmt::FormatDayAndMonth( time, s1 );
        wsprintf( buf, "%s\n%s", s0, s1 );
        break;
    case SCALE_WEEK:
        TIntlFmt::FormatDayAndMonth( time, s0 );
        TIntlFmt::FormatYear( time, s1 );
        wsprintf( buf, "%s\n%s", s0, s1 );
        break;
    case SCALE_MONTH:
        TIntlFmt::FormatMonthName( time, kThreeLetter, s0 );
        TIntlFmt::FormatYear( time, s1 );
        wsprintf( buf, "%s\n%s", s0, s1 );
        break;
    case SCALE_QUARTER:
        TIntlFmt::FormatQuarter( time, s0 );
        TIntlFmt::FormatYear( time, s1 );
        wsprintf( buf, "%s\n%s", s0, s1 );
        break;
    case SCALE_YEAR:
        TIntlFmt::FormatYear( time, s1 );
        wsprintf( buf, "\n%s", s1 );
        break;
    default:
        break;  //error
    }

}

//*****************************************************************************
// GetNote
//
//  Get the note corresponding to the row
//*****************************************************************************
RTNote TTimeLineGrid::GetNote( int row )
{
if( row > 0 && row < itsRowCount )
    {
    RTNoteRow noteRow = (RTNoteRow) (*itsNoteTableP)[row];
    return *noteRow.itsNoteP;
    }
else
    return (RTNote) NOOBJECT;
}

//*****************************************************************************
// IsNoteRow
//
//  Is the row filled with a real note (as opposed
//  to a header)?
//*****************************************************************************
BOOL  TTimeLineGrid::IsNoteRow( int row )
{
if( row > 0 && row < itsRowCount )
    {
    RTNoteRow noteRow = (RTNoteRow) (*itsNoteTableP)[row];
    return noteRow.itsRowType == kNoteTableNote;
    }
else
    return FALSE;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
//          Painting
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

//*****************************************************************************
// CellWidth
//
// NB: this function assumes that all non-header cells are the same width.
// If this changes so that cells can be of different widths, then
// GetNoteRects must be changed.)
//*****************************************************************************
int TTimeLineGrid::CellWidth( PTPaint pPainter, int col )
{

// for printer, size columns so we get an exact fit on each page
if( pPainter->IsPrintPainter() )
    {

    switch( ColType( col ))
        {

        case kColorCol:
            return kColorColWidth;
        case kNoteCol:
            RECT r;
            TPrint::GetPrtPixels( &r );
            int availWidth= r.right - r.left;

            availWidth-= leftMargin + LeftLineWidth( pPainter );
            availWidth-= kColorColWidth + GridWidth( pPainter, kColorCol );
            availWidth-= kMinNoteColWidth + GridWidth( pPainter, kNoteCol );

            //how many timecols fit?
            int nTimeCols = availWidth / ( kTimeColWidth + GridWidth( pPainter, kTimeCol ) );

            //add slop back into note column
            return ( kMinNoteColWidth + availWidth
                     - nTimeCols * ( kTimeColWidth + GridWidth( pPainter, kTimeCol ) ));

        case kTimeCol:
            return kTimeColWidth;

        default:

            SYSERR_STR( "CellWidth: unexpected case in switch statement." );
            return 0;
        }
    }
else
    {
    switch( ColType( col ))
        {

        case kColorCol:    return kColorColWidth;
        case kNoteCol:     return kMinNoteColWidth;
        case kTimeCol:     return kTimeColWidth;
        default:

            SYSERR_STR( "CellWidth: unexpected case in switch statement." );
            return 0;
        }
    }
}

//*****************************************************************************

//*****************************************************************************
int TTimeLineGrid::CellHeight( PTPaint pPainter, int row )
{
    const int defHeight= 19;

    switch (RowType( row )) {

        case kHeaderRow:    
        case kTitleRow:    return 2 * defHeight;
        case kTimeRow:

            if (TSysState::bShowFullText) {

                RTNote note = GetNote( row );

                if (TPen::bPenActive && note.GetInk().HasInk() &&
                    (0 == note.GetText().Len()) ) {

                    RECT r;
                    note.GetInk().Measure( &r );
                    return max( defHeight, r.bottom - r.top );
                }
                else {

                    int width= CellWidth( pPainter, kHeadlineCol );

                    int h;
                    h= pPainter->CalcHeight( note.GetText().Addr(), width, kNormalFont );
                    //limit height of any one note to 90% of window height
                    RECT pageSize;
                    if( pPainter->IsPrintPainter() )
                        TPrint::GetPrtPixels( &pageSize );
                    else
                        GetClientRect( HWindow, &pageSize );

                    h= min( h, (int) (0.9 * (pageSize.bottom - pageSize.top)) );

                    h= max( defHeight, h );
                    return h;
                }
            }
            else {

                return defHeight;
            }

        default:

            //SYSERR_STR( "Cell Height: unexpected case in switch" );
            return 0;
    }
}


//*****************************************************************************

//*****************************************************************************
int  TTimeLineGrid::GridWidth( PTPaint , int col )
{
    switch (ColType( col )) {

        case kColorCol:  return 1;
        case kNoteCol:   return 2;
        case kTimeCol:   return 1;
        default:

            SYSERR_STR( "GridWidth: unexpected case" );
            return 0;
    }
}

//*****************************************************************************

//*****************************************************************************
int  TTimeLineGrid::GridHeight( PTPaint , int row )
{
if (RowType( row + 1 ) == kTitleRow)
    return 2;
else if (RowType( row ) == kHeaderRow)
    return 2;
else
    return 1;
}

//*****************************************************************************
// PaintCell
//*****************************************************************************
void TTimeLineGrid::PaintCell( RTDrawInfo di )
{
    switch( CellType( di.row, di.col )) {

        case kColorHeader:                               break;
        case kNoteHeader:        PaintNoteHeader( di );  break;
        case kTimeHeader:        PaintTimeHeader( di );  break;
        case kColorTitle:        
        case kNoteTitle:         
        case kTimeTitle:         PaintNoteTitle( di );   break;
        case kColor:             PaintColor( di );       break;
        case kNote:              PaintNote( di );        break;
        case kTime:              PaintTime( di );        break;
        default:

            SYSERR_STR( "PaintCell: unexpected case" );
    }
}

//*****************************************************************************
// PaintColor
//*****************************************************************************
void TTimeLineGrid::PaintColor( RTDrawInfo di )
{
    RTNote note = GetNote( di.row );
    di.pPainter->DrawMark( di, note );
}

//*****************************************************************************
// PaintNoteHeader
//*****************************************************************************
void TTimeLineGrid::PaintNoteHeader( RTDrawInfo di )
{
    di.pPainter->Text( di, "\nNote", kNoHilite, kMultiLine | DT_CENTER, kBoldFont );
}

//*****************************************************************************
// PaintNote
//*****************************************************************************
void TTimeLineGrid::PaintNote( RTDrawInfo di )
{
    char buf[200];
    TSafStr title;

    RTNote note= GetNote( di.row );

    note.Title( title );
    title.Get( buf, sizeof( buf ) );
    if( !buf[0] && note.GetInk().HasInk() )
        note.GetInk().Show( di.hdc, &di.cellRect );
    else
        {
        TFont font= (note.IsDone()) ? kStrikeFont : kNormalFont;

        RECT r = di.cellRect;

        if( TSysState::bShowFullText )
            di.pPainter->Text( di, (char *) note.GetText().Addr(), kNoHilite, kMultiLine, font, &r );
        else
            di.pPainter->Text( di, buf, kNoHilite, kSingleLine | DT_LEFT, font, &r );
        }
}

//*****************************************************************************
// PaintTimeHeader
//*****************************************************************************
void TTimeLineGrid::PaintTimeHeader( RTDrawInfo di )
{
    char buf[200];
    TTime cellStart = GetCellStart( di.row, di.col );

    FormatTime( cellStart, buf );
    TGridUtil::PaintBackground( di, cellStart, TRUE, FALSE );

    RECT r= di.cellRect;
    r.top += 5; //text looks better when moved down
    di.pPainter->Text( di, buf, kNoHilite, kMultiLine | DT_CENTER, kBoldFont, &r );
}

//*****************************************************************************
// PaintNoteTitle
//*****************************************************************************
void  TTimeLineGrid::PaintNoteTitle( RTDrawInfo di )
{
    TSafStr buf;

    if( CellType( di.row, di.col ) == kNoteTitle)
        {
        RTNoteRow noteRow = (RTNoteRow) (*itsNoteTableP)[di.row];
        buf.Set( noteRow.itsHeader );
        di.pPainter->FolderTab( di, FALSE, buf );
        }
    else
        {
        di.pPainter->FolderTab( di, TRUE, buf );
        }
}

//*****************************************************************************
// PaintTime
//*****************************************************************************
void TTimeLineGrid::PaintTime( RTDrawInfo di )
{
    // always paint the background for the cell
    TTime cellStart = GetCellStart( di.row, di.col );

    TGridUtil::PaintBackground( di, cellStart, TRUE, FALSE );

    // draw the cell if it's got something in it
    RTNote note = GetNote( di.row );
    if (IsNoteInCol( note, di.col )) {

        if (note.GetStyle() == kTaskStyle) {

            PaintTask( di, cellStart, note );
        }
        else if (note.GetStyle() == kMilestoneStyle) {

            PaintMilestone( di, cellStart, note );
        }
    }
}

// -------------------------------------------------------------------------
// PaintTask
// -------------------------------------------------------------------------
void TTimeLineGrid::PaintTask( RTDrawInfo di, RTTime start,
    RTNote note )
{
    RECT r;

    TBounds bounds( start, TimeOffset( start, 1 ));
    TBoundsPositions positions;

    BOOL overlaps = bounds.NoteOverlaps( note, 1, positions );
    do {

        if (overlaps) {

            // calculate rectangle to fill from positions
            r         = di.cellRect;
            int width = r.right - r.left;
            r.right   = r.left + positions.endPos   * width;
            r.left    = r.left + positions.startPos * width;
            r.right   = max( r.right, r.left + 1 );         // (>= 1 pixel wide)
            r.top     = r.bottom - kTaskHeight;

            if ((TSysState::timeScale < SCALE_WEEK) &&
                note.HollowOnDate( start ))
                {
                di.pPainter->FrameRectangle( di, r, note.GetColor() );
                 }
            else
                {
                di.pPainter->FillRectangle( di, r, note.GetColor() );
                }
        }

    } while (bounds.NoteOverlapsAgain( note, 1, positions, overlaps ));
}

// -------------------------------------------------------------------------
// PaintMilestone
// -------------------------------------------------------------------------
void TTimeLineGrid::PaintMilestone( RTDrawInfo di, RTTime start, RTNote note )
{
    int    center;
    RECT   r;

    if ((TSysState::timeScale <= SCALE_DAY) && !note.GetStart().HasTime()) {

        int midX = (di.cellRect.left + di.cellRect.right)  / 2;
        int midY = (di.cellRect.top  + di.cellRect.bottom) / 2;
        SetRect( &r, di.cellRect.left, midY - 2,
            midX - kMarkSize, midY + 2 );
        di.pPainter->FillRectangle( di, r, note.GetColor() );
        SetRect( &r, midX + kMarkSize + 1, midY - 2,
            di.cellRect.right, midY + 2 );
        di.pPainter->FillRectangle( di, r, note.GetColor() );

        SetRect( &r, midX - kMarkSize, midY - kMarkSize,
            midX + kMarkSize, midY + kMarkSize );
        di.pPainter->FillDiamond( di, r, note.GetColor() );

    }
    else {

        TBounds bounds( start, TimeOffset( start, 1 ));
        TBoundsPositions positions;

        BOOL overlaps = bounds.NoteOverlaps( note, 1, positions );
        do {

            if (overlaps) {

                int width = di.cellRect.right - di.cellRect.left;
                center    = di.cellRect.left + positions.startPos * width;
                center    = max( center, di.cellRect.left  + kMarkSize );
                center    = min( center, di.cellRect.right - kMarkSize );

                r.top    = di.cellRect.bottom - 2 * kMarkSize;
                r.bottom = di.cellRect.bottom;
                r.left   = center - kMarkSize;
                r.right  = center + kMarkSize;

                di.pPainter->FillDiamond( di, r, note.GetColor() );
            }

        } while (bounds.NoteOverlapsAgain( note, 1, positions, overlaps ));
    }
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
//          Basic Mouse Operations
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------


//*****************************************************************************
// bring up note on this row, or zoom in
//*****************************************************************************
void TTimeLineGrid::MousDblClick( RTMousInfo mi )
{
    PTNote pNote;

    switch( CellType( mi.row, mi.col )) {

        case kColor:        // show the note
        case kNote:

            pNote = &GetNote( mi.row );
            GetApplication()->ExecDialog(new TNoteView( TPimFrame::pFrame, IDD_NOTE, pNote));
            InvalidateNote( *pNote );
            break;

        case kTimeHeader:    // go down to the next time unit

            // can't get smaller than SCALE_HOUR
            if (TSysState::timeScale > SCALE_HOUR ) {

                TTime date = GetCellStart( mi.row, mi.col );
                date.CopyTime( TSysState::selTime );
                ResetTime( date, (TTimeScale) (TSysState::timeScale - 1) );
                Notify( kTimeChg, NOOBJECT );
                Notify( kScaleChg, NOOBJECT );
            }
            break;
    }
}

//*****************************************************************************
// MousRtClick
//
// Popup a menu
//*****************************************************************************
void TTimeLineGrid::MousRtClick( RTMousInfo mi )
{
    RTNote note = GetNote( mi.row );

    switch (CellType( mi.row, mi.col ))
        {

        case kTime:

            //only if no freq...
            if( note.GetFreq() == NULL ) {

                //stash away cell coordinates for response functions
                itsMenuRow= mi.row;
                itsMenuCol= mi.col;

                HMENU hMenuRes = LoadMenu( GetApplicationObject()->hInstance,
                    MAKEINTRESOURCE( IDM_TIMELINE_POPUP ));

                HMENU hMenu = GetSubMenu( hMenuRes, 0 );

                // enable appropriate menu items
                switch( note.GetStyle() )
                    {
                    case kTaskStyle:
                        EnableMenuItem( hMenu, IDM_CONVERT_TO_TASK, MF_BYCOMMAND | MF_GRAYED );
                        break;
                    case kMilestoneStyle:
                        EnableMenuItem( hMenu, IDM_CONVERT_TO_MILESTONE, MF_BYCOMMAND | MF_GRAYED );
                        break;
                    default:    //untimed styles
                        EnableMenuItem( hMenu, IDM_SCROLL_TO_NOTE, MF_BYCOMMAND | MF_GRAYED );
                        EnableMenuItem( hMenu, IDM_MOVE_NOTE_HERE, MF_BYCOMMAND | MF_GRAYED );
                        break;
                    }

                ClientToScreen( HWindow, &mi.xy );

                TGridUtil::OurTrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                    mi.xy.x, mi.xy.y, 0, HWindow, NULL );

                DestroyMenu( hMenuRes );
            }
            break;

        case kColor:
        case kNote:

            // popup local note menu (change color, delete, etc. )
            TNoteMenu::PopUp( this, &note, mi.xy.x, mi.xy.y );
            InvalidateNote( note );
            break;

        case kTimeHeader:

            if (TSysState::timeScale > SCALE_HOUR ) {

                TMousHelp::MousHelp( this,
                    "Double-click left button to change the time scale (zoom in)." );
            }
            break;

    }
}



//*****************************************************************************

//*****************************************************************************
void TTimeLineGrid::MousOver( RTMousInfo mi )
{
    switch( CellType( mi.row, mi.col )) {

        case kColor:
        case kNote:

            TCursor::Set( kNoteSelCursor );
            break;

        case kTimeHeader:

            TCursor::Set( (TSysState::timeScale == SCALE_HOUR ) ? kArrowCursor: kZoomCursor );
            break;

        case kTime:

            RECT leftR, rightR, dragR, marqueeR;
            RTNote note = GetNote( mi.row );

            GetNoteRects( note, mi, leftR, dragR, rightR, marqueeR );
            if (PtInRect( &leftR, mi.xy )) {

                TCursor::Set( kGrowLeftCursor );
            }
            else if (PtInRect( &rightR, mi.xy )) {

                TCursor::Set( kGrowRightCursor );
            }
            else if (PtInRect( &dragR, mi.xy)) {

                char reason[100];
                if (!IsAllowedToDrag( note, reason )) {

                    TCursor::Set( kDragIllegalCursor );
                }
                else {

                    TCursor::Set( kDragHorizCursor );
                }
            }
            else if( note.GetFreq() == NULL ) {

            GetNoteRects( note, mi, leftR, dragR, rightR, marqueeR );
                TCursor::Set( kTimeMoveCursor );
            }
            else {

                TCursor::Set( kArrowCursor );
            }
            break;

        default:

            TCursor::Set( kArrowCursor );
            break;
    }
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
//          Right Button Operations
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

//*****************************************************************************
// ScrollToNote
//*****************************************************************************
void TTimeLineGrid::ScrollToNote( RTMessage  )
{
    RTNote note = GetNote( itsMenuRow );

    if (note.GetStart().HasDate()) {

        ResetTime( note.GetStart(), TSysState::timeScale );
        Notify( kTimeChg, NOOBJECT );
    }
    else {
        // This should never happen -- menu items were disabled
        TMessageBox::Ok( this,
            "This note has no dates.",
            "Can't scroll to note" );
        return;
    }
}

//*****************************************************************************
// ConvertToTask
//*****************************************************************************
void TTimeLineGrid::ConvertToTask(RTMessage )
{
    int       d, h;
    TDuration duration;
    TTimeComplex times;

    RTNote note = GetNote( itsMenuRow );

    switch( note.GetStyle() ) {

        case kTaskStyle:

            //shouldn't be able to get here
            break;

        case kMilestoneStyle:

            // make length of task proportional to current view scale
            // to make it easier to resize items
            switch( TSysState::timeScale )
                {
                case SCALE_HOUR:

                    d = 1;
                    h = 1;
                    break;

                case SCALE_DAY:

                    d = 1;
                    h = TProfile::HoursPerDay;
                    break;

                case SCALE_WEEK:

                    d = 1;
                    h = TProfile::HoursPerDay;
                    break;

                case SCALE_MONTH:

                    d = 5;
                    h = TProfile::HoursPerDay;
                    break;

                default:

                    SYSERR_STR( "ConvertToTask: unexpected case in switch" );
                    break;
                }


            // set the day or hour of the duration
            if (note.GetTimeUnit() >= k1Day )
                duration.Set( k1Day,  d );
            else
                duration.Set( k1Hour, h );

            // check for time conflicts
            note.GetTimes( times );
            times.SetDurationAndKeepStart( duration );
            if (TNoteSet::CancelBecauseOfTimeConflict( note,
                times.GetStart(), times.GetEnd() )) {

                return;
            }
                
            // set the new duration for the note
            note.SetDurationAndKeepStart( duration );
            InvalidateNote( note );
            break;

        case kItemStyle:

            // make length of task proportional to current view scale
            // to make it easier to resize items
            // FIXME it's important to keep start time, note units, and duration units
            // in sync (so do it in note)
            TTimeUnit unit;
            int       offset;
            switch( TSysState::timeScale )
                {
                case SCALE_HOUR:

                    unit   = k1Hour;
                    offset = 1;
                    break;

                case SCALE_DAY:

                    unit   = k1Day;
                    offset = 1;
                    break;

                case SCALE_WEEK:

                    unit   = k1Day;
                    offset = 3;
                    break;

                case SCALE_MONTH:

                    unit   = k1Day;
                    offset = 5;
                    break;
                }

            duration.Set( unit, offset );
            TTime startTime= GetCellStart( itsMenuRow, itsMenuCol );
            if (unit > k1Hour) {

                startTime.ClearTime();
            }

            TTime end( startTime, duration );

            // check for time conflicts
            if (TNoteSet::CancelBecauseOfTimeConflict( note, startTime, end )) {

                return;
            }
                
            note.SetStartAndEnd( startTime, end, unit );
            InvalidateNote( note );
            break;

        case kFloatStyle:
            //problem arises if note is in days or weeks,
            // and cellStart has hours -- so clear it
            TTime tempTime= GetCellStart( itsMenuRow, itsMenuCol );
            if( note.GetTimeUnit() >= k1Day )
                tempTime.ClearTime();

            // check for time conflicts
            note.GetTimes( times );
            times.SetStartAndKeepDuration( tempTime );
            if (TNoteSet::CancelBecauseOfTimeConflict( note,
                times.GetStart(), times.GetEnd() )) {

                return;
            }
                
            note.SetStartAndKeepDuration( tempTime );
            InvalidateNote( note );
            break;
    }
}

//*****************************************************************************
// ConvertToMilestone
//*****************************************************************************
void TTimeLineGrid::ConvertToMilestone( RTMessage )
{
    TDuration duration;
    RTNote note = GetNote( itsMenuRow );

    switch( note.GetStyle() ) {

        case kTaskStyle:

            // (time conflict not important here, since the start
            // time is the same)
            note.KeepStartAndClearDuration();
            InvalidateNote( note );
            break;

        case kMilestoneStyle:

            //shouldn't happen
            TMessageBox::Ok( this,
                "This note is already a milestone.",
                "Can't convert to milestone" );
            InvalidateNote( note );
            break;

        case kItemStyle:
        case kFloatStyle:
            //problem arises if note is in days or weeks,
            // and cellStart has hours -- so clear it
            TTime tempTime= GetCellStart( itsMenuRow, itsMenuCol );
            if( note.GetTimeUnit() >= k1Day )
                tempTime.ClearTime();

            // check for time conflicts
            TTimeComplex times;
            note.GetTimes( times );
            times.SetStartAndClearDuration( tempTime );
            if (TNoteSet::CancelBecauseOfTimeConflict( note,
                times.GetStart(), times.GetEnd() )) {

                return;
            }
                
            note.SetStartAndClearDuration( tempTime );
            InvalidateNote( note );
            break;
    }
}

//*****************************************************************************

//*****************************************************************************
void TTimeLineGrid::MoveNoteHere( RTMessage )
{

    // get the new start time and the note
    TTime newStart( GetCellStart( itsMenuRow, itsMenuCol ));
    RTNote note = GetNote( itsMenuRow );

    // if the note's old start had a time, then make sure the new start
    // also has a time; if the note's start had NO time, then
    // make sure the new start time also has no time
    BOOL oldStartHasTime = note.GetStart().HasTime();
    BOOL newStartHasTime = newStart.HasTime();
    if (newStartHasTime && !oldStartHasTime) {

        newStart.ClearTime();
    }
    else if (oldStartHasTime && !newStartHasTime) {

        newStart.CopyTime( note.GetStart() );
    }

    // check for time conflicts
    TTimeComplex times;
    note.GetTimes( times );
    times.SetStartAndKeepDuration( newStart );
    if (TNoteSet::CancelBecauseOfTimeConflict( note,
        times.GetStart(), times.GetEnd() )) {

        return;
    }

    switch( note.GetStyle() ) {

        case kTaskStyle:
        case kMilestoneStyle:

            note.SetStartAndKeepDuration( newStart );
            InvalidateNote( note );
            break;

        case kFloatStyle:
        case kItemStyle:
        default:

            //shouldn't get here
            SYSERR_STR( "Unexpected member of switch statement" );
            break;
    }
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
//*****************************************************************************
BOOL TTimeLineGrid::ValidDragCell( TDragMode , int row, int col )
{
    return CellType( row, col ) == kTime;
}

//*****************************************************************************
// Release
//
// Called from MousDragRelease
//*****************************************************************************
void TTimeLineGrid::Release( void )
{
    RTNote note = GetNote( itsDragRow );
    InvalidateNote( note );
}


//---------------------------------------------------------------------------
// InvalidateNote
//
// Invalidate all copies of this note
//---------------------------------------------------------------------------
void TTimeLineGrid::InvalidateNote( RTNote note )
{
    // invalidate all rows with this note in them
    for (int i = 0; i < itsRowCount; i++)
        if (IsNoteRow( i ) && note == GetNote( i ))
            InvalidateCell( i, -1 );
}

//---------------------------------------------------------------------------
// SetStart
//
// we're dragging the start time
//---------------------------------------------------------------------------
void TTimeLineGrid::SetStart( RTTime start )
{
    if( itsPrecision >= k1Day)
        start.ClearTime();
    itsNewTimes.SetStartAndKeepEnd( start, itsPrecision, 1 );
    itsNewTimes.ConstrainStartToTimeAndEnd(
        itsFirstCell, itsPrecision, 1 );
}


//---------------------------------------------------------------------------
// SetEnd
//
// we're dragging the end
//---------------------------------------------------------------------------
void TTimeLineGrid::SetEnd( RTTime end )
{
    if( itsPrecision >= k1Day)
        end.ClearTime();
    itsNewTimes.SetEndAndKeepStart( end, itsPrecision, 1 );
    itsNewTimes.ConstrainEndToTimeAndStart(
        itsLastCell, itsPrecision, 1 );
}

//---------------------------------------------------------------------------
// SetBoth
//---------------------------------------------------------------------------
void TTimeLineGrid::SetBoth( RTMousInfo, RTTime start )
{
    if( itsPrecision >= k1Day)
        start.ClearTime();
    itsNewTimes.SetStartAndKeepDuration( start );
    itsNewTimes.ConstrainToInterval( itsFirstCell, itsLastCell );
}

//---------------------------------------------------------------------------
// ConstrainMarqueeRect
//
// keep the marquee out of the frozen cols
//---------------------------------------------------------------------------
void TTimeLineGrid::ConstrainMarqueeRect( TDragMode , RECT &r )
{
    r.left = AT_LEAST( r.left, FrozenWidth( scrPainter ));
}

//---------------------------------------------------------------------------
// NumVisibleCells
//
// return the number of visible cells on the screen
//---------------------------------------------------------------------------
int TTimeLineGrid::NumVisibleCells( void )
{
    int averageCellWidth = CellWidth( scrPainter, 35 ) +
                           GridWidth( scrPainter, 35 );
    int availableWidth   = winWidth - FrozenWidth( scrPainter );
    return availableWidth / averageCellWidth;
}
//---------------------------------------------------------------------------
// ScrollStart
//
// scrolling is only allowed horizontally
//---------------------------------------------------------------------------
void TTimeLineGrid::ScrollStart( RTMousInfo, int deltaRow, int deltaCol )
{
    if (deltaCol > 0) {

        // adjust the start by offsetting its time
        if (hScrollPos > 0) {

            TTime start( GetCellStart( itsDragRow, hScrollPos ));
            if (itsPrecision >= k1Day)
                start.ClearTime();
            itsNewTimes.SetStartAndKeepEnd( start, itsPrecision, 1 );

            // adjust the marquee
            itsMarqueeRect.right += kTimeColWidth;
            itsMarqueeRect.right = min( itsMarqueeRect.right, winWidth );
        }

        Scroll( deltaRow, deltaCol );
        UpdateWindow( HWindow );
    }
    else if (deltaCol < 0) {

        // scroll only as far as the end time
        int row, col;
        TTime realEnd = itsNewTimes.GetEnd();
        if (!realEnd.HasTime()) {

            realEnd.SetToMidnightTomorrow();
        }
        TimeToCell( realEnd, row, col );

        if (!IsCellVisible( row, col + 1 )) {

            // get the start time
            // if it's in days or works, ditch hours
            TTime start = GetCellEnd( itsDragRow,
                hScrollPos + NumVisibleCells() + kNumHeadCols );
            if (itsPrecision >= k1Day)
                start.ClearTime();
            itsNewTimes.SetStartAndKeepEnd( start, itsPrecision, 1 );
            itsNewTimes.ConstrainStartToTimeAndEnd( itsFirstCell, itsPrecision, 1 );

            Scroll( deltaRow, deltaCol );
            UpdateWindow( HWindow );
        }

        // adjust the marquee
        POINT p;
        if (TimeToPoint( realEnd, p ) &&
            (itsMarqueeRect.right > p.x)) {

            itsMarqueeRect.right = p.x;
        }
    }
}

//---------------------------------------------------------------------------
// ScrollEnd
//
// scrolling is only allowed horizontally
//---------------------------------------------------------------------------
void TTimeLineGrid::ScrollEnd( RTMousInfo, int deltaRow, int deltaCol )
{
    if (deltaCol < 0) {

        // adjust the end by offsetting its time
        if (hScrollPos < hScrollMax) {

            // get the time
            // if we're only looking for the time
            // to be in days or weeks, forget hours
            TTime end = GetCellStart( itsDragRow,
                hScrollPos + NumVisibleCells() + kNumHeadCols );
            if (itsPrecision >= k1Day)
                end.ClearTime();

            itsNewTimes.SetEndAndKeepStart( end, itsPrecision, 1 );

            // adjust the marquee
            itsMarqueeRect.left -= kTimeColWidth;
            itsMarqueeRect.left = max( itsMarqueeRect.left, FrozenWidth( scrPainter ) );
        }

        Scroll( deltaRow, deltaCol );
        UpdateWindow( HWindow );
    }
    else if (deltaCol > 0) {


        // scroll only as far as the start time
        int row, col;
        TTime realStart = itsNewTimes.GetStart();
        if (!realStart.HasTime()) {

            realStart.SetToMidnight();
        }
        TimeToCell( realStart, row, col );
        if (!IsCellVisible( row, col - 2 )) {

            // adjust the end time to be just after the start time
            TTime end = GetCellStart( itsDragRow, hScrollPos );
            if (itsPrecision >= k1Day)
                end.ClearTime();
            itsNewTimes.SetEndAndKeepStart( end, itsPrecision, 1 );
            itsNewTimes.ConstrainEndToTimeAndStart( itsLastCell, itsPrecision, 1 );

            Scroll( deltaRow, deltaCol );
            UpdateWindow( HWindow );
        }

        // adjust the marquee
        POINT p;
        if (TimeToPoint( realStart, p ) &&
            (itsMarqueeRect.left < p.x)) {

            itsMarqueeRect.left = p.x;
        }
    }
}


//---------------------------------------------------------------------------
// ScrollBoth
//
// scroll and adjust the start and end times to match the new scroll pos
//---------------------------------------------------------------------------
void TTimeLineGrid::ScrollBoth( RTMousInfo , int deltaRow, int deltaCol )
{
    // scroll left (but not past first cell)
    if ((deltaCol > 0) && (hScrollPos > 0)) {

        TTime start( GetCellStart( itsDragRow, hScrollPos ));
        if (!itsNoteP->GetStart().HasTime()) {

            start.ClearTime();
        }
        start.PlusEquals( -itsOffset );
        itsNewTimes.SetStartAndKeepDuration( start );
    }

    // scroll right (but not past last cell)
    else if ((deltaCol < 0) && (hScrollPos < hScrollMax)) {

        TTime start( GetCellEnd( itsDragRow,
            hScrollPos + NumVisibleCells() + kNumHeadCols ));
        if (!itsNoteP->GetStart().HasTime()) {

            start.ClearTime();
        }
        start.PlusEquals( -itsOffset );
        itsNewTimes.SetStartAndKeepDuration( start );
    }

    // scroll
    Scroll( deltaRow, deltaCol );
    UpdateWindow( HWindow );
}


//---------------------------------------------------------------------------
// PointToNote
//---------------------------------------------------------------------------
RTNote TTimeLineGrid::PointToNote( RTMousInfo mi )
{
    if (RowType( mi.row ) != kTimeRow) {

        return (RTNote) NOOBJECT;
    }

    itsDragRow  = mi.row;
    RTNote note = GetNote( mi.row );

    RECT leftR, dragR, rightR, marqueeR;
    GetNoteRects( note, mi, leftR, dragR, rightR, marqueeR );

    if (PtInRect( &leftR,  mi.xy ) ||
        PtInRect( &dragR,  mi.xy ) ||
        PtInRect( &rightR, mi.xy )) {

        return note;
    }
    else {

        return (RTNote) NOOBJECT;
    }
}

//*****************************************************************************
// GetPrecision
//*****************************************************************************
TTimeUnit TTimeLineGrid::GetPrecision( void )
{
    if (itsNoteP->GetTimeUnit() < k1Day) {

        return k1Hour;
    }
    else {

        return k1Day;
    }
}


//*****************************************************************************
// PointToTime
//
// Convert a point WITHIN THE GRID OF LIVE CELLS to a time
//
// NB currently assumes we're using SCREEN grid, not print grid
//*****************************************************************************
void TTimeLineGrid::PointToTime( RTMousInfo mi, RTTime time )
{
    int headWidth    = FrozenWidth( scrPainter );

    int x            = mi.xy.x;
    int logCol       = hScrollPos + (x - headWidth) / kTimeColWidth;
    int extra        = (x - headWidth) % kTimeColWidth;
    double fraction  = ((double) extra) / kTimeColWidth;

    switch( TSysState::timeScale ) {

        long  offset;

        case SCALE_HOUR:

            time   = itsFirstCell;
            offset = (logCol + fraction) * 240;
            int rounding = AT_MOST( k1Hour, itsPrecision );
            offset = offset / kMinutesPerTimeUnit[rounding] * kMinutesPerTimeUnit[rounding];
            time.PlusEquals( k1Hour, (int) (offset / 60) );
            break;

        case SCALE_DAY:

            offset = (logCol + fraction) * 24;
            time   = itsFirstCell.Plus( SCALE_HOUR, (int) offset );
            break;

        case SCALE_WEEK:

            offset = (logCol + fraction) * 7;
            time   = itsFirstCell.Plus( SCALE_DAY, (int) offset );
            break;

        case SCALE_MONTH:

            time   = itsFirstCell.Plus( SCALE_MONTH, logCol );
            offset = fraction * 31;   // FIXME: should be number of days in month, not 31
            time   = time.Plus( SCALE_DAY, (int) offset );
            break;
    }

    // if we're only looking for the time to be in days or weeks, then
    // forget about hours
#ifdef NOT_NOW
    if (itsPrecision >= k1Day) {

        time.ClearTime();
    }
#endif
}

//*****************************************************************************
// GetNoteRects
//
// for Milestones, growStartR and growEndR will be empty
// note: dragR no longer includes growStartR and growEndR
//*****************************************************************************
void TTimeLineGrid::GetNoteRects( RTNote note, RTMousInfo mi,
    RECT &growStartR, RECT &dragR, RECT &growEndR, RECT &marqueeR )
{
    // notes with frequencies get gigantic rectangles
    if (note.GetFreq() != NULL) {

        SetRect( &growStartR,  0, 0, -1, -1 );
        SetRect( &growEndR,    0, 0, -1, -1 );
        SetRect( &marqueeR,    0, 0, -1, -1 );
        SetRect( &dragR,       -32700, mi.cellRect.top, 32700, mi.cellRect.bottom );
        return;
    }

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

            if (!bounds.NoteOverlaps( note, kNumLiveCols, positions )) {

                SetRect( &growStartR,  0, 0, -1, -1 );
                SetRect( &dragR,       0, 0, -1, -1 );
                SetRect( &growEndR,    0, 0, -1, -1 );
                SetRect( &marqueeR,    0, 0, -1, -1 );
                return;
            }
            break;
    }

    // get logical positions for rect
    int cellWidth = kTimeColWidth + 1;
    int left      = (positions.startCell + positions.startPos) * cellWidth;
    int right     = (positions.endCell   + positions.endPos)   * cellWidth;
    int top       = mi.cellRect.top;
    int bottom    = mi.cellRect.bottom;

    // modify logical positions to allow for the fact that the widths of
    // the frozen columns must be added since they are not part of the
    // logical width, but the width of the columns
    // which are scrolled off to the left must be subtracted, since they
    // don't actually appear on the screen but ARE part of the logical width
    int headWidth = FrozenWidth( scrPainter );
    left         += headWidth - hScrollPos * cellWidth;
    right        += headWidth - hScrollPos * cellWidth;


    // create the rectangles
    if (note.GetStyle() == kTaskStyle) {

        // make sure dragR is at least kMinDragWidth wide
        int width = right - left;
        if (width < kMinDragWidth) {

            int delta = (kMinDragWidth - width) / 2;
            SetRect( &dragR,       left - delta, top, right + delta, bottom );
        }
        else {

            SetRect( &dragR,       left, top, right, bottom );
        }

        SetRect( &growStartR, dragR.left - kMinDragWidth, top, dragR.left, bottom );
        SetRect( &growEndR,   dragR.right, top, dragR.right + kMinDragWidth, bottom );
        SetRect( &marqueeR,   left, bottom - kMinDragWidth, right, bottom );
    }
    else {

        SetRect( &dragR, left - kHalfMinDragWidth, top, right + kHalfMinDragWidth, bottom );
        SetRect( &growStartR,  0, 0, -1, -1 );
        SetRect( &growEndR,    0, 0, -1, -1 );
        SetRect( &marqueeR,   left, bottom - kMinDragWidth, right, bottom );
    }

}


//*****************************************************************************
// IsNoteInCol
//*****************************************************************************
BOOL TTimeLineGrid::IsNoteInCol( RTNote note, int col )
{
    if ((note             == NOOBJECT)   ||
        (note.GetStyle()  == kItemStyle) ||
        (note.GetStyle()  == kFloatStyle)) {

       return FALSE;
    }

    // make up a bounds for this cell
    TTime start = GetCellStart( -1, col );
    TBounds bounds( start, TimeOffset( start, 1 ));
    TBoundsPositions positions;

    BOOL overlaps = bounds.NoteOverlaps( note, 1, positions );
    do {

        if (overlaps) {

            return TRUE;
        }
    } while (bounds.NoteOverlapsAgain( note, 1, positions, overlaps ));

    return FALSE;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
//          Scrolling
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

//---------------------------------------------------------------------------
// WMHScroll
//
// Keeps track of the selected time
//---------------------------------------------------------------------------
void TTimeLineGrid::WMHScroll( RTMessage Msg)
{
    // send the corrected version off to basegrid
    TBaseGrid::WMHScroll( Msg );

    // save selected time (preserve hour unless we're the scale is HOURS
    if (TSysState::timeScale == SCALE_HOUR) {

        TSysState::selTime = GetCellStart( 0, hScrollPos + frozenCols );
    }
    else {

        TTime date = GetCellStart( 0, hScrollPos + frozenCols );
        TSysState::selTime.CopyDate( date );
    }

    // notify
    Notify( kTimeChg, NOOBJECT );
}

//---------------------------------------------------------------------------
// WMVScroll
//
// Keeps track of the selected note
//---------------------------------------------------------------------------
void TTimeLineGrid::WMVScroll( RTMessage Msg)
{
    // send the corrected version off to basegrid
    TBaseGrid::WMVScroll( Msg );

    // save note position
    TSysState::currentNoteIndex = vScrollPos;
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
//          Listen
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

// ----------------------------------------------------------------------
// ResetTime
// ----------------------------------------------------------------------
void TTimeLineGrid::ResetTime( RTTime selTime, TTimeScale timeScale )
{
    TSysState::selTime   = selTime;
    TSysState::timeScale = timeScale;

    Reset();
}


// ----------------------------------------------------------------------
// Listen
// ----------------------------------------------------------------------
void TTimeLineGrid::Listen( TSnoopMsg aMsg, RObject )
{
switch( aMsg )
    {
    case kNoteSetChg:
    case kCatNew:
    case kCatDel:

        Reset();
        break;

   case kScaleChg:
   case kTimeChg:

        ResetTime( TSysState::selTime, TSysState::timeScale );
        break;

   default:
        break;
   }
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
TTimeLineGrid::TCellType TTimeLineGrid::CellType( int row, int col )
{
    // titles
    if (row == 0) {

        switch( col ) {

            case -1:  return kNotACell;
            case 0:   return kColorHeader;
            case 1:   return kNoteHeader;
            default:

                if (col < colCnt)
                    return kTimeHeader;
                else
                    return kNotACell;
        }
    }
    else if ((0 < row) && (row < itsRowCount)) {

        if (IsNoteRow( row )) {

            switch( col ) {

                case -1:  return kNotACell;
                case 0:   return kColor;
                case 1:   return kNote;
                default:  

                    if (col < colCnt)
                        return kTime;
                    else
                        return kNotACell;
            }
        }
        else {

            switch( col ) {

                case -1:  return kNotACell;
                case 0:   return kColorTitle;
                case 1:   return kNoteTitle;
                default:  

                    if (col < colCnt)
                        return kTimeTitle;
                    else
                        return kNotACell;
            }
        }
    }
    else {

        return kNotACell;
    }
}



//---------------------------------------------------------------------------
// ColType
//
// Return the type of the col; handles the case where col is -1, as it would
// be if XYWithin were false
//---------------------------------------------------------------------------
TTimeLineGrid::TColType TTimeLineGrid::ColType( int col )
{
    switch( col ) {

        case -1:   return kNotACol;
        case 0:    return kColorCol;
        case 1:    return kNoteCol;
        default:   return kTimeCol;
    }
}

//---------------------------------------------------------------------------
// RowType
//
// Return the type of the row; handles the case where row is -1, as it would
// be if XYWithin were false
//---------------------------------------------------------------------------
TTimeLineGrid::TRowType TTimeLineGrid::RowType( int row )
{
    switch( row ) {

        case -1:   return kNotARow;
        case 0:    return kHeaderRow;
        default:

            if (IsNoteRow( row ))
                if (row < itsRowCount)
                    return kTimeRow;
                else
                    return kNotARow;
            else
                if (row < itsRowCount)
                    return kTitleRow;
                else
                    return kNotARow;
    }
}


//***************************************************************************
//
//                                 Printing
//
//***************************************************************************

//*****************************************************************************

//*****************************************************************************
void TTimeLineGrid::SetDefaultPrintRange( void )
{
rFirstLiquid= frozenRows;
rLastLiquid= rowCnt - 1;

cFirstLiquid= frozenCols + hScrollPos;
cLastLiquid= cFirstLiquid + 20;
}

//*****************************************************************************
// modify index by n, the convert into meaningful representation, such as
// the time and date of that column
// return FALSE if index would be out of range
//*****************************************************************************
BOOL TTimeLineGrid::HChgCellStartIndex( int n, char *buf, int /* size */ )
{
BOOL rc;

rc= ((cFirstLiquid + n) >= frozenCols &&
     (cFirstLiquid + n) < colCnt &&
     (cFirstLiquid + n ) < cLastLiquid );

if( rc )
    cFirstLiquid+= n;

TTime cellStart = GetCellStart( -1, cFirstLiquid );
FormatTime( cellStart, buf );

//kludge: the title may have a \n in it, which we now remove
char *p= strchr( buf, '\n' );
if( p )
    *p= ' ';

return rc;
}

//*****************************************************************************
// modify index by n, the convert into meaningful representation, such as
// the time and date of that column
// return FALSE if index would be out of range
//*****************************************************************************
BOOL TTimeLineGrid::HChgCellEndIndex( int n, char *buf, int /* size */ )
{
BOOL rc;

rc= ((cLastLiquid + n) >= frozenCols &&
     (cLastLiquid + n) < colCnt &&
     (cLastLiquid + n) > cFirstLiquid );

if( rc )
    cLastLiquid+= n;

FormatTime( GetCellStart( -1, cLastLiquid ), buf );
//kludge: the title may have a \n in it, which we now remove
char *p= strchr( buf, '\n' );
if( p )
    *p= ' ';

return rc;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////// FOCUS ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//*****************************************************************************
//set focus to first visible note title
//*****************************************************************************
void TTimeLineGrid::GetDefaultFocusCell( int &row, int &col )
{
row= frozenRows + vScrollPos;                //focus on first visible row!
col= 1;
}

//*****************************************************************************

//*****************************************************************************
void TTimeLineGrid::MousKey( TMousKey key )
{
TMousGrid::MousKey( key );
}

//*****************************************************************************
// only note titles are selectable
//*****************************************************************************
BOOL TTimeLineGrid::IsCellSelectable( int row, int col )
{
return( row > 0 && row < rowCnt && col >= 1  && col < colCnt );
}

