//*****************************************************************************
// month control: show month, handle clicks
//  (descendents are used in holiday, datepick, etc.)
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "calctl.h"

#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __paint__
#include "paint.h"
#endif
#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __fontmap__
#include "fontmap.h"
#endif
#ifndef __msg__
#include "msg.h"
#endif
#ifndef  __cursor__
#include "cursor.h"
#endif
#ifndef  __moushelp__
#include "moushelp.h"
#endif


//*****************************************************************************

//*****************************************************************************
TCalCtl::TCalCtl( PTWindowsObject AParent, WORD resID, RTTime startTime ):
        TMousGrid( AParent, resID )
{
scrPainter= new TPaint;

Attr.Style= (Attr.Style & ~WS_BORDER );

SetLeftMargin( 10 );        //was 3
SetTopMargin( 2 );

// 7 rows: 1 header, 6 for weeks
// 7 columns, 1 for each day, no header column
SetRowCnt( 7 );
SetColCnt( 7 );

//sel time is the "selected" date, and moves with mouse click
itsSelTime= startTime;

// set the date to the first of the month
itsStartTime= startTime;
itsStartTime.SetDay( 1 );
itsFirstWeekday = itsStartTime.GetWeekDay();
itsLastDayOfMonth = itsStartTime.LastDayOfMonth();

int r,c;
TimeToCell( itsSelTime, r, c );
SetFocusCell( r, c );

itsAutoClick= FALSE;
}

//*****************************************************************************

//*****************************************************************************
TCalCtl::TCalCtl( PTWindowsObject AParent, WORD resID, int x, int y, int dx, int dy, RTTime startTime ):
        TMousGrid( AParent, resID, x, y, dx, dy )
{
scrPainter= new TPaint;

Attr.Style= (Attr.Style & ~WS_BORDER );

SetLeftMargin( 10 );        //was 4
SetTopMargin( 2 );

// 7 rows: 1 header, 6 for weeks
// 7 columns, 1 for each day, no header column
SetRowCnt( 7 );
SetColCnt( 7 );

//sel time is the "selected" date, and moves with mouse click
itsSelTime= startTime;

// set the date to the first of the month
itsStartTime= startTime;
itsStartTime.SetDay( 1 );
itsFirstWeekday = itsStartTime.GetWeekDay();
itsLastDayOfMonth = itsStartTime.LastDayOfMonth();

int r,c;
TimeToCell( itsSelTime, r, c );
SetFocusCell( r, c );

itsAutoClick= FALSE;
}

//*****************************************************************************

//*****************************************************************************
TCalCtl::~TCalCtl( void )
{
delete scrPainter;
}

//*****************************************************************************

//*****************************************************************************
void TCalCtl::BeginCellPaint( RTDrawInfo di )
{
}

//*****************************************************************************

//*****************************************************************************
void  TCalCtl::PaintCell( RTDrawInfo di )
{
	char 	buf[100];
    int     day;

    WORD    tStyle=  DT_SINGLELINE | DT_CENTER | DT_VCENTER;

    if( di.row == 0 ) {

        SetColor( di );
        TIntlFmt::DayName( di.col % 7, kOneLetter, buf );
        di.pPainter->Text( di, buf, kNoHilite, tStyle );
    }
    else {
        day = CellToDay( di.row, di.col );
        if (day > 0) {

            wsprintf( buf, "%d", day );

            TTime now;
            now.SetNow();

            TTime cellTime;
            CellToTime( di.row, di.col, cellTime );

            THilite hilite= kNoHilite;

            // choose proper hilite
            if (cellTime.IsSameDay( now, FALSE ))
                hilite= kInvertHilite;
            else if( itsAutoClick
                     && IsFocusCell( di.row, di.col )
                     && !HasFocus()
                    )
                    {
                    //if it's the focus cell, but we don't have focus, then
                    // hilite it w/ conventional frame
                    hilite= kFrameHilite;
                    }

            SetColor( di );
            di.pPainter->Text( di, buf, hilite, tStyle );

        }
    }
}

//*****************************************************************************
// SetColor
//
//  Select the color for the cell based on whether it's a weekend, holiday
//  or ordinary day
//*****************************************************************************
void TCalCtl::SetColor( RTDrawInfo di )
{
    TColorEnv env;
    int       day = CellToDay( di.row, di.col );
    TTime     date( itsStartTime, SCALE_DAY, day - 1 );

    if (day == -1) {

        env = kEnv3D;
    }
    else if (date.IsHoliday()) {

        env = kEnvCalHoliday;
    }
    else if (!date.IsWorkDay()) {

        env = kEnvCalWeekend;
    }
    else {

        env = kEnv3D;
    }

    TGDIMap::SetColorEnvironment( di.hdc, env );
}

//*****************************************************************************

//*****************************************************************************
BOOL TCalCtl::IsCellSelectable( int row, int col )
{
return ( row >= 0 && col >= 0 && row < rowCnt && col < colCnt &&
		0 < CellToDay( row, col ) );
}

//*****************************************************************************
//set focus to first day of month
//*****************************************************************************
void TCalCtl::GetDefaultFocusCell( int &row, int &col )
{
TimeToCell( itsStartTime, row, col );
}

//*****************************************************************************
// if "itsAutoClick" then move of cell cursor by key is treated as a mouse click
//*****************************************************************************
void TCalCtl::SetAutoClick( BOOL b )
{
itsAutoClick= b;
}

//*****************************************************************************

//*****************************************************************************
void TCalCtl::WMSetFocus( RTMessage msg )
{
//repaint, because we need to get rid of selection rectangle
int r, c;
GetFocusCell( r, c );

TMousGrid::WMSetFocus( msg );

InvalidateCell( r, c );

}

//*****************************************************************************

//*****************************************************************************
void TCalCtl::WMKillFocus( RTMessage msg )
{
//repaint, because we want to add selection rectangle
int r, c;
GetFocusCell( r, c );

TMousGrid::WMKillFocus( msg );

InvalidateCell( r, c );
}

//*****************************************************************************
// if autoclick is enabled,
// we handle focus locally -- ( same as selection ) -- so don't show it
//*****************************************************************************
void TCalCtl::PaintFocus( HDC hdc, RECT &r )
{
TGDIMap::SetColorEnvironment( hdc, kEnvBkCtl );
DrawFocusRect( hdc, &r );
}

//*****************************************************************************

//*****************************************************************************
void TCalCtl::MousKey( TMousKey key )
{
TMousInfo mi;
int row, col;

GetFocusCell( row, col );

//build fake mi struct, for mousclick
// emulate mouseclick
mi.bKeyAction= TRUE;
mi.row= row;
mi.col= col;
mi.style= kCellPoint;
mi.state= 0;
mi.xy.x= 0;
mi.xy.y= 0;

//if we're at a boundary, scroll
// we know the current focus cell, if it exists, is selectable
// if the cell in the direction of the arrow is not, then it's
// time to scroll
if( row != -1 && col != -1 )
	switch( key )
		{
		case kUpKey:
			//fake a button push ( -1 year )
			if( ! IsCellSelectable( row -1, col ))
				SendMessage( Parent->HWindow, WM_COMMAND, IDD_CALMINUS2, MAKELONG( HWindow, BN_CLICKED ) );				
            else
                {
                mi.row--;
                SetFocusCell( mi.row, mi.col );
                if( itsAutoClick )
                    MousClick( mi );
                }
            break;
		case kDownKey:
			//fake a button push ( +1 year )
			if( ! IsCellSelectable( row + 1, col ))
				SendMessage( Parent->HWindow, WM_COMMAND, IDD_CALPLUS2, MAKELONG( HWindow, BN_CLICKED ));				
            else
                {
                mi.row++;
                SetFocusCell( mi.row, mi.col );
                if( itsAutoClick )
                    MousClick( mi );
                }
            break;
		case kLeftKey:
			//fake a button push ( -1 month )
			if( ! IsCellSelectable( row, col - 1 ))
				SendMessage( Parent->HWindow, WM_COMMAND, IDD_CALMINUS, MAKELONG( HWindow, BN_CLICKED ) );				
            else
                {
                mi.col--;
                SetFocusCell( mi.row, mi.col );
                if( itsAutoClick )
                    MousClick( mi );
                }
            break;
		case kRightKey:
			//fake a button push ( +1 month )
			if( ! IsCellSelectable( row, col + 1 ))
				SendMessage( Parent->HWindow, WM_COMMAND, IDD_CALPLUS, MAKELONG( HWindow, BN_CLICKED ) );				
            else
                {
                mi.col++;
                SetFocusCell( mi.row, mi.col );
                if( itsAutoClick )
                    MousClick( mi );
                }
            break;
        case kScrUpKey:
                SendMessage( Parent->HWindow, WM_COMMAND, IDD_CALMINUS, MAKELONG( HWindow, BN_CLICKED ) );
                break;

        case kScrDownKey:
                SendMessage( Parent->HWindow, WM_COMMAND, IDD_CALPLUS, MAKELONG( HWindow, BN_CLICKED ));
                break;

        case kScrLeftKey:
                SendMessage( Parent->HWindow, WM_COMMAND, IDD_CALMINUS2, MAKELONG( HWindow, BN_CLICKED ) );
                break;

        case kScrRightKey:
                SendMessage( Parent->HWindow, WM_COMMAND, IDD_CALPLUS2, MAKELONG( HWindow, BN_CLICKED ) );
                break;

        default:
            TMousGrid::MousKey( key );
			break;
		}
	
}


//*****************************************************************************

//*****************************************************************************
void TCalCtl::MousClick( RTMousInfo mi )
{
if( IsCellSelectable( mi.row, mi.col ) )
    {
    itsClickPoint= mi.xy;       //stash away

    // set new selected time
    TTime time;
    CellToTime( mi.row, mi.col, time );
    itsSelTime.CopyDate( time );

    SetFocusCell( mi.row, mi.col );

    SendMessage( Parent->HWindow, WM_COMMAND, Attr.Id,
            MAKELONG( HWindow, CN_CLICKED ));
    }
}

//*****************************************************************************
// MousRtClick
//
//  Info box
//*****************************************************************************
void TCalCtl::MousRtClick( RTMousInfo mi )
{
if( IsCellSelectable( mi.row, mi.col ) )
    {
    TMousHelp::MousHelp( this,
        "Click left button to set the date." );
    }

}

//*****************************************************************************
// return location of last mouse click; for popup menus and such
//*****************************************************************************
void TCalCtl::GetClickPoint( POINT &pt )
{
pt= itsClickPoint;

//translate to screen coords
ClientToScreen( HWindow, &pt );
}

//*****************************************************************************

//*****************************************************************************
void TCalCtl::MousOver( RTMousInfo mi )
{
if (! IsCellSelectable( mi.row, mi.col ) )
    TCursor::Set( kArrowCursor );
else
    TCursor::Set( kDateSelCursor );
}

//*****************************************************************************

//*****************************************************************************
int TCalCtl::CellToDay( int row, int col )
{
	int day;

	day = ( 7 * (row - 1) ) + ( col % 7 - itsFirstWeekday + 1 );
	if (day < 1) {

		day = -1;
	}
    else if (day > itsLastDayOfMonth) {

		day = -1;
	}

	return( day );
}

//*****************************************************************************

//*****************************************************************************
void TCalCtl::TimeToCell( RTTime date, int &row, int &col )
{
   int	d;

   d = date.GetDay() + itsFirstWeekday + 6;
   row = d / 7;
   col = d % 7;
}


//*****************************************************************************

//*****************************************************************************
void TCalCtl::CellToTime( int row, int col, TTime &time )
{
	int	d;

	time = itsStartTime;
	d    = CellToDay( row, col );
	if (d != -1) {

        time.SetDay( d );
	}
}


//*****************************************************************************

//*****************************************************************************
int  TCalCtl::CellWidth( PTPaint, int  )
{
return 18;
}

//*****************************************************************************

//*****************************************************************************
int TCalCtl::CellHeight( PTPaint, int )
{
return 16;
}

//*****************************************************************************

//*****************************************************************************
RTTime TCalCtl::GetSelTime( void )
{
itsSelTime.CopyTime( TSysState::selTime );
return itsSelTime;
}


//*****************************************************************************
// calendar is forced to always show "minor time"
// i.e. if day is not in current month, month moves to show that day
//*****************************************************************************
void TCalCtl::SetSelTime( RTTime time )
{
itsSelTime= time;

//if not in current month view, switch view
if( time.HasDate() )
    {
    TTime temp= time;
    temp.Normalize( SCALE_MONTH );
    if( temp != itsStartTime )
        {
        int r, c;

        itsStartTime= temp;

        // set the date to the first of the month
        itsStartTime.SetDay( 1 );
        itsFirstWeekday = itsStartTime.GetWeekDay();
        itsLastDayOfMonth = itsStartTime.LastDayOfMonth();

        //move focus rect
        TimeToCell( time, r, c );
        SetFocusCell( r, c );
        InvalidateCell( -1, -1 );
        }
    else
        {
        //NOTE: since we may not have focus, but
        // certainly need to update window after changing date
        // we invalidate the old and new selections (as well as set focus)
        int r,c;
        GetFocusCell( r, c );
        InvalidateCell( r, c );

        TimeToCell( time, r, c );
        SetFocusCell( r, c );
        InvalidateCell( r, c );
        }
    }
else
    SetFocusCell( -1, -1 );
}


//*****************************************************************************

//*****************************************************************************
void TCalCtl::GetWindowClass( WNDCLASS & AWndClass )
{
TMousGrid::GetWindowClass( AWndClass );

AWndClass.hbrBackground= TGDIMap::GetScreenBrush( kHbrBkCtl );
AWndClass.style &= ~CS_DBLCLKS;
}

//*****************************************************************************
// update window background brush
//*****************************************************************************
void TCalCtl::WMLocalColor( RTMessage )
{
SetClassWord( HWindow, GCW_HBRBACKGROUND, TGDIMap::GetScreenBrush( kHbrBkCtl ) );
InvalidateRect( HWindow, NULL, TRUE );
}

