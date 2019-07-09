//*****************************************************************************
// folder tabs for views
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "notetab.h"
#ifndef __paint__
#include "paint.h"
#endif
#ifndef __rawgrid__
#include "rawgrid.h"
#endif
#ifndef __catlist__
#include "catlist.h"
#endif
#ifndef __cat__
#include "cat.h"
#endif
#ifndef __cattree__
#include "cattree.h"
#endif
#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __filter__
#include "filter.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __cattable__
#include "cattable.h"
#endif

_CLASSDEF( TLocalCatTable )

class _CLASSTYPE TLocalCatTable
{
    public:

    TLocalCatTable( PTCatList );
    virtual ~TLocalCatTable( void );
    void AddNotes( RTNoteSet noteSet );
    Pvoid BinarySearch( long key )
        { return itsCatTableP->BinarySearch( key ); }
    PTNoteSet GetOtherNotes( void )
        { return itsOtherNotesP; }

    private:

    int        itsCatTableSize;
    PTCatTable itsCatTableP;
    PTNoteSet  *itsNoteSetTable;
    PTNoteSet  itsOtherNotesP;
};

/////////////////////////////////////////////////////////////////////////////
/////////// TNoteTotal ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//*****************************************************************************

//*****************************************************************************
TNoteTotal::TNoteTotal( int level )
{
itsLevel= level;
itsValue= 0L;
itsCatP= NULL;
itsSampleNoteP= NULL;
}

//*****************************************************************************

//*****************************************************************************
void TNoteTotal::SetSample( TNoteOrder order, PTNote noteP )
{
    itsSampleNoteP = noteP;
    itsNoteOrder   = order;
}

//*****************************************************************************

//*****************************************************************************
void TNoteTotal::SetCat( PTCat catP )
{
    itsCatP = catP;
}


//*****************************************************************************

//*****************************************************************************
TNoteTotal::~TNoteTotal( void )
{
}

//*****************************************************************************

//*****************************************************************************
void TNoteTotal::Sum( RTNote note )
{
    itsValue+= note.GetValue();

    //find first
    if( itsFirstTime.HasDate() )
        {
        RTTime start = note.GetStart();
        if( start.HasDate() && itsFirstTime > start )
            itsFirstTime= start;
        }
    else
        itsFirstTime= note.GetStart();

    //find last
    if( itsLastTime.HasDate() )
        {
        RTTime end = note.GetEnd();
        if( end.HasDate() && itsLastTime < end )
            itsLastTime= end;
        }
    else
        itsLastTime= note.GetEnd();

    itsDuration.PlusEquals( note.GetDuration() );
    itsWorkTime.PlusEquals( note.GetWorkTime() );
}

//*****************************************************************************

//*****************************************************************************
void TNoteTotal::Report( RTDrawInfo di )
{
    TSafStr     str;
    TColor      color;
    TNoteStyle  style;
    BOOL        useText, useColor, useStyle;

    color.Set( kRed );
    style = kTaskStyle;
    switch( itsLevel )
        {
        case 3:

            ReportElement( 3, str, useText, color, useColor, style, useStyle );
            break;

        case 2:

            ReportElement( 2, str, useText, color, useColor, style, useStyle );
            break;

        case 1:

            ReportElement( 1, str, useText, color, useColor, style, useStyle );
            break;
        }

    // draw the text
    RECT r = di.cellRect;
    r.left += 20;
    di.pPainter->Text( di, str, kNoHilite,
        kSingleLine | DT_LEFT, kItalicFont, &r );
    if (useColor || useStyle)
        {
        r.left += LOWORD( GetTextExtent( di.hdc, str, strlen( str )));
        di.pPainter->DrawScreenMark( di.hdc, r,
            color, style );
        }
}

//*****************************************************************************
// ReportElement
//
//  Based on the sample note, return information
//  about the current subtotal
//*****************************************************************************
void TNoteTotal::ReportElement( int level,
    RTSafStr text, BOOL &useText,
    TColor &color, BOOL &useColor,
    TNoteStyle &style, BOOL &useStyle )
{
    char buf[100];
    TNoteOrder order = TProfile::NoteOrder[level - 1];
    useText  = FALSE;
    useColor = FALSE;
    useStyle = FALSE;
    TSafStr  temp;

    if (TSysState::CatGrouping != kNoCatGrouping)
        {
        if (itsCatP)
            {
            itsCatP->GetShortName( temp );
            text.Set( "Subtotal " );
            text.Cat( temp );
            }
        else
            {
            text.Set( "Subtotal Unfiled" );
            }
        useText = TRUE;
        return;
        }

    if (!itsSampleNoteP)
        {
        useText = TRUE;
        text.Set( "No notes for subtotal" );
        return;
        }

    switch ( order )
        {
        case kNoteOrder_Color:

            text.Set( "All Color     " );
            useColor = TRUE;
            color = itsSampleNoteP->GetColor();
            break;

        case kNoteOrder_Date:

            TIntlFmt::FormatDate(
                itsSampleNoteP->GetStart(), kShortDate, buf );
            text.Set( "All Start " );
            text.Cat( buf );
            useText = TRUE;
            break;

        case kNoteOrder_Duration:

            TDuration duration = itsSampleNoteP->GetDuration();
            itsSampleNoteP->GetDuration().Format(
                itsSampleNoteP->GetTimeUnit(),
                TProfile::bMixTimeFmtUnits, buf );
            text.Set( "All Elapsed " );
            text.Cat( buf );
            useText = TRUE;
            break;

        case kNoteOrder_Type:

            text.Set( "All Shape     " );
            style = itsSampleNoteP->GetStyle();
            useStyle = TRUE;
            break;

        case kNoteOrder_Alpha:

            char temp[50];
            text.Set( "All  " );
            itsSampleNoteP->GetText().Get( temp, 15 );
            text.Cat( temp );
            useText = TRUE;
            break;
        }
}




/////////////////////////////////////////////////////////////////////////////
/////////// TNoteTable ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
// Constructor
//*****************************************************************************
TNoteTable::TNoteTable( void ) :
    TXArray( 10, 10, kNoSort )
{
}

//*****************************************************************************
// Destructor
//*****************************************************************************
TNoteTable::~TNoteTable( void )
{
}

//*****************************************************************************
// Add a header row to the table
//*****************************************************************************
int TNoteTable::Add( char *header )
{
    PTNoteRow noteRowP = new TNoteRow(
        kNoteTableHeader, header, NULL, NULL );
    TXArray::Add( *noteRowP );
    return 1;
}

//*****************************************************************************
// Add a note row to the table
//*****************************************************************************
int TNoteTable::Add( PTNote noteP )
{
    PTNoteRow noteRowP = new TNoteRow(
        kNoteTableNote, "", noteP, NULL );
    TXArray::Add( *noteRowP );
    return 1;
}

//*****************************************************************************
// Add a total row to the table
//*****************************************************************************
int TNoteTable::Add( PTNoteTotal totalP )
{
    PTNoteRow noteRowP = new TNoteRow(
        kNoteTableTotal, "", NULL, totalP );
    TXArray::Add( *noteRowP );
    return 1;
}

//*****************************************************************************
// Add
//
//  Add everything in the global note set, grouping
//  by the categories in catListP
//*****************************************************************************
int TNoteTable::Add( PTCatList catListP, BOOL useSubtotal )
{
int rowCount = 0;

// add notes to the catTable. The CatTable
// is ordered by categories, and each
// category has a list of notes in that category
PTLocalCatTable catTableP = new TLocalCatTable( catListP );
catTableP->AddNotes( *TSysState::pSelNotes );

// now that there are notesets for each category,
// fill in the NoteTable
int numCats = catListP->getItemsInContainer();
for (int i = 0; i < numCats; i++)
    {
    RTCat cat = (*catListP)[i];
    PTNoteSet noteSetP = (PTNoteSet) catTableP->BinarySearch( (long) &cat );

    // the category doesn't appear in the cat table
    if (((int) noteSetP) == KEYNOTFOUND)
        {
        SYSERR_STR( "Add: didn't find noteset" );
        continue;
        }

    // no notes fell into this category (possible
    // only if the user has chosen to group by a
    // selected set of categories)
    else if (noteSetP == NULL)
        {
        continue;
        }

    // add rows to the note table
    else
        {
        rowCount += Add( noteSetP, &cat, TRUE, useSubtotal );
        }
    }

// add the "other" notes
rowCount += Add( catTableP->GetOtherNotes(), NULL, TRUE, useSubtotal );

// delete the catTable
delete catTableP;

return rowCount;
}

//*****************************************************************************
// Add
//
//  Add everything in the selected note set, grouping
//  by the category in catP. 
//*****************************************************************************
int TNoteTable::Add( PTNoteSet noteSetP, PTCat catP,
    BOOL useHeader, BOOL useSubtotal )
{
    int numNotes = noteSetP->getItemsInContainer();

    // if there are no notes in this category, then
    // don't put it in the list
    if (numNotes == 0) return 0;

    int rowCount = 0;

    // fill in header row
    if (useHeader)
        {
        if (catP)
            {
            TSafStr catName;
            if( catP->GetCatType() == kAddressCatType )
                catP->GetShortName( catName );      //show names w/o root
            else
                catP->GetFullName( catName, TRUE );

            rowCount += Add( SAF2CP( catName ));
            }
        else
            {
            rowCount += Add( "Other" );
            }
        }

    // fill in note rows and total up for subtotal
    PTNoteTotal subP = new TNoteTotal( 1 );
    for (int j = 0; j <  numNotes; j++)
        {
        RTNote note = (*noteSetP)[j];
        rowCount += Add( &note );
        subP->Sum( note );
        }

    // fill in subtotal row
    if (useSubtotal)
        {
        subP->SetCat( catP );
        rowCount += Add( subP );
        }
    else
        {
        delete subP;
        }

    return rowCount;
}


//*****************************************************************************
// AddWithGrouping
//
//  Add notes to the note table, organizing them into
//  groups, based on the CatGrouping option selected
//  in the Group Categories dialog. If this is
//  "selected", then use GroupCatListP as the
//  category list. If it's "automatic" then use the
//  pSelCat's, unless there is exactly 1 pSelCat,
//  in which case, use the subcats of the pSelCat
//  to group the notes. (unless, of course, its "All Selected",
//  in which case use only the top level folders.)
//
//  Got that? 
//*****************************************************************************
int TNoteTable::AddWithGrouping( BOOL useSubtotal )
{
if (TSysState::CatGrouping == kSelectedCatGrouping)
    {
    return Add( TSysState::GroupCatListP, useSubtotal );
    }
else
    {
    PTCatList catListP;
    int rowCount = 0;
    if (TSysState::pFilter->IsAllSelected())
        {
        // TRUE below to indicate we want only top level cats
        catListP = new TCatList( *TSysState::pCategories, TRUE );
        }
    else
        {
        catListP = new TCatList( *TSysState::pFilter->GetSelCats());
        }

    if (catListP->getItemsInContainer() == 1)
        {
        RTCat cat = (RTCat) (*catListP)[0];
        if (cat.HasSubCats())
            {
            PTCatTree catTree = cat.GetSubCats();
            PTCatList listP = new TCatList( *catTree );
            rowCount = Add( listP, useSubtotal );
            listP->flush( TXArray::kNoDelete );
            delete listP;
            }
        else
            {
            rowCount = Add( catListP, useSubtotal );
            }
        }
    else
        {
        rowCount = Add( catListP, useSubtotal );
        }

    delete catListP;
    return rowCount;
    }
}


//*****************************************************************************
// Constructor
//*****************************************************************************
TLocalCatTable::TLocalCatTable( PTCatList catListP )
{
    itsCatTableSize = catListP->getItemsInContainer();
    itsNoteSetTable = (PTNoteSet *)new PTNoteSet[itsCatTableSize];
    for (int i = 0; i < itsCatTableSize; i++) {

        itsNoteSetTable[i] = new TNoteSet;
    }

    itsCatTableP = new TCatTable( catListP, (Pvoid*) itsNoteSetTable );
    itsOtherNotesP = new TNoteSet;
}

//*****************************************************************************
// Destructor
//*****************************************************************************
TLocalCatTable::~TLocalCatTable( void )
{
    for (int i = 0; i < itsCatTableSize; i++) {

        delete itsNoteSetTable[i];
    }
    delete itsOtherNotesP;
    delete itsCatTableP;
    delete itsNoteSetTable;
}

//*****************************************************************************
// AddNotes
//
//  For each note in noteset, this method runs through
//  the catlist of the note and throws the note into
//  the noteset for each cat in the CatTable.
//
//  If none of the note's cats appear as keys in the
//  KeyedList, then add the note to the special
//  noteset "itsOtherNotes."
//*****************************************************************************
void TLocalCatTable::AddNotes( RTNoteSet noteSet )
{
    for (int n = 0; n < noteSet.getItemsInContainer(); n++) {

        RTNote note = noteSet[n];
        RTCatList catList = note.GetCatList();

        // we assume that the note will not fall
        // into any specific category and should
        // therefore be added to itsOtherNotes
        BOOL addToOthers = TRUE;

        for (int i = 0; i < catList.getItemsInContainer(); i++) {

            RTCat cat = (RTCat) catList[i];
            PTNoteSet noteSetP = (PTNoteSet)
                BinarySearch( (long) &cat );

            // the key didn't match anything in the set
            if (noteSetP == (PTNoteSet) KEYNOTFOUND) {

                // this note will automatically
                // go into itsOtherNotes if it
                // doesn't go into some category
            }

            // the key matched, but there isn't any
            // noteset in the table yet
            else if (noteSetP == NULL) {

                SYSERR_STR( "AddNotes: no noteset" );
            }

            // just add the note to the table
            else {

                //don't include more than once
                if( !noteSetP->HasNote( note ) )
                    {
                    noteSetP->add( note );
                    addToOthers = FALSE;
                    }
            }
        }

        // if it didn't go into any category,
        // then dump it into itsOtherNotes
        if (addToOthers) {

             itsOtherNotesP->add( note );
        }
    }
}
