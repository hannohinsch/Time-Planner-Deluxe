#ifndef __basegrid__
#define __basegrid__

_CLASSDEF( TBaseGrid )

#ifndef __rawgrid__
#include "rawgrid.h"
#endif

class _CLASSTYPE TBaseGrid: public TControl, public TRawGrid
{
public:

	TBaseGrid( PTWindowsObject AParent, WORD resID );
	TBaseGrid( PTWindowsObject AParent, WORD anID, int x, int y, int dx, int dy );
	virtual~TBaseGrid( void );

    void ScrollChange( void );

    //common function to completely recalc and redisplay view
    //defined here for benefit of descendents
    virtual void Reset( void ){};

    virtual void GotoRC( int row, int col );
    void Scroll( int row, int col );

    typedef enum
	{	kCellPoint, 	//point is within a cell
		kColPoint,		//point lies on vertical gridline
		kRowPoint,		//point lies on horizontal gridline
		kGridPoint, 	//point lies on grid intersection
		kMarginPoint,	//point lies in window, on margin
        kNCPoint        //point lies outside of client area
	} TPointStyle;

    virtual void SetRowCnt( int cnt );
    virtual void SetColCnt( int cnt );

    virtual void SetRowFreeze( int rows );
    virtual void SetColFreeze( int cols );

    virtual void SetTopMargin( int );
    virtual void SetLeftMargin( int );

protected:

    virtual LPSTR GetClassName() { return "BaseGrid"; };
    virtual void GetWindowClass( WNDCLASS & AWndClass );

    virtual void SetupWindow( void );

    virtual void HideFocus( void ) {};
    virtual void RestoreFocus( void ) {};
    virtual void RestoreFocusAfterScroll( int /*rowDelta*/, int /*colDelta*/ ) {};
    virtual void RestoreFocusAfterResize( void ) {};

    virtual void WMVScroll(RTMessage)= [WM_FIRST + WM_VSCROLL];
    virtual void WMHScroll(RTMessage)= [WM_FIRST + WM_HSCROLL];
    virtual void WMSize(RTMessage)= [WM_FIRST + WM_SIZE];
    virtual void WMPaint(RTMessage)= [WM_FIRST + WM_PAINT ];

	virtual void InvalidateCell( int row, int col );
    virtual void InvalidateRange( int top, int left, int bottom, int right );

    void PointToCell( POINT &point, int &row, int &col, TPointStyle &style, RECT &cellRect );
    void RangeToRect( int top, int left, int bottom, int right, RECT &r );
    BOOL CellToRect( int row, int col, RECT &rFrame, RECT &rCell );

    BOOL IsRowVisible(  int row );
    BOOL IsColVisible(  int col );
    BOOL IsCellVisible( int row, int col );

    HDC screenDC;
    PTPaint scrPainter;

    WORD winHeight;             //window height in pixels
    WORD winWidth;              //window width in pixels

private:

    BOOL bScrollChange;

    void Paint( HDC hdc, PAINTSTRUCT & paintInfo);

	void Defaults( void );
	void SetupScrollBars( int aInitRow, int aInitCol );
    void UpdateScroll( void );
};

#endif
