/***************************************************************************
 *                                                                         *
 *  MODULE     : grafgrid.cpp
 *               implement graph view
 *
 *
 ***************************************************************************/
#include "standard.h"
#pragma hdrstop

#include "grafgrid.h"

#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __frame__
#include "frame.h"
#endif
#ifndef __note__
#include "note.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __error__
#include "error.h"
#endif
#ifndef __worktime__
#include "worktime.h"
#endif
#ifndef __filter__
#include "filter.h"
#endif
#ifndef __moushelp__
#include "moushelp.h"
#endif
#ifndef __notelisd__
#include "notelisd.h"
#endif
#ifndef __paint__
#include "paint.h"
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
#ifndef  __intlfmt__
#include "intlfmt.h"
#endif

// columns
static const int kTickMarkColumn           = 0;
static const int kNumFrozenCols            = 0;
static const int kNumActiveCols            = 200;


// rows
static const int kGraphRow                 = 0;
static const int kValueRow                 = 1;
static const int kTitleRow                 = 2;
static const int kNumFrozenRows            = 0;
static const int kNumActiveRows            = 3;

int TGrafGrid::itsTimeUnitsPerCell[] = { 4, 1, 1, 1 };

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
//          Constructors
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
TGrafGrid::TGrafGrid( PTWindowsObject AParent ):
    TRuleGrid( AParent )
{
    TCursor::Busy( TRUE );

    // create painters
    scrPainter    = new TPaint;
    prtPainter    = new TPaintPrt;

    SetRowCnt(    kNumFrozenRows + kNumActiveRows );
    SetRowFreeze( kNumFrozenRows );
    SetColCnt(    kNumFrozenCols + kNumActiveCols );
    SetColFreeze( kNumFrozenCols );

    // make sure the timescale is at least SCALE_DAY
    TSysState::timeScale = max( SCALE_DAY, TSysState::timeScale );

    ResetTime( TSysState::selTime );

    SetPrtTitle( "Cost/Period Graph" );
    SetDefaultPrintRange();
    TCursor::Busy( FALSE );
}

// -------------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------------
TGrafGrid::~TGrafGrid( void )
{
    delete scrPainter;
    delete prtPainter;
}

// ----------------------------------------------------------------------
// ResetTime
// ----------------------------------------------------------------------
void TGrafGrid::ResetTime( RTTime selTime )
{
    TTime startCell = selTime;

    // normalize startCell; make sure it has a time;
    // fit the right number of time units into the cell
    int timesPerCell = itsTimeUnitsPerCell[TSysState::timeScale];
    startCell.Normalize( TSysState::timeScale );
    startCell.SetHour( (startCell.GetHour() / timesPerCell) * timesPerCell );
    startCell.SetMinute( 0 );

    // set up itsFirstCell and itsLastCell
    int midCol   = kNumActiveCols / 2;
    itsLastCell  = startCell.Plus( TSysState::timeScale,  midCol * timesPerCell );
    itsFirstCell = startCell.Plus( TSysState::timeScale, -midCol * timesPerCell );


    // let the grid set itself up
    Setup();

    // goto the midCol
    GotoRC( 0, midCol );
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
//          Setup
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

// ------------------------------------------------------------------------
// Setup
//
//   Split each note's time and value over the appropriate slots
//   of the graph.
// ------------------------------------------------------------------------
void TGrafGrid::Setup( void )
{
    // create a set of notes that falls within the first and last cells
    TFilter  filter( NULL, TRUE, kCatMatchAny, NULL, TRUE );
    TNoteSet noteset( filter, TSysState::pSelNotes );

    // clear the tables
    for (int i = 0; i < kNumActiveCols; i++) {

        itsSlotTable[i].time.Clear();
        itsSlotTable[i].value = 0.0;
    }


    // the grafgrid only graphs work tasks
    // not milestones or elapsed tasks
    int numNotes = noteset.getItemsInContainer();
    for (int n = 0; n < numNotes; n++ ) {

        RTNote note = noteset[n];
        if (note.GetStyle() == kTaskStyle && note.IsWorkTime()) {

            SetupTask( note );
        }
    }


    // get the biggest time and value
    itsMaxTime.Clear();
    itsMaxValue = 0;

    for (i = 0; i < kNumActiveCols; i++) {

        itsMaxValue = max( itsMaxValue, itsSlotTable[i].value );
        if (itsMaxTime < itsSlotTable[i].time) {

            itsMaxTime  = itsSlotTable[i].time;
        }
    }

    itsMaxTime.PlusEquals( itsMaxTime );
    itsMaxValue  *= 2;
}

// ------------------------------------------------------------------------
// SetupMilestone
//
//   Figure out which slot this note falls into and dump its entire
//   value into that slot (milestones add no time to a slot)
// ------------------------------------------------------------------------
void TGrafGrid::SetupMilestone( RTNote note )
{
    // milestones only add values
    if (!note.GetValue().HasValue()) {

        return;
    }


    // figure out which slot gets the note's value
    TBoundsPositions positions;
    TBounds bounds( itsFirstCell, itsLastCell );
    BOOL overlaps = bounds.NoteOverlaps( note, kNumActiveCols, positions );
    do {

        if (overlaps) {

            itsSlotTable[positions.startCell].value +=
                note.GetValue().ToDouble();
        }

    } while (bounds.NoteOverlapsAgain( note, kNumActiveCols, positions, overlaps ));
}

// ------------------------------------------------------------------------
// SetupTask
// ------------------------------------------------------------------------
void TGrafGrid::SetupTask( RTNote note )
{
    // calculate the WorkTime of the note and return if it has none
    TWorkTime noteWork = note.GetWorkTime();
    if (noteWork.IsZero()) {

        return;
    }

    TBoundsPositions positions;
    TBounds bounds( itsFirstCell, itsLastCell );
    BOOL overlaps = bounds.NoteOverlaps( note, kNumActiveCols, positions );
    do {

        if (!overlaps) {

            continue;
        }

        // whole note falls into 1 slot
        if (positions.startCell == positions.endCell) {

            double value = note.GetValue().ToDouble();
            itsSlotTable[positions.startCell].time.PlusEquals( noteWork );
            itsSlotTable[positions.startCell].value += value;
            continue;
        }

        // whole note not in 1 slot
        long   totalMins  = noteWork.GetNumUnits( k1Min );
        double totalValue = note.GetValue().ToDouble();

        // handle the first cell
        TTime start = note.GetStart();
        TTime end   = GetCellEnd( -1, positions.startCell );
        start       = max( start, itsFirstCell );
        if (!start.HasTime()) {

            start.SetToMidnight();
        }
        SetupCell( positions.startCell,
            start, end, totalMins, totalValue );

        // handle the last cell
        start = GetCellStart( -1, positions.endCell );
        end   = note.GetEnd();
        end   = min( end, itsLastCell );
        if (!end.HasTime()) {

            end.SetToMidnightTomorrow();
        }
        SetupCell( positions.endCell, 
            start, end, totalMins, totalValue );

        // handle the middle cells
        for (int i = positions.startCell + 1; i < positions.endCell; i++) {

            GetCellTimes( -1, i, start, end );
            SetupCell( i, 
                start, end, totalMins, totalValue );
        }

    } while (bounds.NoteOverlapsAgain( note, kNumActiveCols, positions, overlaps ));
}

// ------------------------------------------------------------------------
// SetupCell
//
//   fill the values for the specified cell, using the start and
//   end times specified. Works for times whose units are >= k1Day
// ------------------------------------------------------------------------
void TGrafGrid::SetupCell( int cell, RTTime start, RTTime end, long totalMins, double totalValue )
{
    TWorkTime workTime( start, end );
    long cellMins  = workTime.GetNumUnits( k1Min );
    double cellValue = totalValue * cellMins / totalMins;
    itsSlotTable[cell].time.PlusEquals( workTime );
    itsSlotTable[cell].value += cellValue;
}



// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
//          Painting
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
// CellWidth
// -------------------------------------------------------------------------
int  TGrafGrid::CellWidth( PTPaint pPainter, int  )
{
const kColWidth= 83;

if( pPainter->IsPrintPainter() )
    {
    RECT r;
    TPrint::GetPrtPixels( &r );
    int availWidth= r.right - r.left;

    //try to get a more or less even # of columns onto page
    int nCols= availWidth / ( kColWidth + GridWidth( pPainter, 1 ) );
    int slop= availWidth - nCols * (kColWidth + GridWidth( pPainter, 1 ) );

    //distribute slop over available columns
    return ( kColWidth + slop/nCols );
    }
else
    return kColWidth;
}

// -------------------------------------------------------------------------
// CellHeight
// -------------------------------------------------------------------------
int  TGrafGrid::CellHeight( PTPaint pPainter, int row )
{
if( row == 0 )
    {
    RECT r;
    if( pPainter->IsPrintPainter() )
        TPrint::GetPrtPixels( &r );
    else
        GetClientRect( HWindow, &r );

    return (r.bottom - r.top - 72 - 5);         //5 == size of gridlines
    }                                           //72 == size of other rows
else if( row == 1 )     //value
    return 36;
else // row == 2
    return 36;

}

//*****************************************************************************

//*****************************************************************************
int  TGrafGrid::GridWidth( PTPaint, int  )
{
return 1;
}

//*****************************************************************************

//*****************************************************************************
int  TGrafGrid::GridHeight( PTPaint, int row )
{
int h;

if( row == 0 )
    h= 2;
else
    h= 1;

return h;
}

//*****************************************************************************
// PaintCell
//*****************************************************************************
void  TGrafGrid::PaintCell( RTDrawInfo di )
{
    int row= di.row;
    int col= di.col;

    if (col == kTickMarkColumn) {

        // there are no ticks just yet
    }
    else {

        switch( row ) {

            case kGraphRow:
                PaintGraph( di );
                break;

            case kValueRow:
                PaintValue( di );
                break;

            case kTitleRow:
                PaintTitle( di );
                break;
        }
    }
}

//*****************************************************************************
// PaintGraph
//*****************************************************************************
void TGrafGrid::PaintGraph( RTDrawInfo di )
{
    double    thisValue;
    TWorkTime thisTime;

    int col= di.col;
    RECT *pRect= &di.cellRect;

    // add up the times and values
    thisTime  = itsSlotTable[col].time;
    thisValue = itsSlotTable[col].value;

    if (!itsMaxTime.IsZero()) {

        RECT      durRect;
        double    durPercent;

        long maxMinutes  = itsMaxTime.GetNumUnits( k1Min );
        long thisMinutes = thisTime.GetNumUnits( k1Min );

        durPercent       = ((double) thisMinutes) / maxMinutes;
        durRect          = *pRect;
        durRect.right    = durRect.left + (pRect->right - pRect->left)/2 - 4;
        durRect.left    += 4;
        durRect.top      = durRect.bottom - (int) (durPercent * (pRect->bottom - pRect->top));
        di.pPainter->FillRectangle( di, durRect, kHbrGrafTime );
    }

    if( itsMaxValue != 0.0 ) {

        RECT      valRect;
        double    valPercent;

        valPercent      = (thisValue / itsMaxValue);
        valRect         = *pRect;
        valRect.right   -= 4;
        valRect.left    = valRect.left + (pRect->right - pRect->left)/2 + 4;
        valRect.top     = valRect.bottom - (int)
                         (valPercent * (pRect->bottom - pRect->top));
        di.pPainter->FillRectangle( di, valRect, kHbrGrafCost );
    }

}

//*****************************************************************************
// PaintValue
//*****************************************************************************
void TGrafGrid::PaintValue( RTDrawInfo di )
{
    char      buf[50];
    TSafStr   str;

    TTime cellStart = GetCellStart( -1, di.col );

    // intitialize values
    RECT r           = di.cellRect;
    int height       = (r.bottom - r.top) / 2;

    // time
    if (!itsSlotTable[di.col].time.IsZero()) {

        itsSlotTable[di.col].time.Format(
            (TTimeScale) TProfile::baseTimeFmtUnits,
            TProfile::bMixTimeFmtUnits, buf );

        // if the time would fit underneath the bar, then center it
        int width = LOWORD( GetTextExtent( di.hdc, buf, lstrlen( buf )));
        if (FALSE && width <= (r.right - r.left) / 2 - 12) {

            // center the text under the bar
            RECT durRect     = r;
            durRect.right    = durRect.left + (r.right - r.left) / 2 - 4;
            durRect.left    += 4;
            durRect.bottom   = r.top + height;

            di.pPainter->Text( di, buf, kNoHilite, DT_CENTER,
                kNormalFont, &durRect );
        }

        // otherwise left-justify the time
        else {

            // left-justify the text
            RECT durRect     = r;
            durRect.left    += 4;
            durRect.bottom   = r.top + height;

            di.pPainter->Text( di, buf, kNoHilite, DT_LEFT,
                kNormalFont, &durRect );
        }
    }

    // value
    if (itsSlotTable[di.col].value != 0) {

        TValue value;
        value = itsSlotTable[di.col].value;
        TIntlFmt::FmtDollar( value, str, FALSE );

        // if the value would fit underneath the bar, then center it
        int width = LOWORD( GetTextExtent( di.hdc, str, lstrlen( buf )));
        if ( FALSE && width <= (r.right - r.left) / 2 - 12) {

            // center the text under the bar
            RECT valRect     = r;
            valRect.left     = valRect.left + (r.right - r.left) / 2 + 4;
            valRect.right   -= 4;
            valRect.top      = r.bottom - height;

            di.pPainter->Text( di, str, kNoHilite, DT_CENTER,
                kNormalFont, &valRect );
        }

        // otherwise right-justify
        else {

            // right-justify the text
            RECT valRect     = r;
            valRect.right   -= 4;
            valRect.top      = r.bottom - height;

            di.pPainter->Text( di, str, kNoHilite, DT_RIGHT,
                kNormalFont, &valRect );
        }
    }
}

//*****************************************************************************
// PaintTitle
//*****************************************************************************
void TGrafGrid::PaintTitle( RTDrawInfo di )
{
    char buf[200];

    TTime cellStart = GetCellStart( -1, di.col );
    TGridUtil::PaintBackground( di, cellStart, TRUE, FALSE );
    FormatTime( cellStart, buf );

    RECT r= di.cellRect;
    r.top += 5; //text looks better when moved down
    di.pPainter->Text( di, buf, kNoHilite, kMultiLine | DT_CENTER, kBoldFont, &r );
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
//          Mouse handling
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

//*****************************************************************************
// ShowNoteList
//
//   bring up note list if right click over graph bar
//*****************************************************************************
void TGrafGrid::ShowNoteList( int col )
{
    TTime start, end;

    GetCellTimes( -1, col, start, end );

    TFilter filter( NULL, 
        TRUE,           //select from all categories
        kCatMatchAny,
        NULL,           //search text
        TRUE            //don't include notes w/o dates
        );
    if (TSysState::timeScale == SCALE_DAY)
        filter.SetFilterDates( kDayFilter, start );
    else if (TSysState::timeScale == SCALE_WEEK)
        filter.SetFilterDates( kWeekFilter, start );
    else if (TSysState::timeScale == SCALE_MONTH)
        filter.SetFilterDates( kMonthFilter, start );

    // get all of the currently selected notes that
    // match that filter
    TNoteSet theNotes( filter, TSysState::pSelNotes );
    TNoteSet usedNotes;

    // copy only the used notes
    for (int i = 0; i < theNotes.getItemsInContainer(); i++) {

        RTNote note = theNotes[i];
        BOOL used = (note.GetStyle() == kTaskStyle) &&
                     note.IsWorkTime();
        if (used) {

            usedNotes.add( note );
        }
    }
    
    // popup a list if there are any notes in the set
    int numNotes = usedNotes.getItemsInContainer();
    if (numNotes > 0) {

        TNoteListDialog *noteListDialog = new TNoteListDialog(
            TPimFrame::pFrame, IDD_NOTELIST_DIALOG,
            usedNotes, " Contributing Notes" );

        // if we didn't cancel out of the box then show a note
        int selection = GetApplicationObject()->ExecDialog( noteListDialog );
        if (selection != kCanceledNotelist) {

            GetApplication()->ExecDialog(new TNoteView(
                TPimFrame::pFrame, IDD_NOTE, &usedNotes[selection] ));
        }
    }
}

//*****************************************************************************
// MousRtClick
//
//   Info box 
//*****************************************************************************
void TGrafGrid::MousRtClick( RTMousInfo mi )
{
    if( mi.row == kGraphRow )
        {
        TMousHelp::MousHelp( this,
            "Double-click left button to see the list of notes that fall into this time period." );
        }
    else if( mi.row == kTitleRow && TSysState::timeScale > SCALE_DAY )
        {
        TMousHelp::MousHelp( this,
            "Double-click left button to change the time scale (zoom in)." );
        }
}

//*****************************************************************************

//*****************************************************************************
void TGrafGrid::MousDblClick( RTMousInfo mi )
{
    if( mi.row == kTitleRow && TSysState::timeScale > SCALE_DAY ) {

        // must be done before the timescale is changed
        TSysState::selTime = GetCellStart( -1, mi.col );

        if (TSysState::timeScale > SCALE_DAY) {

            TSysState::timeScale = (TTimeScale) (TSysState::timeScale - 1);
        }

        TSnoop::Notify( kTimeChg, NOOBJECT );

        ResetTime( TSysState::selTime );
        InvalidateCell(-1,-1);
        Notify( kScaleChg, NOOBJECT );
    }
    else if( mi.row == kGraphRow ) {        //bring up note list

        ShowNoteList( mi.col );
    }
}


//*****************************************************************************

//*****************************************************************************
void TGrafGrid::MousOver( RTMousInfo mi )
{
if( mi.row == kTitleRow && TSysState::timeScale > SCALE_DAY )
   TCursor::Set( kZoomCursor );
else if( mi.row == kGraphRow )
   TCursor::Set( kNoteSelCursor );
else
   TCursor::Set( kArrowCursor );
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
void TGrafGrid::WMHScroll( RTMessage Msg)
{
    // send the corrected version off to basegrid
    TBaseGrid::WMHScroll( Msg );

    // save selected time
    TTime date = GetCellStart( 0, hScrollPos + frozenCols );
    TSysState::selTime.CopyDate( date );
    Notify( kTimeChg, NOOBJECT );
}

//---------------------------------------------------------------------------
// WMVScroll
//
// Keeps track of the selected time
//---------------------------------------------------------------------------
void TGrafGrid::WMVScroll( RTMessage Msg)
{
    // send the corrected version off to basegrid
    TBaseGrid::WMVScroll( Msg );

    // save selected time
    TTime date = GetCellStart( 0, hScrollPos + frozenCols );
    TSysState::selTime.CopyDate( date );
    Notify( kTimeChg, NOOBJECT );
}



// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
//          Listen
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

void TGrafGrid::Listen( TSnoopMsg aMsg, RObject )
{
    switch( aMsg ) {

        case kNoteSetChg:
            Setup();
            InvalidateCell(-1,-1);
            break;

        case kTimeChg:
        case kScaleChg:
            ResetTime( TSysState::selTime );
            InvalidateCell(-1,-1);
            break;

        default:

            break;
    }
}

//*****************************************************************************

//*****************************************************************************
void TGrafGrid::SetDefaultPrintRange( void )
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
BOOL TGrafGrid::HChgCellStartIndex( int n, char *buf, int /* size */ )
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
BOOL TGrafGrid::HChgCellEndIndex( int n, char *buf, int /* size */ )
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

// ----------------------------------------------------------------------
// GetCellStart
//
// Default is to ignore row
// ----------------------------------------------------------------------
TTime TGrafGrid::GetCellStart( int /* row */, int col )
{
    return itsFirstCell.Plus( TSysState::timeScale,
        (col - frozenCols) * itsTimeUnitsPerCell[TSysState::timeScale] );
}

// ----------------------------------------------------------------------
// GetCellEnd
//
// Default is to ignore row
// ----------------------------------------------------------------------
TTime TGrafGrid::GetCellEnd( int /* row */, int col )
{
    return itsFirstCell.Plus( TSysState::timeScale,
        (col - frozenCols + 1) * itsTimeUnitsPerCell[TSysState::timeScale] );
}

// ----------------------------------------------------------------------
// GetCellTimes
//
// Default is to ignore row
// ----------------------------------------------------------------------
void TGrafGrid::GetCellTimes( int /* row */, int col, RTTime start, RTTime end )
{
    start = itsFirstCell.Plus( TSysState::timeScale,
        (col - frozenCols)     * itsTimeUnitsPerCell[TSysState::timeScale] );
    end   = itsFirstCell.Plus( TSysState::timeScale,
        (col - frozenCols + 1) * itsTimeUnitsPerCell[TSysState::timeScale] );

}

// ----------------------------------------------------------------------
// FormatTime
// ----------------------------------------------------------------------
void TGrafGrid::FormatTime( RTTime time, char *buf )
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

//////////////////////////////////////////////////////////////////////////////
///////////////////////////// FOCUS //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
//set focus to first visible bar
//*****************************************************************************
void TGrafGrid::GetDefaultFocusCell( int &row, int &col )
{
row= 0;
col= hScrollPos;
}

//*****************************************************************************

//*****************************************************************************
void TGrafGrid::MousKey( TMousKey key )
{
TMousGrid::MousKey( key );
}

//*****************************************************************************
// IsCellSelectable
//*****************************************************************************
BOOL TGrafGrid::IsCellSelectable( int row, int  )
{
    return( row == 0 );
}

