/***************************************************************************
 *                                                                         *
 *  MODULE     : mongrid.cpp
 *               month painter for calendar
 ***************************************************************************/
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "mongrid.h"

#ifndef __sysstate__
#include "sysstate.h"
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
#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __print__
#include "print.h"
#endif
#ifndef __gridutil__
#include "gridutil.h"
#endif
#ifndef  __cursor__
#include "cursor.h"
#endif
#ifndef __frame__
#include "frame.h"
#endif
#ifndef __moushelp__
#include "moushelp.h"
#endif

static const int kMaxSlots     = 10;
static const int kMinSlots     = 3;
static const int kWhiteSpace   = 1; // space between bars
static const int kScaleFactor  = 30;
static const int kNumHeadRows  = 2;
static const int kNumHeadCols  = 0;
static const int kNumDays      = 42;
static const int kDaysPerRow   = 7;
static const int kNumWeeks	   = 6;
static const int kSmallFontHeight = 10;  // height of small font
                                         // FIXME: get the height right
static const int kPixelsPerBar = kSmallFontHeight; // including kWhiteSpace

//***************************************************************************
//
//                                 Constructors
//
//***************************************************************************

//---------------------------------------------------------------------------
//  Constructor
//---------------------------------------------------------------------------
TMonGrid::TMonGrid( PTWindowsObject AParent ):
    TRuleGrid( AParent )
{
    TCursor::Busy( TRUE );

    // create painters
    scrPainter= new TPaint;
    prtPainter= new TPaintPrt;

    // initialize instance variables
    itsEntryTableP = new TEntryTable( kNumDays, kMaxSlots );

    // initialize grid
    SetRowCnt( kNumWeeks + kNumHeadRows );
    SetRowFreeze( kNumHeadRows );
    SetColCnt( kDaysPerRow );
    SetColFreeze( 0 );

    Setup( TSysState::selTime );

    SetPrtTitle( "Month Grid" );
    SetDefaultPrintRange();
    TCursor::Busy( FALSE );
}

//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------
TMonGrid::~TMonGrid( void )
{
    delete scrPainter;
    delete prtPainter;
    delete itsEntryTableP;
}

//***************************************************************************
//
//                                 Setup
//
//***************************************************************************

//---------------------------------------------------------------------------
// Setup
//---------------------------------------------------------------------------
void TMonGrid::Setup( RTTime startTime )
{

    // clear the entryTable
    itsEntryTableP->Clear();

    // set the start and end times (includes enough extra days at
    // the beginning and end to take up all kNumDays slots
    itsStartTime   = startTime;
    itsTargetMonth = startTime;
    itsTargetMonth.SetToMidnight();
    itsTargetMonth.Normalize( SCALE_MONTH );

    itsFirstCell = itsTargetMonth.Plus( SCALE_DAY, -itsTargetMonth.GetWeekDay() );
    itsLastCell  = itsFirstCell.Plus( SCALE_DAY, kNumDays );

    itsNumTimes  = kNumDays;

    // look for notes that fall within the period
    TBoundsPositions positions;
    TBounds bounds( itsFirstCell, itsLastCell );
    int numNotes = (TSysState::pSelNotes)->getItemsInContainer();
    for (int i = 0; i < numNotes; i++ ) {

        RTNote note = (*TSysState::pSelNotes)[i];
        if (NOOBJECT == (RObject) note) continue;

        BOOL overlaps = bounds.NoteOverlaps( note, itsNumTimes, positions );
        do {

            if (overlaps) {

                // if we're showing the headlines then
                // put the text as high as possible in
                // each cell
                if ((!TSysState::bShowBars) &&
                   positions.startCell != positions.endCell) {

                   int start = positions.startCell;
                   int end   = positions.endCell;
                   for (int i = start; i <= end; i++) {

                       positions.startCell = i;
                       positions.endCell   = i;
                       itsEntryTableP->SetEntry( &note,
                            positions, kScaleFactor );
                   }
                }
                else {

                    itsEntryTableP->SetEntry( &note, positions, kScaleFactor );
                }

            }

        } while (bounds.NoteOverlapsAgain( note, itsNumTimes, positions, overlaps ));
    }

    InvalidateCell( -1, -1 );
}

//---------------------------------------------------------------------------
// CellToTime
//---------------------------------------------------------------------------
void TMonGrid::CellToTime( int row, int col, RTTime time )
{
    int entry = CellToEntry( row, col );

    if (entry == -1) {

        time = itsFirstCell;
    }
    else {

        time = itsFirstCell.Plus( SCALE_DAY, entry );
    }
}

//---------------------------------------------------------------------------
// GetCellTimes
//---------------------------------------------------------------------------
void TMonGrid::GetCellTimes( int row, int col, RTTime start, RTTime end  )
{
    CellToTime( row, col, start );
    end = start.Plus( SCALE_DAY, 1 );
}

//---------------------------------------------------------------------------
// CellToEntry
//---------------------------------------------------------------------------
int TMonGrid::CellToEntry( int row, int col )
{
    BOOL validCell= row >= kNumHeadRows && col >= 0 &&
                    row < rowCnt && col < colCnt;

	int day = (row - kNumHeadRows) * kDaysPerRow + col;
    return ( validCell && (0 <= day) && (day < kNumDays)) ? day : -1;
}


//***************************************************************************
//
//                                 Painting
//
//***************************************************************************

//---------------------------------------------------------------------------
// CellWidth
//---------------------------------------------------------------------------
int TMonGrid::CellWidth( PTPaint pPaint, int col )
{
RECT r;

if( pPaint->IsPrintPainter() )
    TPrint::GetPrtPixels( &r );
else
    GetClientRect( HWindow, &r );

// col width is divided evenly over the row and
// needs 1 extra pixel for the grid line
int width = (r.right - r.left - 1);
int slop  = width % 7;
int satSlop = slop / 2;
int sunSlop = slop - satSlop;
int stdColWidth = max( 50,  width / 7 - 1 );

switch( col )
    {
    case 0:  return stdColWidth + sunSlop;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:  return stdColWidth;
    case 6:  return stdColWidth + satSlop;
    
    default:

        SYSERR_STR( "CellWidth: unexpected case in switch" );
        return 0;
    }
}

//---------------------------------------------------------------------------
// CellHeight
//---------------------------------------------------------------------------
int TMonGrid::CellHeight( PTPaint pPaint, int row )
{
static const kTopLineHeight     = 4;
static const kMonthTitleHeight  = 20;
static const kDayTitleHeight    = 20;

RECT r;
if( pPaint->IsPrintPainter() )
    TPrint::GetPrtPixels( &r );
else
    GetClientRect( HWindow, &r );

int heightForDays  = r.bottom - r.top -
    kTopLineHeight - kMonthTitleHeight - kDayTitleHeight;
int stdRowHeight   = max( kMinSlots * kPixelsPerBar + 1,
    (heightForDays ) / kNumWeeks );

// monthTitleHeight absorbs all leftovers from std rows
int monthTitleHeight = max( kMonthTitleHeight,
    r.bottom - r.top - kTopLineHeight - kDayTitleHeight -
    kNumWeeks * stdRowHeight);

// don't forget the grid line
stdRowHeight--;

switch( RowType( row )) {

    case kMonthTitleRow:   return monthTitleHeight;
    case kDayTitleRow:     return kDayTitleHeight;
    case kTimeRow:         return stdRowHeight;
    default:

        SYSERR_STR( "CellHeight: unexpected case in switch" );
        return 0;
    }
}

//---------------------------------------------------------------------------
// GridWidth
//---------------------------------------------------------------------------
int  TMonGrid::GridWidth( PTPaint, int col )
{
switch( ColType( col ))
    {
    case kTimeCol:         return 1;
    default:

        SYSERR_STR( "CellHeight: unexpected case in switch" );
        return 0;
    }
}

//---------------------------------------------------------------------------
// GridHeight
//---------------------------------------------------------------------------
int  TMonGrid::GridHeight( PTPaint, int row )
{
switch( RowType( row ))
    {
    case kMonthTitleRow:   return 1;
    case kDayTitleRow:     return 2;
    case kTimeRow:         return 1;
    default:

        SYSERR_STR( "CellHeight: unexpected case in switch" );
        return 0;
    }
}

//---------------------------------------------------------------------------
// PaintGridCol
//---------------------------------------------------------------------------
void TMonGrid::PaintGridCol( RTDrawInfo di  )
{
    RECT    r = di.cellRect;

    // except for the rightmost column, we don't want
    // to draw in the top row
    if (di.col < kDaysPerRow - 1)
        {
        r.top = max( r.top, TopLineHeight( di.pPainter ) + topMargin + CellHeight( di.pPainter, 0 ));
        }

    di.pPainter->GridLine( di, r );
}

//---------------------------------------------------------------------------
// PaintCell
//---------------------------------------------------------------------------
void TMonGrid::PaintCell( RTDrawInfo di )
{
    switch (CellType( di.row, di.col )) {

        case kMonthTitle:    PaintMonthTitle( di );    break;
        case kDayTitle:      PaintDayTitle( di );      break;
        case kDay:           PaintNote( di );          break;
    }
}

//---------------------------------------------------------------------------
// PaintMonthTitle
//---------------------------------------------------------------------------
void TMonGrid::PaintMonthTitle( RTDrawInfo di )
{
	char buf1[100];
    char buf2[100];
    char buf3[20];

    RECT r;
    if( di.pPainter->IsPrintPainter() )
        {
        TPrint::GetPrtPixels( &r );
        }
    else
        {
        GetClientRect( HWindow, &r );
        }
    r.bottom = CellHeight( di.pPainter, 0 );

    TIntlFmt::FormatMonthName( itsTargetMonth, kFullName, buf1 );
    TIntlFmt::FormatYear( itsTargetMonth, buf2 );
    wsprintf( buf3, "%s %s", buf1, buf2 );
    di.pPainter->Text( di, buf3, kNoHilite,
        kSingleLine | DT_CENTER | DT_VCENTER, kBoldFont, &r );
}

//---------------------------------------------------------------------------
// PaintDayTitle
//---------------------------------------------------------------------------
void TMonGrid::PaintDayTitle( RTDrawInfo di )
{
    char dowBuf[10];
    int  dow = di.col;

    if (CellToEntry( di.row, di.col ) == -1) {

        // no date? just use the day of the week
        TGridUtil::PaintBackground( di, di.col );
    }
    else {

        // otherwise use the date
        TTime day;
        CellToTime( di.row, di.col, day );
        TGridUtil::PaintBackground( di, day, FALSE );
    }


    TIntlFmt::DayName( dow, kThreeLetter, dowBuf );
    WORD tStyle= DT_SINGLELINE | DT_NOPREFIX | DT_EXTERNALLEADING | DT_CENTER | DT_VCENTER;
    di.pPainter->Text( di, dowBuf, kNoHilite, tStyle, kBoldFont );
}

//---------------------------------------------------------------------------
// PaintNote
//---------------------------------------------------------------------------
void TMonGrid::PaintNote( RTDrawInfo di )
{
    RECT    r;
    char    buf[10];
    TTime   day;

    if (CellToEntry( di.row, di.col ) == -1) {

        // no date? just use the day of the week
        TGridUtil::PaintBackground( di, di.col );
    }
    else {

        // otherwise use the date
        CellToTime( di.row, di.col, day );
        TGridUtil::PaintBackground( di, day, FALSE );
    }

    // each bar will take kPixelsPerBar pixels, including
    // kWhiteSpace pixels of white space
    r         = di.cellRect;
    r.top     = di.cellRect.top + 1;
    r.bottom  = r.top + kSmallFontHeight;

    PTNote  pNote;
    long    startPos, stopPos;

    int entry = CellToEntry( di.row, di.col );
    if (entry == -1) return;

    int visibleSlots =
        (CellHeight( di.pPainter, di.row ) - kSmallFontHeight) /
        kSmallFontHeight;
    int lastSlot = min( kMaxSlots, visibleSlots );
        

    for (int slot = 0; slot < lastSlot; slot++) {

        // get the entry for this slot and continue if there is none
        if (itsEntryTableP->GetEntry( entry, slot, &pNote, startPos, stopPos )) {

            TSafStr  title;
            TSafStr  temp;
            pNote->Title( title );

            //show the time on the first entry only; "-" thereafter
            TTime    start = pNote->GetStart();

            #ifdef OLD_CODE
            if( start.IsSameDay( day, FALSE ) )
                {
                if (start.HasTime() )
                    TIntlFmt::FormatCompactTime( start, temp );
                else
                    temp.Set( "* " );

                temp.Cat( title );
                title.Set( temp );
                }
            else
                {
                temp.Set( "- " );
                temp.Cat( title );
                title.Set( temp );
                }
            #else

                if (start.HasTime() )
                    TIntlFmt::FormatCompactTime( start, temp );
                else
                    temp.Set( "* " );

                temp.Cat( title );
                title.Set( temp );

            #endif

            switch( pNote->GetStyle() ) {

                case kMilestoneStyle:

                    if (TSysState::bShowBars)
                        {
                        PaintMilestone( di, r, pNote->GetColor(), !pNote->GetStart().HasTime() );
                        }
                    else
                        {
                        TFont font= (pNote->IsDone()) ? kSmallStrikeFont : kSmallFont;
                        di.pPainter->Text( di, (const char *) title, kNoHilite, kSingleLine,
                            font, &r );
                        }
                    break;

                case kTaskStyle:

                    BOOL hollow = pNote->HollowOnDate( day );
                    if (TSysState::bShowBars)
                        {
                        PaintTask( di, r, pNote->GetColor(),
                            (int) startPos, (int) stopPos, hollow );
                        }
                    else if (!hollow)
                        {
                            TFont font= (pNote->IsDone()) ? kSmallStrikeFont : kSmallFont;
                            di.pPainter->Text( di, (const char*) title, kNoHilite, kSingleLine,
                                font, &r );
                        }
                    break;
            }
        }


        // push the rectangle down
        r.top    += kSmallFontHeight;
        r.bottom += kSmallFontHeight;
    }


    // if the last slot has something in it, then write "more"
    for (int i = lastSlot; i < kMaxSlots; i++) {

        if (itsEntryTableP->GetEntry( entry, i, &pNote, startPos, stopPos )) {

            di.pPainter->Text( di, "more ", kNoHilite, kSingleLine | DT_RIGHT,
                kSmallFont, NULL );
        }
    }

        
    // write the date over the background
    // FIXME make rect smaller
    TTime time;
    CellToTime( di.row, di.col, time );
    wsprintf( buf,"%d", time.GetDay() );

    if (time.GetMonth() == itsTargetMonth.GetMonth()) {

        di.pPainter->Text( di, buf, kNoHilite, kSingleLine, kSmallFont );
    }
    else {

        di.pPainter->Text( di, buf, kNoHilite, kSingleLine, kSmallFont );
    }
}

//---------------------------------------------------------------------------
// PaintMilestone
//
//  Old version that draws just the bars and no text
//---------------------------------------------------------------------------
void TMonGrid::PaintMilestone( RTDrawInfo di, RECT &r, RTColor color, BOOL fullDay )
{
    const int kHalfDiamond = kPixelsPerBar / 2 - 2;
    const int kHalfBarSize = 2;
    RECT leftR, diamondR, rightR;

    int midX = (di.cellRect.left + di.cellRect.right)  / 2;
    int midY = (r.top + r.bottom) / 2;

    SetRect( &diamondR, midX - kHalfDiamond,     midY - kHalfDiamond,
                        midX + kHalfDiamond,     midY + kHalfDiamond );

    if (fullDay) {

        SetRect( &leftR,    di.cellRect.left,        midY - kHalfBarSize,
                            midX - kHalfDiamond,     midY + kHalfBarSize );
        SetRect( &rightR,   midX + kHalfDiamond + 1, midY - kHalfBarSize,
                            di.cellRect.right,       midY + kHalfBarSize );

        di.pPainter->FillRectangle( di, leftR,    color );
        di.pPainter->FillRectangle( di, rightR,   color );
    }

    di.pPainter->FillDiamond(   di, diamondR, color );
}

//---------------------------------------------------------------------------
// PaintTask
//
//  Old version that draws just the bars and no text
//---------------------------------------------------------------------------
void TMonGrid::PaintTask( RTDrawInfo di, RECT &r,
    RTColor color, int startPos, int stopPos, BOOL hollow )
{
    RECT drawR;
    int width    = di.cellRect.right - di.cellRect.left;

    drawR.top    = r.top + 1;
    drawR.bottom = r.bottom;
    drawR.left   = di.cellRect.left + width * startPos / kScaleFactor;
    drawR.right  = di.cellRect.left + width * stopPos  / kScaleFactor;

    if (hollow)
        {
        di.pPainter->FrameRectangle( di, drawR, color );
        }
    else
        {
        di.pPainter->FillRectangle( di, drawR, color );
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
void TMonGrid::MousOver( RTMousInfo mi )
{
if (IsCellSelectable( mi.row, mi.col ))
    {
    int    entry = CellToEntry( mi.row, mi.col );
    if (itsEntryTableP->HasNoteInEntry( entry ) )
        {
        TCursor::Set( kNoteSelCursor );
        }
    else
        {
        TCursor::Set( kZoomCursor  );
        }
    }
else
    {
    TCursor::Set( kArrowCursor  );
    }
}

//---------------------------------------------------------------------------
// MousRtClick
//
// brings up menu
//---------------------------------------------------------------------------
void TMonGrid::MousRtClick( RTMousInfo mi )
{
if (IsCellSelectable( mi.row, mi.col ))
    {
    int    entry = CellToEntry( mi.row, mi.col );
    if (itsEntryTableP->HasNoteInEntry( entry ))
        {
        HMENU hMenuRes = LoadMenu( GetApplicationObject()->hInstance,
            MAKEINTRESOURCE( IDM_MONTH ));
        HMENU hMenu = GetSubMenu( hMenuRes, 0 );
        ClientToScreen( HWindow, &mi.xy );

        itsMenuRow = mi.row;
        itsMenuCol = mi.col;

        TGridUtil::OurTrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
            mi.xy.x, mi.xy.y, 0, HWindow, NULL );

        DestroyMenu( hMenuRes );
        TCursor::Set( kNoteSelCursor );
        }
    else
        {
        TMousHelp::MousHelp( this,
            "Double-click left button to change to week view (zoom in)." );
        }
    }
}


//---------------------------------------------------------------------------
// HandleZoom
//
// Handle the zoom message from the pop up menu
//---------------------------------------------------------------------------
void TMonGrid::HandleZoom( RTMessage )
{
    CellToTime( itsMenuRow, itsMenuCol, TSysState::selTime );
    TSysState::selTime.CopyTime( itsStartTime );
    Notify( kTimeChg, NOOBJECT );

    //emulate menu scale change command
    PostMessage( TPimFrame::pFrame->HWindow, WM_COMMAND, IDM_VIEWWEEK, 0 );
}

//---------------------------------------------------------------------------
// HandleNotes
//
// Handle the notes message from the pop up menu
//---------------------------------------------------------------------------
void TMonGrid::HandleNotes( RTMessage )
{
    int    entry = CellToEntry( itsMenuRow, itsMenuCol );
    if (itsEntryTableP->HasNoteInEntry( entry ))
        {
        TTime start, end;
        CellToTime( itsMenuRow, itsMenuCol, start );
        TGridUtil::NotePopup( start );
        }
}


//---------------------------------------------------------------------------
// MousDblClick
//---------------------------------------------------------------------------
void TMonGrid::MousDblClick( RTMousInfo mi )
{
    if (IsCellSelectable( mi.row, mi.col )) {

        CellToTime( mi.row, mi.col, TSysState::selTime );
        TSysState::selTime.CopyTime( itsStartTime );
        Notify( kTimeChg, NOOBJECT );

        //old way -- deletes view before clean completion
        //TSysState::timeScale = SCALE_DAY;
        //Notify( kScaleChg, NOOBJECT );

        //emulate menu scale change command
        PostMessage( TPimFrame::pFrame->HWindow, WM_COMMAND, IDM_VIEWWEEK, 0 );
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
TMonGrid::TCellType TMonGrid::CellType( int row, int col )
{
    if (col == -1) {

       return kNotACell;
    }

    switch( row ) {

        case -1:  return kNotACell;
        case 0:   return kMonthTitle;
        case 1:   return kDayTitle;
        default:  return kDay;
    }
}



//---------------------------------------------------------------------------
// ColType
//
// Return the type of the col; handles the case where col is -1, as it would
// be if XYWithin were false
//---------------------------------------------------------------------------
TMonGrid::TColType TMonGrid::ColType( int col )
{
    switch( col ) {

        case -1:   return kNotACol;
        default:   return kTimeCol;
    }
}

//---------------------------------------------------------------------------
// RowType
//
// Return the type of the row; handles the case where row is -1, as it would
// be if XYWithin were false
//---------------------------------------------------------------------------
TMonGrid::TRowType TMonGrid::RowType( int row )
{
    switch( row ) {

        case -1:   return kNotARow;
        case 0:    return kMonthTitleRow;
        case 1:    return kDayTitleRow;
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
void TMonGrid::ResetNotes( void )
{
    Setup( TSysState::selTime );
}



//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void TMonGrid::ResetTime( void )
{
    if (TSysState::selTime != itsStartTime ) {

        Setup( TSysState::selTime );
    }
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void TMonGrid::Listen( TSnoopMsg aMsg, RObject )
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

//---------------------------------------------------------------------------
// HeadlineChanged
//
//  The "Headlines" button was clicked so set the notes up
//  again since the text notes go in different slots than
//  bar notes
//---------------------------------------------------------------------------
void TMonGrid::HeadlineChanged( RTMessage )
{
    Setup( TSysState::selTime );
}
//***************************************************************************
//
//                                 Printing
//
//***************************************************************************

//---------------------------------------------------------------------------
// SetDefaultPrintRange
//---------------------------------------------------------------------------
void TMonGrid::SetDefaultPrintRange( void )
{
TBaseGrid::SetDefaultPrintRange();
}

/////////////////////////////////////////////////////////////////////////////
////////////////////////////// FOCUS ////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//*****************************************************************************
//set focus to first date in grid
//*****************************************************************************
void TMonGrid::GetDefaultFocusCell( int &row, int &col )
{
row= 1;
col= 1;
}

//*****************************************************************************

//*****************************************************************************
void TMonGrid::MousKey( TMousKey key )
{
TMousGrid::MousKey( key );
}

//---------------------------------------------------------------------------
// IsCellSelectable (only date entries)
//---------------------------------------------------------------------------
BOOL TMonGrid::IsCellSelectable( int row, int col )
{
    return (CellToEntry( row, col ) != -1);
}


