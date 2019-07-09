//*****************************************************************************
// little control to show note shape and color
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "paint.h"
#include "markctl.h"

//*****************************************************************************

//*****************************************************************************
void TMarkCtl::TMarkCtl( PTWindowsObject aParent, WORD anID ):
                TControl( aParent, anID )
{
itsColor.Set( kGray );           //medium priority
itsStyle= kItemStyle;
}

//*****************************************************************************

//*****************************************************************************
void TMarkCtl::~TMarkCtl( )
{
}

//*****************************************************************************

//*****************************************************************************
void TMarkCtl::Set( RTColor color, TNoteStyle style )
{
itsColor= color;
itsStyle= style;
if( IsWindow( HWindow ) )
    InvalidateRect( HWindow, NULL, TRUE );
}

//*****************************************************************************

//*****************************************************************************
void TMarkCtl::Paint( HDC hdc, PAINTSTRUCT & )
{
RECT r;
GetClientRect( HWindow, &r );
TPaint::DrawScreenMark( hdc, r, itsColor, itsStyle );
}

//*****************************************************************************

//*****************************************************************************
void TMarkCtl::GetWindowClass( WNDCLASS & AWndClass )
{
TControl::GetWindowClass( AWndClass );

AWndClass.hbrBackground= TGDIMap::GetScreenBrush( kHbrBkCtl );
}

