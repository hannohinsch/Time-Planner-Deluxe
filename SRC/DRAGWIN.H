#ifndef __dragwin__
#define __dragwin__

_CLASSDEF( TDragWin )

#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __duration__
#include "duration.h"
#endif
#ifndef __timecomp__
#include "timecomp.h"
#endif

class _CLASSTYPE TDragWin: public TDialog {

public:

	TDragWin( PTWindowsObject AParent, int resourceId );
    virtual ~TDragWin( void );

    void Set( char *message, RTTimeComplex times );

private:
    void SetupWindow( void );

    void WMSetFocus(RTMessage)= [WM_FIRST + WM_SETFOCUS];

	PTStatic   itsMessageCP;
	PTStatic   itsStartCP;
    PTStatic   itsEndCP;
    PTStatic   itsDurationCP;

    TTime      itsStart;
    TTime      itsEnd;
    BOOL       itsGotMessage;
    BOOL       itsTipWinState;
};


#endif
