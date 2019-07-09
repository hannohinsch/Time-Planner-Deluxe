#ifndef __rawgrid__
#define __rawgrid__

_CLASSDEF( TRawGrid )
_CLASSDEF( TDrawInfo )
_CLASSDEF( TPrintView )
_CLASSDEF( TPaint )

#ifndef __safstr__
#include "safstr.h"
#endif

class _CLASSTYPE TDrawInfo
{
public:
    BOOL IsColor();
    RECT cellRect;
    int row;
    int col;
    RECT updRect;
    int updTopCell, updLeftCell, updBottomCell, updRightCell;
    HDC hdc;
    HWND hwnd;
    PTPaint pPainter;
};

//*****************************************************************************
// these are so often used together....
//*****************************************************************************


class _CLASSTYPE TRawGrid
{
friend TPrintView;

public:

    TRawGrid( void );
    virtual~TRawGrid( void );

    // utility functions: get full size of a cell
    int COLWIDTH( PTPaint p, int n ) { return ( CellWidth(p, n) + GridWidth(p, n) ); };
    int ROWHEIGHT( PTPaint p, int n ) { return ( CellHeight(p, n) + GridHeight(p, n) ); };

    // utility fns: map a physical screen coordinate to a logical one
    int LOGROW( int row ) { return ((row < frozenRows)? row: row + vScrollPos); };
    int LOGCOL( int col ) { return ((col < frozenCols)? col: col + hScrollPos); };

    //print support
    void SetPrtTitle( const char * );
    void GetPrtTitle( RTSafStr );
    virtual void SetDefaultPrintRange( void );
    virtual void GetPrtPages( WORD &hPages, WORD &vPages );
    virtual void PrintSection( RTDrawInfo di, WORD hPage, WORD vPage);

    virtual BOOL HChgCellStartIndex( int n, char *buf, int size);
    virtual BOOL HChgCellEndIndex( int n, char *buf, int size);
    virtual BOOL VChgCellStartIndex( int n, char *buf, int size);
    virtual BOOL VChgCellEndIndex( int n, char *buf, int size);

    virtual void PaintGrid( RTDrawInfo, BOOL bExact= FALSE );

    virtual void SetRowCnt( int cnt );
    virtual void SetColCnt( int cnt );

    virtual void SetRowFreeze( int rows );
    virtual void SetColFreeze( int cols );

    virtual void SetTopMargin( int );
    virtual void SetLeftMargin( int );


protected:

    //printing support fns
    virtual int GetNextPageStartCol( int startCol, WORD &lastPix );
    virtual int GetNextPageStartRow( int startRow, WORD &lastPix );

    //cell size fns
    virtual int  CellWidth( PTPaint, int /*col*/ ) { return 0; };
    virtual int  CellHeight( PTPaint, int /*row*/ ) { return 0; };

    virtual int  GridWidth( PTPaint, int /*col*/ ) { return 0; };
    virtual int  GridHeight( PTPaint, int /*row*/ ) { return 0; };

    virtual int  TopLineHeight( PTPaint ) { return 0; };
    virtual int  LeftLineWidth( PTPaint ) { return 0; };

    virtual void BeginCellPaint( RTDrawInfo ) {};
    virtual void PaintCell( RTDrawInfo ) {};
    virtual void EndCellPaint( RTDrawInfo ) {};

    virtual void BeginGridPaint( RTDrawInfo ) {};
    virtual void PaintTopLine( RTDrawInfo ) {};
    virtual void PaintLeftLine( RTDrawInfo ) {};
    virtual void PaintGridCol( RTDrawInfo ) {};
    virtual void PaintGridRow( RTDrawInfo ) {};
    virtual void EndGridPaint( RTDrawInfo ) {};

    virtual void WeekGridNewPageHook( void ) {};

    HDC printDC;            //may be null
    PTPaint prtPainter;

    int frozenCols, frozenRows;
    int vScrollPos, hScrollPos;
    int vScrollMax, hScrollMax;

    int rowCnt;             //origin 1
	int colCnt;

    WORD prtHeight;             //height in pixels
    WORD prtWidth;              //width in pixels

    int initCol;            //indices, origin 0
	int initRow;

	int topMargin;
	int leftMargin;

    //print range
    int rFirstLiquid;                    //first row past frozen
    int cFirstLiquid;                    //first col past frozen
    int rLastLiquid;                     //# of rows past frozen
    int cLastLiquid;                     //# of cols past frozen

    //print title
    TSafStr prtTitle;

};

#endif
