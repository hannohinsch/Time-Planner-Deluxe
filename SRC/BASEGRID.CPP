/***************************************************************************
 *
 *	MODULE	   : basegrid.cpp
 *               base class for grid-oriented child windows
 *
 ***************************************************************************/
#include "standard.h"
#pragma hdrstop

#include "basegrid.h"

#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __fontmap__
#include "fontmap.h"
#endif
#ifndef __paint__
#include "paint.h"
#endif
#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __cursor__
#include "cursor.h"
#endif

//*****************************************************************************
//
//*****************************************************************************
TBaseGrid::TBaseGrid ( PTWindowsObject AParent, WORD resID ):
        TRawGrid(), TControl( AParent, resID )
{
Defaults();

Attr.Style|= WS_HSCROLL | WS_VSCROLL | WS_BORDER | WS_VISIBLE;
}

//*****************************************************************************
//
//*****************************************************************************
TBaseGrid::TBaseGrid ( PTWindowsObject AParent, WORD anID, int x, int y, int dx, int dy ):
     TRawGrid(), TControl( AParent, anID, "", x, y, dx, dy )
{
Defaults();

Attr.Style|= WS_HSCROLL | WS_VSCROLL | WS_BORDER | WS_VISIBLE;
}

//*****************************************************************************

//*****************************************************************************
void TBaseGrid::Defaults( void )
{
winHeight= winWidth= 0;
scrPainter= NULL;
ScrollChange();
}

//***************************************************************************
//
// destructor
//
//**************************************************************************
TBaseGrid::~TBaseGrid( void )
{
}

//*****************************************************************************

//*****************************************************************************
void TBaseGrid::SetupWindow( void )
{
TControl::SetupWindow();

screenDC= GetDC( HWindow );
}

//*****************************************************************************

//*****************************************************************************
void TBaseGrid::ScrollChange( void )
{
bScrollChange= TRUE;
}

//*****************************************************************************

//*****************************************************************************
void TBaseGrid::UpdateScroll( void )
{
SetupScrollBars( initRow, initCol );

bScrollChange= FALSE;
initRow= initCol= -1;

InvalidateRect( HWindow, NULL, TRUE );
}


//*****************************************************************************
// see what values of scrollmax and thumb positions should be
//*****************************************************************************
void TBaseGrid::SetupScrollBars( int aInitRow, int aInitCol )
{
int h,w;
int r,c;

//vertical max: index of row s.t. all further rows fit on screen
//offset so that scrollPos 0 is first non-frozen row
vScrollMax= 0;
h= topMargin + TopLineHeight( scrPainter );

//first, add up size of frozen rows
for(r= 0; r < frozenRows; r++ )
    h+= ROWHEIGHT( scrPainter, r );

//now count backward till we run out of space
if( h < winHeight )
	for(r= rowCnt - 1; r >= 0; r-- )
		{
        h+= ROWHEIGHT( scrPainter, r );
        if( h > winHeight )
            {
            //don't allow scroll past last row
            vScrollMax= min( rowCnt-frozenRows-1, (r+1) - frozenRows );
            vScrollMax= max( 0, vScrollMax );
            break;
            }
		}
else
	vScrollMax= 0;		//if frozen rows occupy entire space, can't scroll


SetScrollRange(HWindow, SB_VERT, 0, vScrollMax, FALSE );
if( aInitRow == -1)
	vScrollPos= min( vScrollPos, vScrollMax );
else
	vScrollPos= min( aInitRow, vScrollMax );
SetScrollPos( HWindow, SB_VERT, vScrollPos, TRUE );


//horizontal max: index of field s.t. all further fields fit on screen
hScrollMax= 0;
w= leftMargin + LeftLineWidth( scrPainter );

//add up frozen cols first
for(c= 0; c < frozenCols; c++ )
    w+= COLWIDTH( scrPainter, c );

//now go backwards
if( w < winWidth )
	for(c= colCnt - 1; c >= 0; c-- )
		{
        w+= COLWIDTH( scrPainter, c );
		if( w > winWidth )
			{
			//never point past last one (FIXME is -2 correct?)
			hScrollMax= min( colCnt-frozenCols-1, (c+1) - frozenCols );
			hScrollMax= max( 0, hScrollMax );
			break;
			}
		}
else
	hScrollMax= 0;

SetScrollRange(HWindow, SB_HORZ, 0, hScrollMax, FALSE );
//show requested cell
if( aInitCol == -1 )
	hScrollPos= min( hScrollPos, hScrollMax );
else
	hScrollPos= min( aInitCol, hScrollMax );
SetScrollPos( HWindow, SB_HORZ, hScrollPos, TRUE );

}

//*****************************************************************************
// respond to size message
//*****************************************************************************
void TBaseGrid::WMSize( RTMessage Msg )
{
HideFocus();

if( winHeight != HIWORD( Msg.LParam ) ||
	winWidth != LOWORD( Msg.LParam )
	)
	{
	winHeight= HIWORD( Msg.LParam );
	winWidth= LOWORD( Msg.LParam );

    SetupScrollBars( -1, -1 );
    }

RestoreFocusAfterResize();
}

//*****************************************************************************
// vertical scroll
//*****************************************************************************
void TBaseGrid::WMVScroll( RTMessage Msg)
{
int oldStart;
int newStart;
int oldPos;
int h;
int r;
RECT dataRect;
int frozenHeight;

//if window is bad, ScrollWindow makes it worse, so first...
UpdateWindow( HWindow );

TCursor::Set( kArrowCursor );

//add up frozen rows
frozenHeight= topMargin + TopLineHeight( scrPainter );
for(r= 0; r < frozenRows; r++ )
    frozenHeight+= ROWHEIGHT( scrPainter, r );

oldPos= vScrollPos;
switch( Msg.WParam )
    {
    case SB_TOP:
        vScrollPos= 0;
        break;
    case SB_BOTTOM:
        vScrollPos= vScrollMax;
        break;
    case SB_LINEUP:
        vScrollPos= max( 0, vScrollPos - 1 );
        break;
    case SB_LINEDOWN:
        vScrollPos= min( vScrollMax, vScrollPos + 1 );
        break;

    case SB_PAGEUP: //first displayed field becomes last
        h= frozenHeight;
        for( r= LOGROW( frozenRows ); r >= 0 && h < winHeight; r-- )
            h+= ROWHEIGHT( scrPainter, r );

        vScrollPos= vScrollPos - max( 1, (LOGROW(frozenRows) -r -1) );
        vScrollPos= max( vScrollPos, 0 );
        break;

    case SB_PAGEDOWN:  //make last displayed field the first displayed
        h= frozenHeight;
        for( r= LOGROW( frozenRows ); r < rowCnt && h < winHeight; r++ )
            h+= ROWHEIGHT( scrPainter, r );

        vScrollPos= vScrollPos + max( 1, (r- LOGROW(frozenRows) -1) );
        vScrollPos= min( vScrollPos, vScrollMax );
        break;

    case SB_THUMBPOSITION:
        vScrollPos= LOWORD( Msg.LParam );
        break;
    default:
        break;
    }


//scrolling rectangle
dataRect.top= frozenHeight;
dataRect.bottom= winHeight;
dataRect.left= 0;
dataRect.right= winWidth;

// calculate pixel shift
oldStart= newStart= 0;
for( r= min( oldPos, vScrollPos); r < max( oldPos, vScrollPos); r++ )
    {
    int rh= ROWHEIGHT( scrPainter, frozenRows + r );

	if( r < oldPos )			//calc up to old postion
		oldStart+= rh;

	if( r < vScrollPos )	   //calc up to new position
		newStart+= rh;
    }
//NB one of these will be 0, the other the number of pixels to shift

HideFocus();

ScrollWindow( HWindow, 0, (oldStart - newStart), &dataRect, &dataRect );

SetScrollPos( HWindow, SB_VERT, vScrollPos, TRUE );

RestoreFocusAfterScroll( vScrollPos - oldPos, 0 );
}

//*****************************************************************************
//
//*****************************************************************************
void TBaseGrid::WMHScroll( RTMessage Msg)
{
int oldStart;
int newStart;
int oldPos;
int w;
int c;
RECT dataRect;
int frozenWidth;

//if window is bad, ScrollWindow makes it worse, so first...
UpdateWindow( HWindow );

TCursor::Set( kArrowCursor );

//add up frozen cols
frozenWidth= leftMargin + LeftLineWidth( scrPainter );
for(c= 0; c < frozenCols; c++ )
    frozenWidth+= COLWIDTH( scrPainter, c );

oldPos= hScrollPos;
switch( Msg.WParam )
    {
    case SB_TOP:
        hScrollPos= 0;
        break;
    case SB_BOTTOM:
        hScrollPos= hScrollMax;
        break;
    case SB_LINEUP:
        hScrollPos= max( 0, hScrollPos - 1 );
        break;
    case SB_LINEDOWN:
        hScrollPos= min( hScrollMax, hScrollPos + 1 );
        break;

    case SB_PAGEUP: //first displayed field becomes last
        w= frozenWidth;
        for( c= LOGCOL( frozenCols ); c >= 0 && w < winWidth; c-- )
            w+= COLWIDTH( scrPainter, c );

        hScrollPos= hScrollPos - max( 1, (LOGCOL( frozenCols ) - c -1) );
        hScrollPos= max( hScrollPos, 0 );
        break;

    case SB_PAGEDOWN:  //make last displayed field the first displayed
		w= frozenWidth;
        for( c= LOGCOL( frozenCols ); c < colCnt && w < winWidth; c++ )
            w+= COLWIDTH( scrPainter, c );

        hScrollPos= hScrollPos + max( 1, (c - LOGCOL( frozenCols ) -1) );
        hScrollPos= min( hScrollPos, hScrollMax );
        break;

    case SB_THUMBPOSITION:
        hScrollPos= LOWORD( Msg.LParam );
        break;
    default:
        break;
    }

// scrolling rectangle
dataRect.top= 0;
dataRect.bottom= winHeight;
dataRect.left= frozenWidth;
dataRect.right= winWidth;

//pixel shift = difference in old and new pix start
oldStart= newStart= 0;
for( c= min( oldPos, hScrollPos) ; c < max( oldPos, hScrollPos ) ; c++ )
    {
    int cw= COLWIDTH( scrPainter, frozenCols + c );

	if( c < oldPos )
		oldStart+= cw;

	if( c < hScrollPos )
		newStart+= cw;
    }
//NB one of these will be 0, the other the number of pixels to shift

HideFocus();

ScrollWindow( HWindow, (oldStart - newStart), 0, &dataRect, &dataRect );

SetScrollPos( HWindow, SB_HORZ, hScrollPos, TRUE );

RestoreFocusAfterScroll( 0, hScrollPos - oldPos );
}

//*****************************************************************************
// we have to hide focus right away, because the default WMPaint() processing
// sends a WM_ERASEBKGND message before it calls Paint() procedure.
//*****************************************************************************
void TBaseGrid::WMPaint( RTMessage Msg )
{
HideFocus();

TControl::WMPaint( Msg );

RestoreFocus();
};


//*****************************************************************************
// update display (handle paint msg)
//*****************************************************************************
void TBaseGrid::Paint( HDC hdc, PAINTSTRUCT &ps )
{
//ignore repaint until scroll bars are ok
if( bScrollChange )
	{
    UpdateScroll();
	return;
	}

TDrawInfo drawInfo;
drawInfo.pPainter= scrPainter;
drawInfo.hdc= hdc;
drawInfo.updRect= ps.rcPaint;
drawInfo.hwnd= HWindow;

PaintGrid( drawInfo );
}

//*****************************************************************************
//  check mouse point; get cell coordinates
//*****************************************************************************
void TBaseGrid::PointToCell( POINT &point, int &aRow, int &aCol, TPointStyle &aStyle, RECT &aRect )
{
WORD rowPixel=0;
WORD colPixel=0;
WORD prevRowPixel=0;
WORD prevColPixel=0;
WORD row, col;

if( point.y >= 0 && point.y < winHeight &&
	point.x >= 0 && point.x < winWidth )
	{
    //default
    aStyle= kMarginPoint;
    aRow= aCol= -1;

    rowPixel= topMargin + TopLineHeight( scrPainter );

    colPixel= leftMargin + LeftLineWidth( scrPainter );

    //FIXME margin points have essentially undefined uncoordinates;
    // really ought to fill in as much as makes sense -- i.e., the margin
    // should be treated as a row or column
    if( point.y >= rowPixel && point.x >= colPixel )
		{

        // first find proper row
		for( row=0; rowCnt > LOGROW( row ); row++ )
			{
            prevRowPixel= rowPixel;
            rowPixel+= CellHeight( scrPainter, LOGROW( row ) );
			if( point.y < rowPixel )
				{
				aRow= LOGROW( row );
                aStyle= kCellPoint;
				break;
				}

            prevRowPixel= rowPixel;
            rowPixel+= GridHeight( scrPainter, LOGROW( row ) );
			if( point.y < rowPixel )
				{
				aRow= LOGROW( row );
                aStyle= kRowPoint;
				break;
				}
			} // next row


		// get proper column
		for( col= 0; colCnt > LOGCOL( col ); col++ )
			{
            prevColPixel= colPixel;
            colPixel+= CellWidth( scrPainter, LOGCOL( col ) );
			if( point.x < colPixel )
				{
				aCol= LOGCOL( col );
                //leave aStyle as set by row analysis
				break;
				}

            prevColPixel= colPixel;
            colPixel+= GridWidth( scrPainter, LOGCOL( col ) );
			if( point.x < colPixel )
				{
				aCol= LOGCOL( col );
                if( aStyle == kCellPoint )
                    aStyle= kColPoint;       //on col grid
                else if( aStyle == kRowPoint )
                    aStyle= kGridPoint;      //on grid intersection
				break;
				}
			} // next col
		}
	}
else
	{
	aRow= -1;
	aCol= -1;
    aStyle= kNCPoint;                    //non-client area point
    }

//fill in cell rectangle
aRect.top= prevRowPixel;
aRect.bottom= rowPixel;
aRect.left= prevColPixel;
aRect.right= colPixel;

}

//*****************************************************************************
// relative scroll
//*****************************************************************************
void TBaseGrid::Scroll( int rowDelta, int colDelta )
{
int r, c;

if( rowDelta == 0 )
    r= -1;
else
    r= vScrollPos - rowDelta;

if( colDelta == 0 )
    c= -1;
else
    c= hScrollPos - colDelta;

GotoRC( r, c );
}

//*****************************************************************************
// force scroll to the nTh logical cell
// (emulates scroll thumb)
//*****************************************************************************
void TBaseGrid::GotoRC( int row, int col )
{
if( !bScrollChange && IsWindow( HWindow ) && IsWindowVisible( HWindow ) )
    {
	if( row != -1)
		{
        row= min( row, vScrollMax );
        SendMessage( HWindow, WM_VSCROLL, (WORD) SB_THUMBPOSITION, (LONG) row );
		}

	if( col != -1 )
		{
        col= min( col, hScrollMax );
        SendMessage( HWindow, WM_HSCROLL, (WORD) SB_THUMBPOSITION, (LONG) col );
		}
    }
else
    {
	//let Paint() handle it
	initCol= col;
	initRow= row;
	ScrollChange();
    }
}



//*****************************************************************************
// -1, -1 invalidates whole window
//*****************************************************************************
void TBaseGrid::InvalidateCell( int row, int col )
{
RECT rFrame;
RECT rCell;

if( IsWindow( HWindow ) && IsWindowVisible( HWindow ) )
    {
	if( row != -1 && col != -1 )
        {
        if( CellToRect( row, col, rFrame, rCell ) )
            InvalidateRect( HWindow, &rFrame, TRUE );
        }
	else if( row == -1 && col != -1 )		//invalidate column
		{
        if( CellToRect( 0, col, rFrame, rCell ) )
            {
            rFrame.top= 0;
            rFrame.bottom= winHeight;
            InvalidateRect( HWindow, &rFrame, TRUE );
            }
        }
	else if( row != -1 && col == -1 )		//invalidate row
		{
        //find top of cell
        if( CellToRect( row, 0, rFrame, rCell ) )
            {
            rFrame.left= 0;
            rFrame.right= winWidth;
            InvalidateRect( HWindow, &rFrame, TRUE );
            }
        }
    else // invalidate whole window
        InvalidateRect( HWindow, NULL, TRUE );
    }
}

//*****************************************************************************
// -1, -1 invalidates whole window
// invalidate onscreen part, even if part is offscreen
//*****************************************************************************
void TBaseGrid::InvalidateRange( int aTop, int aLeft, int aBottom, int aRight )
{
RECT rFrame;

RangeToRect( aTop, aLeft, aBottom, aRight, rFrame );

if( !IsRectEmpty( &rFrame ) && IsWindow( HWindow ) && IsWindowVisible( HWindow ) )
    {
    InvalidateRect( HWindow, &rFrame, TRUE );
    }

}

//*****************************************************************************
// given coordinates of a single cell, get corresponding rectangles
// (e.g., for refresh)
// frame rect includes right and bottom grid lines
// cell rect does not
// return FALSE and empty rect (0,0,0,0) if not onscreen
//*****************************************************************************
BOOL TBaseGrid::CellToRect( int aRow, int aCol, RECT &rFrame, RECT &rCell )
{
WORD rowPixel, colPixel;
int r,c;
BOOL found= TRUE;

//find top of cell
rowPixel= topMargin + TopLineHeight( scrPainter);
for( r= 0; r < rowCnt && found; r++ )
    {
    if( rowPixel > winHeight )      //must be offscreen
        found= FALSE;
    else if( aRow == LOGROW( r ) )
        break;
    else if( aRow < LOGROW( r ) )
        found= FALSE;           //must be hidden

    rowPixel+= ROWHEIGHT( scrPainter, LOGROW( r ) );        //get to start of next
    }

//find left of cell
colPixel= leftMargin + LeftLineWidth( scrPainter );
for( c= 0; c < colCnt && found; c++ )
    {
    if( colPixel > winWidth )      //must be offscreen
        found= FALSE;
    else if( aCol == LOGCOL( c ) )
        break;
    else if( aCol < LOGCOL( c ) )
        found= FALSE;           //must be hidden

    colPixel+= COLWIDTH( scrPainter, LOGCOL( c ) );
    }

if( found )
    {
    rFrame.top= rowPixel;
    rFrame.bottom= rowPixel + ROWHEIGHT( scrPainter, aRow );
    rFrame.left= colPixel;
    rFrame.right= colPixel + COLWIDTH( scrPainter, aCol );
    }
else
    rFrame.top= rFrame.bottom= rFrame.left= rFrame.right= 0;

//adjust cell rect for grid lines
rCell= rFrame;
if( found )
    {
    rCell.right -= GridWidth( scrPainter, aCol );
    rCell.bottom -= GridHeight( scrPainter, aRow );
    }

return found;
}

//*****************************************************************************
// given range of logical cells, calculate corresponding (covering) pixel rect
//*****************************************************************************
void TBaseGrid::RangeToRect( int aTop, int aLeft, int aBottom, int aRight, RECT &aRect )
{
WORD startRowPixel, startColPixel;
WORD endRowPixel, endColPixel;
int r,c;

//find top of range
startRowPixel= topMargin + TopLineHeight( scrPainter );
for( r= 0; r < rowCnt; r++ )
    {
    if( aTop <= LOGROW( r ) )
        break;
    else
        startRowPixel+= ROWHEIGHT( scrPainter, LOGROW( r ) );        //get to start of next
    }

//this row is also included
endRowPixel= startRowPixel + ROWHEIGHT( scrPainter, LOGROW( r ) );

//find last row of range
for( r++ ; r < rowCnt; r++ )
    {
    if( aBottom < LOGROW( r ) )
        break;
    else
        endRowPixel+= ROWHEIGHT( scrPainter, LOGROW( r ) );
    }

//find left of range
startColPixel= leftMargin + LeftLineWidth( scrPainter );
for( c= 0; c < colCnt; c++ )
    {
    if( aLeft <= LOGCOL( c ) )
        break;
    else
        startColPixel+= COLWIDTH( scrPainter, LOGCOL( c ) );
    }

//include col
endColPixel= startColPixel + COLWIDTH( scrPainter, LOGCOL( c ) );

//find right
for( c++; c < colCnt; c++ )
    {
    if( aRight < LOGCOL( c ) )
        break;
    else
        endColPixel+= COLWIDTH( scrPainter, LOGCOL( c ) );
    }


aRect.top= startRowPixel;
aRect.bottom= endRowPixel;
aRect.left= startColPixel;
aRect.right= endColPixel;
}

//*****************************************************************************
// IsCellVisible
//
//   Is the specified cell visible in the window?
//*****************************************************************************
BOOL TBaseGrid::IsCellVisible( int row, int col )
{
    return IsColVisible( col ) && IsRowVisible( row );
}


//*****************************************************************************
// IsColVisible
//
// Is the specified column visible?
//*****************************************************************************
BOOL TBaseGrid::IsColVisible( int col )
{
    // cols out of range are not visible
    if( (col < 0) || (col >= colCnt) )
        return FALSE;

    // frozen cols are visible
    if (col < frozenCols)
        return TRUE;

    //cols less than scroll position are invisible
    if (hScrollPos > (col - frozenCols) )
        return FALSE;

    // march across from left to right, adding up column widths
    // until we've either gotten to the column we're looking for,
    // or we've run out of window width
    int colPixel = leftMargin + LeftLineWidth( scrPainter );
    for (int c = 0; c < colCnt; c++) {

        int logCol = LOGCOL( c );

        if (colPixel >= winWidth)
            return FALSE;           //no room for this column
        else if( col == logCol )
            return TRUE;            //at least some of it fits

        colPixel+= COLWIDTH( scrPainter, logCol );

    }

    SYSERR_STR( "Col count exceeded" );
    return FALSE;  // make compiler happy
}

//*****************************************************************************
// IsRowVisible
//
// Is the specified row visible?
//*****************************************************************************
BOOL TBaseGrid::IsRowVisible( int row )
{
    // rows out of range are not visible
    if( (row < 0) || row >= rowCnt )
        return FALSE;

    // frozen rows are visible
    if (row < frozenRows)
        return TRUE;

    //rows less than scroll position are invisible
    if ( vScrollPos > (row - frozenRows) )
        return FALSE;

    // march from top to bottom, adding up row heights
    // until we've either gotten to the row we're looking for,
    // or we've run out of window height
    int rowPixel = topMargin + TopLineHeight( scrPainter );
    for (int r = 0; r < rowCnt; r++) {

        int logRow = LOGROW( r );

        if (rowPixel >= winHeight)      //no room
            return FALSE;
        else if (row == logRow)         //some must fit...
            return TRUE;

        rowPixel += ROWHEIGHT( scrPainter, LOGROW( r ) );
    }

    SYSERR_STR( "Row count exceeded" );
    return FALSE; //make compiler happy

}

//*****************************************************************************

//*****************************************************************************
void TBaseGrid::GetWindowClass( WNDCLASS & AWndClass )
{
TControl::GetWindowClass( AWndClass );

// it's useful to always have screenDC setup
AWndClass.style |= CS_OWNDC;

AWndClass.hCursor= NULL;
AWndClass.hbrBackground= TGDIMap::GetScreenBrush( kHbrBkView );
}

//*****************************************************************************
//
//*****************************************************************************
void TBaseGrid::SetRowCnt( int cnt )
{
TRawGrid::SetRowCnt( cnt );
ScrollChange();
}

//*****************************************************************************
//

void TBaseGrid::SetColCnt( int cnt )
{
TRawGrid::SetColCnt( cnt );
ScrollChange();
}

//*****************************************************************************
//
//*****************************************************************************
void TBaseGrid::SetColFreeze( int cols )
{
TRawGrid::SetColFreeze( cols );
ScrollChange();
}

//*****************************************************************************
//
//*****************************************************************************
void TBaseGrid::SetRowFreeze( int rows )
{
TRawGrid::SetRowFreeze( rows );
ScrollChange();
}

//*****************************************************************************

//*****************************************************************************
void TBaseGrid::SetTopMargin( int pix )
{
TRawGrid::SetTopMargin( pix );
ScrollChange();
}

//*****************************************************************************

//*****************************************************************************
void TBaseGrid::SetLeftMargin( int pix )
{
TRawGrid::SetLeftMargin( pix );
ScrollChange();
}

