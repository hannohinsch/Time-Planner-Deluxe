#ifndef __gdimap__
#define __gdimap__

#ifndef __color__
#include "color.h"
#endif

_CLASSDEF( TDrawInfo )
_CLASSDEF( TGDIMap )


// a "color environment" consists of a text foreground color, a text
// background color, and a background brush
typedef enum {

    kEnv3D,              //colors for a 3D control
    kEnv3DSel,
    kEnvView,            //colors for a view
    kEnvViewSel,
    kEnvBkCtl,
    kEnvLightScale,      //timepicker scale colors
    kEnvDarkScale,
    kEnvCalWeekend,      //weekend calendar days
    kEnvCalHoliday,      //holiday calendar days
    kEnvFreqStart,       // frequency dialog start day
    kEnvFreqSecond,      // frequency dialog second day
    kEnvFreqLast,        // frequency dialog last day
    kEnvMonth            // hilight some days in the month grid

    } TColorEnv;

typedef enum {

    kStdColors,
    kLCDColors

    } TGDISet;

//
// WARNING: any changes in the order or number of "logical" GDI constants
//          must be matched by updates to the map arrays! (in gdimap.cpp)
//

// brushes
typedef enum {

        // START CONSTANTS AT 0; THEY'RE USED AS ARRAY INDICES

        //backgrounds
        kHbrBkDlg= 0,           //dialog background
        kHbrBkView,            //view
        kHbrBkFrame,           //frame
        kHbrBkCtl,             //standard windows controls
        kHbrBk3D,              //special 3D control (unselected)
        kHbrBk3DSel,           //3D controls, selected
        kHbrBkOffDays,          //backgorund for non-work days
        kHbrBkHolidays,        //backgorund for holidays
        kHbrBkBar,              //tool bar background

        // miscellany
        kHbrGrid,              //grid lines
        kHbrSelect,            //frame for selected cells
        kHbrBorder,            //border for controls, button bar
        kHbrMarkBorder,        //border for note marks
        kHbrCalOverflow,       //overflow line in calendar grids
        kHbrTimeFill,          //cell fill in time-picker
        kHbrTimeMark,          //milestone mark in time picker
        kHbrBrowBlob,          //browser hidden categories
        kHbrGrafTime,           //graf view, time bar
        kHbrGrafCost,           //graf view, cost bar
		kHbrFocus,				//outline for controls w/ focus
        kHbrMultiCell,          //separate multi-cells in weekgrid
        kHbrShadow,             //drop shadow
        kHbrFolderTab,          //base part of folder
        kHbrAddrTitle,          //title background for printed address

        // note colors (priorities)
        kHbrRedNote,           //hi priority
        kHbrBlueNote,          //lo
        kHbrBlueGrayNote,         //med-lo
        kHbrRedGrayNote,        //med-hi
        kHbrGrayNote,         //med

        // frequency dialog colors
        kHbrStart,
        kHbrSecond,
        kHbrLast,

        // owner-draw buttons
        kHbrFlat,
        kHbrLit,
        kHbrShadowed

    } TLogBrush;

typedef enum {

        kHi3DPen,               //3D hilite (top and left)
        kLo3DPen,               //3D lolite (bottom and right)
        kHi3DSelPen,            //3D hilite, selected
        kLo3DSelPen,            //3D lolite, selected
        kMarkBorderPen,         //note mark (e.g., circle) border
        kInkPen,                 //ink drawing pen
        kFolderTabPen

    } TLogPen;

typedef enum {
        kRgb3DText,
        kRgb3DBk,
        kRgb3DSelText,
        kRgb3DSelBk,
        kRgbCalWeekend,
        kRgbCalHoliday,
        kRgbFreqStart,          // frequency dialog start date
        kRgbFreqSecond,         // frequency dialog second date
        kRgbFreqLast,           // frequency dialog last date
        kRgbMonth

    } TTextColor;

class _CLASSTYPE TGDIMap
{
    public:
        TGDIMap( void );
        virtual ~TGDIMap( void );

        static void SetGDIMap( TGDISet set );
        static void SysColorChange();
        static HBRUSH SetColorEnvironment( HDC hdc, TColorEnv env );

        static HBRUSH GetScreenBrush( TLogBrush );
        static HBRUSH GetScreenBrush( TColor );

        static HBRUSH GetBrush( RTDrawInfo, TLogBrush );
        static HBRUSH GetBrush( RTDrawInfo, TColor );

        static int GetHPGray( TLogBrush );
        static int GetHPGray( TColor );

        static HPEN GetScreenPen( TLogPen );
        static HPEN GetPen( RTDrawInfo, TLogPen );

    private:
        void CreatePens();
        void DeletePens();

        void CreateBrushes();
        void DeleteBrushes();

        void CreatePatBrushes();
        void DeletePatBrushes();

        static TLogBrush TranslateColor( TColor color );
        static COLORREF GetScreenColor( TTextColor );

        static HBRUSH *(screenBrushes[]);       //default screen
        static HBRUSH *(lcdBrushes[]);          //hi contrast screen
        static HBRUSH *(colorBrushes[]);        //color print brushes
        static HBRUSH *(printBrushes[]);        //usual print brushes
        static int      hpGraySubs[];           //hp gray-scale substitutes

        static HPEN *(screenPens[]);
        static HPEN *(lcdPens[]);
        static HPEN *(colorPens[]);
        static HPEN *(printPens[]);

        static COLORREF screenColor[];
        static COLORREF lcdColor[];
        static COLORREF colorColor[];
        static COLORREF printColor[];


        HBRUSH cregrdbsh( void );

        static BOOL itsErrorFlag;

        static HPEN hBlackPen;
        static HPEN hDkGrayPen;
        static HPEN hLtGrayPen;
        static HPEN hWhitePen;

        static HBRUSH hbrRed;
        static HBRUSH hbrLtRed;
        static HBRUSH hbrBlue;
        static HBRUSH hbrLtBlue;
        static HBRUSH hbrGreen;
        static HBRUSH hbrCyan;
        static HBRUSH hbrPurple;
        static HBRUSH hbrYellow;
        static HBRUSH hbrLtYellow;
        static HBRUSH hbrBlack;
        static HBRUSH hbrDkGray;
        static HBRUSH hbrDkDkGray;
        static HBRUSH hbrLtGray;
        static HBRUSH hbrWhite;

        static HBRUSH hbrBlueGray;
        static HBRUSH hbrRedGray;
        static HBRUSH hbrWhiteGray;
        static HBRUSH hbrBlackGray;

        static HBRUSH hbrPrtPri1;
        static HBRUSH hbrPrtPri2;
        static HBRUSH hbrPrtPri3;
        static HBRUSH hbrPrtPri4;

        static HBRUSH hbrLCDPri1;
        static HBRUSH hbrLCDPri2;
        static HBRUSH hbrLCDPri3;
        static HBRUSH hbrLCDPri4;

        static HBRUSH hbrSteel;
        static HBRUSH hbrHatch;
        static HBRUSH hbrRedHatch;
        static HBRUSH hbrGreenHatch;
        static HBRUSH hbrGrayHatch;

        static HBRUSH hbrAppWorkspace;
};

#endif
