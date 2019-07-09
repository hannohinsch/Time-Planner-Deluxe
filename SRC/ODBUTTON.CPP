//*****************************************************************************
// our owner-draw buttons
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "gdimap.h"
#include "odbutton.h"
#ifndef __btnutil__
#include "btnutil.h"
#endif
#include "odbutton.h"

#define ODB_AUTOCHECK       1
#define ODB_ENABLEFOCUS     2

//FIXME belongs in either utility.cpp or gdimap.cpp	(?)
void DrawBitMap( HDC hdc, HBITMAP hBitmap, short xStart, short yStart );

//*****************************************************************************
// from Petzold
//*****************************************************************************
void DrawBitMap( HDC hdc, HBITMAP hBitmap, short xStart, short yStart )
{
BITMAP bm;
HDC hdcMem;
POINT ptSize, ptOrg;

hdcMem= CreateCompatibleDC( hdc );
SelectObject( hdcMem, hBitmap );
SetMapMode( hdcMem, GetMapMode( hdc ) );

GetObject( hBitmap, sizeof( BITMAP ), (LPSTR) &bm );

ptSize.x= bm.bmWidth;
ptSize.y= bm.bmHeight;
DPtoLP( hdc, &ptSize, 1 );

ptOrg.x= 0;
ptOrg.y= 0;
DPtoLP( hdcMem, &ptOrg, 1 );

BitBlt( hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem,
        ptOrg.x, ptOrg.y, SRCCOPY );

DeleteDC( hdcMem );
}

//*****************************************************************************

//*****************************************************************************
TODButton::TODButton( PTWindowsObject aParent, int anId,
    int x, int y, int w, int h, PTModule aModule ):
    TButton( aParent, anId, "", x, y, w, h, FALSE, aModule )
{
Attr.Style |= BS_OWNERDRAW;
itsCheckState= BF_UNCHECKED;
}


//*****************************************************************************

//*****************************************************************************
TODButton::TODButton(
    PTWindowsObject aParent, int anId, PTModule aModule ):
    TButton( aParent, anId, aModule )
{
Attr.Style |= BS_OWNERDRAW;
itsCheckState= BF_UNCHECKED;
}

//*****************************************************************************

//*****************************************************************************
TODButton::~TODButton()
{
}


//*****************************************************************************
// Windows does not define an owner-draw radio button,
// so we fake it
// NB we don't have groups, nor auto radio buttons here!
//*****************************************************************************
void TODButton::SetCheck( WORD checkFlag )
{
if( itsCheckState != checkFlag )
    {
    itsCheckState= checkFlag;
    InvalidateRect( HWindow, NULL, FALSE );
    }
}

//*****************************************************************************

//*****************************************************************************
WORD TODButton::GetCheck( void )
{
return itsCheckState;
}


//*****************************************************************************

//*****************************************************************************
void TODButton::ODADrawEntire( DRAWITEMSTRUCT &di )
{
DrawButton( di );
}

//*****************************************************************************

//*****************************************************************************
void TODButton::ODASelect( DRAWITEMSTRUCT &di )
{
DrawButton( di );
}

//*****************************************************************************

//*****************************************************************************
void TODButton::ODAFocus( DRAWITEMSTRUCT &di )
{
DrawButton( di );
}

// ===================================================
//
//                   TODBitMapButton
//
// ===================================================

//*****************************************************************************

//*****************************************************************************
TODBitMapButton::TODBitMapButton(
    PTWindowsObject aParent, int anId,
    int x, int y, int w, int h, PTModule aModule ):
    TODButton( aParent, anId, x, y, w, h, aModule )
{
itsID= anId;
}

//*****************************************************************************

//*****************************************************************************
TODBitMapButton::TODBitMapButton(
    PTWindowsObject aParent, int anId, PTModule aModule ):
    TODButton( aParent, anId, aModule )
{
itsID= anId;
}

//*****************************************************************************

//*****************************************************************************
TODBitMapButton::~TODBitMapButton()
{
}

//*****************************************************************************

//*****************************************************************************
void TODBitMapButton::DrawButton( DRAWITEMSTRUCT &di )
{
HANDLE hBitMap= NULL;

HINSTANCE hInstance= GetApplicationObject()->hInstance;

if( BF_CHECKED == itsCheckState )
    hBitMap= LoadBitmap( hInstance, MAKEINTRESOURCE( VGA_HI+itsID ));
else
    hBitMap= LoadBitmap( hInstance, MAKEINTRESOURCE( VGA_LO+itsID ));

if( hBitMap )
    {
    DrawBitMap( di.hDC, hBitMap, di.rcItem.left + 1, di.rcItem.top + 1 );
    DeleteObject( hBitMap );
    }

//draw selection rect
if( di.itemState & ODS_SELECTED )
    FrameRect( di.hDC, &di.rcItem, TGDIMap::GetScreenBrush( kHbrSelect ) );
else if( di.itemState & ODS_FOCUS )
    FrameRect( di.hDC, &di.rcItem, TGDIMap::GetScreenBrush( kHbrFocus ) );
else
    FrameRect( di.hDC, &di.rcItem, TGDIMap::GetScreenBrush( kHbrBkBar) );

}

// ===================================================
//
//                   TODColorBitMapButton
//
// ===================================================

//*****************************************************************************

//*****************************************************************************
TODColorBitMapButton::TODColorBitMapButton(
    PTWindowsObject aParent, int anId,
    TVirtColor color, PTModule aModule ):
    TODBitMapButton( aParent, anId, aModule )
{
itsColor= color;
}

//*****************************************************************************

//*****************************************************************************
TODColorBitMapButton::~TODColorBitMapButton()
{
}

//*****************************************************************************

//*****************************************************************************
void TODColorBitMapButton::DrawButton( DRAWITEMSTRUCT &di )
{
// the button consists of a gray square with an interior color patch
// of the selected color.  When checked, the border "depresses" and
// the color patch is shifted 2 pixels down and left.

//it's easier to do the all the border stuff as a bitmap, so that's how it's
// done

HINSTANCE hInstance= GetApplicationObject()->hInstance;
HANDLE hBitMap= NULL;

TColor color;
color.Set( itsColor );

RECT rPatch= di.rcItem;
InflateRect( &rPatch, -3, -3 );


if(  di.itemState & ODS_SELECTED )
    {
    hBitMap= LoadBitmap( hInstance, MAKEINTRESOURCE( BMP_HICOLOR ));
    OffsetRect( &rPatch, 1, 1 );
    }
else
    {
    hBitMap= LoadBitmap( hInstance, MAKEINTRESOURCE( BMP_LOCOLOR ));
    }

//draw bitmap
if( hBitMap )
    {
    DrawBitMap( di.hDC, hBitMap, di.rcItem.left + 1, di.rcItem.top + 1 );
    DeleteObject( hBitMap );
    }

//draw color patch
FillRect( di.hDC, &rPatch, TGDIMap::GetScreenBrush( color ) );

//draw selection rect
if( di.itemState & ODS_SELECTED )
    FrameRect( di.hDC, &di.rcItem, TGDIMap::GetScreenBrush( kHbrSelect ) );
else if( di.itemState & ODS_FOCUS )
	FrameRect( di.hDC, &di.rcItem, TGDIMap::GetScreenBrush( kHbrFocus ) );
else
    FrameRect( di.hDC, &di.rcItem, TGDIMap::GetScreenBrush( kHbrBkBar ) );
}

// ===================================================
//
//                   TODTextButton
//
// ===================================================


//*****************************************************************************

//*****************************************************************************
TODTextButton::TODTextButton(
    PTWindowsObject aParent, int anId, char *text,
    int x, int y, int w, int h, PTModule aModule ):
    TODButton( aParent, anId, x, y, w, h, aModule )
{
SetCaption( text );
}

//*****************************************************************************

//*****************************************************************************
TODTextButton::TODTextButton( PTWindowsObject aParent, int anId,
              PTModule aModule ):
    TODButton( aParent, anId, aModule )
{
}

//*****************************************************************************

//*****************************************************************************
TODTextButton::~TODTextButton()
{
}

//*****************************************************************************

//*****************************************************************************
void TODTextButton::DrawButton( DRAWITEMSTRUCT &di )
{
    DrawTextButton( di.hDC, Title, 0, 0, Attr.W, Attr.H,
        (BF_CHECKED == itsCheckState) );
}

//*****************************************************************************

//*****************************************************************************
void TODTextButton::ODASelect( DRAWITEMSTRUCT &di )
{
SetCheck( itsCheckState == BF_CHECKED ?
   BF_UNCHECKED : BF_CHECKED );
DrawButton( di );
}
