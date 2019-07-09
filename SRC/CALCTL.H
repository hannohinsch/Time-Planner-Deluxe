#ifndef __calctl__
#define __calctl__

_CLASSDEF( TCalCtl )

#ifndef __mousgrid__
#include "mousgrid.h"
#endif
#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __msg__
#include "msg.h"
#endif


class _CLASSTYPE TCalCtl: public TMousGrid {

public:

    TCalCtl( PTWindowsObject AParent, WORD resID, RTTime startDate );
    TCalCtl( PTWindowsObject AParent, WORD resID, int x, int y, int dx, int dy, RTTime startDate );
    virtual ~TCalCtl();

    void SetMonth( RTTime time );

    void SetSelTime( RTTime time );
    RTTime GetSelTime( void );

    void SetAutoClick( BOOL );
    void GetClickPoint( POINT & );

protected:

    virtual void WMSetFocus(RTMessage)= [WM_FIRST + WM_SETFOCUS];
    virtual void WMKillFocus(RTMessage)= [WM_FIRST + WM_KILLFOCUS];
    void WMLocalColor( RTMessage ) = [ WM_FIRST + WM_LOCALCOLOR ];

    void BeginCellPaint( RTDrawInfo );
    void PaintCell( RTDrawInfo );
    int  CellWidth( PTPaint, int col );
    int  CellHeight( PTPaint, int row );

    void MousClick( RTMousInfo );
    void MousOver( RTMousInfo );
    void MousRtClick( RTMousInfo );

    virtual LPSTR  GetClassName( void ) { return "CalCtl";}
    virtual void GetWindowClass( WNDCLASS & AWndClass );
    void CellToTime( int row, int col, RTTime time );
    void TimeToCell( RTTime date, int &row, int &col );
    int CellToDay( int row, int col );

	BOOL IsCellSelectable( int row, int col );
    void GetDefaultFocusCell( int &row, int &col );
    void MousKey( TMousKey );
    void PaintFocus( HDC, RECT &);

    void SetColor( RTDrawInfo );

    int     itsFirstWeekday;
    int     itsLastDayOfMonth;
    TTime   itsStartTime;
    TTime   itsSelTime;
    BOOL    itsAutoClick;
    POINT   itsClickPoint;

};


#endif
