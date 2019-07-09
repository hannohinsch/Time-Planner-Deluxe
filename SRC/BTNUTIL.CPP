//*****************************************************************************
// our square button draw routines
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "btnutil.h"
#include "gdimap.h"

void DrawTextButton( HDC dc, char *text, int x, int y, int w, int h, BOOL hilite )
{
    RECT r;
    RECT drawR;

    SetRect( &r, x, y, x + w, y + h );

    if( hilite )
        {

        //outer border
        FrameRect( dc, &r, TGDIMap::GetScreenBrush( kHbrSelect ));
        InflateRect( &r, -1, -1 );

        //shadow
        SetRect( &drawR, r.left, r.top, r.left + 2, r.bottom );
        FillRect( dc, &drawR, TGDIMap::GetScreenBrush( kHbrShadowed ));

        SetRect( &drawR, r.left, r.top, r.right, r.top + 2 );
        FillRect( dc, &drawR, TGDIMap::GetScreenBrush( kHbrShadowed ));

        //inner area
        SetRect( &drawR, r.left + 2, r.top + 2, r.right, r.bottom );
        FillRect( dc, &drawR, TGDIMap::GetScreenBrush( kHbrFlat ));

        TGDIMap::SetColorEnvironment( dc, kEnvLightScale );
        SetRect( &drawR, r.left + 2, r.top + 2, r.right, r.bottom );
        DrawText( dc, text, strlen( text ),
            &drawR, DT_CENTER | DT_VCENTER | DT_SINGLELINE );

        }
    else
        {

        //outer border
        FrameRect( dc, &r, TGDIMap::GetScreenBrush( kHbrSelect ));
        InflateRect( &r, -1, -1 );

        //hilite
        SetRect( &drawR, r.left, r.top, r.right, r.top + 2 );
        FillRect( dc, &drawR, TGDIMap::GetScreenBrush( kHbrLit ));

        SetRect( &drawR, r.left, r.top, r.left + 2, r.bottom );
        FillRect( dc, &drawR, TGDIMap::GetScreenBrush( kHbrLit ));

        //shadow
        SetRect( &drawR, r.left + 1, r.bottom - 2, r.right, r.bottom );
        FillRect( dc, &drawR, TGDIMap::GetScreenBrush( kHbrShadowed ));

        SetRect( &drawR, r.left, r.bottom - 1, r.right, r.bottom );
        FillRect( dc, &drawR, TGDIMap::GetScreenBrush( kHbrShadowed ));

        SetRect( &drawR, r.right - 1, r.top, r.right, r.bottom );
        FillRect( dc, &drawR, TGDIMap::GetScreenBrush( kHbrShadowed ));

        SetRect( &drawR, r.right - 2, r.top + 1, r.right, r.bottom );
        FillRect( dc, &drawR, TGDIMap::GetScreenBrush( kHbrShadowed ));


        //inner area
        SetRect( &drawR, r.left + 2, r.top + 2, r.right - 2, r.bottom - 2 );
        FillRect( dc, &drawR, TGDIMap::GetScreenBrush( kHbrFlat ));

        SetRect( &drawR, r.left + 2, r.top + 2, r.right - 2, r.bottom - 2);
        TGDIMap::SetColorEnvironment( dc, kEnvLightScale );
        DrawText( dc, text, strlen( text ),
            &drawR, DT_CENTER | DT_VCENTER | DT_SINGLELINE );

        }
}
