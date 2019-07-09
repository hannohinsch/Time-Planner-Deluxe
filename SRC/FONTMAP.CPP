//*****************************************************************************

//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "fontmap.h"

#define SAFE_OBJECT_DELETE( h ) if(h) DeleteObject(h);

HANDLE TFontMap::hSmallScrFont= NULL;
HANDLE TFontMap::hSmallStrikeScrFont= NULL;
HANDLE TFontMap::hNormalScrFont= NULL;
HANDLE TFontMap::hBoldScrFont= NULL;
HANDLE TFontMap::hStrikeScrFont= NULL;
HANDLE TFontMap::hItalicScrFont= NULL;
HANDLE TFontMap::hBigScrFont= NULL;
HANDLE TFontMap::hBigBoldScrFont= NULL;
HANDLE TFontMap::hBigStrikeScrFont= NULL;

HANDLE TFontMap::hSmallPrtFont= NULL;
HANDLE TFontMap::hSmallStrikePrtFont= NULL;
HANDLE TFontMap::hNormalPrtFont= NULL;
HANDLE TFontMap::hBoldPrtFont= NULL;
HANDLE TFontMap::hStrikePrtFont= NULL;
HANDLE TFontMap::hItalicPrtFont= NULL;
HANDLE TFontMap::hBigPrtFont= NULL;
HANDLE TFontMap::hBigBoldPrtFont= NULL;
HANDLE TFontMap::hBigStrikePrtFont= NULL;

//*****************************************************************************

//*****************************************************************************
TFontMap::TFontMap( void )
{
itsErrorFlag= FALSE;

CreateFonts();

//actually, Windows won't start us unless there's 10k free GDI heap,
//which is plenty for us.  Therefore, I don't think we can ever get this
//error.
if( itsErrorFlag )
    {
    char *buf= "Font initialization failed.\nExit; then restart Windows";
    MessageBox( NULL, buf, "Font Creation Error", MB_ICONHAND | MB_SYSTEMMODAL );
    }
}

//*****************************************************************************

//*****************************************************************************
TFontMap::~TFontMap()
{
DeleteFonts();
}

//*****************************************************************************

//*****************************************************************************
void TFontMap::CreateFonts()
{
LOGFONT logFont;
HFONT hFont;
HDC hdc;
int pixelsPerInch, pointSize;

hdc= GetDC( NULL );     //screen device
pixelsPerInch= GetDeviceCaps( hdc, LOGPIXELSY );
ReleaseDC( NULL, hdc );

//attributes for all sizes...
char *scrFontName= "MS Sans Serif";
memset( &logFont, 0,  sizeof( logFont ));
logFont.lfQuality= PROOF_QUALITY;
logFont.lfPitchAndFamily= VARIABLE_PITCH | FF_SWISS;
strcpy( (char *) &logFont.lfFaceName[0], scrFontName );

pointSize= 8;
logFont.lfHeight= -1 * ( pointSize * pixelsPerInch + 36 ) / 72;
hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hNormalScrFont= hFont;

logFont.lfStrikeOut= TRUE;
hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hStrikeScrFont= hFont;
logFont.lfStrikeOut= FALSE;

logFont.lfWeight= FW_BOLD;
hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hBoldScrFont= hFont;
logFont.lfWeight= FW_NORMAL;


logFont.lfItalic= TRUE;
hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hItalicScrFont= hFont;
logFont.lfItalic= FALSE;

pointSize= 10;
logFont.lfHeight= -1 * ( pointSize * pixelsPerInch + 36 ) / 72;

logFont.lfWeight= FW_BOLD;
hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hBigBoldScrFont= hFont;
logFont.lfWeight= FW_NORMAL;

hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hBigScrFont= hFont;

logFont.lfStrikeOut= TRUE;
hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hBigStrikeScrFont= hFont;
logFont.lfStrikeOut= FALSE;


pointSize= 6;
logFont.lfHeight= -1 * ( pointSize * pixelsPerInch + 36 ) / 72;

//
// For 6 pt type, we need "smalle"; MS Sans Serif doesn't come that small
// and Arial looks crummy
//
strcpy( (char *) &logFont.lfFaceName[0], "smalle" );

logFont.lfWeight= FW_NORMAL;
hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hSmallScrFont= hFont;

logFont.lfStrikeOut= TRUE;
hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hSmallStrikeScrFont= hFont;
logFont.lfStrikeOut= FALSE;


////////////////// PRINTER FONTS ///////////////////////////////////////
//NOTE does size math based on screen device!
// This way, scaling works reasonabley well, because we use
// same ratio of cell size to font size as on screen

hdc= GetDC( NULL );
pixelsPerInch= GetDeviceCaps( hdc, LOGPIXELSY );
ReleaseDC( NULL, hdc );

//attributes for all sizes...
char *prtFontName= "Times New Roman";
memset( &logFont, 0,  sizeof( logFont ));
logFont.lfQuality= PROOF_QUALITY;
logFont.lfPitchAndFamily= VARIABLE_PITCH | FF_ROMAN;
strcpy( (char *) &logFont.lfFaceName[0], prtFontName );

pointSize= 8;
logFont.lfHeight= -1 * ( pointSize * pixelsPerInch + 36 ) / 72;
hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hNormalPrtFont= hFont;

logFont.lfStrikeOut= TRUE;
hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hStrikePrtFont= hFont;
logFont.lfStrikeOut= FALSE;

logFont.lfWeight= FW_BOLD;
hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hBoldPrtFont= hFont;
logFont.lfWeight= FW_NORMAL;


logFont.lfItalic= TRUE;
hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hItalicPrtFont= hFont;
logFont.lfItalic= FALSE;

pointSize= 10;
logFont.lfHeight= -1 * ( pointSize * pixelsPerInch + 36 ) / 72;

logFont.lfWeight= FW_BOLD;
hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hBigBoldPrtFont= hFont;
logFont.lfWeight= FW_NORMAL;

hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hBigPrtFont= hFont;

logFont.lfStrikeOut= TRUE;
hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hBigStrikePrtFont= hFont;
logFont.lfStrikeOut= FALSE;



pointSize= 6;
logFont.lfHeight= -1 * ( pointSize * pixelsPerInch + 36 ) / 72;
logFont.lfWeight= FW_NORMAL;
hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hSmallPrtFont= hFont;

logFont.lfStrikeOut= TRUE;
hFont= CreateFontIndirect( &logFont );
if( !hFont )
    {
    hFont= GetStockObject( SYSTEM_FONT );
    itsErrorFlag= TRUE;
    }
hSmallStrikePrtFont= hFont;
logFont.lfStrikeOut= FALSE;

};

//*****************************************************************************

//*****************************************************************************
void TFontMap::DeleteFonts()
{

SAFE_OBJECT_DELETE( hSmallScrFont )
SAFE_OBJECT_DELETE( hSmallStrikeScrFont )

SAFE_OBJECT_DELETE( hNormalScrFont )
SAFE_OBJECT_DELETE( hItalicScrFont )
SAFE_OBJECT_DELETE( hBoldScrFont )
SAFE_OBJECT_DELETE( hStrikeScrFont )
SAFE_OBJECT_DELETE( hBigBoldScrFont )
SAFE_OBJECT_DELETE( hBigScrFont )
SAFE_OBJECT_DELETE( hBigStrikeScrFont )

SAFE_OBJECT_DELETE( hSmallPrtFont )
SAFE_OBJECT_DELETE( hSmallStrikePrtFont )
SAFE_OBJECT_DELETE( hNormalPrtFont )
SAFE_OBJECT_DELETE( hItalicPrtFont )
SAFE_OBJECT_DELETE( hStrikePrtFont )
SAFE_OBJECT_DELETE( hBoldPrtFont )
SAFE_OBJECT_DELETE( hBigBoldPrtFont )
SAFE_OBJECT_DELETE( hBigPrtFont )
SAFE_OBJECT_DELETE( hBigStrikePrtFont )
};


//*****************************************************************************

//*****************************************************************************
HANDLE TFontMap::GetScreenFont( TFont font )
{
HANDLE h;

switch( font )
    {
    case kBoldFont:
        h= hBoldScrFont;
        break;

    case kItalicFont:
        h= hItalicScrFont;
        break;

    case kBigFont:
        h= hBigScrFont;
        break;

    case kBigBoldFont:
        h= hBigBoldScrFont;
        break;

    case kBigStrikeFont:
        h= hBigStrikeScrFont;
        break;

    case kSmallFont:
        h= hSmallScrFont;
        break;

    case kSmallStrikeFont:
        h= hSmallStrikeScrFont;
        break;

    case kStrikeFont:
        h= hStrikeScrFont;
        break;

    case kNormalFont:
    default:
        h= hNormalScrFont;
        break;
    }

return h;
}

//*****************************************************************************

//*****************************************************************************
HANDLE TFontMap::GetPrintFont( TFont font )
{
HANDLE h;

switch( font )
    {
    case kBoldFont:
        h= hBoldPrtFont;
        break;

    case kItalicFont:
        h= hItalicPrtFont;
        break;

    case kBigFont:
        h= hBigPrtFont;
        break;

    case kBigBoldFont:
        h= hBigBoldPrtFont;
        break;

    case kBigStrikeFont:
        h= hBigStrikePrtFont;
        break;

    case kSmallFont:
        h= hSmallPrtFont;
        break;

    case kSmallStrikeFont:
        h= hSmallStrikePrtFont;
        break;

    case kStrikeFont:
        h= hStrikePrtFont;
        break;

    case kNormalFont:
    default:
        h= hNormalPrtFont;
        break;
    }

return h;
}

//*****************************************************************************
// return height of font
// FIXME if this is called often, use cache to store sizes for
// screen and printer DCs
//*****************************************************************************
WORD TFontMap::GetFontHeight( HDC hdc, TFont font )
{
TEXTMETRIC tm;
HANDLE hFont= NULL;

if( DT_RASDISPLAY == GetDeviceCaps( hdc, TECHNOLOGY ) )
    hFont= GetScreenFont( font );
else
    hFont= GetPrintFont( font );


HANDLE hOldPaintFont= SelectObject( hdc, hFont );

GetTextMetrics( hdc, &tm );

SelectObject( hdc, hOldPaintFont );

return (tm.tmHeight + tm.tmExternalLeading);
}

