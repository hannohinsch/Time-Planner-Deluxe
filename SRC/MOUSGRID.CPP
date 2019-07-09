/***************************************************************************
 *
 *  MODULE     : mousgrid.cpp
 *               adds standard mouse and char handling,
 *               and cell selection to basegrid
 *
 ***************************************************************************/
#include "standard.h"
#pragma hdrstop

#include "frame.h"
#include "mousgrid.h"

#define ENABLE_FOCUS        FALSE
#define ENABLE_KEYBOARD     FALSE
#define ENABLE_CAPTURE      FALSE

const kTimerID= 1;

//*****************************************************************************
//
//*****************************************************************************
TMousGrid::TMousGrid ( PTWindowsObject AParent, WORD resID ): TBaseGrid( AParent, resID )
{
selectCol= selectRow= -1;
anchorCol= anchorRow= -1;
rangeColExtent= rangeRowExtent= 0;

bRangeInProgress= FALSE;
bDragInProgress= FALSE;
bHasFocus= FALSE;
bVisibleFocus= FALSE;
bUseTimerWithLButton = TRUE;
bInvalidFocusRect= FALSE;
itsIsCaptive= FALSE;
itsClickInProgress= FALSE;

}

//*****************************************************************************
//
//*****************************************************************************
TMousGrid::TMousGrid ( PTWindowsObject AParent, WORD anID, int x, int y, int dx, int dy ):
	 TBaseGrid( AParent, anID, x, y, dx, dy )
{
selectCol= selectRow= -1;
anchorCol= anchorRow= -1;
rangeColExtent= rangeRowExtent= 0;

bRangeInProgress= FALSE;
bDragInProgress= FALSE;
bHasFocus= FALSE;
bVisibleFocus= FALSE;
bUseTimerWithLButton = TRUE;
bInvalidFocusRect= FALSE;
itsIsCaptive= FALSE;
itsClickInProgress= FALSE;

}

//*****************************************************************************
// all the zeros are filled in after creation.....
//*****************************************************************************
TMousGrid::TMousGrid ( PTWindowsObject AParent ):
	 TBaseGrid( AParent, 0, 0, 0, 0, 0 )
{
selectCol= selectRow= -1;
anchorCol= anchorRow= -1;
rangeColExtent= rangeRowExtent= 0;

bRangeInProgress= FALSE;
bDragInProgress= FALSE;
bHasFocus= FALSE;
bVisibleFocus= FALSE;
bUseTimerWithLButton = TRUE;
bInvalidFocusRect= FALSE;
itsIsCaptive= FALSE;

}

//***************************************************************************
//
// destructor
//
//**************************************************************************
TMousGrid::~TMousGrid( void )
{
// release mouse
if( itsIsCaptive )
    {
    ReleaseCapture();
    if( itsOldCaptureH )
        SetCapture( itsOldCaptureH );
    itsIsCaptive= FALSE;
    }
}

//*****************************************************************************
// given a Windows mouse message of any sort, update our MousInfo object (itsInfo)
//*****************************************************************************
void TMousGrid::UpdateInfo( RTMessage Msg )
{
itsInfo.bKeyAction= FALSE;

itsInfo.xy= MAKEPOINT( Msg.LParam );
itsInfo.state= Msg.WParam;

//fills in row, col, style, cellRect from xy
PointToCell( itsInfo.xy, itsInfo.row, itsInfo.col, itsInfo.style, itsInfo.cellRect );

#if ENABLE_CAPTURE       //elaborate mouse capture

//update the mouse capture state: should be captured if in window,
// released when it goes out of window and all buttons are up

RECT r;
GetClientRect( HWindow, &r );

POINT xy= itsInfo.xy;
ClientToScreen( HWindow, &xy );

if( HWindow == WindowFromPoint( xy ) && PtInRect( &r, itsInfo.xy ) )
    {
    if( !itsIsCaptive )
        {
        itsOldCaptureH= SetCapture( HWindow );
        itsIsCaptive= TRUE;
        }
    }
else
    {
    //release only if both buttons are up
    if( itsIsCaptive && !(itsInfo.state & ( MK_LBUTTON | MK_RBUTTON ) ))
        {
        ReleaseCapture();
        if( itsOldCaptureH )
            SetCapture( itsOldCaptureH );
        itsIsCaptive= FALSE;
        }
    else  // we're not in window, but button is down
        {
        }
    }
#endif      //elaborate mouse capture

}


//////////////////////////////////////////////////////////////////////////////
// WINDOWS MSG HANDLERS
/////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
//
//*****************************************************************************
void TMousGrid::WMDoubleClick( RTMessage Msg )
{
if( !itsClickInProgress )
    {
    itsClickInProgress= TRUE;
    UpdateInfo( Msg );
    UpdateFocus();

    MousDblClick( itsInfo );
    itsClickInProgress= FALSE;
    }
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::WMLButtonDown( RTMessage Msg )
{
if( !itsClickInProgress )
    {
    itsClickInProgress= TRUE;

    if (bUseTimerWithLButton) {

        #if ! ENABLE_CAPTURE       //simple mouse capture
        SetCapture( HWindow );
        #endif

        //get timer ticks while mouse is captured
        SetTimer( HWindow, kTimerID, 100, NULL );
    }

    UpdateInfo( Msg );
    UpdateFocus();

    MousClick( itsInfo );

    itsClickInProgress= FALSE;
    }
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::WMRButtonDown( RTMessage Msg )
{
if( !itsClickInProgress )
    {
    itsClickInProgress= TRUE;

    //ignore if we're dragging the left button
    if( !bDragInProgress && !bRangeInProgress )
        {
        UpdateInfo( Msg );
        UpdateFocus();

        MousRtClick( itsInfo );
        }

    itsClickInProgress= FALSE;
    }
}

//*****************************************************************************
// drag and grab release
//*****************************************************************************
void TMousGrid::WMLButtonUp( RTMessage Msg )
{
if( !itsClickInProgress )
    {
    itsClickInProgress= TRUE;

    if (bUseTimerWithLButton) {

        #if ! ENABLE_CAPTURE   //simple mouse capture
        ReleaseCapture();
        #endif

        KillTimer( HWindow, kTimerID);
    }

    UpdateInfo( Msg );
    UpdateFocus();

    if( bDragInProgress )
        {
        bDragInProgress= FALSE;
        MousDragRelease( itsInfo );
        }

    if( bRangeInProgress )
        {
        bRangeInProgress= FALSE;
        MousRangeRelease( itsInfo );
        }

    itsClickInProgress= FALSE;
    }
}

//*****************************************************************************
// drag and grab release
//*****************************************************************************
void TMousGrid::WMRButtonUp( RTMessage Msg )
{
if( !itsClickInProgress )
    {
    itsClickInProgress= TRUE;

    UpdateInfo( Msg );
    UpdateFocus();

    itsClickInProgress= FALSE;
    }
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::WMMouseActivate( RTMessage msg )
{
DefWndProc( msg );
}

//*****************************************************************************
// mouse positioned over something in grid.
// If mouse input is captured, we will get msgs even if
// mouse is outside of grid.
//*****************************************************************************
void TMousGrid::WMMouseMove( RTMessage Msg )
{
UpdateInfo( Msg );

if( bRangeInProgress )
    MousRangeOver( itsInfo );
else if( bDragInProgress )
    MousDragOver( itsInfo );
else
    MousOver( itsInfo );
}

//*****************************************************************************
// timer msgs are handled like mouse move messages
// the point of this is to allow things like auto-scrolling
//*****************************************************************************
void TMousGrid::WMTimer( RTMessage )
{

//note absence of UpdateInfo() call here

if( bRangeInProgress )
    MousRangeOver( itsInfo );
else if( bDragInProgress )
    MousDragOver( itsInfo );
else
    MousOver( itsInfo );
}


/////////////////////////////////////////////////////////////////////////////
// SELECTION HANDLING
/////////////////////////////////////////////////////////////////////////////

#if 0
//*****************************************************************************

//*****************************************************************************
BOOL TMousGrid::IsCellSelected( int row, int col )
{
return( row == selectRow && col == selectCol );
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::SetSelection( int row, int col )
{
if( IsCellSelectable( row, col) )
    {

    //invalidate old selection
    if( selectRow >= 0 && selectCol >= 0 )
        {
        InvalidateCell( selectRow, selectCol );
        }

    //set new selection
    selectCol= col;
    selectRow= row;

    //invalidate new selection
    InvalidateCell( row, col );
    }
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::GetSelection( int &row, int &col )
{
row= selectRow;
col= selectCol;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// RANGE HANDLING
/////////////////////////////////////////////////////////////////////////////

//*****************************************************************************

//*****************************************************************************
BOOL TMousGrid::IsCellInRange( int row, int col )
{
BOOL rc= FALSE;

int t,l,b,r;
if( GetRangeRect( t,l,b,r ) )
    {
    if( row >= t && row <= b && col >= l && col <= r )
        rc= TRUE;
    }

return rc;
}

//*****************************************************************************
//  NB setting a new range anchor clears current range
//*****************************************************************************
void TMousGrid::SetRangeAnchor( int row, int col )
{
// as a side effect, we invalidate all cells in old range
SetRangeExtent( 0, 0 );
if( anchorRow != -1 && anchorCol != -1 )
    InvalidateCell( anchorRow, anchorCol );

anchorCol= col;
anchorRow= row;
// better redraw anchor cell, too
if( anchorRow != -1 && anchorCol != -1 )
    InvalidateCell( anchorRow, anchorCol );

}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::GetRangeAnchor( int &row, int &col )
{
row= anchorRow;
col= anchorCol;
}

//*****************************************************************************
//
//*****************************************************************************
void TMousGrid::SetRangeExtent( int height, int width )
{
int t1, l1, b1, r1;
int t0, l0, b0, r0;
RECT rOld, rNew, rIntersect, rInvalid;

if( anchorRow == -1 || anchorCol == -1 )
    return;     //must have anchor before range!

if( rangeRowExtent == height && rangeColExtent == width )
    return;     //no change

BOOL bOld= GetRangeRect( t0, l0, b0, r0 );

//update range extent
rangeRowExtent= height;
rangeColExtent= width;

BOOL bNew= GetRangeRect( t1, l1, b1, r1 );

//
//figure out how much of window to invalidate
//

//get current invalid rect
BOOL bDirty= GetUpdateRect( HWindow, &rInvalid, FALSE );

//invalidate all that might need to be changed
if( bOld )
    InvalidateRange( t0,l0,b0,r0 );

if( bNew )
    InvalidateRange( t1,l1,b1,r1 );

//however, the intersection of old and new doesn't need to be updated,
//so go ahead and validate it
// (do only if both old and new exist, and neither is degenerate)
if( bOld && bNew &&
    !( t0 == b0 && l0 == r0) &&
    !( t1 == b1 && l1 == r1) )
    {
    RangeToRect( t0, l0, b0, r0, rOld );
    RangeToRect( t1, l1, b1, r1, rNew );
    IntersectRect( &rIntersect, &rOld, &rNew );
    ValidateRect( HWindow, &rIntersect );

    // re-invalidate original invalid rect, because we may have
    // accidentally validated it (above)
    if( bDirty )
        InvalidateRect( HWindow, &rInvalid, FALSE );
    }
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::GetRangeExtent( int &height, int &width )
{
height= rangeRowExtent;
width= rangeColExtent;
}

//*****************************************************************************
// gets rect equivalent of current range
//*****************************************************************************
BOOL TMousGrid::GetRangeRect( int &top, int &left, int &bottom, int &right )
{
BOOL rc= FALSE;

if( anchorRow != -1 && anchorCol != -1 )
    {
    top= min( anchorRow, anchorRow + rangeRowExtent );
    left= min( anchorCol, anchorCol + rangeColExtent );

    bottom= max( anchorRow, anchorRow + rangeRowExtent );
    right= max( anchorCol, anchorCol + rangeColExtent );

    rc= TRUE;
    }

return rc;
}

//////////////////////////////////////////////////////////////////////////////
// VIRTUAL MSG RESPONSE FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
//
//*****************************************************************************
BOOL TMousGrid::IsCellSelectable( int row, int col )
{
return ( row >= 0 && row < rowCnt && col >= 0 && col < colCnt );
}

//*****************************************************************************
//
//
//*****************************************************************************
void TMousGrid::MousClick( RTMousInfo info )
{
    //start range drag, if wanted
    bRangeInProgress= MousRangeInit( info );

    //start drag, if wanted
    bDragInProgress = MousDragInit( info );
}

//*****************************************************************************
//
//*****************************************************************************
void TMousGrid::MousDblClick( RTMousInfo )
{
}

//*****************************************************************************
//
//*****************************************************************************
void TMousGrid::MousRtClick( RTMousInfo  )
{
}

//*****************************************************************************
//
//*****************************************************************************
void TMousGrid::MousOver( RTMousInfo )
{
}

//*****************************************************************************
// overload this and return TRUE in grid to enable range selection
//*****************************************************************************
BOOL TMousGrid::MousRangeInit( RTMousInfo info )
{
SetRangeAnchor( info.row, info.col );

return FALSE;
}

//*****************************************************************************
// update range extent to include current cell
//*****************************************************************************
void TMousGrid::MousRangeOver( RTMousInfo info )
{

if( XYWithin( info ) )
    {
    if( (anchorCol+rangeColExtent != info.col ||
        anchorRow+rangeRowExtent != info.row ) &&
        IsCellSelectable( info.row, info.col ) )
        {
        SetRangeExtent( info.row - anchorRow, info.col - anchorCol );
        }
    }
}

//*****************************************************************************
//
//*****************************************************************************
void TMousGrid::MousRangeRelease( RTMousInfo info )
{

if( XYWithin( info ) )
    {
    SetRangeExtent( info.row - anchorRow, info.col - anchorCol );
    }
}

//*****************************************************************************
//
//*****************************************************************************
BOOL TMousGrid::MousDragInit( RTMousInfo )
{
return FALSE;       //we can't handle grab
}

//*****************************************************************************
//
//*****************************************************************************
void TMousGrid::MousDragOver( RTMousInfo )
{
}

//*****************************************************************************
//
//*****************************************************************************
void TMousGrid::MousDragRelease( RTMousInfo )
{
}


//////////////////////////////////////////////////////////////////////////////
//////////////////// FOCUS HANDLING //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//*****************************************************************************

//*****************************************************************************
void TMousGrid::WMSetFocus(RTMessage Msg)
{

if( bInvalidFocusRect || !IsCellVisible( selectRow, selectCol ) )
    GetDefaultFocusCell( selectRow, selectCol );

//show focus rect
RECT rFrame, rCell;
if( CellToRect( selectRow, selectCol, rFrame, rCell ) )
    {
    focusRect= rCell;
    bInvalidFocusRect= FALSE;

    // NB if this window is going to be repainted anyway, then there is no reason to
    // draw the focus rect now -- just let it be restored by paint msg processing
    // Results in cleaner painting.
    if( !GetUpdateRect( HWindow, NULL, FALSE ) )
        {
        HDC hdc= GetDC( HWindow );
        PaintFocus( hdc, rCell );
        ReleaseDC( HWindow, hdc );

        bVisibleFocus= TRUE;
        }
    }

bHasFocus= TRUE;

DefWndProc( Msg );
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::WMKillFocus(RTMessage Msg)
{

if( bVisibleFocus )
    {
    HDC hdc= GetDC( HWindow );
    PaintFocus( hdc, focusRect );
    ReleaseDC( HWindow, hdc );
    bVisibleFocus= FALSE;
    }

bHasFocus= FALSE;

DefWndProc( Msg );
}

//*****************************************************************************

//*****************************************************************************
BOOL TMousGrid::IsFocusCell( int row, int col )
{
return( row ==  selectRow && col ==  selectCol  );
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::GetFocusCell( int &row, int &col )
{
row= selectRow;
col= selectCol;
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::SetFocusCell( int row, int col )
{
if( IsCellSelectable( row, col ))
    {
    RECT rFrame, rCell;
    HDC hdc= GetDC( HWindow );

    //get rid of old
    if( bHasFocus && bVisibleFocus )
        {
        PaintFocus( hdc, focusRect );
        bVisibleFocus= FALSE;
        }

    selectCol= col;
    selectRow= row;

    //paint new
    if( bHasFocus && CellToRect( selectRow, selectCol, rFrame, rCell ) )
        {
        PaintFocus( hdc, rCell );
        focusRect= rCell;
        bVisibleFocus= TRUE;
        }

    ReleaseDC( HWindow, hdc );
    }
}


//*****************************************************************************
// update focus values, based on "itsInfo" values
// (called on mouse click in window)
//*****************************************************************************
void TMousGrid::UpdateFocus( void )
{
if( !bHasFocus )
	SetFocus( HWindow );	 //NB This will call WMSetFocus !!!
#if 0       //don't move focus rect via mouse click anymore
else
    SetFocusCell( itsInfo.row, itsInfo.col );
#endif
}

//*****************************************************************************
// hide focus
//*****************************************************************************
void TMousGrid::HideFocus( void )
{
if( bHasFocus && bVisibleFocus )    //remove rect
    {
    HDC hdc= GetDC( HWindow );
    PaintFocus( hdc, focusRect );
    ReleaseDC( HWindow, hdc );
    bVisibleFocus= FALSE;       //no longer onscreen
    }
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::RestoreFocus( void )
{
if( bHasFocus )    //restore rect
    {
    HDC hdc= GetDC( HWindow );

    if( !bInvalidFocusRect )
        PaintFocus( hdc, focusRect );
    else
        {
        bInvalidFocusRect= FALSE;

        if( !IsCellVisible( selectRow, selectCol ) )
            GetDefaultFocusCell( selectRow, selectCol );

        //paint new rect
        RECT rFrame, rCell;
        CellToRect( selectRow, selectCol, rFrame, rCell );
        PaintFocus( hdc, rCell );
        focusRect= rCell;
        }

    ReleaseDC( HWindow, hdc );
    bVisibleFocus= TRUE;
    }
}

//*****************************************************************************
// We scrolled. Adjust focus cell to remain visible onscreen
// FIXME We could do this faster if the # of pixels scrolled and the scrollrect
// were passed in.
//*****************************************************************************
void TMousGrid::RestoreFocusAfterScroll( int rowDelta, int colDelta )
{

if( bHasFocus )
    {
    if( !IsRowVisible( selectRow ))
        {
        selectRow+= rowDelta;
        }

    if( !IsColVisible( selectCol ))
        {
        selectCol+= colDelta;
        }

    HDC hdc= GetDC( HWindow );

    //paint new
    RECT rFrame, rCell;
    if( CellToRect( selectRow, selectCol, rFrame, rCell ) )
        {
        PaintFocus( hdc, rCell );
        focusRect= rCell;
        bVisibleFocus= TRUE;
        }

    ReleaseDC( HWindow, hdc );
    }
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::RestoreFocusAfterResize( void )
{
if( bHasFocus )
    {
    if( !IsCellVisible( selectRow, selectCol ) )
        {
        GetDefaultFocusCell( selectRow, selectCol );
        }

    HDC hdc= GetDC( HWindow );

    //paint new
    RECT rFrame, rCell;
    if( CellToRect( selectRow, selectCol, rFrame, rCell ) )
        {
        PaintFocus( hdc, rCell );
        focusRect= rCell;
        bVisibleFocus= TRUE;
        }

    ReleaseDC( HWindow, hdc );
    }
}

//*****************************************************************************
// cell size may have changed, or cell may no longer exist (e.g., rowCnt or
// colCnt may have changed)
//*****************************************************************************
void TMousGrid::InvalidateFocus( void )
{
HideFocus();
bInvalidFocusRect= TRUE;

//the proper focus cell will reappear next time RestoreFocus()
// is called; typically after (and no later than) a WM_PAINT msg
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::PaintFocus( HDC hdc, RECT &r )
{
#if ENABLE_FOCUS
DrawFocusRect( hdc, &r );
#endif
}

/////////////////////////////////////////////////////////////////////////////
//////////////////// KEYBOARD HANDLING //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
// tell windows that we want to handle chars ourselves; needed if
// grid is a control in dialog or modeless message handler is installed
//*****************************************************************************
void TMousGrid::WMGetDlgCode(RTMessage Msg)
{
#if ENABLE_KEYBOARD
Msg.Result= DLGC_WANTARROWS | DLGC_WANTCHARS;
#else
DefWndProc( Msg );
#endif
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::WMKeyUp(RTMessage Msg)
{
#if ENABLE_KEYBOARD
WORD vk= Msg.WParam;

BOOL bShift= 0x8000 & GetKeyState( VK_SHIFT );

//if this is a shift out of shift state...
if( vk == VK_SHIFT && !bShift )
    MousKey( kShiftUpKey );
#else

// release, process char, recapture -- otherwise sys keys won't work
if( itsIsCaptive && !(itsInfo.state & ( MK_LBUTTON | MK_RBUTTON ) ))
    {
    ReleaseCapture();
    itsIsCaptive= FALSE;
    DefWndProc( Msg );
    SetCapture( HWindow );
    itsIsCaptive= TRUE;
    }
else
    DefWndProc( Msg );

#endif
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::WMKeyDown(RTMessage Msg)
{
#if ENABLE_KEYBOARD
WORD vk= Msg.WParam;

BOOL bShift= 0x8000 & GetKeyState( VK_SHIFT );
BOOL bCtrl= 0x8000 & GetKeyState( VK_CONTROL );

switch( vk )
    {
    case VK_UP:
            if( bCtrl )
                MousKey( kScrUpKey );
            else
                MousKey( kUpKey );
            break;

    case VK_DOWN:
            if( bCtrl )
                MousKey( kScrDownKey );
            else
                MousKey( kDownKey );
            break;

    case VK_LEFT:
            if( bCtrl )
                MousKey( kScrLeftKey );
            else
                MousKey( kLeftKey );
            break;

    case VK_RIGHT:
            if( bCtrl )
                MousKey( kScrRightKey );
            else
                MousKey( kRightKey );
            break;

    case VK_PRIOR:
            if( bShift )
                MousKey( kScrLeftKey );
            else
                MousKey( kScrUpKey );
            break;

    case VK_NEXT:
            if( bShift )
                MousKey( kScrRightKey );
            else
                MousKey( kScrDownKey );
            break;

    default:
			DefWndProc( Msg );
            break;
    }
#else

// release, process char, recapture -- otherwise sys keys won't work
if( itsIsCaptive && !(itsInfo.state & ( MK_LBUTTON | MK_RBUTTON ) ))
    {
    ReleaseCapture();
    itsIsCaptive= FALSE;
    DefWndProc( Msg );
    SetCapture( HWindow );
    itsIsCaptive= TRUE;
    }
else
    DefWndProc( Msg );

#endif
}

//*****************************************************************************
// 
//*****************************************************************************
void TMousGrid::WMChar(RTMessage Msg)
{
#if ENABLE_KEYBOARD
char c= (char) Msg.WP.Lo;

BOOL bShift= 0x8000 & GetKeyState( VK_SHIFT );
BOOL bCtrl= 0x8000 & GetKeyState( VK_CONTROL );

switch( c )
    {
    case ' ':
        if( bShift )
            MousKey( kRightClickKey );
        else if( bCtrl )
            MousKey( kDblClickKey );
        else
            MousKey( kLeftClickKey );

		break;

    default:
		DefWndProc( Msg );
		break;
    }
#else

// release, process char, recapture -- otherwise sys keys won't work
if( itsIsCaptive && !(itsInfo.state & ( MK_LBUTTON | MK_RBUTTON ) ))
    {
    ReleaseCapture();
    itsIsCaptive= FALSE;
    DefWndProc( Msg );
    SetCapture( HWindow );
    itsIsCaptive= TRUE;
    }
else
    DefWndProc( Msg );

#endif
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::WMSysChar( RTMessage Msg )
{
// release, process char, recapture -- otherwise sys keys won't work
if( itsIsCaptive && !(itsInfo.state & ( MK_LBUTTON | MK_RBUTTON ) ))
    {
    ReleaseCapture();
    itsIsCaptive= FALSE;

    DefWndProc( Msg );

    SetCapture( HWindow );
    itsIsCaptive= TRUE;
    }
else
    DefWndProc( Msg );
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::MousKey( TMousKey key )
{
BOOL bShift= 0x8000 & GetKeyState( VK_SHIFT );

int newCol= selectCol;
int newRow= selectRow;

itsInfo.bKeyAction= TRUE;

switch( key )
    {
    case kUpKey:
            //skip to last frozen row if we're now on first unfrozen row
            if( frozenRows > 0 && selectRow == frozenRows + vScrollPos )
                newRow= frozenRows - 1;     //jump to last frozen row
            else
                newRow--;
            break;

    case kDownKey:
            //if on last frozen row, jump to first non-frozen
            if( frozenRows > 0 && selectRow == frozenRows - 1 )
                newRow= frozenRows + vScrollPos;
            else
                newRow++;
            break;

    case kLeftKey:
            if( frozenCols > 0 && selectCol == frozenCols + hScrollPos )
                newCol= frozenCols - 1;
            else
                newCol--;
            break;

    case kRightKey:
            if( frozenCols > 0 && selectCol == frozenCols - 1 )
                newCol= frozenCols + hScrollPos;
            else
                newCol++;
            break;

    case kScrUpKey:
            Scroll( 1, 0 );
            break;

    case kScrDownKey:
            Scroll( -1, 0 );
            break;

    case kScrLeftKey:
            Scroll( 0, 1 );
            break;

    case kScrRightKey:
            Scroll( 0, -1 );
            break;

    case kShiftUpKey:
            if( bRangeInProgress )
                {
                MousRangeRelease( itsInfo );
                bRangeInProgress= FALSE;
                }

            if( bDragInProgress )
                {
                MousDragRelease( itsInfo );
                bDragInProgress= FALSE;
                }
            break;

    case kLeftClickKey:
    case kRightClickKey:
    case kDblClickKey:
            if( bHasFocus )
				{
				// emulate mouseclick
                RECT rFrame, rCell;
                CellToRect( selectRow, selectCol, rFrame, rCell );

                itsInfo.bKeyAction= TRUE;
                itsInfo.row= selectRow;
                itsInfo.col= selectCol;
				itsInfo.style= kCellPoint;
				itsInfo.state= 0;
                itsInfo.xy.x= rCell.left + (rCell.right - rCell.left)/2;
                itsInfo.xy.y= rCell.top + (rCell.bottom - rCell.top)/2;

                if( key == kDblClickKey )
                    MousDblClick( itsInfo );
                else if( key == kRightClickKey )
                    MousRtClick( itsInfo );
                else
                    MousClick( itsInfo );
				}
			break;
    default:
            break;
    }

// common actions for cursor movement keys
if( ( newRow != selectRow || newCol != selectCol ) &&
    bHasFocus &&
    IsCellSelectable( newRow, newCol ) &&
    IsCellVisible( newRow, newCol ) )
    {
    //build fake mousInfo
    RECT rFrame, rCell;
    CellToRect( selectRow, selectCol, rFrame, rCell );

    itsInfo.row= selectRow;
    itsInfo.col= selectCol;
    itsInfo.style= kCellPoint;
    itsInfo.state= 0;
    itsInfo.xy.x= rCell.left + (rCell.right - rCell.left)/2;
    itsInfo.xy.y= rCell.top + (rCell.bottom - rCell.top)/2;

    // shift cursor starts drag
    if( bShift && !bRangeInProgress )
        bRangeInProgress= MousRangeInit( itsInfo );
    if( bShift && !bDragInProgress )
        bDragInProgress= MousDragInit( itsInfo );

    // and emulates mousover
    CellToRect( newRow, newCol, rFrame, rCell );
    itsInfo.row= newRow;
    itsInfo.col= newCol;
    itsInfo.xy.x= rCell.left + (rCell.right - rCell.left)/2;
    itsInfo.xy.y= rCell.top + (rCell.bottom - rCell.top)/2;
    if( bRangeInProgress )
        {
        MousRangeOver( itsInfo );
        }

    if( bDragInProgress )
        {
        MousDragOver( itsInfo );
        }

    SetFocusCell( newRow, newCol );
    }

// if drag or range started (perhaps by MousKey() ), we immediately
// cancel it unless the shift key is down
if( bRangeInProgress && !bShift )
    {
    MousRangeRelease( itsInfo );
    bRangeInProgress= FALSE;
    }

if( bDragInProgress && !bShift )
    {
    MousDragRelease( itsInfo );
    bDragInProgress= FALSE;
    }

itsInfo.bKeyAction= FALSE;
}

//*****************************************************************************

//*****************************************************************************
void TMousGrid::GetDefaultFocusCell( int &row, int &col )
{
row= col= 0;
}


//////////////////////////////////////////////////////////////////////////////
// OWL PARAPHENALIA
//////////////////////////////////////////////////////////////////////////////

//*****************************************************************************

//*****************************************************************************
void TMousGrid::GetWindowClass( WNDCLASS & AWndClass )
{
TBaseGrid::GetWindowClass( AWndClass );

AWndClass.style |= CS_DBLCLKS;
}

//*****************************************************************************
// UseTimerWithLButton
//
// Sets the internal flag that controls whether the grid turns capturing
// and the timer on in WMLButton messages.
//
// Grids that use pop-up menus associated with the left mouse button
// should turn this variable off, or funny things happen to the cursor
// when it's outside the real grid
//*****************************************************************************
void TMousGrid::UseTimerWithLButton( BOOL on )
{
    bUseTimerWithLButton = on;
}

