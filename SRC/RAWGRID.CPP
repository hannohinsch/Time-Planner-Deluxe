/***************************************************************************
 *
 *  MODULE     : rawgrid.cpp
 *               base class for print grids
 *
 ***************************************************************************/
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "rawgrid.h"

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
#ifndef __print__
#include "print.h"
#endif

//*****************************************************************************

//*****************************************************************************
BOOL TDrawInfo::IsColor( void )
{
int colors= GetDeviceCaps( hdc, NUMCOLORS );
return( colors > 2 );
};

//=============================================================================

//*****************************************************************************

//*****************************************************************************
void TRawGrid::TRawGrid( void )
{
prtPainter= NULL;

vScrollPos= hScrollPos= 0;
vScrollMax= hScrollMax= 0;
rowCnt= colCnt= 0;
frozenRows= frozenCols= 0;
initRow= initCol= -1;
topMargin= leftMargin= 0;

prtTitle.Set( "Time Planner Report" );
SetDefaultPrintRange();
}

//*****************************************************************************

//*****************************************************************************
void TRawGrid::~TRawGrid( void )
{
}

//*****************************************************************************
// control which cells to print
//*****************************************************************************
void TRawGrid::SetDefaultPrintRange( void )
{
rFirstLiquid= frozenRows;
rLastLiquid= rowCnt - 1;

cFirstLiquid= frozenCols;
cLastLiquid= colCnt - 1;
}


//*****************************************************************************

//*****************************************************************************
void TRawGrid::SetPrtTitle( const char * title )
{
prtTitle.Set( title );
}

//*****************************************************************************

//*****************************************************************************
void TRawGrid::GetPrtTitle( RTSafStr title )
{
title= prtTitle;
}

//*****************************************************************************
//
//*****************************************************************************
void TRawGrid::GetPrtPages( WORD &hPages, WORD &vPages )
{
WORD pix;

hPages= 0;
vPages= 0;

int startRow= rFirstLiquid;
while( startRow != -1 )
    {
    startRow= GetNextPageStartRow( startRow, pix );
    vPages++;
    }

int startCol= cFirstLiquid;
while( startCol != -1 )
    {
    startCol= GetNextPageStartCol( startCol, pix );
    hPages++;
    }

}


//*****************************************************************************
// Given a starting row, fill up a page, and return the row with which
// the next page should start
// Return -1 if all rows fit on this page
// lastPix is set to maximum pixel used on this page
//*****************************************************************************
int TRawGrid::GetNextPageStartRow( int startRow, WORD &lastPix )
{
WORD pix= 0;
WORD maxPix;
int row;
RECT r;
int i;

TPrint::GetPrtPixels( &r );
maxPix= r.bottom - r.top;

//leave room for margins
pix+= topMargin + TopLineHeight( prtPainter );

//leave room for frozen rows
for( i= 0; i < frozenRows; i++ )
    pix+= ROWHEIGHT( prtPainter, i );

// now see how many rows fit into remaining space
//NOTE assumes that startRow is past frozenRows
row= startRow;
while( pix < maxPix && row <= rLastLiquid )
    {
    pix+= ROWHEIGHT( prtPainter, row );
    row++;
    }

if( pix > maxPix )
    row--;                          //start next page one row back
else if( row > rLastLiquid )
    row= -1;                        //there is no next page

lastPix= pix;       //return max

return row;
}

//*****************************************************************************
// similar to above
//*****************************************************************************
int TRawGrid::GetNextPageStartCol( int startCol, WORD &lastPix )
{
WORD pix= 0;
WORD maxPix;
int col;
RECT r;
int i;

TPrint::GetPrtPixels( &r );
maxPix= r.right - r.left;

//leave room for margins
pix+= leftMargin + LeftLineWidth( prtPainter );

//subtract room for frozen cols
for( i= 0; i < frozenCols; i++ )
    pix+= COLWIDTH( prtPainter, i );

// now see how many cols fit into remaining space
// NOTE Assumption that startCol is past frozenCols
col= startCol;
while( pix < maxPix && col <= cLastLiquid )
    {
    pix += COLWIDTH( prtPainter, col );
    col++;
    }

if( pix > maxPix )
    col--;                          //start next page one col back
else if( col > cLastLiquid )
    col= -1;                        //there is no next page

lastPix= pix;                       //return max pix

return col;
}

//*****************************************************************************
// prints grid page
//*****************************************************************************
void TRawGrid::PrintSection( RTDrawInfo di, WORD hPage, WORD vPage)
{
int saved_vScrollPos= vScrollPos;
int saved_hScrollPos= hScrollPos;
int i;

WORD pix;

//get proper horz. scroll positions for this page
int nextPageCol= cFirstLiquid;
int thisPageCol;
for( i= 0; i <= hPage && -1 != nextPageCol; i++ )
    {
    thisPageCol= nextPageCol;
    nextPageCol= GetNextPageStartCol( thisPageCol, pix );
    }
hScrollPos= thisPageCol - frozenCols;

// the long notes in the week grid have to be
// set up separately for each week, and hScrollPos
// has to be known in order to determine which
// week we're printing. On the other hand, since
// vScrollPos will change depending on the height
// of the long notes for this week, we have to
// put the hook in between calculating hScrollPos
// and vScrollPos
WeekGridNewPageHook();

//set maximum horz. pixel
di.updRect.right= min( pix, di.updRect.right );

//get proper vert. scroll positions for this page
int nextPageRow= rFirstLiquid;
int thisPageRow;
for( i= 0; i <= vPage && -1 != nextPageRow; i++ )
    {
    thisPageRow= nextPageRow;
    nextPageRow= GetNextPageStartRow( thisPageRow, pix );
    }
vScrollPos= thisPageRow - frozenRows;

//set maximum vert. pix
di.updRect.bottom= min( pix, di.updRect.bottom );

//print only what's in updRect, not the whole page
IntersectClipRect( di.hdc, di.updRect.left, di.updRect.top,
                   di.updRect.right, di.updRect.bottom );

//the TRUE specifies that we don't want partial cells printed
//at page borders
PaintGrid( di, TRUE );

vScrollPos= saved_vScrollPos;
hScrollPos= saved_hScrollPos;
}

//*****************************************************************************
// modify index by n, then convert into meaningful representation, such as
// the time and date of that column
// return FALSE if index would be out of range
//
// NB Overloaded by classes that allow subrange printing
//*****************************************************************************
BOOL TRawGrid::HChgCellStartIndex( int /*n*/, char *buf, int /*size*/ )
{
wsprintf( buf, "" );
return FALSE;
}

//*****************************************************************************
// see above
//*****************************************************************************
BOOL TRawGrid::HChgCellEndIndex( int /*n*/, char *buf, int /*size*/ )
{
wsprintf( buf, "" );
return FALSE;
}

//*****************************************************************************
// see above
//*****************************************************************************
BOOL TRawGrid::VChgCellStartIndex( int /*n*/, char *buf, int /*size*/ )
{
wsprintf( buf, "" );
return FALSE;
}

//*****************************************************************************
// see above
//*****************************************************************************
BOOL TRawGrid::VChgCellEndIndex( int /*n*/, char *buf, int /*size*/ )
{
wsprintf( buf, "" );
return FALSE;
}


//*****************************************************************************
// figure out which cells need to be repainted, and call the appropriate
// paint functions
//
// If bExact, then paint a cell iff all of it can be drawn.  This is used
// by print routines to avoid drawing partial cells at edges of pages.
//*****************************************************************************
void TRawGrid::PaintGrid( RTDrawInfo drawInfo, BOOL bExact )
{
int r, c;
int firstRow, limitRow, firstCol, limitCol;
int leftPix, rightPix, topPix, bottomPix;
int delta;
RECT rDraw;

//figure out a rectangular matrix of cells that cover the invalid region

PTPaint ourPainter= drawInfo.pPainter;

topPix= topMargin + TopLineHeight( ourPainter );
firstRow= 0;        //default
if( topPix <= drawInfo.updRect.top )
    {
    for( r=0; rowCnt > LOGROW( r ); r++ )
        {
        delta= ROWHEIGHT( ourPainter, LOGROW( r ) );

        if( (topPix+delta) > drawInfo.updRect.top )
            break;
        else
            topPix+= delta;
        }
    firstRow= r;
    }

//now find end
bottomPix= topPix;
for( r= firstRow; rowCnt > LOGROW( r ); r++ )
	{
    if( bottomPix >= drawInfo.updRect.bottom )
		break;

    bottomPix+= ROWHEIGHT( ourPainter, LOGROW( r ) );
    }

//go back a row iff bExact was specified, and we went too far
if( bExact && bottomPix > drawInfo.updRect.bottom && r > firstRow )
    {
    r--;
    bottomPix-= ROWHEIGHT( ourPainter, LOGROW( r ) );
    }

limitRow= r;


//columns
leftPix= leftMargin + LeftLineWidth( ourPainter );
firstCol= 0;        //default
if( leftPix <= drawInfo.updRect.left )
    {
    for( c=0; colCnt > LOGCOL( c ); c++ )
        {
        delta= COLWIDTH( ourPainter, LOGCOL( c ) );

        if( (leftPix+delta) > drawInfo.updRect.left )
            break;
        else
            leftPix+= delta;
        }
    firstCol= c;
    }

//now find end
rightPix= leftPix;
for( c= firstCol; colCnt > LOGCOL( c ); c++ )
	{
    if( rightPix >= drawInfo.updRect.right )
		break;

    rightPix+= COLWIDTH( ourPainter, LOGCOL( c ) );
    }

//go back one col iff bExact was specified, and we went too far
if( bExact && rightPix > drawInfo.updRect.right && c > firstCol )
    {
    c--;
    rightPix-= COLWIDTH( ourPainter, LOGCOL( c ) );
    }

limitCol= c;

//
//Now call the appropriate paint functions
//
drawInfo.updTopCell= LOGROW( firstRow );
drawInfo.updLeftCell= LOGCOL( firstCol );
drawInfo.updBottomCell= LOGROW( limitRow );
drawInfo.updRightCell= LOGCOL( limitCol );

SetRectEmpty( &drawInfo.cellRect );
drawInfo.row= drawInfo.col= -1;
BeginGridPaint( drawInfo );

rDraw.left= leftPix;
rDraw.right= rightPix;

if( topMargin + TopLineHeight( ourPainter ) >= drawInfo.updRect.top )
    {
    rDraw.top= topMargin;
    rDraw.bottom= rDraw.top + TopLineHeight( ourPainter );
    drawInfo.cellRect= rDraw;
    // leftPix includes left line width, which we need to exclude
    // in order to hit the top left pixel of screen
    drawInfo.cellRect.left -= LeftLineWidth( ourPainter );
    PaintTopLine( drawInfo );
    }

rDraw.bottom= topPix;     //bottom of last valid row
for( r= firstRow; r < limitRow; r++ )
	{
    rDraw.top= rDraw.bottom + CellHeight( ourPainter, LOGROW( r ) );
    rDraw.bottom= rDraw.top + GridHeight( ourPainter, LOGROW( r ) );
    drawInfo.cellRect= rDraw;
    drawInfo.row=r;
    drawInfo.col=-1;
    PaintGridRow( drawInfo );
    }


rDraw.top= topPix;
rDraw.bottom= bottomPix;

if( leftMargin + LeftLineWidth( ourPainter ) >= drawInfo.updRect.left )
    {
    rDraw.left= leftMargin;
    rDraw.right= rDraw.left + LeftLineWidth( ourPainter );
    drawInfo.cellRect= rDraw;
    PaintLeftLine( drawInfo );
    }


rDraw.right= leftPix;           //end of last valid col
for( c= firstCol; c < limitCol; c++ )
	{
    rDraw.left= rDraw.right + CellWidth( ourPainter, LOGCOL( c ) );
    rDraw.right= rDraw.left + GridWidth( ourPainter, LOGCOL( c ) );
    drawInfo.cellRect= rDraw;
    drawInfo.row=-1;
    drawInfo.col=c;
    PaintGridCol( drawInfo );
    }

SetRectEmpty( &drawInfo.cellRect );
EndGridPaint( drawInfo );

BeginCellPaint( drawInfo );

rDraw.top= topPix;
for( r= firstRow; r < limitRow; r++ )
	{
    rDraw.bottom = rDraw.top + CellHeight( ourPainter, LOGROW( r ) );
	rDraw.left= leftPix;
	for( c= firstCol; c < limitCol; c++ )
		{
        rDraw.right= rDraw.left + CellWidth( ourPainter, LOGCOL( c ) );

        drawInfo.row= LOGROW( r );
        drawInfo.col= LOGCOL( c );
        drawInfo.cellRect= rDraw;
        PaintCell( drawInfo );

        rDraw.left= rDraw.right + GridWidth( ourPainter, LOGCOL( c ) );
        }
    rDraw.top= rDraw.bottom + GridHeight( ourPainter, LOGROW( r ) );
    }

SetRectEmpty( &drawInfo.cellRect );
drawInfo.row= drawInfo.col= -1;
EndCellPaint( drawInfo );
}



//*****************************************************************************
//
//*****************************************************************************
void TRawGrid::SetRowCnt( int cnt )
{
rowCnt= cnt;
}

//*****************************************************************************
//
//*****************************************************************************
void TRawGrid::SetColCnt( int cnt )
{
colCnt= cnt;
}

//*****************************************************************************
//
//*****************************************************************************
void TRawGrid::SetColFreeze( int cols )
{
frozenCols= cols;
}

//*****************************************************************************
//
//*****************************************************************************
void TRawGrid::SetRowFreeze( int rows )
{
frozenRows= rows;
}

//*****************************************************************************

//*****************************************************************************
void TRawGrid::SetTopMargin( int pix )
{
topMargin= pix;
}

//*****************************************************************************

//*****************************************************************************
void TRawGrid::SetLeftMargin( int pix )
{
leftMargin= pix;
}

