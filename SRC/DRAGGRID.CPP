//*****************************************************************************
// parent of views that allow dragging
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "draggrid.h"

#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __profile__
#include "profile.h"
#endif
#ifndef __worktime__
#include "worktime.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif

//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------
TDragGrid::TDragGrid( PTWindowsObject AParent,
    TScrollDirection direction, int deltaCol ):
    TRuleGrid( AParent )
{
    itsLastPoint.x      = itsLastPoint.y = -999;
    itsScrollDirection  = direction;
    itsDeltaCol         = deltaCol;
    itsGrowStartCursor  = kGrowUpCursor;
    itsGrowEndCursor    = kGrowDownCursor;
    itsDragCursor       = kDragBothCursor;
    itsDragWinP         = NULL;
}

//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------
TDragGrid::~TDragGrid( void )
{
}

//---------------------------------------------------------------------------
// MousDragInit
//---------------------------------------------------------------------------
BOOL TDragGrid::MousDragInit( RTMousInfo mi  )
{
    if (mi.bKeyAction) return FALSE;

    if (!ValidDragCell( itsMode, mi.row, mi.col )) return FALSE;

    // find out if the cursor is on top of a note's bar
    RECT growStartR, growEndR, dragR;
    RTNote note = PointToNote( mi );
    if (note == NOOBJECT) return FALSE;

    // initialize some instance variables
    itsNoteP        = &note;
    itsAnchorPoint  = mi.xy;
    itsPrecision    = GetPrecision();
    note.GetTimes( itsNewTimes );

    char reason[100];
    itsAllowedToDrag = IsAllowedToDrag( note, reason );
    if (!itsAllowedToDrag) {

        OpenDragWindow( TRUE );
        itsDragWinP->Set( reason, itsNewTimes );
        return TRUE;
    }

    TTime time;
    GetNoteRects( note, mi, growStartR, dragR, growEndR, itsMarqueeRect );
    if (PtInRect( &growStartR, mi.xy )) {

        itsMode = kGrowStart;
        PointToTime( mi, time );
        TTime start( note.GetStart() );
        if (!start.HasTime())
            start.SetToMidnight();
        itsOffset.Set( start, time );
    }
    else if (PtInRect( &growEndR, mi.xy )) {

        itsMode = kGrowEnd;
        PointToTime( mi, time );
        TTime end( note.GetEnd() );
        if (!end.HasTime())
            end.SetToMidnight();
        itsOffset.Set( end, time );
    }
    else if (PtInRect( &dragR, mi.xy )) {

        itsMode = kDrag;
        PointToTime( mi, time );
        TTime start( note.GetStart() );
        if (!start.HasTime())
            start.SetToMidnight();
        itsOffset.Set( start, time );
    }
    else {

        // there is a tiny (single pixel) discrepency between the
        // PointToNote's idea of where the note is, and GetNoteRect's
        // idea of where the note is. Rather than figure it out, we
        // just bail out, with the result that the drag cursors are
        // actually 1 pixel narrower than they look
        return FALSE;
    }

    // if we made it here, there must be something to do
    itsMarqueeVisible = FALSE;
    OpenDragWindow( FALSE );
    itsDragWinP->Set( "", itsNewTimes );
    return TRUE;
}

//---------------------------------------------------------------------------
// IsAllowedToDrag
//
// don't allow dragging if the note is in hours and
// extends over more than 1 day
// don't allow dragging if the note has a frequency
//---------------------------------------------------------------------------
BOOL TDragGrid::IsAllowedToDrag( RTNote note, char *message )
{
    if (note.GetFreq() != NULL) {

        strcpy( message, "Can't drag notes with frequencies" );
        return FALSE;
    }
    else {

        message[0] = '\0';
        return TRUE;
    }
}


//---------------------------------------------------------------------------
// MousDragOver
//---------------------------------------------------------------------------
void TDragGrid::MousDragOver( RTMousInfo mi )
{
    if (!itsAllowedToDrag) return;

    // handle dragging within the grid
    if (ValidDragCell( itsMode, mi.row, mi.col )) {

        // DragOvers don't cause flicker: people do
        if ((mi.xy.x != itsLastPoint.x) || (mi.xy.y != itsLastPoint.y)) {

            itsLastPoint = mi.xy;
            DragActive( mi );
        }
        return;
    }

    // Handle vertical autoscrolling
    if ((itsScrollDirection == kVertical) || (itsScrollDirection == kBoth)) {

        if (XYAbove( mi ) || XYInFrozenRow( mi )) {

            ScrollTime( mi, 1, 0 );
        }
        else if (XYBelow( mi )) {

            ScrollTime( mi, -1, 0 );
        }
    }


    // Handle horizontal autoscrolling
    if ((itsScrollDirection == kHorizontal) || (itsScrollDirection == kBoth)) {

        if (XYLeft( mi ) || XYInFrozenCol( mi )) {

            ScrollTime( mi, 0, itsDeltaCol );
        }
        else if (XYRight( mi )) {

            ScrollTime( mi, 0, -itsDeltaCol );
        }
    }
}

//---------------------------------------------------------------------------
// MousDragRelease
//
// Clear the last drag rectangle
//---------------------------------------------------------------------------
void TDragGrid::MousDragRelease( RTMousInfo mi )
{

    if (!itsAllowedToDrag) {

        CloseDragWindow();
        return;
    }

    // if the mouse isn't released inside the grid, in the same
    // row then don't update the note's information
    if (ValidDragCell( itsMode, mi.row, mi.col )) {

        // check for time conflicts
        if (!TNoteSet::CancelBecauseOfTimeConflict( *itsNoteP,
            itsNewTimes.GetStart(), itsNewTimes.GetEnd() )) {

            // either there's no time conflict, or the user
            // decided to save the note despite the time conflict
            itsNoteP->SetTimes( itsNewTimes );
            TSysState::bSystemDirty  = TRUE;
        }
    }

    CloseDragWindow();
    Release();
}


//---------------------------------------------------------------------------
// DragActive
//
// Handle dragging as long as the cursor is in the active column
//---------------------------------------------------------------------------
void TDragGrid::DragActive( RTMousInfo mi )
{
    // turn off marquee
    DrawMarquee( itsMarqueeRect, FALSE );

    switch (itsMode) {

        case kGrowStart:  DragStart( mi );  break;
        case kGrowEnd:    DragEnd(   mi );  break;
        case kDrag:       DragBoth(  mi );  break;
    }

    itsAnchorPoint = mi.xy;
    DrawMarquee( itsMarqueeRect, TRUE );
    MoveDragWindow( mi.xy );
    itsDragWinP->Set( "", itsNewTimes );
}

//---------------------------------------------------------------------------
// DragStart
//
// Change start and duration to match the new drag position
//---------------------------------------------------------------------------
void TDragGrid::DragStart( RTMousInfo mi )
{
    if ((itsScrollDirection == kVertical) || (itsScrollDirection == kBoth)){

        int delta = (mi.xy.y - itsAnchorPoint.y);
        if (delta == 0) return;

        itsMarqueeRect.top  += delta;
    }
    else {

        int delta = (mi.xy.x - itsAnchorPoint.x);
        if (delta == 0) return;

        itsMarqueeRect.left += delta;
    }

    // change the start time and duration to match the new position
    TTime start;
    PointToTime( mi, start );
    start.PlusEquals( -itsOffset );
    start.Normalize( itsPrecision );
    SetStart( start );
}

//---------------------------------------------------------------------------
// DragEnd
//
// Change end and duration to match the new drag position
//---------------------------------------------------------------------------
void TDragGrid::DragEnd( RTMousInfo mi )
{
    if ((itsScrollDirection == kVertical) || (itsScrollDirection == kBoth)){

        int delta = (mi.xy.y - itsAnchorPoint.y);
        if (delta == 0) return;

        itsMarqueeRect.bottom += delta;
    }
    else {

        int delta = (mi.xy.x - itsAnchorPoint.x);
        if (delta == 0) return;

        itsMarqueeRect.right  += delta;
    }

    // change the end and duration to match the new position
    TTime end;
    PointToTime( mi, end );
    end.PlusEquals( itsPrecision, 1 );
    end.PlusEquals( -itsOffset );
    end.Normalize( itsPrecision );
    SetEnd( end );
}

//---------------------------------------------------------------------------
// DragBoth
//
// Change start and end times to match the new drag position
//---------------------------------------------------------------------------
void TDragGrid::DragBoth( RTMousInfo mi )
{
    // move the hilite rectangle
    int deltaX = mi.xy.x - itsAnchorPoint.x;
    int deltaY = mi.xy.y - itsAnchorPoint.y;
    switch (itsScrollDirection) {

        case kHorizontal: OffsetRect( &itsMarqueeRect, deltaX, 0 );      break;
        case kVertical:   OffsetRect( &itsMarqueeRect,      0, deltaY ); break;
        case kBoth:       OffsetRect( &itsMarqueeRect, deltaX, deltaY ); break;
    }

    // change the times to match the new position
    TTime start;
    PointToTime( mi, start );
    start.PlusEquals( -itsOffset );
    start.Normalize( itsPrecision );
    SetBoth( mi, start );
}

//---------------------------------------------------------------------------
// ScrollTime
//
// Scroll and either drag or grow the time by 'amount.' (Marquee stays
// off until the cursor goes back inside the current row.)
//---------------------------------------------------------------------------
void TDragGrid::ScrollTime( RTMousInfo mi, int deltaRow, int deltaCol )
{
    DrawMarquee( itsMarqueeRect, FALSE );

    switch( itsMode ) {

        case kGrowStart: ScrollStart( mi, deltaRow, deltaCol );  break;
        case kGrowEnd:   ScrollEnd(   mi, deltaRow, deltaCol );  break;
        case kDrag:      ScrollBoth(  mi, deltaRow, deltaCol );  break;
    }

    itsDragWinP->Set( "", itsNewTimes );
}


//---------------------------------------------------------------------------
// DrawMarquee
//
// Turn the marquee on or off (only does something if it's necessary)
//---------------------------------------------------------------------------
void TDragGrid::DrawMarquee( RECT &r, BOOL on )
{
    if (on && !itsMarqueeVisible) {

        // marquee is being turned on
        itsMarqueeVisible = TRUE;
    }
    else if ((!on) && itsMarqueeVisible) {

        // marquee is being turned off
        itsMarqueeVisible = FALSE;
    }
    else {

        // marquee doesn't need to be changed
        return;
    }

    // constrain the rectangle as appropriate for the grid
    RECT r2 = r;
    ConstrainMarqueeRect( itsMode, r2 );

    // invert the rectangle
    HANDLE dc = GetDC( HWindow );
    if (!IsRectEmpty( &r2 )) DrawFocusRect( dc, &r2 );
    ReleaseDC( HWindow, dc );
}

//---------------------------------------------------------------------------
// GetPrecision
//
// Return the units that should be used for precision in dragging
//---------------------------------------------------------------------------
TTimeUnit TDragGrid::GetPrecision( void )
{
    if( itsNoteP->GetTimeUnit() < k1Day) {

        switch( TProfile::dayviewIncrement ) {

            case kDayview_5_Minutes:     return k1Min;
            case kDayview_6_Minutes:     return k1Min;
            case kDayview_10_Minutes:    return k5Min;
            case kDayview_12_Minutes:    return k6Min;
            case kDayview_15_Minutes:    return k5Min;
            case kDayview_20_Minutes:    return k10Min;
            case kDayview_30_Minutes:    return k15Min;
            case kDayview_60_Minutes:    return k30Min;
            case kDayview_120_Minutes:   return k1Hour;
            default:                     return k1Hour;
        }
    }
    else {

        return itsNoteP->GetTimeUnit();
    }
}

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
//
//          Drag Window Operations
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

//---------------------------------------------------------------------------
// OpenDragWindow
//---------------------------------------------------------------------------
void TDragGrid::OpenDragWindow( BOOL hasMessage )
{
    itsDragWinP = new TDragWin( this,
        hasMessage ? IDD_DRAGWIN_WITH_MESSAGE : IDD_DRAGWIN );
    GetApplicationObject()->MakeWindow( itsDragWinP );
    EnableWindow( itsDragWinP->HWindow, FALSE );
    PlaceDragWindow();
    itsDragWinP->Show( SW_SHOW );
}


//---------------------------------------------------------------------------
// MoveDragWindow
//---------------------------------------------------------------------------
void TDragGrid::MoveDragWindow( POINT p )
{
    // if the point is within the drag window, then move the drag window
    // out of the way
    ClientToScreen( HWindow, &p );
    if (PtInRect( &itsDragWinR, p )) {

        PlaceDragWindow();
    }
}

//---------------------------------------------------------------------------
// PlaceDragWindow
//
// the basic idea is to put the window to the top right of the cursor
// unless we don't have room, in which case we move it elsewhere
//---------------------------------------------------------------------------
void TDragGrid::PlaceDragWindow( void )
{
    // turn off the marquee temporarily
    BOOL toggleMarquee = itsMarqueeVisible;
    DrawMarquee( itsMarqueeRect, FALSE );

    //x, y pixel distance between window and cursor
    const offset= 23;

    // orient window to top, right of cursor
    POINT xy;
    GetCursorPos( &xy );

    RECT r;
    GetWindowRect( itsDragWinP->HWindow, &r );
    int width   = r.right  - r.left;
    int height  = r.bottom - r.top;

    //move it diagonal to cursor position
    xy.y -= offset + height;
    if( xy.y <= 0 )
        xy.y+= 2 * offset + height;

    xy.x -= offset + width;
    if( xy.x <= 0 )
        xy.x+= 2 * offset + width;

    // move the window to its new position
    SetRect( &itsDragWinR, xy.x, xy.y, xy.x+width, xy.y+height );

    MoveWindow( itsDragWinP->HWindow, xy.x, xy.y, width, height, TRUE );

    //it's important to redraw window before
    //drawing marquee again
    UpdateWindow( HWindow );

    //restore marquee
    DrawMarquee( itsMarqueeRect, toggleMarquee );
}

//---------------------------------------------------------------------------
// CloseDragWindow
//---------------------------------------------------------------------------
void TDragGrid::CloseDragWindow( void )
{
    if (itsDragWinP != NULL) {

        delete itsDragWinP;
        itsDragWinP = NULL;
    }

    itsMarqueeVisible= FALSE;
}



//---------------------------------------------------------------------------
// CursorForDrag
//---------------------------------------------------------------------------
BOOL TDragGrid::CursorForDrag( TCursorStyle &style, RTMousInfo mi )
{
    RTNote note = PointToNote( mi );
    if (note != NOOBJECT) {

        RECT growStartR, growEndR, dragR, marqueeR;
        GetNoteRects( note, mi, growStartR, dragR, growEndR, marqueeR );
        if (PtInRect( &growStartR, mi.xy )) {

            style= itsGrowStartCursor;
            return TRUE;
        }
        else if (PtInRect( &growEndR, mi.xy )) {

            style= itsGrowEndCursor;
            return TRUE;
        }
        else if (PtInRect( &dragR, mi.xy)) {

            char reason[100];
            if (!IsAllowedToDrag( note, reason )) {

                style= kDragIllegalCursor;
            }
            else {

                style= itsDragCursor;
            }
            return TRUE;
        }
        else if (mi.style != kCellPoint) {

            // if the cursor is on a grid line, then use the old cursor
            return FALSE;
        }
        else {

            return FALSE;
        }
    }

    return FALSE;
}



//---------------------------------------------------------------------------
// FrozenHeight
//
// returns the total height of the frozen rows in pixels
//---------------------------------------------------------------------------
int TDragGrid::FrozenHeight( PTPaint painter )
{
    int height = 0;

    for (int i = 0; i < frozenRows; i++) {

        height += GridHeight( painter, i );
        height += CellHeight( painter, i );
    }

    return height;
}

//---------------------------------------------------------------------------
// FrozenWidth
//
// returns the total width of the frozen columns in pixels
//---------------------------------------------------------------------------
int TDragGrid::FrozenWidth( PTPaint painter )
{
    int width = 0;

    for (int i = 0; i < frozenCols; i++) {

        width += GridWidth( painter, i );
        width += CellWidth( painter, i );
    }

    return width;
}
