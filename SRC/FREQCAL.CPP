//*****************************************************************************
// calendar grid used in frequency control
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "freqcal.h"

#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __fontmap__
#include "fontmap.h"
#endif

//*****************************************************************************

//*****************************************************************************
TFreqCal::TFreqCal( PTWindowsObject AParent, WORD resID, RTTime startDate )
   : TCalCtl( AParent, resID, startDate )
{
    SetLeftMargin( 1 );        //was 3
    SetTopMargin(  0 );

    ClearCells();
}


//*****************************************************************************

//*****************************************************************************
TFreqCal::~TFreqCal( void )
{
}

//*****************************************************************************

//*****************************************************************************
int  TFreqCal::CellWidth( PTPaint, int  )
{
    return 18;
}

//*****************************************************************************

//*****************************************************************************
int TFreqCal::CellHeight( PTPaint, int )
{
    return 15;
}

//*****************************************************************************

//*****************************************************************************
void TFreqCal::BeginCellPaint( RTDrawInfo di )
{
    SelectObject( di.hdc, TFontMap::GetScreenFont( kSmallFont ) );
}

//*****************************************************************************

//*****************************************************************************
void  TFreqCal::PaintCell( RTDrawInfo di )
{
	char 	buf[100];
	TTime	date;

    WORD    tStyle=  DT_SINGLELINE | DT_CENTER | DT_VCENTER;

    if( di.row == 0 ) {

        TGDIMap::SetColorEnvironment( di.hdc, kEnv3D );
        TIntlFmt::DayName( di.col % 7, kOneLetter, buf );
        di.pPainter->Text( di, buf, kNoHilite, tStyle );
    }
    else {

        DrawDay( di );
    }
}

//*****************************************************************************

//*****************************************************************************
void TFreqCal::DrawDay( RTDrawInfo di )
{
	char 	buf[100];
    int     day;

    day = CellToDay( di.row, di.col );
    if (day <= 0) {

        return;
    }

    wsprintf( buf, "%d", day );
    WORD      tStyle    = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
    switch( itsCellTypes[day] ) {

        case TFreq::kStartCell:

            FillRect( di.hdc, &di.cellRect, TGDIMap::GetScreenBrush( kHbrStart ));
            TGDIMap::SetColorEnvironment( di.hdc, kEnvFreqStart );
            di.pPainter->Text( di, buf, kNoHilite, tStyle );
            break;

        case TFreq::kSecondCell:

            FillRect( di.hdc, &di.cellRect, TGDIMap::GetScreenBrush( kHbrSecond ));
            TGDIMap::SetColorEnvironment( di.hdc, kEnvFreqSecond );
            di.pPainter->Text( di, buf, kNoHilite, tStyle );
            break;

        case TFreq::kEndCell:

            FillRect( di.hdc, &di.cellRect, TGDIMap::GetScreenBrush( kHbrLast ));
            TGDIMap::SetColorEnvironment( di.hdc, kEnvFreqLast );
            di.pPainter->Text( di, buf, kNoHilite, tStyle );
            break;

        case TFreq::kAdditionCell:
        case TFreq::kFrequencyCell:

            TGDIMap::SetColorEnvironment( di.hdc, kEnv3D );
            di.pPainter->Text( di, buf, kInvertHilite, tStyle );
            break;

        case TFreq::kExcludeCell:

            TGDIMap::SetColorEnvironment( di.hdc, kEnv3D );
            di.pPainter->Text( di, buf, kInvertHilite, tStyle );
            RECT r = di.cellRect;
            MoveTo( di.hdc, r.left, (r.top + r.bottom) / 2 );
            LineTo( di.hdc, r.right,(r.top + r.bottom) / 2 );
            MoveTo( di.hdc, r.left, (r.top + r.bottom) / 2 + 1);
            LineTo( di.hdc, r.right,(r.top + r.bottom) / 2 + 1);
            break;

        case TFreq::kNormalCell:
        default:

            TGDIMap::SetColorEnvironment( di.hdc, kEnv3D );
            di.pPainter->Text( di, buf, kNoHilite, tStyle );
            break;
    }

}

//*****************************************************************************
// treat rt click is same as left click
//*****************************************************************************
void TFreqCal::MousRtClick( RTMousInfo mi )
{
MousClick( mi );
}

//*****************************************************************************
// SetCellType
//
// Mark the cell for a different type of drawing
//*****************************************************************************
void TFreqCal::SetCellType( RTTime date, TFreq::TCellType cellType )
{
    int row, col;

    itsCellTypes[date.GetDay()] = cellType;
    TimeToCell( date, row, col );
    InvalidateCell( row, col );
}

//*****************************************************************************
// GetCellType
//*****************************************************************************
TFreq::TCellType TFreqCal::GetCellType( RTTime date )
{
    return itsCellTypes[date.GetDay()];
}

//*****************************************************************************

//*****************************************************************************
void TFreqCal::ClearCells( void )
{
    for (int i = 0; i < kNumDays; i++) {

        itsCellTypes[i] = TFreq::kNormalCell;
    }

    InvalidateCell( -1, -1 );
}



//*****************************************************************************

//*****************************************************************************
void TFreqCal::GetWindowClass( WNDCLASS & AWndClass )
{
TCalCtl::GetWindowClass( AWndClass );

AWndClass.hbrBackground= TGDIMap::GetScreenBrush( kHbrBkCtl );
}

