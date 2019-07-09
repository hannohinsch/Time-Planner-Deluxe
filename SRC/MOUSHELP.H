#ifndef __moushelp__
#define __moushelp__

_CLASSDEF( TMousHelp )

#ifndef __safstr__
#include "safstr.h"
#endif

class _CLASSTYPE TMousHelp: public TDialog
{

public:

    static void TMousHelp::MousHelp( PTWindowsObject parent, char *text );

private:

    TMousHelp( PTWindowsObject parent, char *text );
    ~TMousHelp( void );
    void HandleRelease( RTMessage ) = [ WM_FIRST + WM_RBUTTONUP ];
    void SetupWindow( void );

    HANDLE   itsOldCaptureH;
    PTStatic itsTextCP;
    TSafStr  itsText;
};

#endif
