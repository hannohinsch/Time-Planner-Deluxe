#ifndef __ourdlg__
#define __ourdlg__

#include "msg.h"
#ifndef __helpid__
#include "helpid.h"
#endif

_CLASSDEF( TOurDialog )

class _CLASSTYPE TOurDialog : public TDialog {

public:

    TOurDialog( PTWindowsObject parent, LPSTR name, PTModule module = NULL );
    TOurDialog( PTWindowsObject parent, int resID, PTModule module = NULL );
    virtual ~TOurDialog();

    static void CenterWindow( HANDLE parentWindow, HANDLE window );

protected:

    void NCPaint( RTMessage )       = [WM_FIRST + WM_NCPAINT];
    void NCActivate( RTMessage )    = [WM_FIRST + WM_NCACTIVATE];
    void NCLButtonDown( RTMessage ) = [WM_FIRST + WM_NCLBUTTONDOWN];
    void WMLButtonUp( RTMessage )   = [WM_FIRST + WM_LBUTTONUP];
    void WMMouseMove( RTMessage )   = [WM_FIRST + WM_MOUSEMOVE];
    void NCPaintHelp( RTMessage  )  = [WM_FIRST + WM_PAINTHELP];

    virtual void DisplayHelp( void );
    virtual void SetupWindow( void );
    virtual void PaintHelp( void );

    BOOL    itsMouseDown;
    BOOL    itsButtonOn;
    RECT    itsButtonR;
    HANDLE  itsParentCaptureH;
    HANDLE  itsOldCapture;
    DWORD   itsHelpTopic;
};

#endif
