//*****************************************************************************
// provides one level of indirection to brushes, pens, and fonts
// (allows remapping for different devices, preferences)
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "error.h"
#include "gdimap.h"
#include "paint.h"
#include "profile.h"
#include "rawgrid.h"

// rgb colors
#define rgbBlack       RGB(   0,   0,   0 )
#define rgbDkDkGray    RGB(  64,  64,  64 );
#define rgbDkGray      RGB( 127, 127, 127 )
#define rgbLtGray      RGB( 192, 192, 192 )
#define rgbWhite       RGB( 255, 255, 255 )

#define rgbBlue        RGB(   0,   0, 255 )
#define rgbLtBlue      RGB( 192, 255, 255 )
#define rgbRed         RGB( 255,   0,   0 )
#define rgbLtRed       RGB( 255, 192, 192 )
#define rgbDkGreen     RGB(   0, 127,  64 )
#define rgbGreen       RGB(   0, 127,   0 )
#define rgbYellow      RGB( 255, 255,   0 )
#define rgbLtYellow    RGB( 255, 255, 232 )
#define rgbCyan        RGB(   0, 255, 255 )
#define rgbPurple      RGB( 255,   0, 255 )

// static object members
BOOL TGDIMap::itsErrorFlag;

#define SAFE_OBJECT_DELETE( h ) if(h) DeleteObject(h);

// private (base) gdi objects

HPEN TGDIMap::hBlackPen= NULL;
HPEN TGDIMap::hDkGrayPen= NULL;
HPEN TGDIMap::hLtGrayPen= NULL;
HPEN TGDIMap::hWhitePen= NULL;

HBRUSH TGDIMap::hbrRed= NULL;
HBRUSH TGDIMap::hbrLtRed= NULL;
HBRUSH TGDIMap::hbrBlue= NULL;
HBRUSH TGDIMap::hbrLtBlue= NULL;
HBRUSH TGDIMap::hbrYellow= NULL;
HBRUSH TGDIMap::hbrLtYellow= NULL;
HBRUSH TGDIMap::hbrGreen= NULL;
HBRUSH TGDIMap::hbrBlack= NULL;
HBRUSH TGDIMap::hbrDkGray= NULL;
HBRUSH TGDIMap::hbrLtGray= NULL;
HBRUSH TGDIMap::hbrWhite= NULL;
HBRUSH TGDIMap::hbrDkDkGray= NULL;
HBRUSH TGDIMap::hbrCyan= NULL;
HBRUSH TGDIMap::hbrPurple= NULL;

HBRUSH TGDIMap::hbrSteel= NULL;
HBRUSH TGDIMap::hbrHatch= NULL;
HBRUSH TGDIMap::hbrRedHatch= NULL;
HBRUSH TGDIMap::hbrGreenHatch= NULL;
HBRUSH TGDIMap::hbrGrayHatch= NULL;

HBRUSH TGDIMap::hbrBlueGray= NULL;
HBRUSH TGDIMap::hbrRedGray= NULL;
HBRUSH TGDIMap::hbrWhiteGray= NULL;
HBRUSH TGDIMap::hbrBlackGray= NULL;

HBRUSH TGDIMap::hbrPrtPri1= NULL;
HBRUSH TGDIMap::hbrPrtPri2= NULL;
HBRUSH TGDIMap::hbrPrtPri3= NULL;
HBRUSH TGDIMap::hbrPrtPri4= NULL;

HBRUSH TGDIMap::hbrLCDPri1= NULL;
HBRUSH TGDIMap::hbrLCDPri2= NULL;
HBRUSH TGDIMap::hbrLCDPri3= NULL;
HBRUSH TGDIMap::hbrLCDPri4= NULL;

HBRUSH TGDIMap::hbrAppWorkspace= NULL;

//mapping tables ==> be sure to update ALL tables when adding constants <==

HBRUSH *(TGDIMap::screenBrushes[]) =
    {
    &hbrSteel,          //  kHbrBkDlg,             //dialog background
    &hbrWhite,          //  kHbrBkView,            //view
    &hbrLtYellow,       //  kHbrBkFrame,           //frame
    &hbrLtGray,         //  kHbrBkCtl,             //standard windows controls
    &hbrLtGray,         //  kHbrBk3D,              //special 3D control (unselected)
    &hbrDkGray,         //  kHbrBk3DSel,           //3D controls, selected
    &hbrRedHatch,       //  kHbrBkOffDays,         //background for non-work days
    &hbrRedHatch,       //  kHbrBkHoliDays,        //background for non-work days
    &hbrLtGray,         //  kHbrBkBar,             //tool bar background

    //  miscellany
    &hbrLtGray,         //  kHbrGrid,              //grid lines
    &hbrBlack,          //  kHbrSelect,            //frame for selected cells
    &hbrBlack,          //  kHbrBorder,            //border for controls, button bar
    &hbrDkGray,         //  kHbrMarkBorder,        //border for note marks
    &hbrBlack,          //  kHbrCalOverflow,       //overflow line in calendar grids
    &hbrBlack,          //  kHbrTimeFill,          //cell fill in time-picker
    &hbrDkGray,         //  kHbrTimeMark,          //milestone mark in time picker
    &hbrLtGray,         //  kHbrBrowBlob,          //browser hidden categories
    &hbrPurple,         //  kHbrGrafTime,           //graf view, time bar
    &hbrCyan,           //  kHbrGrafCost,           //graf view, cost bar
	&hbrHatch,			//	kHbrFocus,				//outline for controls w/ focus
    &hbrGrayHatch,      //  kHbrMultiCell           //separator for multi-cells in weekgrid
    &hbrDkGray,         //  kHbrShadow              //drop shadow
    &hbrLtBlue,         //  kHbrFolderTab
    &hbrLtBlue,         //  kHbrAddrTitle

    //  note colors (priorities)
    &hbrRed,            //  kHbrRedNote,           //hi priority
    &hbrBlue,           //  kHbrBlueNote,          //lo
    &hbrBlueGray,       //  kHbrBlueGrayNote,         //med-lo
    &hbrRedGray,        //  kHbrRedGrayNote,        //med-hi
    &hbrDkGray,         //  kHbrGrayNote,         //med

    // frequency dialog
    &hbrGreen,            // kHbrFreqFirst
    &hbrBlue,          // kHbrFreqSecond
    &hbrRed,           // kHbrFreqLast

    &hbrLtGray,         // kHbrFlat
    &hbrWhite,          // kHbrLit
    &hbrDkGray          // kHbrShadowed

    };

HBRUSH *(TGDIMap::lcdBrushes[]) =
    {
    &hbrSteel,          //  kHbrBkDlg,             //dialog background
    &hbrWhite,          //  kHbrBkView,            //view
    &hbrLtGray,         //  kHbrBkFrame,           //frame
    &hbrWhite,         //  kHbrBkCtl,             //standard windows controls
    &hbrWhite,          //  kHbrBk3D,              //special 3D control (unselected)
    &hbrBlack,          //  kHbrBk3DSel,           //3D controls, selected
    &hbrRedHatch,       //  kHbrBkOffDays,         //background for non-work days
    &hbrRedHatch,        //  kHbrBkHolidays,        //background for holidays
    &hbrLtGray,         //  kHbrBkBar,              //tool bar background

    //  miscellany
    &hbrLtGray,         //  kHbrGrid,              //grid lines
    &hbrBlack,          //  kHbrSelect,            //frame for selected cells
    &hbrBlack,          //  kHbrBorder,            //border for controls, button bar
    &hbrBlack,          //  kHbrMarkBorder,        //border for note marks
    &hbrBlack,          //  kHbrCalOverflow,       //overflow line in calendar grids
    &hbrBlack,          //  kHbrTimeFill,          //cell fill in time-picker
    &hbrDkGray,         //  kHbrTimeMark,          //milestone mark in time picker
    &hbrLtGray,         //  kHbrBrowBlob,          //browser hidden categories
    &hbrLtGray,           //  kHbrGrafTime,           //graf view, time bar
    &hbrDkGray,         //  kHbrGrafCost,           //graf view, cost bar
	&hbrHatch,			//	kHbrFocus,				//outline for controls w/ focus
    &hbrGrayHatch,          //  kHbrMultiCell           //separator for multi-cells in weekgrid
    &hbrDkGray,         //  kHbrShadow              //drop shadow
    &hbrLtGray,         //  kFolderTab
    &hbrLtGray,         //  kHbrAddrTitle

    //  note colors (priorities)
    &hbrWhite,          //  kHbrRedNote,           //hi priority
    &hbrBlack,          //  kHbrBlueNote,          //lo
    &hbrBlackGray,      //  kHbrBlueGrayNote,         //med-lo
    &hbrWhiteGray,      //  kHbrRedGrayNote,        //med-hi
    &hbrDkGray,         //  kHbrGrayNote,         //med

    //  frequency dialog
    &hbrDkGray,         //  kHbrFreqStart
    &hbrDkGray,         //  kHbrFreqSecond
    &hbrDkGray,         //  kHbrFreqLast

    &hbrLtGray,         // kHbrFlat
    &hbrWhite,          // kHbrLit
    &hbrDkGray          // kHbrShadowed

    };

HBRUSH *(TGDIMap::colorBrushes[]) =           //color print brushes
    {
    &hbrSteel,          //  kHbrBkDlg,             //dialog background
    &hbrWhite,          //  kHbrBkView,            //view
    &hbrLtYellow,       //  kHbrBkFrame,           //frame
    &hbrLtGray,         //  kHbrBkCtl,             //standard windows controls
    &hbrLtGray,         //  kHbrBk3D,              //special 3D control (unselected)
    &hbrDkGray,         //  kHbrBk3DSel,           //3D controls, selected
    &hbrLtRed,       //  kHbrBkOffDays,         //background for non-work days
    &hbrLtRed,       //  kHbrBkHoliDays,        //background for non-work days
    &hbrLtGray,         //  kHbrBkBar,             //tool bar background

    //  miscellany
    &hbrLtGray,         //  kHbrGrid,              //grid lines
    &hbrBlack,          //  kHbrSelect,            //frame for selected cells
    &hbrBlack,          //  kHbrBorder,            //border for controls, button bar
    &hbrDkGray,         //  kHbrMarkBorder,        //border for note marks
    &hbrBlack,          //  kHbrCalOverflow,       //overflow line in calendar grids
    &hbrBlack,          //  kHbrTimeFill,          //cell fill in time-picker
    &hbrDkGray,         //  kHbrTimeMark,          //milestone mark in time picker
    &hbrLtGray,         //  kHbrBrowBlob,          //browser hidden categories
    &hbrPurple,         //  kHbrGrafTime,           //graf view, time bar
    &hbrCyan,           //  kHbrGrafCost,           //graf view, cost bar
	&hbrHatch,			//	kHbrFocus,				//outline for controls w/ focus
    &hbrGrayHatch,      //  kHbrMultiCell           //separator for multi-cells in weekgrid
    &hbrDkGray,         //  kHbrShadow              //drop shadow
    &hbrLtBlue,         //  kHbrFolderTab
    &hbrLtBlue,         //  kHbrAddrTitle

    //  note colors (priorities)
    &hbrRed,            //  kHbrRedNote,           //hi priority
    &hbrBlue,           //  kHbrBlueNote,          //lo
    &hbrLtBlue,       //  kHbrBlueGrayNote,         //med-lo
    &hbrLtRed,        //  kHbrRedGrayNote,        //med-hi
    &hbrDkGray,         //  kHbrGrayNote,         //med

    // frequency dialog
    &hbrGreen,            // kHbrFreqFirst
    &hbrBlue,          // kHbrFreqSecond
    &hbrRed,           // kHbrFreqLast

    &hbrLtGray,         // kHbrFlat
    &hbrWhite,          // kHbrLit
    &hbrDkGray          // kHbrShadowed

    };

//NB actually, many of these brushes are not used in printing
HBRUSH *(TGDIMap::printBrushes[]) =
    {
    &hbrSteel,          //  kHbrBkDlg,             //dialog background
    &hbrWhite,          //  kHbrBkView,            //view
    &hbrLtGray,         //  kHbrBkFrame,           //frame
    &hbrWhite,          //  kHbrBkCtl,             //standard windows controls
    &hbrWhite,          //  kHbrBk3D,              //special 3D control (unselected)
    &hbrBlack,          //  kHbrBk3DSel,           //3D controls, selected
    &hbrPrtPri2,         //  kHbrBkOffDays,         //background for non-work days
    &hbrPrtPri2,         //  kHbrBkHolidays,        //background for holidays
    &hbrLtGray,         //  kHbrBkBar,              //tool bar background

    //  miscellany
    &hbrBlack,          //  kHbrGrid,              //grid lines
    &hbrBlack,          //  kHbrSelect,            //frame for selected cells
    &hbrBlack,          //  kHbrBorder,            //border for controls, button bar
    &hbrBlack,          //  kHbrMarkBorder,        //border for note marks
    &hbrBlack,          //  kHbrCalOverflow,       //overflow line in calendar grids
    &hbrBlack,          //  kHbrTimeFill,          //cell fill in time-picker
    &hbrDkGray,         //  kHbrTimeMark,          //milestone mark in time picker
    &hbrLtGray,         //  kHbrBrowBlob,          //browser hidden categories
    &hbrLtGray,         //  kHbrGrafTime,           //graf view, time bar
    &hbrDkGray,         //  kHbrGrafCost,           //graf view, cost bar
	&hbrHatch,			//	kHbrFocus,				//outline for controls w/ focus
    &hbrBlack,          //  kHbrMultiCell           //separator for multi-cells in weekgrid
    &hbrDkGray,         //  kHbrShadow              //drop shadow
    &hbrPrtPri1,         //  kFolderTab
    &hbrPrtPri1,         //  kHbrAddrTitle

    //  note colors (priorities)
    &hbrPrtPri1,        //  kHbrRedNote,           //hi priority
    &hbrBlack,          //  kHbrBlueNote,          //lo
    &hbrPrtPri4,        //  kHbrBlueGrayNote,         //med-lo
    &hbrPrtPri2,        //  kHbrRedGrayNote,        //med-hi
    &hbrDkGray,         //  kHbrGrayNote,         //med

    //  frequency dialog
    &hbrDkGray,         //  kHbrFreqStart
    &hbrDkGray,         //  kHbrFreqSecond
    &hbrDkGray,         //  kHbrFreqLast

    &hbrLtGray,         // kHbrFlat
    &hbrWhite,          // kHbrLit
    &hbrDkGray          // kHbrShadowed

};

// HP laser printers can print gray-scales much faster than real
// brushes, so this table, which has the same # of elements as the
// other brush tables, specifies a gray scale value to substitute
// for the brush.  Allowable values are 2, 10, 15, 30, 45, 70, 90, 100%.
// A value of -1 means the print routine should not attempt a substitution
// (just use normal print brush)
int TGDIMap::hpGraySubs[] = {
    -1,          //  kHbrBkDlg,             //dialog background
    -1,          //  kHbrBkView,            //view
    -1,         //  kHbrBkFrame,           //frame
    -1,          //  kHbrBkCtl,             //standard windows controls
    -1,          //  kHbrBk3D,              //special 3D control (unselected)
    -1,          //  kHbrBk3DSel,           //3D controls, selected
    2,         //  kHbrBkOffDays,         //background for non-work days
    2,         //  kHbrBkHolidays,        //background for holidays
    -1,         //  kHbrBkBar,              //tool bar background

    //  miscellany
    100,          //  kHbrGrid,              //grid lines
    -1,          //  kHbrSelect,            //frame for selected cells
    -1,          //  kHbrBorder,            //border for controls, button bar
    100,          //  kHbrMarkBorder,        //border for note marks
    100,          //  kHbrCalOverflow,       //overflow line in calendar grids
    -1,          //  kHbrTimeFill,          //cell fill in time-picker
    -1,         //  kHbrTimeMark,          //milestone mark in time picker
    45,         //  kHbrBrowBlob,          //browser hidden categories
    30,         //  kHbrGrafTime,           //graf view, time bar
    70,         //  kHbrGrafCost,           //graf view, cost bar
    -1,          //  kHbrFocus,              //outline for controls w/ focus
    100,          //  kHbrMultiCell           //separator for multi-cells in weekgrid
    -1,         //  kHbrShadow              //drop shadow
    -1,         //  kFolderTab
    2,          //  kHbrAddrTitle

    //  note colors (priorities)
    2,        //  kHbrRedNote,           //hi priority
    100,          //  kHbrBlueNote,          //lo
    70,        //  kHbrBlueGrayNote,         //med-lo
    30,        //  kHbrRedGrayNote,        //med-hi
    45,         //  kHbrGrayNote,         //med

    //  frequency dialog
    -1,         //  kHbrFreqStart
    -1,         //  kHbrFreqSecond
    -1,         //  kHbrFreqLast

    -1,         // kHbrFlat
    -1,          // kHbrLit
    -1          // kHbrShadowed


};

HPEN *( TGDIMap::screenPens[])=
    {
    &hWhitePen,         // kHi3Dpen
    &hDkGrayPen,        // kLo3DPen
    &hBlackPen,         // kHi3DSelPen
    &hLtGrayPen,        // kLo3DSelPen
    &hDkGrayPen,        // kMarkBorderPen
    &hBlackPen,         // kInkPen
    &hLtGrayPen         // kFolderTabPen
    };

HPEN *( TGDIMap::lcdPens[])=
    {
    &hWhitePen,         // kHi3Dpen
    &hBlackPen,         // kLo3DPen
    &hBlackPen,         // kHi3DSelPen
    &hWhitePen,         // kLo3DSelPen
    &hBlackPen,         // kMarkBorderPen
    &hBlackPen,          //kInkPen
    &hDkGrayPen         // kFolderTabPen
    };

HPEN *( TGDIMap::colorPens[])=      //color printer
    {
    &hWhitePen,         // kHi3Dpen
    &hDkGrayPen,        // kLo3DPen
    &hBlackPen,         // kHi3DSelPen
    &hLtGrayPen,        // kLo3DSelPen
    &hDkGrayPen,        // kMarkBorderPen
    &hBlackPen,         // kInkPen
    &hLtGrayPen         // kFolderTabPen
    };

HPEN *( TGDIMap::printPens[])=
    {
    &hWhitePen,         // kHi3Dpen
    &hBlackPen,         // kLo3DPen
    &hBlackPen,         // kHi3DSelPen
    &hWhitePen,         // kLo3DSelPen
    &hBlackPen,         // kMarkBorderPen
    &hBlackPen,          //kInkPen
    &hBlackPen         // kFolderTabPen
    };

COLORREF TGDIMap::screenColor[]=
    {
    rgbBlack,           //    kRgb3DText,
    rgbLtGray,          //    kRgb3DBk,
    rgbWhite,           //    kRgb3DSelText,
    rgbDkGray,          //    kRgb3DSelBk,
    rgbRed,           //    kRgbCalWeekend
    rgbRed,           //    kRgbCalHoliday
    rgbGreen,           //    kRgbFreqStart
    rgbBlue,            //    kRgbFreqSecond
    rgbRed,             //    kRgbFreqLast
    rgbRed            //    kRgbMonth
    };

COLORREF TGDIMap::lcdColor[]=
    {
    rgbBlack,           //    kRgb3DText,
    rgbWhite,           //    kRgb3DBk,
    rgbWhite,           //    kRgb3DSelText,
    rgbBlack,           //    kRgb3DSelBk,
    rgbBlack,           //    kRgbCalWeekend
    rgbBlack,           //    kRgbCalHoliday
    rgbBlack,           //    kRgbFreqStart
    rgbBlack,           //    kRgbFreqSecond
    rgbBlack,           //    kRgbFreqLast
    rgbBlack            //    kRgbMonth
    };

COLORREF TGDIMap::colorColor[]=     //color printer
    {
    rgbBlack,           //    kRgb3DText,
    rgbLtGray,          //    kRgb3DBk,
    rgbWhite,           //    kRgb3DSelText,
    rgbDkGray,          //    kRgb3DSelBk,
    rgbRed,           //    kRgbCalWeekend
    rgbRed,           //    kRgbCalHoliday
    rgbGreen,           //    kRgbFreqStart
    rgbBlue,            //    kRgbFreqSecond
    rgbRed,             //    kRgbFreqLast
    rgbRed            //    kRgbMonth
    };

COLORREF TGDIMap::printColor[]=     //bw printer
    {
    rgbBlack,           //    kRgb3DText,
    rgbWhite,           //    kRgb3DBk,
    rgbWhite,           //    kRgb3DSelText,
    rgbBlack,           //    kRgb3DSelBk,
    rgbBlack,           //    kRgbCalWeekend
    rgbBlack,           //    kRgbCalHoliday
    rgbBlack,           //    kRgbFreqStart
    rgbBlack,           //    kRgbFreqSecond
    rgbBlack,           //    kRgbFreqLast
    rgbBlack            //    kRgbMonth
    };


//*****************************************************************************

//*****************************************************************************
TGDIMap::TGDIMap( void )
{
itsErrorFlag= FALSE;

CreateBrushes();
CreatePatBrushes();
CreatePens();

SetGDIMap( TProfile::colorSet );

//actually, Windows won't start us unless there's 10k free GDI heap,
//which is plenty for us.  Therefore, I don't think we can ever get this
//error.
if( itsErrorFlag )
    {
    char *buf= "Graphics initialization failed.\nExit; then restart Windows";
    MessageBox( NULL, buf, "GDI Creation Error", MB_ICONHAND | MB_SYSTEMMODAL );
    }
}

//*****************************************************************************

//*****************************************************************************
TGDIMap::~TGDIMap()
{
DeleteBrushes();
DeletePatBrushes();
DeletePens();
}

//*****************************************************************************
//
//*****************************************************************************
void TGDIMap::SetGDIMap( TGDISet map )
{
TProfile::colorSet= map;
};

//*****************************************************************************

//*****************************************************************************
void TGDIMap::SysColorChange()
{
//ASSUMES that the brush is not selected into any DC!
DeleteObject( hbrAppWorkspace );
hbrAppWorkspace= CreateSolidBrush( GetSysColor( COLOR_APPWORKSPACE ) );
}

//*****************************************************************************
//FIXME error checks!
//*****************************************************************************
void TGDIMap::CreatePatBrushes( void )
{
HANDLE hBitMap;
HANDLE hInstance= GetApplicationObject()->hInstance;

//good on LCDs; not so good on color
hbrHatch= cregrdbsh();
if( !hbrHatch )
    {
    hbrHatch= GetStockObject( BLACK_BRUSH );
    itsErrorFlag= TRUE;
    }

//the "steel" brush, used as noteview background
hBitMap= LoadBitmap( GetApplicationObject()->hInstance, MAKEINTRESOURCE(BMP_STEEL) );

if( hBitMap )
    hbrSteel= CreatePatternBrush( hBitMap );
else
    itsErrorFlag= TRUE;

if( !hbrSteel )
    {
    hbrSteel= GetStockObject( WHITE_BRUSH );
    itsErrorFlag= TRUE;
    }

if( hBitMap )
    DeleteObject( hBitMap );

hBitMap= LoadBitmap( hInstance, MAKEINTRESOURCE(BMP_BLUEGRAY));
hbrBlueGray= CreatePatternBrush( hBitMap );
DeleteObject( hBitMap );

hBitMap= LoadBitmap( hInstance, MAKEINTRESOURCE(BMP_REDGRAY));
hbrRedGray= CreatePatternBrush( hBitMap );
DeleteObject( hBitMap );

hBitMap= LoadBitmap( hInstance, MAKEINTRESOURCE(BMP_WHITEGRAY));
hbrWhiteGray= CreatePatternBrush( hBitMap );
DeleteObject( hBitMap );

hBitMap= LoadBitmap( hInstance, MAKEINTRESOURCE(BMP_BLACKGRAY));
hbrBlackGray= CreatePatternBrush( hBitMap );
DeleteObject( hBitMap );

hBitMap= LoadBitmap( hInstance, MAKEINTRESOURCE(BMP_PRTPRI1));
hbrPrtPri1= CreatePatternBrush( hBitMap );
DeleteObject( hBitMap );

hBitMap= LoadBitmap( hInstance, MAKEINTRESOURCE(BMP_PRTPRI2));
hbrPrtPri2= CreatePatternBrush( hBitMap );
DeleteObject( hBitMap );

hBitMap= LoadBitmap( hInstance, MAKEINTRESOURCE(BMP_PRTPRI3));
hbrPrtPri3= CreatePatternBrush( hBitMap );
DeleteObject( hBitMap );

hBitMap= LoadBitmap( hInstance, MAKEINTRESOURCE(BMP_PRTPRI4));
hbrPrtPri4= CreatePatternBrush( hBitMap );
DeleteObject( hBitMap );

hBitMap= LoadBitmap( hInstance, MAKEINTRESOURCE(BMP_NONWORKDAY) );
hbrRedHatch= CreatePatternBrush( hBitMap );
DeleteObject( hBitMap );

hBitMap= LoadBitmap( hInstance, MAKEINTRESOURCE(BMP_HOLIDAY) );
hbrGreenHatch= CreatePatternBrush( hBitMap );
DeleteObject( hBitMap );

hBitMap= LoadBitmap( hInstance, MAKEINTRESOURCE(BMP_DRAGSTRIPE) );
hbrGrayHatch= CreatePatternBrush( hBitMap );
DeleteObject( hBitMap );


}

//*****************************************************************************

//*****************************************************************************
void TGDIMap::DeletePatBrushes( void )
{
SAFE_OBJECT_DELETE( hbrSteel )
SAFE_OBJECT_DELETE( hbrHatch )

SAFE_OBJECT_DELETE( hbrBlueGray )
SAFE_OBJECT_DELETE( hbrRedGray )
SAFE_OBJECT_DELETE( hbrWhiteGray )
SAFE_OBJECT_DELETE( hbrBlackGray )

SAFE_OBJECT_DELETE( hbrPrtPri1 )
SAFE_OBJECT_DELETE( hbrPrtPri2 )
SAFE_OBJECT_DELETE( hbrPrtPri3 )
SAFE_OBJECT_DELETE( hbrPrtPri4 )

SAFE_OBJECT_DELETE( hbrRedHatch )
SAFE_OBJECT_DELETE( hbrGrayHatch )
SAFE_OBJECT_DELETE( hbrGreenHatch )
}


//*****************************************************************************

//*****************************************************************************
void TGDIMap::CreateBrushes()
{

hbrAppWorkspace= CreateSolidBrush( GetSysColor( COLOR_APPWORKSPACE ) );

hbrBlack=  GetStockObject( BLACK_BRUSH );
hbrWhite=  GetStockObject( WHITE_BRUSH );
hbrLtGray= GetStockObject( LTGRAY_BRUSH );
hbrDkGray= GetStockObject( GRAY_BRUSH );
hbrDkDkGray= GetStockObject( DKGRAY_BRUSH );

hbrRed= CreateSolidBrush( rgbRed );
if( !hbrRed )
    {
    hbrRed= hbrWhite;
    itsErrorFlag= TRUE;
    }

hbrLtRed= CreateSolidBrush( rgbLtRed );
if( !hbrLtRed )
    {
    hbrLtRed= hbrWhite;
    itsErrorFlag= TRUE;
    }


hbrGreen= CreateSolidBrush( rgbGreen );
if( !hbrGreen )
    {
    hbrGreen= hbrBlack;
    itsErrorFlag= TRUE;
    }

hbrBlue= CreateSolidBrush( rgbBlue );
if( !hbrBlue )
    {
    hbrBlue= hbrBlack;
    itsErrorFlag= TRUE;
    }

hbrLtBlue= CreateSolidBrush( rgbLtBlue );
if( !hbrLtBlue )
    {
    hbrLtBlue= hbrBlack;
    itsErrorFlag= TRUE;
    }

hbrYellow= CreateSolidBrush( rgbYellow );
if( !hbrYellow )
    {
    hbrYellow= hbrWhite;
    itsErrorFlag= TRUE;
    }

hbrLtYellow= CreateSolidBrush( rgbLtYellow );
if( !hbrLtYellow )
    {
    hbrLtYellow= hbrWhite;
    itsErrorFlag= TRUE;
    }

hbrCyan= CreateSolidBrush( rgbCyan );
if( !hbrCyan )
    {
    hbrCyan= hbrLtGray;
    itsErrorFlag= TRUE;
    }

hbrPurple= CreateSolidBrush( rgbPurple );
if( !hbrPurple )
    {
    hbrPurple= hbrDkGray;
    itsErrorFlag= TRUE;
    }



};

//*****************************************************************************

//*****************************************************************************
void TGDIMap::DeleteBrushes()
{
SAFE_OBJECT_DELETE( hbrRed )
SAFE_OBJECT_DELETE( hbrLtRed )
SAFE_OBJECT_DELETE( hbrGreen )
SAFE_OBJECT_DELETE( hbrBlue )
SAFE_OBJECT_DELETE( hbrLtBlue )
SAFE_OBJECT_DELETE( hbrYellow )
SAFE_OBJECT_DELETE( hbrLtYellow )

SAFE_OBJECT_DELETE( hbrCyan )
SAFE_OBJECT_DELETE( hbrPurple )

SAFE_OBJECT_DELETE( hbrAppWorkspace )
};

//*****************************************************************************

//*****************************************************************************
void TGDIMap::CreatePens()
{
//std 1 pixel pens...
hWhitePen  = GetStockObject( WHITE_PEN );
hBlackPen  = GetStockObject( BLACK_PEN );

hLtGrayPen = CreatePen( PS_SOLID, 1, rgbLtGray );
if( !hLtGrayPen )
    {
    hLtGrayPen= hWhitePen;
    itsErrorFlag= TRUE;
    }

hDkGrayPen = CreatePen( PS_SOLID, 1, rgbDkGray );
if( !hDkGrayPen )
    {
    hDkGrayPen= hBlackPen;
    itsErrorFlag= TRUE;
    }

}

//*****************************************************************************

//*****************************************************************************
void TGDIMap::DeletePens()
{
SAFE_OBJECT_DELETE( hLtGrayPen )
SAFE_OBJECT_DELETE( hDkGrayPen )
};



//*****************************************************************************
// cregrdbsh - Create a hatch brush for Excel-style line grids
// [courtesy HJW]
//*****************************************************************************
HBRUSH TGDIMap::cregrdbsh( void )
{
    BITMAP bmp = { 0, 8, 8, 2, 1, 1 };
    LOGBRUSH logbsh = {BS_PATTERN, 0L};
    char bitpat[]  =  {
        0xAA, 0x00, 0x55, 0x00, 0xAA, 0x00, 0x55, 0x00,
        0xAA, 0x00, 0x55, 0x00, 0xAA, 0x00, 0x55, 0x00};

    HBITMAP bmphnd;
    HBRUSH bsh;

    bsh = NULL;
    bmp.bmBits = bitpat;
    bmphnd = CreateBitmapIndirect( &bmp );
    if ( bmphnd )
        {
        logbsh.lbHatch = bmphnd;
        bsh = CreateBrushIndirect( &logbsh );
        itsErrorFlag |= (NULL == bsh);
        DeleteObject( bmphnd );
        }
    else
        itsErrorFlag |= TRUE;

    return bsh;
}/********************** end cregrdbsh **********************/



// ---------------------------------------------------
//	SetColorEnvironment
//
//  Calls the proper windows functions to set up the
//  environment called for. This is different depending
//  on what the system is capable of. Returns a color
//  brush that matches the bakcground.
//
//  An hdc equal to NULL is allowed, in which case
//  the only thing that happens is that the brush is
//  returned.
// ---------------------------------------------------
HANDLE TGDIMap::SetColorEnvironment( HDC hdc, TColorEnv env )
{
HBRUSH hBrush;

    switch( env ) {

        case kEnvLightScale:

            if (hdc != NULL) {

                SetTextColor( hdc, rgbBlack );
                SetBkColor( hdc, rgbLtGray );
            }
            hBrush= hbrLtGray;
            break;

        case kEnvDarkScale:

            if (hdc != NULL) {

                SetTextColor( hdc, rgbWhite );
                SetBkColor( hdc, rgbDkGray );
            }
            hBrush= hbrDkGray;
            break;


        case kEnvView:

            if (hdc != NULL) {

                SetTextColor( hdc, rgbBlack );
                SetBkColor( hdc, rgbWhite );
            }
            hBrush= hbrWhite;
            break;

        case kEnvViewSel:

            if (hdc != NULL) {

                SetTextColor( hdc, rgbWhite );
                SetBkColor( hdc, rgbBlack );
            }
            hBrush= hbrBlack;
            break;

        case kEnv3D:

            if (hdc != NULL) {

                SetTextColor( hdc, GetScreenColor( kRgb3DText ) );
                SetBkColor( hdc, GetScreenColor( kRgb3DBk ) );
            }
            hBrush= GetScreenBrush( kHbrBk3D );
            break;

        case kEnv3DSel:

            if (hdc != NULL) {

                SetTextColor( hdc, GetScreenColor( kRgb3DSelText ) );
                SetBkColor( hdc, GetScreenColor( kRgb3DSelBk ) );
            }
            hBrush= GetScreenBrush( kHbrBk3DSel );
            break;

        case kEnvCalWeekend:        //variation of 3D ctl, used for calendars
            if (hdc != NULL) {

                SetTextColor( hdc, GetScreenColor( kRgbCalWeekend ) );
                SetBkColor( hdc, GetScreenColor( kRgb3DBk ) );
            }
            hBrush= GetScreenBrush( kHbrBk3D );
            break;

        case kEnvCalHoliday:        //variation of 3D ctl, used for calendars
            if (hdc != NULL) {

                SetTextColor( hdc, GetScreenColor( kRgbCalHoliday ) );
                SetBkColor( hdc, GetScreenColor( kRgb3DBk ) );
            }
            hBrush= GetScreenBrush( kHbrBk3D );
            break;

        case kEnvFreqStart:        // frequency dialog start date
            if (hdc != NULL) {

                SetTextColor( hdc, GetScreenColor( kRgb3DSelText ) );
                SetBkColor( hdc, GetScreenColor( kRgbFreqStart ) );
            }
            hBrush= GetScreenBrush( kHbrBk3D );
            break;

        case kEnvFreqSecond:        // frequency dialog second date
            if (hdc != NULL) {

                SetTextColor( hdc, GetScreenColor( kRgb3DSelText ) );
                SetBkColor( hdc, GetScreenColor( kRgbFreqSecond ) );
            }
            hBrush= GetScreenBrush( kHbrBk3D );
            break;

        case kEnvFreqLast:        // frequency dialog second last
            if (hdc != NULL) {

                SetTextColor( hdc, GetScreenColor( kRgb3DSelText ) );
                SetBkColor( hdc, GetScreenColor( kRgbFreqLast ) );
            }
            hBrush= GetScreenBrush( kHbrBk3D );
            break;

        case kEnvMonth:        // month grid: days within the month
            if (hdc != NULL) {

                SetTextColor( hdc, GetScreenColor( kRgbMonth ) );
                SetBkColor( hdc, rgbWhite );
            }
            hBrush= hbrWhite;
            break;

        case kEnvBkCtl:
            // these guys are not subject to color mapping, because
            // they should use same colors as buttons
            // FIXME -- use system button colors!
            if (hdc != NULL ) {

            SetTextColor( hdc, rgbBlack );
            SetBkColor( hdc, rgbLtGray );
            }
            hBrush= hbrLtGray;
            break;

        default:
            SYSERR( ERR_GEN );
            hBrush= NULL;
            break;
    }

return( hBrush );
}

//*****************************************************************************

//*****************************************************************************
HBRUSH TGDIMap::GetScreenBrush( TLogBrush log)
{
if( log < ( sizeof( screenBrushes ) / sizeof( HBRUSH * ) ) )
    switch( TProfile::colorSet )
        {
        case kStdColors:
            return *( screenBrushes[ log ] );

        case kLCDColors:
        default:
            return *( lcdBrushes[ log ] );
        }
else
    SYSERR( ERR_GEN );

return *(screenBrushes[ 0 ]);      //stop compiler warning
}

//*****************************************************************************

//*****************************************************************************
HBRUSH TGDIMap::GetBrush( RTDrawInfo di, TLogBrush log )
{
if( di.pPainter->IsPrintPainter() )
    {
    if( log >= ( sizeof( printBrushes ) / sizeof( HBRUSH * ) ) )
        SYSERR( ERR_GEN );

    if( di.IsColor() )
        return *( colorBrushes[ log ] );
	else
		return *( printBrushes[ log ] );
    }
else
    {
    if( log >= ( sizeof( screenBrushes ) / sizeof( HBRUSH * ) ) )
        SYSERR( ERR_GEN );

    switch( TProfile::colorSet )
        {
        case kStdColors:
            return *( screenBrushes[ log ] );

        case kLCDColors:
        default:
            return *( lcdBrushes[ log ] );
        }

    }
}

//*****************************************************************************

//*****************************************************************************
HPEN TGDIMap::GetScreenPen( TLogPen log )
{
if( log < ( sizeof( screenPens ) / sizeof( HPEN * ) ) )
    switch( TProfile::colorSet )
        {
        case kStdColors:
            return *( screenPens[ log ] );

        case kLCDColors:
        default:
            return *( lcdPens[ log ] );
        }
else
    SYSERR( ERR_GEN );

return *(screenPens[ 0 ]);      //stop compiler warning
}

//*****************************************************************************

//*****************************************************************************
HPEN TGDIMap::GetPen( RTDrawInfo di, TLogPen log )
{
if( di.pPainter->IsPrintPainter() )
    {
    if( log >= ( sizeof( printPens ) / sizeof( HPEN * ) ) )
        SYSERR( ERR_GEN );

    if( di.IsColor() )
        return *( colorPens[ log ] );
	else   
		return *( printPens[ log ] );
		
    }
else
    {
    if( log >= ( sizeof( screenPens ) / sizeof( HPEN * ) ) )
        SYSERR( ERR_GEN );

    switch( TProfile::colorSet )
        {
        case kStdColors:
            return *( screenPens[ log ] );

        case kLCDColors:
        default:
            return *( lcdPens[ log ] );
        }
    }
}

//*****************************************************************************

//*****************************************************************************
TLogBrush TGDIMap::TranslateColor( TColor color )
{
switch( color.Get() )
    {
    case kRed:
            return kHbrRedNote;
    case kBlue:
            return kHbrBlueNote;
    case kBlueGray:
            return kHbrBlueGrayNote;
    case kRedGray:
            return kHbrRedGrayNote;
    case kGray:        //default
    default:
            return kHbrGrayNote;
    }
}

//*****************************************************************************
// get brush corresponding to note color
//*****************************************************************************
HBRUSH TGDIMap::GetScreenBrush( TColor color )
{
return GetScreenBrush( TranslateColor( color ) );      //use translated constant
}

//*****************************************************************************
// get brush corresponding to note color
//*****************************************************************************
HBRUSH TGDIMap::GetBrush( RTDrawInfo di, TColor color )
{
return GetBrush( di, TranslateColor( color ) );      //use translated constant
}


//*****************************************************************************

//*****************************************************************************
COLORREF TGDIMap::GetScreenColor( TTextColor rgb )
{
if( rgb < ( sizeof( screenColor ) / sizeof( COLORREF ) ) )
    switch( TProfile::colorSet )
        {
        case kStdColors:
            return screenColor[ rgb ];

        case kLCDColors:
        default:
            return lcdColor[ rgb ];
        }
else
    SYSERR( ERR_GEN );

return screenColor[ 0 ];      //stop compiler warning
}


//*****************************************************************************

//*****************************************************************************
int TGDIMap::GetHPGray( TLogBrush log )
{
if( (int)log >= ( sizeof( hpGraySubs ) / sizeof( int ) ) )
    SYSERR( ERR_GEN );

return hpGraySubs[ log ];
}

//*****************************************************************************

//*****************************************************************************
int TGDIMap::GetHPGray( TColor color )
{
return GetHPGray( TranslateColor( color ) );
}

