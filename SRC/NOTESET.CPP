//*****************************************************************************
// a set of notes
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "noteset.h"

#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __note__
#include "note.h"
#endif
#ifndef __filter__
#include "filter.h"
#endif
#ifndef __classid__
#include "classid.h"
#endif
#ifndef __error__
#include "error.h"
#endif
#ifndef __memnote__
#include "memnote.h"
#endif
#ifndef __memordlg__
#include "memordlg.h"
#endif
#ifndef  __cursor__
#include "cursor.h"
#endif
#ifndef __xarray__
#include "xarray.h"
#endif
#ifndef __timeconf__
#include "timeconf.h"
#endif

//*****************************************************************************

//*****************************************************************************
TNoteSet::TNoteSet( void )
{
// notesets that are created this way have a
// high likelihood of needing only very few elements
pNoteArray= new TXArray(5,256);
}

//*****************************************************************************
// copy references to selected notes from global noteset into this newly constructed one
//*****************************************************************************
TNoteSet::TNoteSet( RTFilter filter, PTNoteSet noteUniverse )
{
pNoteArray= new TXArray(256,256);

// use the global note set if no specific note set was passed in
if (noteUniverse == NULL) {

   noteUniverse = TSysState::pAllNotes;
}


// prepare the filter's CatTable data in
// order to speed up filter.match
filter.PrepareCatTable();

int n= noteUniverse->getItemsInContainer();
for( int i=0; i < n; i++ )
    {
    RTNote note= ( * noteUniverse ) [ i ];

    BOOL found = filter.Match( note );
    if( found )
        {
        pNoteArray->add( note, TXArray::kNoSort );
        }
    }

// throw away the filter's CatTable (since it's
// based on the selected cats within the filter,
// which may change, it's not safe to keep it around)
filter.DestroyCatTable();

//sort the notes!
pNoteArray->sort();
}

//*****************************************************************************
// copy references to selected notes from global noteset into this newly constructed one
// used in TNoteTab, which needs a fast way to make subset notesets
//*****************************************************************************
TNoteSet::TNoteSet( RTCat cat, PTNoteSet noteUniverse )
{
pNoteArray= new TXArray(256,256);

int n= noteUniverse->getItemsInContainer();
for( int i=0; i < n; i++ )
    {
    RTNote note= ( * noteUniverse ) [ i ];
    RTCatList catList= note.GetCatList();

    if( catList.IsCatOrChildInList( cat ) )
        pNoteArray->add( note, TXArray::kNoSort );
    }

//sort the notes!
pNoteArray->sort();
}

//*****************************************************************************

//*****************************************************************************
TNoteSet:: ~TNoteSet( void )
{
delete pNoteArray;
}

//*****************************************************************************
// public utility function
// NB actually allocates storage for note!
//*****************************************************************************
PTNote TNoteSet::CopyNote( RTNote oldNote )
{
PTNote pNote;

if (!RoomForMore( 1 ))
    {
    return NULL;
    }

pNote= new TNote( oldNote );

TCursor::Busy( TRUE );

pNoteArray->add( *pNote, TXArray::kNoSort );

TCursor::Busy( FALSE );

return pNote;
}

//*****************************************************************************
// public utility function
// NB actually allocates storage for note!
//*****************************************************************************
PTNote TNoteSet::CreateNote( PTWindowsObject aParent )
{
PTNote pNote;

if (!RoomForMore( 1 ))
    {
    return NULL;
    }

int n= TSysState::pMemNotes->getItemsInContainer();
if (n > 0 )
    {
    // pick memorized note
    TMemorizedDialog *dialog = new TMemorizedDialog( aParent, IDD_MEMORIZED_DIALOG, &pNote );
    int rc = GetApplicationObject()->ExecDialog( dialog );

    // if they cancelled then bail out
    if (rc == IDCANCEL )
        return NULL;
    }
 else
    {
    pNote= new TNote();
    }

TCursor::Busy( TRUE );

pNoteArray->add( *pNote, TXArray::kNoSort );

TCursor::Busy( FALSE );

return pNote;
}

//*****************************************************************************
//
//*****************************************************************************
void TNoteSet::detach( RTNote note )
{
pNoteArray->detach( note );
}

//*****************************************************************************

//*****************************************************************************
void TNoteSet::add( RTNote note )
{
pNoteArray->add( note, TXArray::kNoSort );
}

//*****************************************************************************

//*****************************************************************************
void TNoteSet::flush( TXArray::TDeleteStyle dt )
{
pNoteArray->flush( dt );
}

//*****************************************************************************

//*****************************************************************************
BOOL TNoteSet::HasNote( RTNote note )
{
return( NULL!= pNoteArray->HasMember( note ) );
}

//*****************************************************************************
// FindNote
//
//  Does any note "exactly match" the note
//  we're asking about?
//*****************************************************************************
RTNote TNoteSet::FindNote( RTNote note )
{
int n= getItemsInContainer();
for( int i= 0; i < n; i++ )
    {
    if( note.ExactlyMatches( (RTNote) (*pNoteArray)[ i ] ))
        return (RTNote) (*pNoteArray)[ i ];
    }

return (RTNote) NOOBJECT;
}

//*****************************************************************************

//*****************************************************************************
RTNote TNoteSet::operator []( int index )
{
return (RTNote) (*pNoteArray)[ index ];
}

//*****************************************************************************
// sort criteria changed; resort the noteset
//*****************************************************************************
void TNoteSet::Sort( void )
{
TCursor::Busy( TRUE );

pNoteArray->sort();

TCursor::Busy( FALSE );
}

//*****************************************************************************
// do a consistency check
// to date, all that's checked for is orphan cats
//*****************************************************************************
BOOL TNoteSet::HealthCheck( void )
{
BOOL rc= TRUE;

int n= pNoteArray->getItemsInContainer();
for( int i= 0; i < n; i++ )
	{
    RTNote note= (RTNote) (*pNoteArray)[i];

    rc= note.HealthCheck();

    if(!rc)
        break;
	}

return rc;
}

//*****************************************************************************
//
//*****************************************************************************
classType TNoteSet::isA( void ) const
{
return IDNoteSet;
}

char * TNoteSet::nameOf( void ) const
{
return "TNoteSet";
}

hashValueType TNoteSet::hashValue( void ) const
{
return 0; //FIXME
}

int TNoteSet::isEqual(const Object & obj) const
{
return( this == & obj );
}

void TNoteSet::printOn(ostream far&) const
{
//later
}

//*****************************************************************************
// stream support
//*****************************************************************************
PTStreamable TNoteSet::build( void )
{
return( new TNoteSet( streamableInit ) );
}

TStreamableClass RegTNoteSet( "TNoteSet", TNoteSet::build, __DELTA(TNoteSet) );

//*****************************************************************************

//*****************************************************************************
Pvoid TNoteSet::read( Ripstream is )
{
int numNotes;
int version;

is >> version;

if( version == 2 )      //switched to writing note pointers (v1.00)
	{
	//catlist
	is >> numNotes;

    pNoteArray= new TXArray( numNotes, 10 );

	for( int i= 0; i < numNotes; i++ )
		{
        PTNote pNote;

        is >> pNote;

        // assume it's an unordered set
        pNoteArray->add( *pNote, TXArray::kNoSort );
		}
	 }
else if( version == 1 )
	{
	//catlist
	is >> numNotes;

    pNoteArray= new TXArray( numNotes, 10 );

	for( int i= 0; i < numNotes; i++ )
		{
        PTNote pNote= new TNote( streamableInit );

        is >> *pNote;

        // assume it's an unordered set
        pNoteArray->add( *pNote, TXArray::kNoSort );
        }
	 }
else
	SYSERR( ERR_VER );

//now sort it
pNoteArray->sort();

return this;
}

//*****************************************************************************

//*****************************************************************************
void TNoteSet::write( Ropstream os )
{
os << (int) 2;      //version

//noteset
os << pNoteArray->getItemsInContainer();

int n= pNoteArray->getItemsInContainer();

// errors are caught locally so we can clean up, and signalled upward
CATCHBUF cBuf;
LPCATCHBUF pOldCBuf= TError::SetCatchBuf( cBuf );
TErrCode rc= (TErrCode) Catch( cBuf );
if( rc == 0 )
    {
    for( int i=0; i < n; i++ )
        {
        RTNote note= (RTNote) (*pNoteArray)[i];

        os << &note;
        }
    }

//restore saved error handler
TError::SetCatchBuf( pOldCBuf );

//signal error to caller
if( rc )
    SYSTHROW( rc );
}


//*****************************************************************************
// RoomForMore
//
// Is there room for howManyMore notes in the noteset?
//*****************************************************************************
BOOL TNoteSet::RoomForMore( int howManyMore,
    BOOL showError, BOOL showWarning,
    char *altError, char *altWarning )
{
    int numNotes = pNoteArray->getItemsInContainer();

    // still plenty of room
    if (numNotes + howManyMore <= 2980 )
        {
        return TRUE;
        }

    // close to the edge but still ok
    if (numNotes + howManyMore <= 3000)
        {
        if (showWarning)
            {
            KSTR warning( "You are near the 3000 note limit." );

            if (altWarning != NULL)
                {
                warning.Set( altWarning );
                }

            TMessageBox::Ok( TPimFrame::pFrame,
                (char*) (const char*) warning,
                "Too Many Notes" );
            }

        return TRUE;
        }
    

    // over the edge
    if (showError)
        {
        KSTR error( "You are over the 3000 note limit." );

        if (altError != NULL)
            {
            error.Set( altError );
            }

        TMessageBox::Ok( TPimFrame::pFrame,
            (char*) (const char*) error,
            "Too Many Notes" );
        }

    return FALSE;

}

//*****************************************************************************
// ModifyForChangedWorkTime
//
// Check each note in the set to see if it needs
// updating because the user changed the definition
// of worktime (in the holiday or worktime dialog)
//*****************************************************************************
int TNoteSet::ModifyForChangedWorkTime( void )
{
    int rc= 0;

    // modify the notes as necessary to match the
    // new work times
    for( int i = 0; i < getItemsInContainer(); i++) {

        RTNote note = (RTNote) (*pNoteArray)[i];
        if( note.ModifyForChangedWorkTime() )
            rc++;
    }

    return rc;
}

//*****************************************************************************

//*****************************************************************************
void TNoteSet::UpdateUnitLength( void )
{
    // update the notes
    for( int i = 0; i < getItemsInContainer(); i++) {

        RTNote note = (RTNote) (*pNoteArray)[i];
        note.UpdateUnitLength();
    }
}


//*****************************************************************************
// CancelBecauseOfTimeConflict
//*****************************************************************************
BOOL TNoteSet::CancelBecauseOfTimeConflict( RTNote note, RTTime newStart, RTTime newEnd )
{
    PTNoteSet noteUniverseP;

    // a note with no times doesn't conflict
    if (!(newStart.HasDate() || newEnd.HasDate())) {

        return FALSE;
    }

    // make sure we're supposed to check for conflicts in the first place
    switch (TProfile::TimeConflict){

        case kTimeConflict_Never:

            return FALSE;

        case kTimeConflict_Cat:

            noteUniverseP = TSysState::pSelNotes;
            break;

        case kTimeConflict_Always:

            noteUniverseP = TSysState::pAllNotes;
            break;
    }

    TFilter filter( NULL, TRUE, kCatMatchAny, NULL, TRUE );
    if (newEnd.HasDate()) {

        // task
        filter.SetFilterDates( newStart, newEnd );
    }
    else {

        // milestone: the filter must have a valid end time inorder
        // to match notes in the noteset, so we make the end time
        // either 1 minute or 1 day after the start depending on
        // the type of milestone
        if (newStart.HasTime()) {

            filter.SetFilterDates( newStart, newStart.Plus( k1Min, 1 ) );
        }
        else {

            filter.SetFilterDates( newStart, newStart );
        }
    }
    TNoteSet noteSet( filter, noteUniverseP );

    // don't count this note
    noteSet.detach( note );

    BOOL cancelBecauseOfTimeConflict = FALSE;
    if (noteSet.getItemsInContainer() > 0) {

        TTimeConflictDialog *dialogP = new TTimeConflictDialog(
            TPimFrame::pFrame, IDD_TIME_CONFLICT, noteSet );
        cancelBecauseOfTimeConflict = (IDCANCEL == GetApplicationObject()->ExecDialog( dialogP ));
    }

    return cancelBecauseOfTimeConflict;
}
