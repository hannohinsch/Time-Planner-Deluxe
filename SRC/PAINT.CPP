//*****************************************************************************
// routines that paint stuff in a grid cell
// includes variant objects for views, ctls, printer
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "paint.h"

#ifndef __profile__
#include "profile.h"
#endif
#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __fontmap__
#include "fontmap.h"
#endif
#ifndef __basegrid__
#include "basegrid.h"
#endif
#ifndef __utility__
#include "utility.h"
#endif
#ifndef __print__
#include "print.h"
#endif

//
// structure for DrawPatternRect printer escape
//
struct PRECT_STRUCT     //def seems to have been left out of windows.h!
    {
    POINT ptPosition;
    POINT ptSize;
    WORD  wStyle;
    WORD  wPattern;
    };

//*****************************************************************************

//*****************************************************************************
void TPaint::TPaint( void )
{
itsOwnDC= NULL;
itsLastFont= (TFont) -1;
}

//*****************************************************************************

//*****************************************************************************
void TPaint::TPaint( HDC hDC )
{
itsOwnDC= hDC;
itsLastFont= (TFont) -1;
}

//*****************************************************************************

//*****************************************************************************
void TPaint::~TPaint( )
{
}

//*****************************************************************************
// Realize() -- set brush origin to top-left of cell, so hatched areas will
// look OK after scrolling.
//*****************************************************************************
void TPaint::Realize( RTDrawInfo di, HBRUSH hbr )
{
// assumption: brush is not currently selected into any DC

UnrealizeObject( hbr );

//set brush org to upper left corner of cell
POINT xy;
xy.y= di.cellRect.top;
xy.x= di.cellRect.left;

LPtoDP( di.hdc, &xy, 1 );
ClientToScreen( di.hwnd, &xy );

#if 0
WORD xy.x = (8 - xy.x % 8) % 8;
WORD xy.y = (8 - xy.y % 8) % 8;
#endif

SetBrushOrg( di.hdc, xy.x, xy.y );
}

// ---------------------------------------------------
//  Text() (view style)
//
//	Draw text inside a rectangle, hilighting the text
//  as specified by the parameters.
// ---------------------------------------------------
void TPaint::Text( RTDrawInfo di, const char *buf, THilite hilite,
                   WORD style, TFont font, RECT *drawRect )
{
    RECT    r;
    if( drawRect == NULL )      //if no explicit drawRect
        r = di.cellRect;
    else
        r= *drawRect;

    //set font
    HANDLE hOldFont= SelectFont( di.hdc, font );

    //figure out how much of string fits into width
    // to avoid chopping characters

    int     stringLength= strlen( (char *) buf );

TextSub( di, r, (char *) buf, stringLength, NULL, 0, hilite, style );

if( hOldFont )
    SelectObject( di.hdc, hOldFont );
}

//*****************************************************************************
// draw left and right justified text
//*****************************************************************************
void TPaint::TextLR( RTDrawInfo di, const char *lBuf, const char *rBuf,
                     THilite hilite, WORD style, TFont font, RECT *drawRect )
{
RECT    r;
if( drawRect == NULL )      //if no explicit drawRect
    r = di.cellRect;
else
    r= *drawRect;

style &= ~DT_LEFT;
style &= ~DT_RIGHT;
style &= ~DT_CENTER;

//set font
HANDLE hOldFont= SelectFont( di.hdc, font );

TextSub( di, r, (char *) lBuf, strlen( lBuf ),
                (char *) rBuf, strlen( rBuf ),
                 hilite, style );

if( hOldFont )
    SelectObject( di.hdc, hOldFont );
}

//*****************************************************************************
// our basic text function
//*****************************************************************************
void TPaint::OurDrawText( HDC hdc, char *buf, int n, RECT *pR, WORD style )
{
int border= max( 1, min( 3, (pR->right - pR->left)/ 15) );

WORD alignStyle= 0;
WORD x= pR->left;
WORD y= pR->top;

//horizontal alignment
if( style & DT_RIGHT )
    {
    alignStyle |= TA_RIGHT;
    x= pR->right - border;
    }
else if( style & DT_CENTER )
    {
    alignStyle |= TA_CENTER;
    x= pR->left + (pR->right - pR->left)/2;
    }
else
    {
    alignStyle |= TA_LEFT;
    x= pR->left + border;
    }

if( style & DT_SINGLELINE )
    {
    //chars on single line ( whole words )
    char *p= buf;
    WORD lineLength;
    CharBreaker( hdc, p, lineLength, pR->right - pR->left - 2 * border );
    n= min( n, lineLength );

    //vertical alignment
    if( style & DT_TOP )
        {
        alignStyle |= TA_TOP;
        y= pR->top;
        }
    else if( style & DT_VCENTER )
        {
        DWORD hw= GetTextExtent( hdc, buf, n );
        WORD h= HIWORD( hw );
        y= pR->top + (pR->bottom - pR->top)/2 - h/2;
        alignStyle |= TA_TOP;
        }
    else
        {
        alignStyle |= TA_BOTTOM;
        y= pR->bottom;
        }

    //write it
    WORD old= SetTextAlign( hdc, alignStyle );
    ExtTextOut( hdc, x, y, ETO_CLIPPED, pR, buf, n, NULL );
    SetTextAlign( hdc, old );
    }
else  //multi-line (always top aligned)
    {
    //line height
    TEXTMETRIC tm;
    GetTextMetrics( hdc, &tm );
    WORD textHeight=  tm.tmHeight + tm.tmExternalLeading;

    WORD oldAlign= SetTextAlign( hdc, alignStyle | TA_TOP );

    char *pEOL = (char *) buf;
    char *pSOL = (char *) buf;
    BOOL rc= TRUE;
    while( rc )
        {
        WORD lineLength;
        rc= LineBreaker( hdc, pEOL, lineLength, pR->right - pR->left - 2 * border );

        ExtTextOut( hdc, x, y, ETO_CLIPPED, pR, pSOL, lineLength, NULL );
        //move to next line
        y+= textHeight;
        pSOL= pEOL;
        }

    SetTextAlign( hdc, oldAlign );
    }
}

//*****************************************************************************

//*****************************************************************************
void TPaint::TextSub( RTDrawInfo di, RECT &r,
                      const char *lBuf, int nLeft,
                      const char *rBuf, int nRight,
                      THilite hilite, WORD style )
{
BOOL bLR= (rBuf != NULL );

if( bLR )
    {
    OurDrawText( di.hdc, (char *) lBuf, nLeft, &r, style | DT_LEFT );
    OurDrawText( di.hdc, (char *) rBuf, nRight, &r, style | DT_RIGHT );
    }
else
    {
    OurDrawText( di.hdc, (char *) lBuf, nLeft, &r, style );
    }

Hilite( di, hilite );

}

//*****************************************************************************

//*****************************************************************************
void TPaint::Hilite( RTDrawInfo di, THilite hilite )
{
switch( hilite )
    {
    case kFrameHilite:
        FrameRect( di.hdc, &di.cellRect, TGDIMap::GetScreenBrush( kHbrSelect ) );
        break;

    case kRangeHilite:
    case kInvertHilite:
        InvertRect( di.hdc, &di.cellRect );
        break;

    case kNoHilite:
    default:
        break;

    }
}

// ---------------------------------------------------
//	DrawFilledRect
//
//	Fill the rectangle r using the brush.
// ---------------------------------------------------
void TPaint::FillRectangle( RTDrawInfo di, RECT &rect, TLogBrush brush )
{
HBRUSH hBrush= TGDIMap::GetBrush( di, brush );
Realize( di, hBrush );
FillRect( di.hdc, &rect, hBrush );
}

// ---------------------------------------------------
//	DrawFilledRect
//
//	Fill the rectangle r using the brush.
// ---------------------------------------------------
void TPaint::FillRectangle( RTDrawInfo di, RECT &rect, HBRUSH brush )
{
FillRect( di.hdc, &rect, brush );
}

// ---------------------------------------------------
//	DrawFilledRect
//
//	Fill the rectangle r using the brush.
// ---------------------------------------------------
void TPaint::FillRectangle( RTDrawInfo di, RECT &r, TColor color )
{
if( kRed == color.Get() && TProfile::colorSet == kLCDColors )
    { //kRed, the highest priority, is actually white on LCDs, requires border
    HDC hdc= di.hdc;

    HPEN hOldPen= SelectObject( hdc, TGDIMap::GetScreenPen( kMarkBorderPen ) );
    HBRUSH hOldBrush= SelectObject( hdc, TGDIMap::GetScreenBrush( color ) );

    Rectangle( hdc, r.left, r.top, r.right, r.bottom );

    SelectObject( hdc, hOldPen );
    SelectObject( hdc, hOldBrush );
    }
else
    {
    HBRUSH hBrush= TGDIMap::GetScreenBrush( color );
    FillRect( di.hdc, &r, hBrush );
    }
}

// ---------------------------------------------------
//	FolderTab
//
//	Fill the rectangle r using the brush.
// ---------------------------------------------------
void TPaint::FolderTab( RTDrawInfo di, BOOL placeHolder, RTSafStr text )
{
    RECT  r = di.cellRect;

    if (placeHolder)
        {
        r.top  = di.cellRect.bottom - 6;

        FillRectangle( di, r, kHbrFolderTab );

        HPEN oldPen= SelectObject( di.hdc,
            TGDIMap::GetScreenPen( kFolderTabPen ));
        MoveTo( di.hdc, r.left, r.top );
        LineTo( di.hdc, r.right, r.top );
        MoveTo( di.hdc, r.left, r.bottom );
        LineTo( di.hdc, r.right, r.bottom );
        SelectObject( di.hdc, oldPen );
        }
    else 
        {
        //describes folder tab, in relation to start point
        const kNumCurvePoints= 44;
        POINT curve[ kNumCurvePoints ]=
        {
        {0, 0},
        {0,-6},
        {4, 0},
        {0, -1},
        {1, 0},
        {1, -1},
        {0, -1},
        {1, -1},
        {0, -1},
        {1, -1},
        {0, -1},
        {1, -1},
        {0, -1},
        {1, -1},
        {0, -1},
        {1, -1},
        {0, -1},
        {1, -1},
        {0, -1},
        {1, -1},
        {1, 0},
        {1, -1},
        {0,0},          //calculated specially!
        {1,1},
        {1,0},
        {1, 1},
        {0, 1},
        {1, 1},
        {0, 1},
        {1, 1},
        {0, 1},
        {1, 1},
        {0, 1},
        {1, 1},
        {0, 1},
        {1, 1},
        {0, 1},
        {1, 1},
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 1},
        {4, 0},
        {0, 6}
        };

        //give the curve absolute positions
        curve[0].x= r.left - 1;     //start on gridline!
        curve[0].y= r.bottom;
        for( int i=1; i < kNumCurvePoints; i++ )
            {
            if( i == 22 )   //jump across cell
                {
                int x= curve[ i - 1 ].x;
                curve[i].x= r.right - ( x - r.left ) - 1;
                curve[i].y= curve[ i - 1 ].y;
                }
            else
                {
                curve[i].x += curve[ i - 1 ].x;
                curve[i].y += curve[ i - 1 ].y;
                }
            }

        // draw the polygon with outline
        HPEN oldPen= SelectObject( di.hdc,
            TGDIMap::GetScreenPen( kFolderTabPen ));

        HBRUSH hFolder= TGDIMap::GetScreenBrush( kHbrFolderTab );
        Realize( di, hFolder );
        HBRUSH hOldBrush= SelectObject( di.hdc, hFolder );

        Polygon( di.hdc, curve, kNumCurvePoints );

        SelectObject( di.hdc, oldPen );

        // set the font and mode
        SelectFont( di.hdc, kBoldFont );
        WORD oldAlign= SetTextAlign( di.hdc,
            TA_LEFT | TA_BOTTOM );
        SetBkMode( di.hdc, TRANSPARENT );

        // calculate the position of the text
        int width = LOWORD( GetTextExtent( di.hdc,
            SAF2CP( text ), text.Len()));
        int tabWidth = r.right - r.left - 20;
        int x, y;
        if (width < tabWidth)
            {
            // center
            x = r.left + 10 + (tabWidth - width) / 2;
            y = r.bottom - 4;
            }
        else
            {
            // left-justify
            x = r.left + 10;
            y = r.bottom - 4;
            }

        // put the text on
        InflateRect( &r, -10, 0 );
        ExtTextOut( di.hdc, x, y, ETO_CLIPPED, &r,
            SAF2CP( text ), text.Len(), NULL );

        // reset the modes
        SetBkMode( di.hdc, OPAQUE );
        SelectFont( di.hdc, kNormalFont );
        SetTextAlign( di.hdc, oldAlign );
        SelectObject( di.hdc, hOldBrush );

    }
}


// ---------------------------------------------------
//	FrameRectangle
//
//	Draw the rectangle frame without filling it
// ---------------------------------------------------
void TPaint::FrameRectangle( RTDrawInfo di, RECT &r, TColor color )
{
    HDC hdc= di.hdc;

    HPEN hOldPen= SelectObject( hdc, TGDIMap::GetScreenPen( kMarkBorderPen ) );
    HBRUSH hOldBrush= SelectObject( hdc, TGDIMap::GetScreenBrush( kHbrBkView ) );

    MoveTo( hdc, r.left, r.top  );
    LineTo( hdc, r.right, r.top );
    MoveTo( hdc, r.left, r.bottom - 1 );
    LineTo( hdc, r.right, r.bottom - 1 );

    SelectObject( hdc, hOldPen );
    SelectObject( hdc, hOldBrush );
}


// ---------------------------------------------------
//	DrawDiamond
//
//	Draw a diamond within the rectangle
// ---------------------------------------------------
void TPaint::FillDiamond( RTDrawInfo di, RECT &rect, TColor color )
{
POINT  points[ 4 ];
RECT   r    = rect;

// make sure the rectangle is an even number of pixels across and down
if (((r.right - r.left) % 2) == 1)
    {
    r.right--;
    }

if (((r.bottom - r.top) % 2) == 1)
    {
    r.bottom--;
    }

int    midX = (r.left + r.right) / 2;
int    midY = (r.top + r.bottom) / 2;

points[0].x= midX;
points[0].y= r.top;

points[1].x= r.right;
points[1].y= midY;

points[2].x= midX;
points[2].y= r.bottom;

points[3].x= r.left;
points[3].y= midY;

HBRUSH oldBrush = SelectObject( di.hdc, TGDIMap::GetScreenBrush( color ) );
HPEN oldPen= SelectObject( di.hdc, TGDIMap::GetScreenPen( kMarkBorderPen ) );

Polygon( di.hdc, points, 4 );

SelectObject(di.hdc, oldPen );
SelectObject(di.hdc, oldBrush);
}

//*****************************************************************************
// break line into whole words that fit width
// on exit, p points to beginning of next line
//*****************************************************************************
BOOL TPaint::LineBreaker( HDC hdc, char * &p, WORD &n, WORD maxWidth )
{
int wordCount= 0;
char *pLastWord;
char *pLineStart= p;

//number of characters in line
n= 0;

//move to possible word break
while( *p )
    {
    pLastWord= p;

    // all characters > 32 stay together
    while( *p > (char) 32 )
        {
        p++;
        n++;
        }

    //measure text so far this line
    DWORD extent= GetTextExtent( hdc, pLineStart, n );
    if( LOWORD( extent ) > maxWidth )
        {
        if( wordCount > 0 )
            {
            p= pLastWord;                   //point to line break
            n= (int)( p - pLineStart );     //back up char cnt
            return TRUE;                    //indicate there is more
            }
        else
            {  //problem: first word is longer than entire line!
               // so move char by char, break arbitrarily
            int accum= 0;
            for( int i=0; accum < maxWidth; i++ )
                {
                accum= LOWORD( GetTextExtent( hdc, pLastWord, i ) );
                }

            p= pLastWord + i - 1;
            n= (int)( p - pLineStart );     //back up char cnt
            return TRUE;
            }
        }

    //break on embedded \n or cr-lf
    if( *p == (char) 13 || *p == (char) 10 )
        {
        p++;    //point past it

        // cr may be followed by lf
        if( *p == (char) 10 )
            p++;

        //is there another line?
        return (*p != NULL);
        }

    //move past word break character (unless it's the end of string)
    if( *p == '\0' )
        break;
    else
        {
        p++;
        n++;
        wordCount++;
        }
    }

return FALSE;       //no more text left
}

//*****************************************************************************
// figure out how many characters of string *p fit into line of
// length maxWidth, and return TRUE if not all of them fit.  On exit
// p points to next char.
//*****************************************************************************
BOOL TPaint::CharBreaker( HDC hdc, char * &p, WORD &n, WORD maxWidth )
{
char *pLineStart= p;

//number of characters in line
n= 0;

//move to possible word break
while( *p )
    {
    //measure text so far this line
    DWORD extent= GetTextExtent( hdc, pLineStart, n );
    if( LOWORD( extent ) > maxWidth )
        {
        p= p--;                         //point to line break
        n= (int)( p - pLineStart );     //back up char cnt
        return TRUE;                    //indicate there is more
        }

    //break on embedded \n or cr-lf
    if( *p == (char) 13 || *p == (char) 10 )
        {
        p++;    //point past it

        // cr may be followed by lf
        if( *p == (char) 10 )
            p++;

        //is there another line?
        return (*p != '\0');
        }
    else if( *p == '\0' )
        break;
    else
        {
        p++;
        n++;
        }
    }

return FALSE;       //no more text left
}

//*****************************************************************************
// how high would this text be if drawn by Text()?  (Multiline)
//*****************************************************************************
int TPaint::CalcHeight( const char *buf, int width, TFont font, int defHeight )
{
int h= 0;
RECT r;
HDC hdc= GimmeDC();

HANDLE hOldFont= SelectFont( hdc, font );

SetRect( &r, 0, 0, width, defHeight );
int border= max( 1, min( 3, (r.right - r.left)/ 15) );
r.left+= border;
r.right-= border;

//count # of lines
char *p = (char *) buf;
WORD len;       //unused
WORD lines = 1;
while( LineBreaker( hdc, p, len, r.right - r.left ) )
    lines++;

TEXTMETRIC tm;
GetTextMetrics( hdc, &tm );
WORD textHeight=  tm.tmHeight + tm.tmExternalLeading;

h= lines * textHeight;

SelectObject( hdc, hOldFont );

TrashDC( hdc );

return h;
}


//*****************************************************************************
// how wide would this text be if drawn by Text()?   (singleline)
//*****************************************************************************
int TPaint::CalcWidth( const char *buf, TFont font )
{
HDC hdc= GimmeDC();
HANDLE hOldFont= SelectFont( hdc, font );
int width = LOWORD( GetTextExtent( hdc, buf, strlen( buf )));
SelectObject( hdc, hOldFont );
TrashDC( hdc );

return (width);
}


//*****************************************************************************

//*****************************************************************************
void TPaint::GridLine( RTDrawInfo di, RECT &r )
{
HBRUSH oldBrush = SelectObject( di.hdc, TGDIMap::GetScreenBrush( kHbrGrid ));

PatBlt(di.hdc, r.left, r.top,
    r.right - r.left, r.bottom - r.top, PATCOPY);

SelectObject(di.hdc, oldBrush);
}


//*****************************************************************************
// selects SCREEN font (overloaded by printer painters, of course)
//*****************************************************************************
HFONT TPaint::SelectFont( HDC hdc, TFont font )
{
HFONT hOldFont= NULL;

// don't reload if it's already in this dc
if( itsOwnDC == NULL || font != itsLastFont )
    {
    hOldFont= SelectObject( hdc, TFontMap::GetScreenFont( font ) );
    SetTextCharacterExtra( hdc, 1 );

    itsLastFont= font;
    }

return hOldFont;
}

//*****************************************************************************
// screen and print painters use different techniques here
//*****************************************************************************
HDC TPaint::GimmeDC( void )
{
if( itsOwnDC )
    return itsOwnDC;              //use DC from constructor
else
    return GetDC( NULL );       //get screen device DC
}

//*****************************************************************************

//*****************************************************************************
void TPaint::TrashDC( HDC hdc )
{
if( !itsOwnDC )
    ReleaseDC( NULL, hdc );
}

//*****************************************************************************
// draw note mark on screen
// NB also used in noteview and timepicker, which don't have TPaint objects
//*****************************************************************************
void TPaint::DrawScreenMark( HDC hdc, RECT &r, TColor color, TNoteStyle style )
{
HPEN hOldPen= SelectObject( hdc, TGDIMap::GetScreenPen( kMarkBorderPen ) );
HBRUSH hOldBrush= SelectObject( hdc, TGDIMap::GetScreenBrush( color ) );

switch( style )
    {
    case kTaskStyle:
        {
        r.top+= 2;
        r.left+= 2;
        r.bottom= r.top + 14;
        r.right= r.left + 14;
        Rectangle( hdc, r.left, r.top, r.right, r.bottom );
        }
        break;

    case kMilestoneStyle:
        // this is not the same style as the diamond
        // used for timelines and calendars
        {
        POINT points[ 4 ];

        points[0].x= r.left + 1 + 8;
        points[0].y= r.top + 1;

        points[1].x= r.left + 1 + 16;
        points[1].y= r.top + 1 + 8;

        points[2].x= r.left + 1 + 8;
        points[2].y= r.top + 1 + 16;

        points[3].x= r.left + 1;
        points[3].y= r.top + 1 + 8;

        Polygon( hdc, points, 4 );
        }
        break;

    case kItemStyle:
        {
        r.top+=2;
        r.left+=2;
        r.right= r.left + 15;
        r.bottom= r.top + 15;

        Ellipse( hdc, r.left, r.top, r.right, r.bottom );
        }
        break;

    case kFloatStyle:

        r.top    += 2;
        r.left   += 2;
        r.bottom  = r.top + 14;
        r.right   = r.left + 14;
        Rectangle( hdc, r.left, r.top, r.right, r.bottom );

        //add inner square, same color as view background
        //FIXME should pick up window background brush ( to which we
        // don't have access )
        // so maybe create mark without overwriting background
        InflateRect( &r, -3, -3 );
        SelectObject( hdc, TGDIMap::GetScreenBrush( kHbrBkView ));
        Rectangle( hdc, r.left, r.top, r.right, r.bottom );
        break;

    default:
        break;
    }

SelectObject( hdc, hOldPen );
SelectObject( hdc, hOldBrush );
}

//*****************************************************************************

//*****************************************************************************
void TPaint::DrawMark( RTDrawInfo di, RTNote note )
{
TPaint::DrawScreenMark( di.hdc, di.cellRect, note.GetColor(), note.GetStyle() );
}

//////////////////////// PRINTVIEW //////////////////////////////////////////

//
//  Although one can theoretically use the same GDI calls to draw on both
//  the screen and the printer, in fact things work MUCH better (faster)
//  if one takes advantage of the DRAWPATTERNRECT printer escape available
//  on most (all?) laser printers.  So, all the draw functions have their
//  screen and printer variants. -hmh
//

//*****************************************************************************
// draw gridline, using appropriate method
//*****************************************************************************
void TPaintPrt::GridLine( RTDrawInfo di, RECT &rect )
{
int escType= DRAWPATTERNRECT;

if( !di.IsColor() && Escape( di.hdc, QUERYESCSUPPORT, sizeof(int), (const char *) &escType, NULL ) )
    {
    //supports DRAWPATTERNRECT

    PRECT_STRUCT pattern;
    RECT r;

    //set r to clipped rect
    IntersectRect( &r, &rect, &di.updRect );

    //adjust r for scaling, origin
    LPtoDP( di.hdc, (LPPOINT) &r, 2 );

    pattern.ptPosition.x= r.left;
    pattern.ptPosition.y= r.top;
    pattern.ptSize.x= r.right - r.left;
    pattern.ptSize.y= r.bottom - r.top;
    pattern.wStyle= 0;      //black rule
    pattern.wPattern= 0;

    Escape( di.hdc, DRAWPATTERNRECT, sizeof(PRECT_STRUCT),
           (const char *) &pattern, NULL );

    }
else if( RC_BITBLT & GetDeviceCaps( di.hdc, RASTERCAPS ) )
    {
    //supports patblt
    HBRUSH oldBrush = SelectObject( di.hdc, TGDIMap::GetBrush( di, kHbrGrid ));

    PatBlt(di.hdc, rect.left, rect.top,
        rect.right - rect.left, rect.bottom - rect.top, BLACKNESS);

    SelectObject(di.hdc, oldBrush);
    }
else
    {
    //use Rectangle()
    HBRUSH oldBrush = SelectObject( di.hdc, TGDIMap::GetBrush( di, kHbrGrid ));
    HPEN oldPen= SelectObject( di.hdc, GetStockObject( BLACK_PEN ));  //FIXME

    Rectangle(di.hdc, rect.left, rect.top, rect.right, rect.bottom );

    SelectObject(di.hdc, oldPen );
    SelectObject(di.hdc, oldBrush);
    }

}

//*****************************************************************************

//*****************************************************************************
void TPaintPrt::TextSub( RTDrawInfo di, RECT &r,
                      const char *lBuf, int nLeft,
                      const char *rBuf, int nRight,
                      THilite hilite, WORD style )
{
TGDIMap::SetColorEnvironment( di.hdc, kEnvView );
TPaint::TextSub( di, r, lBuf, nLeft, rBuf, nRight, hilite, style );
}

//*****************************************************************************

//*****************************************************************************
void TPaintPrt::DrawMark( RTDrawInfo di, RTNote note )
{
RECT r= di.cellRect;
HDC hdc= di.hdc;

HPEN hOldPen= SelectObject( hdc, TGDIMap::GetPen( di, kMarkBorderPen ) );
HBRUSH hOldBrush= SelectObject( hdc, TGDIMap::GetBrush( di, note.GetColor() ) );

switch( note.GetStyle() )
    {
    case kTaskStyle:
        {
        r.top+= 2;
        r.left+= 2;
        r.bottom= r.top + 14;
        r.right= r.left + 14;
        Rectangle( hdc, r.left, r.top, r.right, r.bottom );
        }
        break;

    case kMilestoneStyle:
        // this is not the same style as the diamond
        // used for timelines and calendars
        {
        POINT points[ 4 ];

        points[0].x= r.left + 1 + 8;
        points[0].y= r.top + 1;

        points[1].x= r.left + 1 + 16;
        points[1].y= r.top + 1 + 8;

        points[2].x= r.left + 1 + 8;
        points[2].y= r.top + 1 + 16;

        points[3].x= r.left + 1;
        points[3].y= r.top + 1 + 8;

        Polygon( hdc, points, 4 );
        }
        break;

    case kItemStyle:
        {
        r.top+=2;
        r.left+=2;
        r.right= r.left + 15;
        r.bottom= r.top + 15;

        Ellipse( hdc, r.left, r.top, r.right, r.bottom );
        }
        break;

    case kFloatStyle:

        r.top    += 2;
        r.left   += 2;
        r.bottom  = r.top + 14;
        r.right   = r.left + 14;
        Rectangle( hdc, r.left, r.top, r.right, r.bottom );

        //add inner square, same color as view background
        InflateRect( &r, -3, -3 );
        SelectObject( hdc, TGDIMap::GetBrush( di, kHbrBkView ) );
        Rectangle( hdc, r.left, r.top, r.right, r.bottom );
        break;

    default:
        break;
    }

SelectObject( hdc, hOldPen );
SelectObject( hdc, hOldBrush );
}


// ---------------------------------------------------
//	FolderTab
//
//	Fill the rectangle r using the brush.
// ---------------------------------------------------
void TPaintPrt::FolderTab( RTDrawInfo di, BOOL placeHolder, RTSafStr text )
{
    RECT  r = di.cellRect;

    if (placeHolder)
        {
        r.top  = di.cellRect.bottom - 6;

        //
        //HBRUSH oldBrush= SelectObject( di.hdc,
        //    TGDIMap::GetBrush( di, kHbrFolderTab ));

        FillRectangle( di, r, kHbrFolderTab );

        //SelectObject( di.hdc, oldBrush );

        HPEN oldPen= SelectObject( di.hdc,
            TGDIMap::GetPen( di, kFolderTabPen ));
        MoveTo( di.hdc, r.left, r.top );
        LineTo( di.hdc, r.right, r.top );
        MoveTo( di.hdc, r.left, r.bottom );
        LineTo( di.hdc, r.right, r.bottom );
        SelectObject( di.hdc, oldPen );
        }
    else 
        {
        //describes folder tab, in relation to start point
        const kNumCurvePoints= 44;
        POINT curve[ kNumCurvePoints ]=
        {
        {0, 0},
        {0,-6},
        {4, 0},
        {0, -1},
        {1, 0},
        {1, -1},
        {0, -1},
        {1, -1},
        {0, -1},
        {1, -1},
        {0, -1},
        {1, -1},
        {0, -1},
        {1, -1},
        {0, -1},
        {1, -1},
        {0, -1},
        {1, -1},
        {0, -1},
        {1, -1},
        {1, 0},
        {1, -1},
        {0,0},          //calculated specially!
        {1,1},
        {1,0},
        {1, 1},
        {0, 1},
        {1, 1},
        {0, 1},
        {1, 1},
        {0, 1},
        {1, 1},
        {0, 1},
        {1, 1},
        {0, 1},
        {1, 1},
        {0, 1},
        {1, 1},
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 1},
        {4, 0},
        {0, 6}
        };

        //give the curve absolute positions
        curve[0].x= r.left - 1;     //start on gridline!
        curve[0].y= r.bottom;
        for( int i=1; i < kNumCurvePoints; i++ )
            {
            if( i == 22 )   //jump across cell
                {
                int x= curve[ i - 1 ].x;
                curve[i].x= r.right - ( x - r.left ) - 1;
                curve[i].y= curve[ i - 1 ].y;
                }
            else
                {
                curve[i].x += curve[ i - 1 ].x;
                curve[i].y += curve[ i - 1 ].y;
                }
            }

        // draw the polygon
        HPEN oldPen= SelectObject( di.hdc,
            TGDIMap::GetPen( di, kFolderTabPen ));
        HBRUSH hOldBrush= SelectObject( di.hdc,
            TGDIMap::GetBrush( di, kHbrFolderTab ));

        Polygon( di.hdc, curve, kNumCurvePoints );

        SelectObject( di.hdc, oldPen );

        // set the fontand mode
        SelectFont( di.hdc, kBoldFont );
        WORD oldAlign= SetTextAlign( di.hdc,
            TA_LEFT | TA_BOTTOM );
        SetBkMode( di.hdc, TRANSPARENT );

        // calculate the position of the text
        int width = LOWORD( GetTextExtent( di.hdc,
            SAF2CP( text ), text.Len()));
        int tabWidth = r.right - r.left - 20;
        int x, y;
        if (width < tabWidth)
            {
            // center
            x = r.left + 10 + (tabWidth - width) / 2;
            y = r.bottom - 4;
            }
        else
            {
            // left-justify
            x = r.left + 10;
            y = r.bottom - 4;
            }

        // put the text on
        InflateRect( &r, -10, 0 );
        ExtTextOut( di.hdc, x, y, ETO_CLIPPED, &r,
            SAF2CP( text ), text.Len(), NULL );

        // reset the modes
        SetBkMode( di.hdc, OPAQUE );
        SelectFont( di.hdc, kNormalFont );
        SetTextAlign( di.hdc, oldAlign );
        SelectObject( di.hdc, hOldBrush );
    }
}

//*****************************************************************************

//*****************************************************************************
void TPaintPrt::Hilite( RTDrawInfo, THilite )
{
//don't draw hilites on printed reports!
}


// ---------------------------------------------------
//	DrawFilledRect
//
//	Fill the rectangle r using the brush.
// ---------------------------------------------------
void TPaintPrt::FillRectangle( RTDrawInfo di, RECT &rect, TLogBrush brush )
{
const escType= DRAWPATTERNRECT;

//use DrawPatternRect escape and HP gray scales, if possible
int hpGray= TGDIMap::GetHPGray( brush );

if( !di.IsColor() && hpGray > 0 && Escape( di.hdc, QUERYESCSUPPORT, sizeof(int), (const char *) &escType, NULL ) )
    {
    //supports DRAWPATTERNRECT

    PRECT_STRUCT pattern;
    RECT r;

    //set r to clipped rect
    IntersectRect( &r, &rect, &di.updRect );

    //adjust r for scaling, origin
    LPtoDP( di.hdc, (LPPOINT) &r, 2 );

    pattern.ptPosition.x= r.left;
    pattern.ptPosition.y= r.top;
    pattern.ptSize.x= r.right - r.left;
    pattern.ptSize.y= r.bottom - r.top;
    pattern.wStyle= 2;      //gray scale
    pattern.wPattern= hpGray;

    Escape( di.hdc, DRAWPATTERNRECT, sizeof(PRECT_STRUCT),
           (const char *) &pattern, NULL );

    }
else
    {
    FillRect( di.hdc, &rect, TGDIMap::GetBrush( di, brush ) );
    }


}

// ---------------------------------------------------
//	DrawFilledRect
//
//  Fill the rectangle r using the color.
//  on the printer, we have a border
// ---------------------------------------------------
void TPaintPrt::FillRectangle( RTDrawInfo di, RECT &rect, TColor color )
{
const escType= DRAWPATTERNRECT;

//use DrawPatternRect escape and HP gray scales, if possible
int hpGray= TGDIMap::GetHPGray( color );

if( !di.IsColor() && hpGray > 0 && Escape( di.hdc, QUERYESCSUPPORT, sizeof(int), (const char *) &escType, NULL ) )
    {
    //supports DRAWPATTERNRECT

    PRECT_STRUCT pattern;
    RECT r;

    //set r to clipped rect
    IntersectRect( &r, &rect, &di.updRect );

    //adjust r for scaling, origin
    LPtoDP( di.hdc, (LPPOINT) &r, 2 );

    pattern.ptPosition.x= r.left;
    pattern.ptPosition.y= r.top;
    pattern.ptSize.x= r.right - r.left;
    pattern.ptSize.y= r.bottom - r.top;
    pattern.wStyle= 2;      //gray scale
    pattern.wPattern= hpGray;

    Escape( di.hdc, DRAWPATTERNRECT, sizeof(PRECT_STRUCT),
           (const char *) &pattern, NULL );

    //
    //now draw the 4 sides of rectangle ( 1 pixel border )
    //
    pattern.wStyle= 0;          //black rule
    pattern.wPattern= 0;        //ignored

    //top
    pattern.ptPosition.x= r.left;
    pattern.ptPosition.y= r.top;
    pattern.ptSize.x= r.right - r.left;
    pattern.ptSize.y= 1;
    Escape( di.hdc, DRAWPATTERNRECT, sizeof(PRECT_STRUCT),
           (const char *) &pattern, NULL );

    //left
    pattern.ptPosition.x= r.left;
    pattern.ptPosition.y= r.top;
    pattern.ptSize.x= 1;
    pattern.ptSize.y= r.bottom - r.top;
    Escape( di.hdc, DRAWPATTERNRECT, sizeof(PRECT_STRUCT),
           (const char *) &pattern, NULL );

    //bottom
    pattern.ptPosition.x= r.left;
    pattern.ptPosition.y= r.bottom;
    pattern.ptSize.x= r.right - r.left;
    pattern.ptSize.y= 1;
    Escape( di.hdc, DRAWPATTERNRECT, sizeof(PRECT_STRUCT),
           (const char *) &pattern, NULL );

    //right
    pattern.ptPosition.x= r.right;
    pattern.ptPosition.y= r.top;
    pattern.ptSize.x= 1;
    pattern.ptSize.y= r.bottom - r.top;
    Escape( di.hdc, DRAWPATTERNRECT, sizeof(PRECT_STRUCT),
           (const char *) &pattern, NULL );

    }
else
    {
    HDC hdc= di.hdc;

    HPEN hOldPen= SelectObject( hdc, TGDIMap::GetPen( di, kMarkBorderPen ) );
    HBRUSH hOldBrush= SelectObject( hdc, TGDIMap::GetBrush( di, color ) );

    Rectangle( hdc, rect.left, rect.top, rect.right, rect.bottom );

    SelectObject( hdc, hOldPen );
    SelectObject( hdc, hOldBrush );
    }
}

// ---------------------------------------------------
//	FrameRectangle
//
//  Draw the frame of the rectangle without filling it
// ---------------------------------------------------
void TPaintPrt::FrameRectangle( RTDrawInfo di, RECT &rect, TColor color )
{
const escType= DRAWPATTERNRECT;

//use DrawPatternRect escape and HP gray scales, if possible
int hpGray= TGDIMap::GetHPGray( color );

if( !di.IsColor() && hpGray > 0 && Escape( di.hdc, QUERYESCSUPPORT, sizeof(int), (const char *) &escType, NULL ) )
    {
    //supports DRAWPATTERNRECT

    PRECT_STRUCT pattern;
    RECT r;

    //set r to clipped rect
    IntersectRect( &r, &rect, &di.updRect );

    //adjust r for scaling, origin
    LPtoDP( di.hdc, (LPPOINT) &r, 2 );

    pattern.ptPosition.x= r.left;
    pattern.ptPosition.y= r.top;
    pattern.ptSize.x= r.right - r.left;
    pattern.ptSize.y= r.bottom - r.top;
    pattern.wStyle= 2;      //gray scale
    pattern.wPattern= hpGray;

    // don't draw the fill pattern
    // Escape( di.hdc, DRAWPATTERNRECT, sizeof(PRECT_STRUCT),
    //      (const char *) &pattern, NULL );

    //
    //now draw the 4 sides of rectangle ( 1 pixel border )
    //
    pattern.wStyle= 0;          //black rule
    pattern.wPattern= 0;        //ignored

    //top
    pattern.ptPosition.x= r.left;
    pattern.ptPosition.y= r.top;
    pattern.ptSize.x= r.right - r.left;
    pattern.ptSize.y= 1;
    Escape( di.hdc, DRAWPATTERNRECT, sizeof(PRECT_STRUCT),
           (const char *) &pattern, NULL );

    //bottom
    pattern.ptPosition.x= r.left;
    pattern.ptPosition.y= r.bottom;
    pattern.ptSize.x= r.right - r.left;
    pattern.ptSize.y= 1;
    Escape( di.hdc, DRAWPATTERNRECT, sizeof(PRECT_STRUCT),
           (const char *) &pattern, NULL );


    }
else
    {
    HDC hdc= di.hdc;

    HPEN hOldPen= SelectObject( hdc, TGDIMap::GetPen( di, kMarkBorderPen ) );
    HBRUSH hOldBrush= SelectObject( hdc, TGDIMap::GetBrush( di, kHbrBkView ) );

    MoveTo( hdc, rect.left, rect.top  );
    LineTo( hdc, rect.right, rect.top );
    MoveTo( hdc, rect.left, rect.bottom );
    LineTo( hdc, rect.right, rect.bottom );

    SelectObject( hdc, hOldPen );
    SelectObject( hdc, hOldBrush );
    }
}

// ---------------------------------------------------
//	DrawDiamond
//
//	Draw a diamond within the rectangle
// ---------------------------------------------------
void TPaintPrt::FillDiamond( RTDrawInfo di, RECT &rect, TColor color )
{
POINT  points[ 4 ];
RECT   r    = rect;

// make sure the rectangle is an even number of pixels across and down
if (((r.right - r.left) % 2) == 1)
    {
    r.right--;
    }

if (((r.bottom - r.top) % 2) == 1)
    {
    r.bottom--;
    }

int    midX = (r.left + r.right) / 2;
int    midY = (r.top + r.bottom) / 2;

points[0].x= midX;
points[0].y= r.top;

points[1].x= r.right;
points[1].y= midY;

points[2].x= midX;
points[2].y= r.bottom;

points[3].x= r.left;
points[3].y= midY;

HBRUSH oldBrush = SelectObject( di.hdc, TGDIMap::GetBrush( di, color ) );
HPEN oldPen= SelectObject( di.hdc, GetStockObject( BLACK_PEN ));  //FIXME

Polygon( di.hdc, points, 4 );

SelectObject(di.hdc, oldPen );
SelectObject(di.hdc, oldBrush);
}


//*****************************************************************************
// how high would this text be if drawn by Text()?  (Multiline)
//*****************************************************************************
int TPaintPrt::CalcHeight( const char *buf, int width, TFont font, int defHeight )
{
return TPaint::CalcHeight( buf, width, font, defHeight );
}


//*****************************************************************************
// how wide would this text be if drawn by Text()?   (singleline)
//*****************************************************************************
int TPaintPrt::CalcWidth( const char *buf, TFont font )
{
return TPaint::CalcWidth( buf, font );
}

//*****************************************************************************
// selects PRINTER font
// NB assumes that all font changes to print dc are made through this function!
//*****************************************************************************
HFONT TPaintPrt::SelectFont( HDC hdc, TFont font )
{
HFONT hOldFont= NULL;

// don't reload if it's already in this dc
if( TRUE || font != itsLastFont )       //FIXME always load font; dc fonts get trashed elsewhere!
    {
    hOldFont= SelectObject( hdc, TFontMap::GetPrintFont( font ) );
    SetTextCharacterExtra( hdc, 1 );

    itsLastFont= font;
    }

return hOldFont;
}

//*****************************************************************************
// screen and print painters use different techniques for getting a DC
//*****************************************************************************
HDC TPaintPrt::GimmeDC( void )
{
return TPrint::hPrintDC;
}

//*****************************************************************************

//*****************************************************************************
void TPaintPrt::TrashDC( HDC )
{
//do nothing; prt dc is global
}

/////////////////////////////////// PAINT 3D //////////////////////////////

//*****************************************************************************
// draw text in 3D cell
//*****************************************************************************
void TPaint3D::TextSub( RTDrawInfo di, RECT &r,
                      const char *lBuf, int nLeft,
                      const char *rBuf, int nRight,
                      THilite hilite, WORD style )
{

if( hilite == kInvertHilite )
    {
    FillRect( di.hdc, &di.cellRect, TGDIMap::GetScreenBrush( kHbrBk3DSel ));

    // use black pen top and left
    HANDLE hOldPen= SelectObject( di.hdc, TGDIMap::GetScreenPen( kHi3DSelPen ) );
    MoveTo( di.hdc, di.cellRect.left, di.cellRect.bottom-2 );
    LineTo( di.hdc, di.cellRect.left, di.cellRect.top );
    LineTo( di.hdc, di.cellRect.right-1, di.cellRect.top );

    //change to ltgray pen bottom and right
    SelectObject( di.hdc, TGDIMap::GetScreenPen( kLo3DSelPen ) );
    LineTo( di.hdc, di.cellRect.right-1, di.cellRect.bottom-1 );
    LineTo( di.hdc, di.cellRect.left, di.cellRect.bottom-1 );

    SelectObject( di.hdc, hOldPen );

    TGDIMap::SetColorEnvironment( di.hdc, kEnv3DSel );
    }
else
    {

    FillRect( di.hdc, &di.cellRect, TGDIMap::GetScreenBrush( kHbrBk3D ));

    //use white pen top and left
    HANDLE hOldPen= SelectObject( di.hdc, TGDIMap::GetScreenPen( kHi3DPen ) );
    MoveTo( di.hdc, di.cellRect.left, di.cellRect.bottom-2 );
    LineTo( di.hdc, di.cellRect.left, di.cellRect.top );
    LineTo( di.hdc, di.cellRect.right-1, di.cellRect.top );

    //change to dkgray pen bottom and right
    SelectObject( di.hdc, TGDIMap::GetScreenPen( kLo3DPen ) );
    LineTo( di.hdc, di.cellRect.right-1, di.cellRect.bottom-1 );
    LineTo( di.hdc, di.cellRect.left, di.cellRect.bottom-1 );

    SelectObject( di.hdc, hOldPen );

    TGDIMap::SetColorEnvironment( di.hdc, kEnv3D );
    }

//don't overwrite hilite lines
r.top++;
r.left++;
r.bottom--;
r.right--;

//don't pass 'hilite' arg because we did it in here (above)
TPaint::TextSub( di, r, lBuf, nLeft, rBuf, nRight, kNoHilite, style );
}

