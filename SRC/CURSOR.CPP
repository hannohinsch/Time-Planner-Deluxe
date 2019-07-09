//*****************************************************************************
// mouse cursor management
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "cursor.h"
#include "error.h"

HCURSOR TCursor::hUnBusyCursor= NULL;
HCURSOR TCursor::cursorTable[ kNumCursors ];
int     TCursor::busyCount= 0;

//*****************************************************************************

//*****************************************************************************
TCursor::TCursor( void )
{
CreateAppCursors();
}


//*****************************************************************************

//*****************************************************************************
TCursor::~TCursor( void )
{
DestroyAppCursors();
}

//*****************************************************************************

//*****************************************************************************
void TCursor::Set( TCursorStyle style )
{
HCURSOR h= NULL;

if( style < kNumCursors )
    h= cursorTable[ style ];

ASSERT( ERR_GEN, h!= NULL );

SetCursor( h );
hUnBusyCursor= h;     //keep track of desired, unbusy cursor
}

//*****************************************************************************
// manage hourglass cursor
//*****************************************************************************
int TCursor::Busy( BOOL aBusy )
{

if( aBusy )
    {
    if( busyCount == 0 )        //make busy
        {
        SetCursor( cursorTable[ kBusyCursor ] );
        }
    busyCount++;
    }
else
    {
    busyCount--;
    if( busyCount == 0 )        //restore unbusy cursor
        {
        if( hUnBusyCursor != NULL )
            SetCursor( hUnBusyCursor );
        else
            SetCursor( cursorTable[ kArrowCursor ] );
        }
    }

ASSERT( ERR_GEN, busyCount >= 0 );
if( busyCount < 0 )
    busyCount= 0;

return busyCount;
}

//*****************************************************************************

//*****************************************************************************
void TCursor::CreateAppCursors( )
{
HANDLE hInstance= GetApplicationObject()->hInstance;

cursorTable[ kArrowCursor      ] = LoadCursor( NULL, MAKEINTRESOURCE( IDC_ARROW ) );
cursorTable[ kBusyCursor       ] = LoadCursor( NULL, MAKEINTRESOURCE( IDC_WAIT ) );

cursorTable[ kCatSelCursor     ] = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_CATSEL ) );
cursorTable[ kOutlineCursor    ] = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_OUTLINE ) );
cursorTable[ kDateSelCursor    ] = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_DATESEL ) );
cursorTable[ kNoteSelCursor    ] = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_NOTESEL ) );
cursorTable[ kZoomCursor       ] = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_ZOOM ) );
cursorTable[ kNoteBarCursor    ] = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_NOTEBAR ) );
cursorTable[ kLongNoteBarCursor] = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_LONGNOTEBAR ) );

cursorTable[ kDragHorizCursor  ] = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_DRAG_HORIZ ) );
cursorTable[ kDragVertCursor   ] = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_DRAG_VERT ) );
cursorTable[ kDragBothCursor   ] = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_DRAG_BOTH ) );
cursorTable[ kGrowLeftCursor   ] = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_GROW_LEFT ) );
cursorTable[ kGrowRightCursor  ] = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_GROW_RIGHT ) );
cursorTable[ kGrowUpCursor     ] = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_GROW_UP ) );
cursorTable[ kGrowDownCursor   ] = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_GROW_DOWN ) );

cursorTable[ kDragIllegalCursor] = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_NO_DRAG ) );
cursorTable[ kTimeMoveCursor]    = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_TIME_MOVE ) );
cursorTable[ kNewNoteCursor]     = LoadCursor( hInstance, MAKEINTRESOURCE( IDC_NEW_NOTE ) );
}

//*****************************************************************************

//*****************************************************************************
void TCursor::DestroyAppCursors()
{
TCursorStyle style;

//restore std cursor
HANDLE h= LoadCursor( NULL, MAKEINTRESOURCE( IDC_ARROW ) );
SetCursor( h );

for( style= kFirstCursor; style < kNumCursors; ((int) style)++ )
    {
    if( style != kArrowCursor && style != kBusyCursor )
        DestroyCursor( cursorTable[ style ] );
    }
}



