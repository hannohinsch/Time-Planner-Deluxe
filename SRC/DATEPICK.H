#ifndef __datepick__
#define __datepick__

_CLASSDEF( TDatePick )

#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __calctl__
#include "calctl.h"
#endif
#ifndef __msg__
#include "msg.h"
#endif

_CLASSDEF( TODTextButton )

class _CLASSTYPE TDatePick: public TControl {

public:

    TDatePick( PTWindowsObject AParent, WORD resID, RTTime startDate );
    TDatePick( PTWindowsObject AParent, WORD resID, int x, int y, int dx, int dy, RTTime startDate );
    virtual ~TDatePick();

    void SetupWindow( void );
    LPSTR  GetClassName( void ) { return "DatePick";}
    void GetWindowClass( WNDCLASS & AWndClass );

    RTTime GetSelTime( void );
    void SetSelTime( RTTime time );

private:
    void WMCtlColor(   RTMessage Msg) = [ WM_FIRST + WM_CTLCOLOR];
    void WMLocalColor( RTMessage )    = [ WM_FIRST + WM_LOCALCOLOR ];
	void WMSetFocus(   RTMessage )    = [ WM_FIRST + WM_SETFOCUS ];
    void WMDrawItem( RTMessage msg )  = [ WM_FIRST + WM_DRAWITEM ];

    void GridClick(    RTMessage Msg )= [ ID_FIRST + IDD_CALMONGRID ];
    void Minus(        RTMessage Msg )= [ ID_FIRST + IDD_CALMINUS ];
    void Minus2(       RTMessage Msg )= [ ID_FIRST + IDD_CALMINUS2 ];
    void Plus(         RTMessage Msg )= [ ID_FIRST + IDD_CALPLUS ];
    void Plus2(        RTMessage Msg )= [ ID_FIRST + IDD_CALPLUS2 ];
    void Title(        RTMessage Msg )= [ ID_FIRST + IDD_CALTITLE ];
    void ResetTitle( void );

    void TellSelChanged( void );

    TTime selTime;
    PTCalCtl pGrid;
    PTODTextButton pTitle;
    PTODTextButton pMinus;
    PTODTextButton pMinus2;
    PTODTextButton pPlus;
    PTODTextButton pPlus2;
};

#endif
