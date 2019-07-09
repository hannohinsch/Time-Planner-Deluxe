#ifndef __markctl__
#define __markctl__

_CLASSDEF( TMarkCtl )

#ifndef __note__
#include "note.h"
#endif
#ifndef __color__
#include "color.h"
#endif

class _CLASSTYPE TMarkCtl: public TControl
{
public:
    TMarkCtl( PTWindowsObject aParent, WORD anID );
    virtual ~TMarkCtl( );

    void Set( RTColor, TNoteStyle );

private:
    LPSTR  GetClassName( void ) { return "MarkCtl";}
    void GetWindowClass( WNDCLASS & AWndClass );
    void Paint( HDC hdc, PAINTSTRUCT &paintInfo );

    TColor itsColor;
    TNoteStyle itsStyle;

};

#endif
