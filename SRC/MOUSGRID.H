#ifndef __mousgrid__
#define __mousgrid__

#ifndef __basegrid__
#include "basegrid.h"
#endif
#ifndef __paint__
#include "paint.h"
#endif 

_CLASSDEF( TMousGrid )
_CLASSDEF( TMousInfo )

class _CLASSTYPE TMousInfo
{
public:
    TBaseGrid::TPointStyle style;  //relationship of mouse location to cell
    int row;            //cell row
    int col;            //cell col
    WORD state;         //mouse state (from Windows)
    RECT cellRect;      //bounding rectangle of cell
    POINT xy;           //current mouse location (Window coords)
	BOOL bKeyAction;	//this op due to key emulation, not mouse
};

// limited set of keys that emulate mouse actions
typedef enum {
	kUpKey,
	kDownKey,
	kLeftKey,
	kRightKey,

    kScrUpKey,
    kScrDownKey,
    kScrLeftKey,
    kScrRightKey,

    kLeftClickKey,
    kRightClickKey,
    kDblClickKey,

    kShiftUpKey

    } TMousKey;
	

class _CLASSTYPE TMousGrid: public TBaseGrid
{

public:
    TMousGrid( PTWindowsObject AParent, WORD resID );
    TMousGrid( PTWindowsObject AParent );
    TMousGrid( PTWindowsObject AParent, WORD anID, int x, int y, int dx, int dy );
    virtual~TMousGrid( void );

    virtual LPSTR GetClassName() { return "MousGrid"; };
    virtual void GetWindowClass( WNDCLASS & AWndClass );

    BOOL IsCellInRange( int row, int col );
    void SetRangeAnchor( int row, int col );
    void SetRangeExtent( int height, int width );
    BOOL GetRangeRect( int &top, int &left, int &bottom, int &right );
    void GetRangeAnchor( int &row, int &col );
    void GetRangeExtent( int &height, int &width );

    BOOL HasFocus( void ) { return bHasFocus; };
    BOOL IsFocusCell( int row, int col );
	void SetFocusCell( int row, int col );
	void GetFocusCell( int &row, int &col );

    //query the position of a mouse point in relation to the window
    BOOL XYWithin( RTMousInfo mi )     { return mi.xy.y >= 0 && mi.xy.y <= winHeight &&  mi.xy.x >= 0 && mi.xy.x <= winWidth; };
    BOOL XYAbove( RTMousInfo mi )      { return mi.xy.y < 0; };
    BOOL XYBelow( RTMousInfo mi )      { return mi.xy.y > winHeight; };
    BOOL XYLeft( RTMousInfo mi )       { return                                    mi.xy.x < 0; };
    BOOL XYRight( RTMousInfo mi )      { return                                    mi.xy.x > winWidth; };
    BOOL XYAboveLeft( RTMousInfo mi )  { return mi.xy.y < 0 &&                        mi.xy.x < 0; };
    BOOL XYAboveCenter( RTMousInfo mi ){ return mi.xy.y < 0 &&                        mi.xy.x >= 0 && mi.xy.x <= winWidth; };
    BOOL XYAboveRight( RTMousInfo mi ) { return mi.xy.y < 0 &&                        mi.xy.x > winWidth; };
    BOOL XYCenterLeft( RTMousInfo mi ) { return mi.xy.y >= 0 && mi.xy.y <= winHeight &&  mi.xy.x < 0; };
    BOOL XYCenterRight( RTMousInfo mi ){ return mi.xy.y >= 0 && mi.xy.y <= winHeight &&  mi.xy.x > winWidth; };
    BOOL XYBelowLeft( RTMousInfo mi )  { return mi.xy.y > winHeight &&                mi.xy.x < 0; };
    BOOL XYBelowCenter( RTMousInfo mi ){ return mi.xy.y > winHeight &&                mi.xy.x >= 0 && mi.xy.x <= winWidth; };
    BOOL XYBelowRight( RTMousInfo mi ) { return mi.xy.y > winHeight &&                mi.xy.x > winWidth; };

    BOOL XYInFrozenRow( RTMousInfo mi ){ return (0 <= mi.row) && (mi.row < frozenRows); }
    BOOL XYInFrozenCol( RTMousInfo mi ){ return (0 <= mi.col) && (mi.col < frozenCols); }

    virtual void UseTimerWithLButton( BOOL on );

protected:

    BOOL bRangeInProgress;
    BOOL bDragInProgress;

    virtual BOOL IsCellSelectable( int row, int col );
    virtual void PaintFocus( HDC, RECT & );
    virtual void GetDefaultFocusCell( int &row, int &col );

	virtual void MousKey( TMousKey );

    virtual void MousClick( RTMousInfo );
    virtual void MousDblClick( RTMousInfo );
    virtual void MousRtClick( RTMousInfo );
    virtual void MousOver( RTMousInfo );

    virtual BOOL MousRangeInit( RTMousInfo );
    virtual void MousRangeOver( RTMousInfo );
    virtual void MousRangeRelease( RTMousInfo );

    virtual BOOL MousDragInit( RTMousInfo );
    virtual void MousDragOver( RTMousInfo );
    virtual void MousDragRelease( RTMousInfo );

    virtual void HideFocus( void );
    virtual void RestoreFocus( void );
    virtual void RestoreFocusAfterScroll( int /*rowDelta*/, int /*colDelta*/ );
    virtual void RestoreFocusAfterResize( void );
    virtual void InvalidateFocus( void );

    virtual void WMSetFocus(RTMessage)= [WM_FIRST + WM_SETFOCUS];
    virtual void WMKillFocus(RTMessage)= [WM_FIRST + WM_KILLFOCUS];

private:
    void WMMouseActivate(RTMessage)= [WM_FIRST + WM_MOUSEACTIVATE];
    void WMLButtonDown(RTMessage)= [WM_FIRST + WM_LBUTTONDOWN ];
    void WMDoubleClick(RTMessage)= [WM_FIRST + WM_LBUTTONDBLCLK ];
    void WMRButtonDown(RTMessage)= [WM_FIRST + WM_RBUTTONDOWN ];
    void WMMouseMove(RTMessage)= [WM_FIRST + WM_MOUSEMOVE];
    void WMLButtonUp(RTMessage)= [WM_FIRST + WM_LBUTTONUP];
    void WMRButtonUp(RTMessage)= [WM_FIRST + WM_RBUTTONUP];

    void WMTimer(RTMessage)= [WM_FIRST + WM_TIMER];

    void WMKeyDown(RTMessage)= [WM_FIRST + WM_KEYDOWN];
    void WMKeyUp(RTMessage)= [WM_FIRST + WM_KEYUP];
    void WMChar(RTMessage)= [WM_FIRST + WM_CHAR ];
	void WMGetDlgCode(RTMessage)= [WM_FIRST + WM_GETDLGCODE ];
    void WMSysChar(RTMessage)= [WM_FIRST + WM_SYSCHAR];

    void UpdateInfo( RTMessage );
    void UpdateFocus( void );

    TMousInfo itsInfo;

    BOOL bUseTimerWithLButton;
    BOOL bInvalidFocusRect;

    int selectRow;
    int selectCol;
    int anchorRow;
    int anchorCol;
    int rangeRowExtent;
    int rangeColExtent;

    BOOL    bHasFocus;
    BOOL    bVisibleFocus;
    RECT    focusRect;
    BOOL    itsIsCaptive;
    HANDLE  itsOldCaptureH;
    BOOL    itsClickInProgress;
};

#endif
