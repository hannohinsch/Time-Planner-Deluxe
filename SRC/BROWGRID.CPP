/***************************************************************************
 *                                                                         *
 *  MODULE     : browgrid.cpp
 *               implement crosstab view
 *
 ***************************************************************************/
#include "standard.h"
#pragma hdrstop

#include "rc.h"

#include "browgrid.h"

#ifndef __frame__
#include "frame.h"
#endif
#ifndef __note__
#include "note.h"
#endif
#ifndef __noteview__
#include "noteview.h"
#endif
#ifndef __cat__
#include "cat.h"
#endif
#ifndef __catlist__
#include "catlist.h"
#endif
#ifndef __cattree__
#include "cattree.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __pen__
#include "pen.h"
#endif
#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __paint__
#include "paint.h"
#endif
#ifndef __print__
#include "print.h"
#endif
#ifndef  __cursor__
#include "cursor.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __notemenu__
#include "notemenu.h"
#endif
#ifndef __notetab__
#include "notetab.h"
#endif
#ifndef __filter__
#include "filter.h"
#endif
#ifndef __moushelp__
#include "moushelp.h"
#endif

//============================================================================
// The TBrowGrid class functions (override virtual fns in TRuleGrid abstract class)
//============================================================================

const int kNonCatCols  = 2;
const int kNonNoteRows = 1;

//*****************************************************************************
// Cosntructor
//*****************************************************************************
TBrowGrid::TBrowGrid( PTWindowsObject AParent): TRuleGrid( AParent )
{
    TCursor::Busy( TRUE );      //turned off in setupwindow()

    scrPainter     = new TPaint;
    prtPainter     = new TPaintPrt;

    itsCatTableP   = new TXArray( 10, 10 );
    itsCatListP    = new TCatList;
    itsNoteTableP  = new TNoteTable;

    Reset();

    SetRowFreeze( 1 );
    SetColFreeze( kNonCatCols );

    SetPrtTitle( "Folder Assignments" );
    SetDefaultPrintRange();
}

//*****************************************************************************

//*****************************************************************************
TBrowGrid::~TBrowGrid( void )
{
    delete scrPainter;
    delete prtPainter;

    // delete the old information
    delete itsCatListP;
    itsCatTableP->flush( TXArray::kDelete );
    delete itsCatTableP;
    itsNoteTableP->flush( TXArray::kDelete );
    delete itsNoteTableP;
}

//*****************************************************************************

//*****************************************************************************
void TBrowGrid::SetupWindow( void )
{
TRuleGrid::SetupWindow();

TCursor::Busy( FALSE );
}

// --------------------------------------------------------------
//
//  The set of notes changed (or was set for the first time)
//  Collect the categories and set up the grid.
// --------------------------------------------------------------
void TBrowGrid::Reset( void )
{
    int       depth;

    // clear out the old data
    itsCatListP->flush( TXArray::kNoDelete );
    itsCatTableP->flush( TXArray::kDelete );
    itsNoteTableP->Flush( TXArray::kDelete );

    // tell the grid to redraw itself
    InvalidateFocus();
    InvalidateCell(-1,-1);

    // collect categories used in the current selected noteset
    // and allow 1 column per category in that set
    itsCatListP->AddVisibleCats( TSysState::pCategories );
    itsNumCats  = itsCatListP->getItemsInContainer();
    SetColCnt( kNonCatCols + itsNumCats );

    // initialize the BrowGridData array
    itsMaxCatDepth    = 0;

    // run through each category collecting information for each
    for (int i = 0; i < itsNumCats; i++) {

        // create a BrowGridData item and initialize it
        TBrowGridData *catEntryP = new TBrowGridData;
        catEntryP->catP          = &(*itsCatListP)[i];
        catEntryP->catP->GetNickname( catEntryP->nickname, TRUE, 8 );

        //limit characters per level to 8, to insure that name fits
        // into column header
        catEntryP->catP->GetFullName( catEntryP->longName, TRUE, '\n', 8 );
        depth = catEntryP->catP->GetDepth();

        // add the data item to the array  ( add unsorted )
        itsCatTableP->add( (RObject) *catEntryP, TXArray::kNoSort );


        // keep track of greatest category depth in order
        // to be able to create enough room in the top row of grid
        itsMaxCatDepth = (depth > itsMaxCatDepth) ? depth : itsMaxCatDepth;
    }

    itsCatTableP->sort();          //now sort it

    if (TSysState::CatGrouping == kNoCatGrouping)
        ResetNotesNoGrouping();
    else
        ResetNotesWithGrouping();

    //set initial selection
    int r= TSysState::currentNoteIndex;
    r= max( 0, min( r, itsRowCount  - 1));
    GotoRC( r, -1 );
}

//*****************************************************************************
// ResetNotesNoGrouping
//*****************************************************************************
void TBrowGrid::ResetNotesNoGrouping( void )
{
    // header + 1 row per note
    itsRowCount  = itsNoteTableP->Add( "" );
    itsRowCount += itsNoteTableP->Add(
        TSysState::pSelNotes, NULL, FALSE, FALSE );
    SetRowCnt( itsRowCount );
}

//*****************************************************************************
// ResetNotesWithgrouping
//*****************************************************************************
void TBrowGrid::ResetNotesWithGrouping( void )
{
//first, get rid of old info
itsNoteTableP->Flush( TXArray::kDelete );

// put in a dummy header row for the column headings
itsRowCount = itsNoteTableP->Add( "" );

// fill itsNoteTableP with group notes
itsRowCount += itsNoteTableP->AddWithGrouping( FALSE );

SetRowCnt( itsRowCount );
}

//*****************************************************************************

//*****************************************************************************
int  TBrowGrid::CellWidth( PTPaint pPainter, int col )
{
const kColorColWidth= 19;
const kMinNoteColWidth= 153;
const kCatColWidth= 53;

if( pPainter->IsPrintPainter() )
    {
    switch( ColType( col ) )
        {

        case kStyleCol:
            return kColorColWidth;

        case kNameCol:
            RECT r;
            TPrint::GetPrtPixels( &r );
            int availWidth= r.right - r.left;

            availWidth-= leftMargin + LeftLineWidth( pPainter );
            availWidth-= kColorColWidth + GridWidth( pPainter, 0 );
            availWidth-= kMinNoteColWidth + GridWidth( pPainter, 1 );

            //how many cat cols fit?
            int nCatCols = availWidth / ( kCatColWidth + GridWidth( pPainter, 2 ) );

            //add slop back into note column
            return ( kMinNoteColWidth + availWidth
                     - nCatCols * ( kCatColWidth + GridWidth( pPainter, 2 ) ));

        case kCatCol:
            return kCatColWidth;

        }
    }
else
    {
    switch( ColType( col ))
        {
        case kStyleCol:   return kColorColWidth;
        case kNameCol:    return kMinNoteColWidth;
        case kCatCol:     return kCatColWidth;
        }
    }

return 0;
}


// ----------------------------------------------------------------------------
// CellHeight
//
//  Row 0 must be high enough to allow the full category names to
//  appear as headers
// ----------------------------------------------------------------------------
int TBrowGrid::CellHeight( PTPaint pPainter, int row )
{
const int defHeight = 19;
int h = defHeight;

switch( RowType( row ))
    {
	case kHeaderRow:

        return max( 1, itsMaxCatDepth ) * 17;

    case kTitleRow:

        return 2 * defHeight;

	case kNoteRow:

        if( TSysState::bShowFullText)
            {
            RTNote note= GetNote( row );

            if( TPen::bPenActive && note.GetInk().HasInk() && (0 == note.GetText().Len()) )
                {
                RECT r;
                note.GetInk().Measure( &r );
                h= max( defHeight, r.bottom - r.top );
                }
            else
                {
                int width= CellWidth( pPainter, 1 );

                h= pPainter->CalcHeight( note.GetText().Addr(), width, kNormalFont );

                //limit height of any one note to 90% of window height
                RECT pageSize;
                if( pPainter->IsPrintPainter() )
                    TPrint::GetPrtPixels( &pageSize );
                else
                    GetClientRect( HWindow, &pageSize );

                h= min( h, (int) (0.9 * (pageSize.bottom - pageSize.top)) );

                h= max( defHeight, h );
                }
            }
    }

return h;
}

//*****************************************************************************

//*****************************************************************************
int  TBrowGrid::GridWidth( PTPaint, int col )
{
return (ColType( col ) == kNameCol) ? 2 : 1;
}

//*****************************************************************************

//*****************************************************************************
int  TBrowGrid::GridHeight( PTPaint, int row )
{
if (RowType( row + 1 ) == kTitleRow)
    return 2;
else if (RowType( row ) == kHeaderRow)
    return 2;
else
    return 1;
}


//*****************************************************************************

//*****************************************************************************
void  TBrowGrid::PaintCell( RTDrawInfo di )
{
    switch( CellType( di.row, di.col ))
        {
        case kHeaderName:   PaintHeaderName( di );  break;
        case kHeaderCat:    PaintHeaderCat( di );   break;
        case kTitleName:
        case kTitleStyle:
        case kTitleCat:     PaintTitleName( di );   break;
        case kNoteStyle:    PaintNoteStyle( di );   break;
        case kNoteName:     PaintNoteName( di );    break;
        case kNoteCat:      PaintNoteCat( di );     break;
        }
}


//*****************************************************************************
// GetNote
//
//  Get the note corresponding to the row
//*****************************************************************************
RTNote TBrowGrid::GetNote( int row )
{
if( row > 0 && row < itsRowCount )
    {
    RTNoteRow noteRow = (RTNoteRow) (*itsNoteTableP)[row];
    return *noteRow.itsNoteP;
    }
else
    return (RTNote) NOOBJECT;
}

//*****************************************************************************
// IsNoteRow
//
//  Is the row filled with a real note (as opposed
//  to a header)?
//*****************************************************************************
BOOL  TBrowGrid::IsNoteRow( int row )
{
if( row > 0 && row < itsRowCount )
    {
    RTNoteRow noteRow = (RTNoteRow) (*itsNoteTableP)[row];
    return noteRow.itsRowType == kNoteTableNote;
    }
else
    return FALSE;
}

//*****************************************************************************
// PaintHeaderName
//*****************************************************************************
void  TBrowGrid::PaintHeaderName( RTDrawInfo di )
{
    di.pPainter->Text( di, "Note", kNoHilite, kMultiLine | DT_CENTER, kBoldFont );
}

//*****************************************************************************
// PaintHeaderCat
//*****************************************************************************
void  TBrowGrid::PaintHeaderCat( RTDrawInfo di )
{
    char buf[256];
    char buf2[256];

    int  catNum = di.col - kNonCatCols;
    ((RTBrowGridData) (*itsCatTableP)[catNum]).longName.Get( buf, 256 );
    PTCat catP = ((RTBrowGridData) (*itsCatTableP)[catNum]).catP;
    if (catP->HasSubCats())
        {
        wsprintf( buf2, "%c%s", (catP->IsShowingSubCats()) ? '-' : '+', buf );
        di.pPainter->Text( di, buf2, kNoHilite, kMultiLine );
        }
    else
        {
        di.pPainter->Text( di, buf, kNoHilite, kMultiLine );
        }
}

//*****************************************************************************
// PaintTitleName
//*****************************************************************************
void  TBrowGrid::PaintTitleName( RTDrawInfo di )
{
    TSafStr buf;

    if( CellType( di.row, di.col ) == kTitleName)
        {
        RTNoteRow noteRow = (RTNoteRow) (*itsNoteTableP)[di.row];
        buf.Set( noteRow.itsHeader );
        di.pPainter->FolderTab( di, FALSE, buf );
        }
    else
        {
        di.pPainter->FolderTab( di, TRUE, buf );
        }
}

//*****************************************************************************
// PaintNoteStyle
//*****************************************************************************
void  TBrowGrid::PaintNoteStyle( RTDrawInfo di )
{
    RTNote note= GetNote( di.row );
    di.pPainter->DrawMark( di, note );
}

//*****************************************************************************
// PaintNoteName
//*****************************************************************************
void  TBrowGrid::PaintNoteName( RTDrawInfo di )
{
    char buf[256];

    RTNote note= GetNote( di.row );

    TSafStr title;
    note.Title( title );
    title.Get( buf, sizeof( buf ) );
    if( !buf[0] && note.GetInk().HasInk() )
        {
        note.GetInk().Show( di.hdc, &di.cellRect );
        }
    else
        {
        TFont font= (note.IsDone()) ? kStrikeFont : kNormalFont;

        RECT r = di.cellRect;

        if( TSysState::bShowFullText )
            di.pPainter->Text( di, (char *) note.GetText().Addr(), kNoHilite, kMultiLine, font, &r );
        else
            di.pPainter->Text( di, buf, kNoHilite, kSingleLine | DT_LEFT, font, &r );
        }
}

//*****************************************************************************
// PaintNoteCat
//*****************************************************************************
void  TBrowGrid::PaintNoteCat( RTDrawInfo di )
{
    RTNote note= GetNote( di.row );

    int catNum = di.col - kNonCatCols;
    RTBrowGridData catEntry = ((RTBrowGridData) (*itsCatTableP)[catNum]);
    if( note.GetCatList().IsCatInList( *catEntry.catP ))
        {
        di.pPainter->Text( di, catEntry.nickname, kNoHilite, kSingleLine );
        }
    else if ( !catEntry.catP->IsShowingSubCats() &&
        note.GetCatList().IsCatOrChildInList( *catEntry.catP ))
        {
        //draw mark indicating hidden selection
        RECT r= di.cellRect;
        InflateRect( &r, -4, - 4);      //margins
        r.top= r.bottom - 8;            //use 8-pixel high mark

        di.pPainter->FillRectangle( di, r, kHbrBrowBlob );
        }
}


//*****************************************************************************
// MousClick
//
// Assign/Unassign this category to this note
//*****************************************************************************
void  TBrowGrid::MousClick( RTMousInfo mi )
{
    int col = mi.col;
    int row = mi.row;

    if (CellType( row, col ) == kHeaderCat)
        {
        TRuleGrid::MousClick( mi );

        PTCat catP = ((RTBrowGridData) (*itsCatTableP)[col - 2]).catP;
        if (catP->HasSubCats())
            {
            TCursor::Busy( TRUE );
            catP->ToggleVisibleSubCats();
            itsCatListP->flush();
            itsCatTableP->flush(TXArray::kDelete);
            Reset();
            Notify( kCatVis, NOOBJECT );
            TCursor::Busy( FALSE );
            }
        }
    else if (CellType( row, col ) == kNoteCat)
        {
        TRuleGrid::MousClick( mi );

        // figure out what category goes with this column
        int catNum = col - kNonCatCols;
        RTBrowGridData catEntry = ((RTBrowGridData) (*itsCatTableP)[catNum]);
        RTNote note= GetNote( mi.row );
        if (note.GetCatList().IsCatInList( *catEntry.catP )) {

            note.GetCatList().detach( *catEntry.catP );
        }
        else {

            note.GetCatList().add( *catEntry.catP );
        }

        // Dirty
        TSysState::bSystemDirty = TRUE;

    }
}

//*****************************************************************************

//*****************************************************************************
void TBrowGrid::InvalidateNote( RTNote note )
{
    // invalidate all rows with this note in them
    for (int i = 0; i < itsRowCount; i++)
        if (IsNoteRow( i ) && note == GetNote( i ))
            InvalidateCell( i, -1 );
}


//*****************************************************************************

//*****************************************************************************
void  TBrowGrid::MousDblClick( RTMousInfo mi )
{
    if (CellType( mi.row, mi.col ) == kNoteName)
        {
        PTNote pNote = &GetNote( mi.row );
        GetApplication()->ExecDialog(new TNoteView( TPimFrame::pFrame, IDD_NOTE, pNote));
        InvalidateNote( *pNote );
       }
    else
        {
        MousClick( mi );
        }
}

//*****************************************************************************

//*****************************************************************************
void TBrowGrid::MousRtClick( RTMousInfo mi )
{
    switch( CellType( mi.row, mi.col ))
        {
        case kHeaderCat:

            // check to see if the cat has any subcats
            PTCat catP = ((RTBrowGridData) (*itsCatTableP)[mi.col - 2]).catP;
            if (catP->HasSubCats())
                {
                if (catP->IsShowingSubCats())
                    TMousHelp::MousHelp( this,
                        "Click left button to hide subfolders." );
                else
                    TMousHelp::MousHelp( this,
                        "Click left button to show subfolders." );
                }
            break;

        case kNoteName:

            // popup local note menu (change color, delete, etc. )
            PTNote pNote = &GetNote( mi.row );
            TNoteMenu::PopUp( this, pNote, mi.xy.x, mi.xy.y );
            InvalidateNote( *pNote );
            break;

        case kNoteCat:

            int catNum = mi.col - kNonCatCols;
            RTBrowGridData catEntry = ((RTBrowGridData) (*itsCatTableP)[catNum]);
            RTNote note= GetNote( mi.row );
            if (note.GetCatList().IsCatInList( *catEntry.catP ))
                {
                TMousHelp::MousHelp( this,
                    "Click left button to remove note from this folder." );
                }
            else
                {
                TMousHelp::MousHelp( this,
                    "Click left button to assign note to this folder." );
                }
            break;
        }
}


//*****************************************************************************

//*****************************************************************************
void TBrowGrid::MousOver( RTMousInfo mi )
{
    switch( CellType( mi.row, mi.col )) {

        case kHeaderCat:

            // check to see if the cat has any subcats
            PTCat catP = ((RTBrowGridData) (*itsCatTableP)[mi.col - 2]).catP;
            if (catP->HasSubCats())
                TCursor::Set( kOutlineCursor );
            else
                TCursor::Set( kArrowCursor );
            break;

        case kNoteName:      TCursor::Set( kNoteSelCursor );  break;
        case kNoteCat:       TCursor::Set( kCatSelCursor );   break;

        case kNoteStyle:
        case kTitleName:
        case kTitleStyle:
        case kTitleCat:
        case kHeaderStyle:
        case kHeaderName:
        case kNotACell:      TCursor::Set( kArrowCursor );    break;
     }
}

// -------------------------------------------------------------------------
//
//          Scrolling
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

//---------------------------------------------------------------------------
// WMHScroll
//
// Keeps track of the selected note
//---------------------------------------------------------------------------
void TBrowGrid::WMHScroll( RTMessage Msg)
{
    // send the corrected version off to basegrid
    TBaseGrid::WMHScroll( Msg );

    // save state
    TSysState::currentNoteIndex = vScrollPos;
}

//---------------------------------------------------------------------------
// WMVScroll
//
// Keeps track of the selected note
//---------------------------------------------------------------------------
void TBrowGrid::WMVScroll( RTMessage Msg)
{
    // send the corrected version off to basegrid
    TBaseGrid::WMVScroll( Msg );

    // save state
    TSysState::currentNoteIndex = vScrollPos;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////// FOCUS ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//*****************************************************************************
//set focus to first visible note title
//*****************************************************************************
void TBrowGrid::GetDefaultFocusCell( int &row, int &col )
{
if( rowCnt > frozenRows )
    row= frozenRows + vScrollPos;                //focus on first visible row!
else
    row= 0;

col= 1;
}

//*****************************************************************************

//*****************************************************************************
void TBrowGrid::MousKey( TMousKey key )
{
TMousGrid::MousKey( key );
}

//*****************************************************************************
// only note titles are selectable
//*****************************************************************************
BOOL TBrowGrid::IsCellSelectable( int row, int col )
{
return CellType( row, col ) == kNoteName;
}


//***************************************************************************
//
//                                 CellType
//
//***************************************************************************

//---------------------------------------------------------------------------
// CellType
//
// Return the type of the cell; handles the case where row or col is -1,
// as it would be if XYWithin were false
//---------------------------------------------------------------------------
TBrowGrid::TCellType TBrowGrid::CellType( int row, int col )
{
    switch( row ) {

        case -1:

            return kNotACell;

        case 0:

            switch( col ) {

                case -1: return kNotACell;
                case 0:  return kHeaderStyle;
                case 1:  return kHeaderName;
                default:

                    // make sure it's in range
                    if (col - kNonCatCols < itsNumCats)
                        return kHeaderCat;
                    else
                        return kNotACell;
            }

        default:

            if (IsNoteRow( row )) {

                switch( col ) {

                    case -1: return kNotACell;
                    case 0:  return kNoteStyle;
                    case 1:  return kNoteName;
                    default:

                        // make sure it's in range
                        if (col - kNonCatCols < itsNumCats)
                            return kNoteCat;
                        else
                            return kNotACell;
                }
            }
            else {

                switch( col ) {

                    case -1: return kNotACell;
                    case 0:  return kTitleStyle;
                    case 1:  return kTitleName;
                    default:

                        // make sure it's in range
                        if (col - kNonCatCols < itsNumCats)
                            return kTitleCat;
                        else
                            return kNotACell;
                }
            }
    }
}


//---------------------------------------------------------------------------
// ColType
//
// Return the type of the col; handles the case where col is -1, as it would
// be if XYWithin were false
//---------------------------------------------------------------------------
TBrowGrid::TColType TBrowGrid::ColType( int col )
{
    switch( col ) {

        case -1:   return kNotACol;
        case 0:    return kStyleCol;
        case 1:    return kNameCol;
        default:

            // make sure it's in range
            if (col - kNonCatCols < itsNumCats)
                return kCatCol;
            else
                return kNotACol;
    }
}

//---------------------------------------------------------------------------
// RowType
//
// Return the type of the row; handles the case where row is -1, as it would
// be if XYWithin were false
//---------------------------------------------------------------------------
TBrowGrid::TRowType TBrowGrid::RowType( int row )
{
    if (IsNoteRow( row )) {

        switch( row ) {

            case -1:   return kNotARow;
            case 0:    return kHeaderRow;
            default:   return kNoteRow;
        }
    }
    else {

        switch( row ) {

            case -1:   return kNotARow;
            case 0:    return kHeaderRow;
            default:   return kTitleRow;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
/////////////////////// TBrowGridData ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
    TBrowGridData::TBrowGridData( void ) { };
    TBrowGridData::~TBrowGridData( void ) { };

    int TBrowGridData::operator < ( const TBrowGridData &other ) const { return( *catP < *other.catP ); }
    int TBrowGridData::operator == ( const TBrowGridData &other ) const { return( *catP == *other.catP ); }

    classType TBrowGridData::isA() const { return 13; }    //FIXME assign classid
    char * TBrowGridData::nameOf() const { return( "BrowGridData" ); }
    hashValueType TBrowGridData::hashValue() const { return 0; }
    int TBrowGridData::isEqual( const Object  &other ) const { return( *this == other ); }
    int TBrowGridData::isLessThan( const Object &other ) const
        { return( ((TBrowGridData &) *this) < ((TBrowGridData &) other) ); }
    void TBrowGridData::printOn( ostream & ) const { }


//*****************************************************************************

//*****************************************************************************
void TBrowGrid::SetDefaultPrintRange( void )
{
TBaseGrid::SetDefaultPrintRange();
}

//*****************************************************************************
// note selection set changed
//*****************************************************************************
void TBrowGrid::Listen( TSnoopMsg aMsg, RObject )
{
switch( aMsg )
    {
    case kNoteSetChg:
    case kCatVis:
    case kCatNew:
    case kCatDel:

        // delete the old information
        itsCatListP->flush();
        itsCatTableP->flush(TXArray::kDelete);

        // initialize
        Reset();
        break;
    default:
        break;
    }
}

