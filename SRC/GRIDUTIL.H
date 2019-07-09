#ifndef __gridutil__
#define __gridutil__

_CLASSDEF( TGridUtil )
_CLASSDEF( TNote )

#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __basegrid__
#include "basegrid.h"
#endif

class _CLASSTYPE TGridUtil {

    public:

    static void PaintBackground( RTDrawInfo di,
        RTTime day, BOOL weekInOneCell = TRUE,
        BOOL checkTime = TRUE );
    static void PaintBackground( RTDrawInfo di, int dow );
    static void NotePopup( RTTime time );
    static void ShowNote( PTNote pNote );
    static void OurTrackPopupMenu(HMENU hMenu, UINT fuFlags, int x, int y, int reserved,
                                      HWND hWnd, const RECT* lprc );

    static void DrawMarquee(  PTWindowsObject parent, RECT &r, BOOL on );
};


#endif
