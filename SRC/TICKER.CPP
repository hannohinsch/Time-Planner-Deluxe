//*****************************************************************************
// timepicker "slider" control
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "ticker.h"

#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __duration__
#include "duration.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __paint__
#include "paint.h"
#endif
#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __cursor__
#include "cursor.h"
#endif

const int kColCount                    = 720;
const int kColsOnScreen                = 19;
const int kMidCol                      = kColCount / 2;
const int kColWidth                    = 20;
const int kLineHeight                  = 17;

static int PreviousCol;

// ------------------------------------------------------------------
// Constructor
// ------------------------------------------------------------------
TTicker::TTicker( PTWindowsObject AParent, int resId, RTTime startTime,
    RTDuration duration, PTTimeUnit unitP ):
    TMousGrid( AParent, resId )
{
    scrPainter= new TPaint;
    prtPainter= new TPaintPrt;

    itsTimeUnitP     = unitP;
    itsDuration      = duration;
    itsStartTime     = startTime;

    if (!duration.IsZero() && startTime.HasDate()) {

        itsMode = kStartEnd;
    }
    else if (startTime.HasDate()) {

        itsMode = kMilestoneOnly;
    }
    else {

        itsMode = kDurationOnly;
    }

    SetRowCnt( 2 );
    SetRowFreeze( 0 );
    SetColCnt( kColCount );
    SetColFreeze( 0 );

    NormalizeCells();
    CalculateSelection( TRUE );
}

// ------------------------------------------------------------------
// Destructor
// ------------------------------------------------------------------
TTicker::~TTicker( void )
{
    delete scrPainter;
    delete prtPainter;
}

// -------------------------------------------------------------------------
// NormalizeCells
//
//     Normalizes itsStartTime and itsFirstCell based on itsTimeUnit
// -------------------------------------------------------------------------
void TTicker::NormalizeCells( void )
{
    switch( itsMode ) {

        case kDurationOnly:     itsDuration.Normalize( *itsTimeUnitP ); break;
        case kMilestoneOnly:    NormalizeStartTime();                   break;
        case kStartEnd:         NormalizeCellsStartEnd();               break;
        default:

            SYSERR_STR( "NormalizeCells: unexpected case" );
    }

    InvalidateCell( -1, -1 );
}


// -------------------------------------------------------------------------
// NormalizeStartTime
// -------------------------------------------------------------------------
void TTicker::NormalizeStartTime( void )
{
    itsStartTime.Normalize( *itsTimeUnitP );
    itsFirstCell = itsStartTime.Plus( *itsTimeUnitP, -kMidCol );
}

// -------------------------------------------------------------------------
// NormalizeCellsStartEnd
// -------------------------------------------------------------------------
void TTicker::NormalizeCellsStartEnd( void )
{
    itsDuration.Normalize( *itsTimeUnitP );
    NormalizeStartTime();

    // make sure there's a duration
    if (itsDuration.IsZero()) {

        itsDuration.Set( *itsTimeUnitP, 1 );
    }
}



// ------------------------------------------------------------------
// GetColWidth
// ------------------------------------------------------------------
int  TTicker::GetColWidth( int  )
{
    return( kColWidth );
}

// ------------------------------------------------------------------
// GetRowHeight
// ------------------------------------------------------------------
int  TTicker::GetRowHeight( int )
{
    return( kLineHeight * 2 );
}


// ------------------------------------------------------------------
// PaintCell
// ------------------------------------------------------------------
void TTicker::PaintCell( RTDrawInfo di )
{
    static int paintNeighborCount = 0;

    switch( di.row ) {

        case 0: PaintRow0( di ); break;
        case 1: PaintRow1( di ); break;
    }

    if (di.cellRect.left == LeftLineWidth( di.pPainter )) {

        TDrawInfo info = di;
        info.cellRect.left  =0;
        info.cellRect.right = 1;
        PaintLeftLine( info );
    }

    // the label that appears in this cell may be controlled
    // by the cell to the right.  If so, make it paint its stuff.
    char buf[256];
    TDrawInfo toTheRight= di;
    toTheRight.col++;
    if( (paintNeighborCount == 0) &&
        (Label(                  toTheRight, buf ) ||
         Row1TextDurationNeeded( toTheRight, buf ) ||
         Row1TextTimeNeeded(     toTheRight, buf ))) {

        paintNeighborCount++;
        toTheRight.cellRect.left+= kColWidth;
        toTheRight.cellRect.right+= kColWidth;
        PaintCell( toTheRight );
        paintNeighborCount--;
    }


}

// ------------------------------------------------------------------
// PaintTopLine
// ------------------------------------------------------------------
void TTicker::PaintTopLine( RTDrawInfo di )
{
    FrameRect( di.hdc, &di.cellRect, TGDIMap::GetScreenBrush( kHbrBorder ) );
}

// ------------------------------------------------------------------
// PaintLeftLine
// ------------------------------------------------------------------
void TTicker::PaintLeftLine( RTDrawInfo di )
{
    FrameRect( di.hdc, &di.cellRect, TGDIMap::GetScreenBrush( kHbrBorder ) );
}

// ------------------------------------------------------------------
// PaintRow0
//
//    Paint time and ticks
// ------------------------------------------------------------------
void TTicker::PaintRow0( RTDrawInfo di )
{

    char  buf[256];

    if (Label( di, buf )) {

        PaintTick( di, TRUE );

        di.cellRect.left  -= kColWidth;
        int l = strlen( buf );
        buf[l] = '\n';
        buf[l+1] = '\0';

        TGDIMap::SetColorEnvironment( di.hdc, kEnvView );
        di.pPainter->Text( di, buf, kNoHilite,
            kMultiLine | DT_CENTER,
            kNormalFont, &di.cellRect );
        di.cellRect.left += kColWidth;
    }
    else {

        PaintTick( di, FALSE );
    }



    if( IsCellInRange( di.row, di.col ) ) {

        // if there's a duration then invert the cells
        if (!itsDuration.IsZero()) {

            RECT r = di.cellRect;
            r.top  = r.bottom - 10;
            r.bottom--;
            FillRect( di.hdc, &r, TGDIMap::GetScreenBrush( kHbrTimeMark ) );

            int row, col, width, height;
            GetRangeAnchor( row, col );
            GetRangeExtent( height, width );
        }

        else {

            PaintArrow( di, TRUE );
        }
    }
}

// ------------------------------------------------------------------
// PaintArrow
// ------------------------------------------------------------------
void TTicker::PaintArrow( RTDrawInfo di, BOOL leftSide )
{
    POINT  points[3];

    if (leftSide) {

        points[0].x = di.cellRect.left;
        points[0].y = di.cellRect.bottom - 1;
        points[1].x = di.cellRect.left + 10;
        points[1].y = di.cellRect.bottom - 1;
        points[2].x = di.cellRect.left;
        points[2].y = di.cellRect.bottom - 20;

    }
    else {

        points[0].x = di.cellRect.right;
        points[0].y = di.cellRect.bottom - 1;
        points[1].x = di.cellRect.right - 10;
        points[1].y = di.cellRect.bottom - 1;
        points[2].x = di.cellRect.right;
        points[2].y = di.cellRect.bottom - 20;
    }


	// draw it
    HANDLE h = CreatePolygonRgn( points, 3, ALTERNATE );
    FillRgn( di.hdc, h, TGDIMap::GetScreenBrush( kHbrTimeMark ) );
    DeleteObject( h );
}


// ------------------------------------------------------------------
// PaintRow1
// ------------------------------------------------------------------
void TTicker::PaintRow1( RTDrawInfo di )
{
    Row1Color( di );
    Row1Text( di );

    // bottom line
    MoveTo( di.hdc, di.cellRect.left,  di.cellRect.bottom );
    LineTo( di.hdc, di.cellRect.right, di.cellRect.bottom );
}


// ------------------------------------------------------------------
// ColToTime
// ------------------------------------------------------------------
void TTicker::ColToTime( int col, RTTime time )
{
    time = itsFirstCell.Plus( *itsTimeUnitP, col - 1 );
}


// ------------------------------------------------------------------
// TimeToCol
//
// the fact that
// ------------------------------------------------------------------
void TTicker::TimeToCol( RTTime time, int &col )
{
    TDuration duration( itsFirstCell, time );
    col = (int) duration.GetNumUnits( *itsTimeUnitP );

    // we don't use the the first column: it's there only to make sure
    // that the text can be drawn over the tick mark, so we need to
    // add a column. (but if the unit is days, then GetNumUnits
    // will have added 1 already, since apr 4 to apr 4 is one day)
    if (*itsTimeUnitP != k1Day) {

        col++;
    }
}


// ------------------------------------------------------------------
// PaintTick
//
//    Paint tick
// ------------------------------------------------------------------
void TTicker::PaintTick( RTDrawInfo di, BOOL longTick )
{

    int top    = di.cellRect.top;
    int left   = di.cellRect.left;
    int bottom = di.cellRect.bottom - 1;
    int height = di.cellRect.bottom - di.cellRect.top;

    if (longTick) {

        MoveTo( di.hdc, left, bottom );
        LineTo( di.hdc, left, top + height / 2);
    }
    else {

        MoveTo( di.hdc, left, bottom );
        LineTo( di.hdc, left, bottom - height / 4);
    }

    // bottom line
    MoveTo( di.hdc, left, bottom );
    LineTo( di.hdc, di.cellRect.right - 1, bottom  );
}



// ------------------------------------------------------------------
// Row1Text
//
//    Decide what text should go into row 1 based on the unit
// ------------------------------------------------------------------
void TTicker::Row1Text( RTDrawInfo di )
{
    if (itsMode == kDurationOnly) {

        Row1TextDuration( di );
    }
    else {

        Row1TextTime( di );
    }
}

// ------------------------------------------------------------------
// Row1TextDurationNeeded
// ------------------------------------------------------------------
BOOL TTicker::Row1TextDurationNeeded( RTDrawInfo di, char *text )
{
    if ((di.col % kColsOnScreen) == (kColsOnScreen / 2)) {

        switch (*itsTimeUnitP) {

            case k1Min:
            case k5Min:
            case k6Min:
            case k10Min:
            case k12Min:
            case k15Min:
            case k20Min:
            case k30Min:
            case k1Hour:  strcpy( text, "Hours" );     return TRUE;
            case k1Day:   strcpy( text, "Days" );      return TRUE;
            case k1Week:  strcpy( text, "Weeks" );     return TRUE;
       }
    }

    return FALSE;
}

// ------------------------------------------------------------------
// Row1TextDuration
// ------------------------------------------------------------------
void TTicker::Row1TextDuration( RTDrawInfo di )
{
    char text[80];

    if (Row1TextDurationNeeded( di, text )) {

        RECT    r = di.cellRect;
        r.left   -= kColWidth;

        TGDIMap::SetColorEnvironment( di.hdc, kEnvDarkScale );
        di.pPainter->Text( di, text, kNoHilite,
            kSingleLine | DT_CENTER | DT_VCENTER, kNormalFont, &r );
    }
}

// ------------------------------------------------------------------
// Row1TextTime
// ------------------------------------------------------------------
BOOL TTicker::Row1TextTimeNeeded( RTDrawInfo di, char *text )
{
    TTime   time;
    char    buf1[80];

    wsprintf( text, "" );
    ColToTime( di.col, time );
    int d = time.GetDay();
    int y = time.GetYear();

    // ------------
    // days get labels every Sunday
    if ((*itsTimeUnitP == k1Day) && (time.GetWeekDay() == 0)) {

        TIntlFmt::FormatDayAndMonth( time, buf1 );
        wsprintf( text, "%s\n%d", buf1, y );
        return TRUE;
    }

    // ------------
    // weeks get labels on the third week of the month
    else if ((*itsTimeUnitP == k1Week) && (((d - 1) / 7) == 2)) {

        TIntlFmt::FormatMonthName( time, kThreeLetter, buf1 );
        wsprintf( text, "%s\n%d", buf1, y );
        return TRUE;
    }

    // ------------
    // hours get labels depending on the time units
    BOOL    amCondition;
    BOOL    pmCondition;
    int h = time.GetHour();
    int m = time.GetMinute();

    switch( *itsTimeUnitP ) {

        case k1Min:

            // every 10 minutes on the :05's
            amCondition = (h <= 12) && ((m % 10) == 5);
            pmCondition = (h >  12) && ((m % 10) == 5);
            break;

        case k5Min:
        case k6Min:

            // every hour on the :30's
            amCondition = (h <= 12) && (m == 30);
            pmCondition = (h >  12) && (m == 30);
            break;

        case k10Min:
        case k12Min:

            // 1,3,5,7,9,11 o'clock am and pm
            amCondition = (h <= 12) && (h % 2 == 1) && (m == 0);
            pmCondition = (h >  12) && (h % 2 == 1) && (m == 0);
            break;

        case k15Min:

            // 1, 4, 7, 10 o'clock am and pm
            amCondition = (h <= 12) && (h % 3 == 1) && (m == 0);
            pmCondition = (h >  12) && (h % 3 == 1) && (m == 0);
            break;

        case k20Min:

            // 2, 6, 10 o'clock am and pm
            amCondition = (h <= 12) && (h % 4 == 2) && (m == 0);
            pmCondition = (h >  12) && (h % 4 == 2) && (m == 0);
            break;

        case k30Min:

            // 3, 9 o'clock am and pm
            amCondition = (h <= 12) && (h % 6 == 3) && (m == 0);
            pmCondition = (h >  12) && (h % 6 == 3) && (m == 0);
            break;

        case k1Hour:

            // once in the morning, one in the evening
            amCondition = (h == 6);
            pmCondition = (h == 18);
            break;

        default:

            // (days and weeks that need labels were dealt with above)
            return FALSE;
    }


    // unit <= k1Hour fall through to here
    if (amCondition) {

        TIntlFmt::FormatDayAndMonth( time, buf1 );
        wsprintf( text, "am\n%s", buf1 );
        return TRUE;
    }
    else if (pmCondition) {

        TIntlFmt::FormatDayAndMonth( time, buf1 );
        wsprintf( text, "pm\n%s", buf1 );
        return TRUE;
    }
    else {

        return FALSE;
    }
}

// ------------------------------------------------------------------
// Row1TextTime
// ------------------------------------------------------------------
void TTicker::Row1TextTime( RTDrawInfo di )
{
    char    text[80];
    if (Row1TextTimeNeeded( di, text )) {

        RECT    r = di.cellRect;
        r.left   -= kColWidth;

        di.pPainter->Text( di, text, kNoHilite,
            kMultiLine | DT_CENTER | DT_TOP, kNormalFont, &r );
    }
}

// ------------------------------------------------------------------
// Row1Color
//
//    Based on the column and the time fill in the color of row 1,
//    before any text is drawn
// ------------------------------------------------------------------
void TTicker::Row1Color( RTDrawInfo di )
{
    if (itsMode == kDurationOnly) {

        // just make it gray
        HBRUSH hbr= TGDIMap::SetColorEnvironment( di.hdc, kEnvDarkScale );
        di.pPainter->FillRectangle( di, di.cellRect, hbr );
    }
    else {

        Row1ColorTime( di );
    }
}


// ------------------------------------------------------------------
// Row1ColorTime
//
//    Based on the column and the time fill in the color of row 1,
//    before any text is drawn
// ------------------------------------------------------------------
void TTicker::Row1ColorTime( RTDrawInfo di )
{
    TTime  time;
    BOOL   light = TRUE;

    ColToTime( di.col, time );

    switch( *itsTimeUnitP ) {

        case k1Min:
        case k5Min:
        case k6Min:
        case k10Min:
        case k12Min:
        case k15Min:
        case k20Min:
        case k30Min:
        case k1Hour:

            light = (time.GetHour() >= 12);
            break;

        case k1Day:

            int weekDay = time.GetWeekDay();
            light = (1 <= weekDay) && (weekDay <= 5);
            break;

        case k1Week:

            light = (time.GetMonth() % 2) == 0;
            break;
    }


    if (light) {

        HBRUSH hbr= TGDIMap::SetColorEnvironment( di.hdc, kEnvLightScale );
        di.pPainter->FillRectangle( di, di.cellRect, hbr );
    }
    else {

        HBRUSH hbr= TGDIMap::SetColorEnvironment( di.hdc, kEnvDarkScale );
        di.pPainter->FillRectangle( di, di.cellRect, hbr );
    }

}



// ------------------------------------------------------------------
// Label
//
//    Returns TRUE if the cell gets a long tick, FALSE for a short one.
//    If it gets a long tick, it also gets text in row 0, which is
//    specified in buf
// ------------------------------------------------------------------
BOOL TTicker::Label( RTDrawInfo di, char *buf )
{
    if (itsMode == kDurationOnly) {

        return( LabelDuration( di, buf ));
    }
    else {

        return( LabelTime( di, buf ));
    }
}

// ------------------------------------------------------------------
// LabelDuration
//
//    Returns TRUE if the cell gets a long tick, FALSE for a short one.
//    If it gets a long tick, it also gets text in row 0, which is
//    specified in buf
//
//    Works for durations only, not real times
// ------------------------------------------------------------------
BOOL TTicker::LabelDuration( RTDrawInfo di, char *buf )
{
    BOOL label   = FALSE;
    BOOL whole = TRUE;

    switch( *itsTimeUnitP ) {

        case k1Min:  label = ((di.col % 5) == 1); whole = FALSE; break;
        case k5Min:  label = ((di.col % 3) == 1); whole = FALSE; break;
        case k6Min:  label = ((di.col % 5) == 1); whole = FALSE; break;
        case k10Min: label = ((di.col % 6) == 1);                break;
        case k15Min: label = ((di.col % 4) == 1);                break;
        case k12Min: label = ((di.col % 5) == 1);                break;
        case k20Min: label = ((di.col % 3) == 1);                break;
        case k30Min: label = ((di.col % 2) == 1);                break;
        case k1Hour: label = ((di.col % 2) == 1);                break;
        case k1Day:  label = ((di.col % 2) == 1);                break;
        case k1Week: label = ((di.col % 2) == 1);                break;
        default:     return( FALSE );
    }


    // unit <= k1hour fall through to here if they need a label
    if (label) {

        if (*itsTimeUnitP < k1Hour) {

            int mins, hours;
            mins  = kMinutesPerTimeUnit[*itsTimeUnitP] * (di.col - 1);
            hours = mins / 60;
            mins  = mins % 60;

            if (whole) {

                wsprintf( buf, "%d", hours );
            }
            else {

                wsprintf( buf, "%d:%0.2d", hours, mins );
            }
        }
        else {

            wsprintf( buf, "%d", di.col - 1 );
        }
        return( TRUE );
    }
    else {

        return( FALSE );
    }
}

// ------------------------------------------------------------------
// LabelTime
//
//    Returns TRUE if the cell gets a long tick, FALSE for a short one.
//    If it gets a long tick, it also gets text in row 0, which is
//    specified in buf
//
//    Works for real times, rather than just durations
// ------------------------------------------------------------------
BOOL TTicker::LabelTime( RTDrawInfo di, char *buf )
{
    TTime time;
    BOOL label;

    ColToTime( di.col, time );
    int h = time.GetHour();
    int m = time.GetMinute();


    switch( *itsTimeUnitP ) {

        case k1Min:

            // every 5 minutes
            label = ((m % 5) == 0);
            break;

        case k5Min:

            // every 15 minutes
            label = ((m % 15) == 0);
            break;

        case k6Min:

            // every half hour
            label = ((m % 30) == 0);
            break;

        case k10Min:
        case k15Min:
        case k12Min:
        case k20Min:
        case k30Min:

            // every hour
            label = (m == 0);
            break;

        case k1Hour:

            // every even hour
            label = ((h % 2) == 0);
            break;

        case k1Day:

            int weekDay = time.GetWeekDay();
            if ((1 <= weekDay) && (weekDay <= 5)) {

                TIntlFmt::DayName( weekDay, kOneLetter, buf );
                return( TRUE );
            }
            else {

                return( FALSE );
            }

        case k1Week:

            wsprintf( buf, "%d", time.GetDay() );
            return( TRUE );

        default:

            return( FALSE );
    }


    // unit <= k1hour fall through to here if they need a label
    if (label) {

        TIntlFmt::FormatTime( time, TRUE, buf );
        if (buf[strlen( buf ) - 1] == 'm') {

            buf[strlen( buf ) - 3] = '\0';
        }
        return( TRUE );
    }
    else {

        return( FALSE );
    }
}

// ------------------------------------------------------------------
// MousRangeInit
// ------------------------------------------------------------------
BOOL TTicker::MousRangeInit( RTMousInfo mi )
{
    // initialize the previous column to make sure all drawing is done
    PreviousCol = -99;

    switch (itsMode) {

        case kDurationOnly:  return RangeInitDuration( mi );
        case kMilestoneOnly: return RangeInitMilestone( mi );
        case kStartEnd:      return RangeInitStartEnd( mi );
        default:

            SYSERR_STR( "MousRangeInit: unexpected case" );
            return FALSE;
    }

}


// ------------------------------------------------------------------
// MousRangeOver
//
//   Tell the parent that the value is changing
// ------------------------------------------------------------------
void TTicker::MousRangeOver( RTMousInfo mi )
{
    // make sure we're in the first row!
    mi.row = 0;

    switch (itsMode) {

        case kDurationOnly:   RangeOverDuration(  mi ); return;
        case kMilestoneOnly:  RangeOverMilestone( mi ); return;
        case kStartEnd:       RangeOverStartEnd(  mi ); return;
    }
}

// ------------------------------------------------------------------
// MousRangeRelease
//
//      Set the end time
// ------------------------------------------------------------------
void TTicker::MousRangeRelease( RTMousInfo mi )
{
    switch( itsMode ) {

        case kDurationOnly:  RangeReleaseDuration(  mi );  return;
        case kMilestoneOnly: RangeReleaseMilestone( mi );  return;
        case kStartEnd:      RangeReleaseStartEnd(  mi );  return;
        default:

            SYSERR_STR( "MousRangeRelease: unexpected case" );
    }

}

// ------------------------------------------------------------------
// RangeInitDuration
// ------------------------------------------------------------------
BOOL TTicker::RangeInitDuration( RTMousInfo  )
{
    int top, left, bottom, right;

    GetRangeRect(   top, left, bottom, right );
    SetRangeAnchor( 0, 1 );
    SetRangeExtent( 0, right - left );
    CalculateDuration();
    return TRUE;
}

// ------------------------------------------------------------------
// RangeOverDuration
// ------------------------------------------------------------------
void TTicker::RangeOverDuration( RTMousInfo mi )
{
    if (XYLeft( mi )) {

        Scroll( 0, 1 );
        UpdateWindow( HWindow );
    }
    else if (XYRight( mi )) {

        Scroll( 0, -1 );
        UpdateWindow( HWindow );
    }
    else if (XYWithin( mi ) && (mi.col != PreviousCol)) {

        PreviousCol = mi.col;

        // if the col goes to the left of zero, the duration is zero
        if (mi.col < 1) {

            SetRangeAnchor( -1, -1 );
            itsDuration.Clear();
        }

        // otherwise, set the starting position of the selection
        else {

            int row, col, height, width;

            GetRangeAnchor( row, col );
            GetRangeExtent( height, width );

            // erase the old arrow at the non-anchor end
            InvalidateCell( 0, col + width );

            // if the duration used to be 0, then the col
            // might be -1 and needs to be reset to 1
            if (col == -1) {

                SetRangeAnchor( 0, 1 );
            }

            // let MousGrid do the real work and then calculate the duration
            TMousGrid::MousRangeOver( mi );
            CalculateDuration();

            // draw the new arrow at the non-anchor end
            GetRangeExtent( height, width );
            InvalidateCell( 0, col + width );
        }
    }


    // notify dad
    SendMessage( Parent->HWindow, WM_COMMAND, Attr.Id,
        MAKELONG( HWindow, 0 ));
}

// ------------------------------------------------------------------
// RangeReleaseDuration
// ------------------------------------------------------------------
void TTicker::RangeReleaseDuration( RTMousInfo mi )
{
    TMousGrid::MousRangeRelease( mi );

    if (mi.col == 0) {

        SetRangeAnchor( -1, -1 );
    }

    CalculateDuration();

    // tell dad
    SendMessage( Parent->HWindow, WM_COMMAND, Attr.Id,
        MAKELONG( HWindow, 0 ));
}

// ------------------------------------------------------------------
// RangeInitMilestone
// ------------------------------------------------------------------
BOOL TTicker::RangeInitMilestone( RTMousInfo mi )
{
    SetRangeAnchor( 0, mi.col );
    return TRUE;
}

// ------------------------------------------------------------------
// RangeOverMilestone
// ------------------------------------------------------------------
void TTicker::RangeOverMilestone( RTMousInfo mi )
{
    if (XYLeft( mi )) {

        int  row, col;
        GetRangeAnchor( row, col );
        if (col < 1) return;

        SetRangeAnchor( row, col - 1 );
        Scroll( 0, 1 );
        CalculateStart();
        UpdateWindow( HWindow );
    }
    else if (XYRight( mi )) {

        int  row, col;
        GetRangeAnchor( row, col );
        if (col >= kColCount - 1) return;

        SetRangeAnchor( row, col + 1 );
        Scroll( 0, -1 );
        CalculateStart();
        UpdateWindow( HWindow );
    }
    else if (XYWithin( mi ) && (mi.style == kCellPoint) && (mi.col != PreviousCol)) {

        PreviousCol = mi.col;

        // only 1 cell should be selected in milestoneOnly mode
        // so reset the selection so that it's the same as the new column
        SetRangeAnchor( 0, mi.col );
        TMousGrid::MousRangeOver( mi );

        CalculateStart();
    }
    else {

        return;
    }

    SendMessage( Parent->HWindow, WM_COMMAND, Attr.Id,
        MAKELONG( HWindow, 0 ));
}


// ------------------------------------------------------------------
// RangeReleaseMilestone
// ------------------------------------------------------------------
void TTicker::RangeReleaseMilestone( RTMousInfo mi )
{
    TMousGrid::MousRangeRelease( mi );
    CalculateStart();

    // tell dad
    SendMessage( Parent->HWindow, WM_COMMAND, Attr.Id,
        MAKELONG( HWindow, 0 ));
}

// ------------------------------------------------------------------
// RangeInitStartEnd
// ------------------------------------------------------------------
BOOL TTicker::RangeInitStartEnd( RTMousInfo mi )
{
    int top, left, bottom, right;

    // there better be a range, since the minimum is 1 column
    if (!GetRangeRect( top, left, bottom, right )) {

        SYSERR_STR( "RangeInitStartEnd: no range!" );
        return FALSE;
    }


    // we're only interested in row 0 and cols within the grid
    if ((mi.row != 0) || (mi.col < 1) || (mi.col >= kColCount)) {

        return FALSE;
    }

    // a click in the col to the left of the range means we're growing left
    if (left - 1 == mi.col) {

        SetRangeAnchor( 0, right );
        SetRangeExtent( 0, -(right - left + 1) );
        itsScrollMode = kGrowLeft;
        return TRUE;
    }


    // a click in the col to the right of the range means we're growing right
    else if (mi.col == right + 1) {

        SetRangeAnchor( 0, left );
        SetRangeExtent( 0, right - left + 1 );
        itsScrollMode = kGrowRight;
        return TRUE;
    }

    // a click within the range means we're dragging
    else if ((left <= mi.col) && (mi.col <= right)) {

        SetRangeAnchor( 0, left );
        SetRangeExtent( 0, right - left );
        itsDragOffset = mi.col - left;
        itsScrollMode = kDrag;
        return TRUE;
    }

    // a click anywhere else means that we should move the note so
    // it starts at the time clicked on, but we don't need to drag at all
    else {

        int width, height;

        ColToTime( mi.col, itsStartTime );
        GetRangeExtent( width, height );
        SetRangeAnchor( 0, mi.col );
        SetRangeExtent( width, height );

        InvalidateCell( -1, -1 );

        // tell dad about the change
        SendMessage( Parent->HWindow, WM_COMMAND, Attr.Id,
            MAKELONG( HWindow, 0 ));
        return FALSE;
    }
}

// ------------------------------------------------------------------
// RangeOverStartEnd
// ------------------------------------------------------------------
void TTicker::RangeOverStartEnd( RTMousInfo mi )
{
    int  row, col, height, width, bound;

    GetRangeAnchor( row, col );
    GetRangeExtent( height, width );

    if (XYLeft( mi )) {

        bound = hScrollPos;
        switch( itsScrollMode ) {

            case kGrowLeft:
            case kDrag:

                Scroll( 0, 1 );
                break;

            case kGrowRight:

                // don't scroll past the anchor point
                if (hScrollPos >= col) {

                    Scroll( 0, 1 );
                }
                break;
        }
    }
    else if (XYRight( mi )) {

        bound = hScrollPos + kColsOnScreen;
        switch( itsScrollMode ) {

            case kGrowLeft:

                // don't scroll past the anchor point
                if (hScrollPos + kColsOnScreen - 2 <= col) {

                    Scroll( 0, -1 );
                }
                break;

            case kGrowRight:
            case kDrag:

                Scroll( 0, -1 );
                break;
        }
    }
    else if (XYWithin( mi ) && (mi.style == kCellPoint) && (mi.col != PreviousCol)) {

        bound = mi.col;
        PreviousCol = mi.col;
    }
    else {

        return;
    }

    switch( itsScrollMode ) {

        case kGrowLeft:

            SetRangeExtent( 0, min( 0, bound - col ));
            InvalidateCell( 0, -1 );
            CalculateStart();
            CalculateDuration();
            UpdateWindow( HWindow );
            break;

        case kGrowRight:

            SetRangeExtent( 0, max( 0, bound - col ));
            InvalidateCell( 0, -1 );
            CalculateDuration();
            UpdateWindow( HWindow );
            break;

        case kDrag:

            SetRangeAnchor( row, bound - itsDragOffset );
            SetRangeExtent( 0, width );
            InvalidateCell( 0, -1 );
            CalculateStart();
            CalculateDuration();
            UpdateWindow( HWindow );
            break;
    }


    SendMessage( Parent->HWindow, WM_COMMAND, Attr.Id,
        MAKELONG( HWindow, 0 ));
}

// ------------------------------------------------------------------
// RangeReleaseStartEnd
// ------------------------------------------------------------------
void TTicker::RangeReleaseStartEnd( RTMousInfo  )
{
    // tell dad
    SendMessage( Parent->HWindow, WM_COMMAND, Attr.Id,
        MAKELONG( HWindow, 0 ));
}



// ------------------------------------------------------------------
// CalculateDuration
// ------------------------------------------------------------------
void TTicker::CalculateDuration( void )
{
    int top, left, bottom, right, height, width;

    if (GetRangeRect( top, left, bottom, right )) {

        GetRangeExtent( height, width );
        left  = (left  == -1) ? hScrollPos         : left;
        right = (right == -1) ? hScrollPos + width : right;
        itsDuration.Set( *itsTimeUnitP, right - left + 1 );
    }
    else {

        itsDuration.Clear();
    }
}


// ------------------------------------------------------------------
// CalculateStart
// ------------------------------------------------------------------
void TTicker::CalculateStart( void )
{
    int top, left, bottom, right;

    if (GetRangeRect( top, left, bottom, right )) {

        if (left == -1) {

            ColToTime( hScrollPos, itsStartTime );
        }
        else {

            ColToTime( left, itsStartTime );
        }
    }
    else {

        itsStartTime.Clear();
    }
}


// ------------------------------------------------------------------
// IsCellSelectable
// ------------------------------------------------------------------
BOOL TTicker::IsCellSelectable( int row, int  )
{
    return( row != 1 );
}


// ------------------------------------------------------------------
// SetTimeUnit
// ------------------------------------------------------------------
void TTicker::SetTimeUnit( TTimeUnit unit )
{
    *itsTimeUnitP   = unit;

    // clear time out if the unit is day or week
    if ((unit >= k1Day) && itsStartTime.HasTime()) {

        itsStartTime.ClearTime();
    }

    // set to 9 am if the unit is hours and the start had no time
    else if ((unit < k1Day) && !itsStartTime.HasTime()) {

        itsStartTime.SetHour( 9 );
    }

    NormalizeCells();
    CalculateSelection( TRUE );
}


// ------------------------------------------------------------------
// SetStart
// ------------------------------------------------------------------
void TTicker::SetStart( RTTime time )
{
    itsFirstCell = time;
    itsFirstCell.PlusEquals( *itsTimeUnitP, -kMidCol );
    itsFirstCell.Normalize( *itsTimeUnitP );
    if (*itsTimeUnitP >= k1Day) {

        itsFirstCell.ClearTime();
    }

    CalculateSelection( FALSE );
    GotoRC( -1, kMidCol - kColsOnScreen / 2 );
    InvalidateCell( -1, -1 );
}


// ------------------------------------------------------------------
// SwitchModes
// ------------------------------------------------------------------
void TTicker::SwitchModes( TTickerMode mode )
{
    itsMode = mode;

    switch (mode) {

        case kDurationOnly:

            int length = (int) itsDuration.GetNumUnits( *itsTimeUnitP );
            itsStartTime.Clear();
            SetRangeAnchor( 0, 1 );
            SetRangeExtent( 1, length - 1 );

            GotoRC( 0, max( 0, length - kColsOnScreen + 2 ));
            SetFocusCell( 0, length );

            InvalidateCell( -1, -1 );
            break;

        case kMilestoneOnly:

            itsDuration.Clear();
            if (!itsStartTime.HasDate()) {

                itsStartTime.SetNow();
                if (*itsTimeUnitP >= k1Day) {

                    itsStartTime.ClearTime();
                }
                NormalizeCells();
            }

            CalculateSelection( TRUE );     //also sets focusCell
            break;

        case kStartEnd:

            if (!itsStartTime.HasDate()) {

                itsStartTime.SetNow();
                if (*itsTimeUnitP >= k1Day) {

                    itsStartTime.ClearTime();
                }
                NormalizeCells();
            }

            // make sure there is a minimum duration
            if (itsDuration.IsZero()) {

                itsDuration.Set( *itsTimeUnitP, 1 );
            }

            CalculateSelection( TRUE );     //also sets focusCell
            break;
    }
}


// ------------------------------------------------------------------
// GetTimes
// ------------------------------------------------------------------
void TTicker::GetTimes( RTTime start, RTTime end, RTDuration duration )
{
   start    = itsStartTime;
   end      = start.Plus( itsDuration );
   duration = itsDuration;
}



// ------------------------------------------------------------------
// CalculateSelection
// ------------------------------------------------------------------
void TTicker::CalculateSelection( BOOL gotoSelection )
{
    int col;

    int width = (int) itsDuration.GetNumUnits( *itsTimeUnitP );

    if (itsStartTime.HasDate()) {

        TimeToCol( itsStartTime, col );
        SetRangeAnchor( 0, col );
        SetRangeExtent( 1, max( 0, width - 1 ) );
        if (gotoSelection) {

            GotoRC( -1, col - kColsOnScreen / 2 );
            SetFocusCell( 0, col - kColsOnScreen / 2 );
        }
    }
    else {

        SetRangeAnchor( 0, 1 );
        SetRangeExtent( 1, max( 0, width - 1 )  );
        if (gotoSelection) {

            GotoRC( -1, 0 );
            SetFocusCell( 0, 0 );
        }
    }

}

//---------------------------------------------------------------------------
// MousOver
//---------------------------------------------------------------------------
void TTicker::MousOver( RTMousInfo mi )
{
    int top, left, bottom, right;

    if ((mi.row != 0) || !GetRangeRect( top, left, bottom, right )) {

        TCursor::Set( kArrowCursor );
        return;
    }

    switch( itsMode ) {

        case kDurationOnly:

            TCursor::Set( kGrowRightCursor );
            return;

        case kMilestoneOnly:

            if ((left <= mi.col) && (mi.col <= right)) {

                TCursor::Set( kDragHorizCursor );
                return;
            }
            break;

        case kStartEnd:

            if (mi.col == left - 1) {

                TCursor::Set( kGrowLeftCursor );
                return;
            }
            else if (mi.col == right + 1) {

                TCursor::Set( kGrowRightCursor );
                return;
            }
            else if ((left <= mi.col) && (mi.col <= right)) {

                TCursor::Set( kDragHorizCursor );
                return;
            }
            break;
    }

    TCursor::Set( kArrowCursor );
}

// ------------------------------------------------------------------
// WMHScroll
//
//    Translates THUMBTRACK to THUMBPOSITION messages for TBaseGrid
// ------------------------------------------------------------------
void TTicker::WMHScroll( RTMessage Msg )
{
    // change the message to thumbposition so that the ticker tracks
    // DURING THE DRAG. Otherwise, it won't update until the drag is
    // complete
    if (Msg.WParam == SB_THUMBTRACK) {

        Msg.WParam = SB_THUMBPOSITION;

        // if the cursor went out of the scroll bar, the position
        // will be 0, which makes the ticker jump back to zero
        // (which we wish to avoid)
        if (Msg.LP.Lo != 0) {

            TBaseGrid::WMHScroll( Msg );
        }
    }
    else {

        TBaseGrid::WMHScroll( Msg );
    }

}

//*****************************************************************************

//*****************************************************************************
void TTicker::GetDefaultFocusCell( int &row, int &col )
{
    switch( itsMode ) {

        case kDurationOnly:

            row= 0; col = 1;
            break;

        case kMilestoneOnly:
        case kStartEnd:

            row= 0; col = hScrollPos;
            break;

        default:
            break;
    }

}

