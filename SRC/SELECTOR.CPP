#include "standard.h"
#pragma hdrstop

#include "selector.h"
#include "msg.h"

// ------------------------------------------------------------------------
//	Constructor
//
// ------------------------------------------------------------------------
TSelector::TSelector( PTWindowsObject AParent, WORD resID,
    int numItems, TSelectorData *data ):
    TRuleGrid( AParent, resID )
{
    scrPainter= new TPaint;
    prtPainter= new TPaintPrt;

    itsNumItems       = numItems;
    itsData           = data;
    itsCellWidth      = 22;
    itsCellHeight     = 19;
    itsTextStyle      = DT_SINGLELINE | DT_LEFT;

    SetColCnt( 1 );
    SetRowCnt( numItems );
}


// ------------------------------------------------------------------------
//	Destructor
// ------------------------------------------------------------------------
TSelector::~TSelector( void )
{
    delete scrPainter;
    delete prtPainter;
}


// ------------------------------------------------------------------------
//	SetData
//
//  Modify itsSelectorData and itsNumItems
// ------------------------------------------------------------------------
void TSelector::SetData( int numItems, TSelectorData *data )
{
    itsNumItems = numItems;
    itsData     = data;
    SetRowCnt( numItems );

    // force a redraw
    InvalidateCell( -1, -1 );
}

// ------------------------------------------------------------------------
//	MsgClick
//
//	Send the parent a notification message
// ------------------------------------------------------------------------
void TSelector::MousClick( RTMousInfo mi  )
{
    if( mi.style == kCellPoint )
        {
        itsData[mi.row].selected = !itsData[mi.row].selected;
        InvalidateCell( mi.row, -1 );
        SendMessage( Parent->HWindow, WM_COMMAND, Attr.Id,
            MAKELONG( HWindow, CN_CLICKED ));
        }
}


// ------------------------------------------------------------------------
//  CellWidth
//
//	Return the column width
// ------------------------------------------------------------------------
int TSelector::CellWidth( PTPaint, int  )
{
    return (itsCellWidth);
}

// ------------------------------------------------------------------------
//  CellHeight
//
//	Return the row height
// ------------------------------------------------------------------------
int TSelector::CellHeight( PTPaint, int  )
{
    return (itsCellHeight);
}

// ------------------------------------------------------------------------
//	PaintCell
//
//	Draw a cell
// ------------------------------------------------------------------------
void TSelector::PaintCell( RTDrawInfo di )
{
    int row= di.row;

    THilite hilite = itsData[row].selected ?
              kInvertHilite : kNoHilite;

    di.pPainter->Text( di, *(itsData[row].pText), hilite, itsTextStyle, kNormalFont );

}

// -------------------------------------------------
// WMSize
//
//	Set the size of the rows and cols
// -------------------------------------------------
void TSelector::WMSize( RTMessage Msg )
{

    itsCellWidth  = LOWORD( Msg.LParam ) -2;        //-2 for gridlines

    TRuleGrid::WMSize( Msg );
}







// ------------------------------------------------------------------------
//	Constructor
//
// ------------------------------------------------------------------------
TSingleSelector::TSingleSelector( PTWindowsObject AParent, WORD resID,
    int numItems, TSelectorData *data ):
    TSelector( AParent, resID, numItems, data )
{
    MakeSelection();
}


// ------------------------------------------------------------------------
//	Destructor
// ------------------------------------------------------------------------
TSingleSelector::~TSingleSelector( void )
{
}

// ------------------------------------------------------------------------
//	MakeSelection
//
//  Make sure that only 1 item is selected and make
//  itsSelection that item
// ------------------------------------------------------------------------
void TSingleSelector::MakeSelection( void )
{
    BOOL selectionFound = FALSE;

    itsSelection = 0;

    // the first item selected becomes the selection
    // and all other selections are turned off
    for (int i = 0; i < itsNumItems; i++)
        {
        if (itsData[i].selected)
            {
            if (selectionFound)
                {
                itsData[i].selected = FALSE;
                }
            else
                {
                itsSelection = i;
                selectionFound = TRUE;
                }
            }
        }
}

// ------------------------------------------------------------------------
//	SetData
//
//  Modify itsSelectorData and itsNumItems
// ------------------------------------------------------------------------
void TSingleSelector::SetData( int numItems, TSelectorData *data )
{
    TSelector::SetData( numItems, data );
    MakeSelection();
}

// ------------------------------------------------------------------------
//	GetSelection
//
//	Return the selection
// ------------------------------------------------------------------------
long TSingleSelector::GetSelection( void )
{
	return( itsData[itsSelection].value );
}

// ------------------------------------------------------------------------
//	SetSelection
//
//	Set the selection
// ------------------------------------------------------------------------
void TSingleSelector::SetSelection( int item )
{
    if ((item >= 0) && (item < itsNumItems))
        {
        itsData[itsSelection].selected = FALSE;
        InvalidateCell( itsSelection, 0 );

	    itsSelection = item;
        itsData[itsSelection].selected = TRUE;
        InvalidateCell( itsSelection, 0 );
        }
}

// ------------------------------------------------------------------------
//	MsgClick
//
//	Make a selection and send the parent a notification message
// ------------------------------------------------------------------------
void TSingleSelector::MousClick( RTMousInfo mi  )
{
    if( mi.style == kCellPoint )
        {
        SetSelection( mi.row );
        SendMessage( Parent->HWindow, WM_COMMAND, Attr.Id,
            MAKELONG( HWindow, CN_CLICKED ));
        }
}

// ------------------------------------------------------------------------
//	MsgDblClick
//
//	Make a selection and send the parent a notification message
// ------------------------------------------------------------------------
void TSingleSelector::MousDblClick( RTMousInfo mi  )
{
    if( mi.style == kCellPoint )
        {
        SetSelection( mi.row );
        SendMessage( Parent->HWindow, WM_COMMAND, Attr.Id,
            MAKELONG( HWindow, CN_DBLCLICKED ));
        }
}

