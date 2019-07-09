//*****************************************************************************
// calendar control used in holiday dialog
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "holcal.h"

#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __fontmap__
#include "fontmap.h"
#endif

//*****************************************************************************

//*****************************************************************************
THolCal::THolCal( PTWindowsObject AParent, WORD resID, RTTime startDate )
   : TCalCtl( AParent, resID, startDate )
{
    SetLeftMargin( 1 );        //was 3
    SetTopMargin(  0 );

    itsMouseDay = 1;

    ClearCells();
}


//*****************************************************************************

//*****************************************************************************
THolCal::~THolCal( void )
{
}

//*****************************************************************************

//*****************************************************************************
int  THolCal::CellWidth( PTPaint, int  )
{
    return 18;
}

//*****************************************************************************

//*****************************************************************************
int THolCal::CellHeight( PTPaint, int )
{
    return 15;
}

//*****************************************************************************

//*****************************************************************************
void THolCal::BeginCellPaint( RTDrawInfo di )
{
    SelectObject( di.hdc, TFontMap::GetScreenFont( kSmallFont ) );
}

//*****************************************************************************

//*****************************************************************************
void  THolCal::PaintCell( RTDrawInfo di )
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
void THolCal::DrawDay( RTDrawInfo di )
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

        case kHolidayCell:

            FillRect( di.hdc, &di.cellRect, TGDIMap::GetScreenBrush( kHbrSecond ));
            TGDIMap::SetColorEnvironment( di.hdc, kEnvFreqSecond );
            di.pPainter->Text( di, buf, kNoHilite, tStyle );
            break;

        case kFixedHolidayCell:

            FillRect( di.hdc, &di.cellRect, TGDIMap::GetScreenBrush( kHbrStart ));
            TGDIMap::SetColorEnvironment( di.hdc, kEnvFreqStart );
            di.pPainter->Text( di, buf, kNoHilite, tStyle );
            break;

        case kNormalCell:
        default:

            TGDIMap::SetColorEnvironment( di.hdc, kEnv3D );
            di.pPainter->Text( di, buf, kNoHilite, tStyle );
            break;
    }

}

//*****************************************************************************
// SetCellType
//
// Mark the cell for a different type of drawing
//*****************************************************************************
void THolCal::SetCellType( RTTime date, TCellType cellType )
{
    int row, col;

    itsCellTypes[date.GetDay()] = cellType;
    TimeToCell( date, row, col );
    InvalidateCell( row, col );
}

//*****************************************************************************
// GetCellType
//*****************************************************************************
THolCal::TCellType THolCal::GetCellType( RTTime date )
{
    return itsCellTypes[date.GetDay()];
}

//*****************************************************************************

//*****************************************************************************
void THolCal::ClearCells( void )
{
    for (int i = 0; i < kNumDays; i++) {

        itsCellTypes[i] = kNormalCell;
    }

    InvalidateCell( -1, -1 );
}



//*****************************************************************************

//*****************************************************************************
void THolCal::GetWindowClass( WNDCLASS & AWndClass )
{
    TCalCtl::GetWindowClass( AWndClass );

    AWndClass.hbrBackground= TGDIMap::GetScreenBrush( kHbrBkCtl );
}

//---------------------------------------------------------------------------
// MousOver
//
// Send the parent a message about the day if it's different than the last
// one
//---------------------------------------------------------------------------
void THolCal::MousOver( RTMousInfo mi )
{
    int day = CellToDay( mi.row, mi.col );
    if (day != itsMouseDay) {

        itsMouseDay = day;
        SendMessage( Parent->HWindow, WM_COMMAND, Attr.Id,
            MAKELONG( HWindow, MOUSE_MESSAGE ));
    }
}

//*****************************************************************************
// treat rt click same as left click
//*****************************************************************************
void THolCal::MousRtClick( RTMousInfo mi )
{
MousClick( mi );
}

//---------------------------------------------------------------------------
// GetMouseDate
//
// Return the day that the mouse is over
//---------------------------------------------------------------------------
void THolCal::GetMouseDate( TCellType &cellType, RTTime day )
{
    cellType = (itsMouseDay > 0) ? itsCellTypes[itsMouseDay] : kNotACell;

    day      = itsSelTime;
    day.SetDay( itsMouseDay );
    day.ClearTime();
}

