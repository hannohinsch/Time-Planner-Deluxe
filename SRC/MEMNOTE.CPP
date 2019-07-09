//*****************************************************************************
// memorized note object and set
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "safstr.h"
#include "note.h"
#include "memnote.h"

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TMemorizedNote::TMemorizedNote( void )
{
    itsNoteP = NULL;
}

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TMemorizedNote::TMemorizedNote( PTNote pNote, const char *name )
{
    itsNoteP = new TNote( *pNote );
    itsName.Set( name );
}

// -------------------------------------------------
// Destructor
// -------------------------------------------------
TMemorizedNote::~TMemorizedNote( void )
{
    if (itsNoteP != NULL) {

       delete itsNoteP;
       itsNoteP = NULL;
    }
}

// -------------------------------------------------
// IsEqual
// -------------------------------------------------
int TMemorizedNote::isEqual( const Object &other ) const
{
    return( ((RTMemorizedNote) *this).itsName == ((RTMemorizedNote) other).itsName );
}

// -------------------------------------------------
// isLessThan
// -------------------------------------------------
int TMemorizedNote::isLessThan( const Object &other ) const
{
    return( ((RTMemorizedNote) *this).itsName < ((RTMemorizedNote) other).itsName );
}

//*****************************************************************************
// stream support
//*****************************************************************************
PTStreamable TMemorizedNote::build( void )
{
return( new TMemorizedNote( streamableInit ) );
}

TStreamableClass RegTMemorizedNote( "TMemorizedNote", TMemorizedNote::build, __DELTA(TMemorizedNote) );

//*****************************************************************************

//*****************************************************************************
Pvoid TMemorizedNote::read( Ripstream is )
{
int version;

is >> version;

if( version == 1 )
	{
    is >> itsNoteP;
    is >> itsName;
	}
else
	SYSERR( ERR_VER );

return this;
}


//*****************************************************************************

//*****************************************************************************
void TMemorizedNote::write( Ropstream os )
{

// errors are caught locally so we can clean up, and signalled upward
CATCHBUF cBuf;
LPCATCHBUF pOldCBuf= TError::SetCatchBuf( cBuf );
TErrCode rc= (TErrCode) Catch( cBuf );
if( rc == 0 )
    {
    os << (int) 1;  //version

    os << itsNoteP;
    os << itsName;
    }

//restore saved error handler
TError::SetCatchBuf( pOldCBuf );

//if error, signal to caller
if( rc )
    SYSTHROW( rc );
}

//*****************************************************************************
// HealthCheck
//
//  Makes sure the note has a valid name and note
//*****************************************************************************
BOOL TMemorizedNote::HealthCheck( void )
{
    if (!itsName.HealthCheck())
        return FALSE;

    if (!itsNoteP->HealthCheck())
        return FALSE;

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//////////////////////// TMemNoteSet ////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//*****************************************************************************

//*****************************************************************************
RTMemorizedNote TMemNoteSet::FindNote( RTMemorizedNote note )
{
int n= getItemsInContainer();
for( int i= 0; i < n; i++ )
    {
    if( note.isEqual( (RTMemorizedNote) (*this)[i] ))
        return (RTMemorizedNote) (*this)[i];
    }

return (RTMemorizedNote) NOOBJECT;
}

//*****************************************************************************
// stream support
//*****************************************************************************
PTStreamable TMemNoteSet::build( void )
{
return( new TMemNoteSet( streamableInit ) );
}

TStreamableClass RegTMemNoteSet( "TMemNoteSet", TMemNoteSet::build, __DELTA(TMemNoteSet) );

//*****************************************************************************

//*****************************************************************************
Pvoid TMemNoteSet::read( Ripstream is )
{
int version, num;

is >> version;

if( version == 2 )
    {
    is >> num;

    for( int i= 0; i < num; i++ )
		{
        PTMemorizedNote pNote;

        is >> pNote;
        add( *pNote );
		}

    }
else if( version == 1 )  // HTP version 1.00
	{
    is >> num;

    for( int i= 0; i < num; i++ )
		{
        PTNote pNote;
        TSafStr name( streamableInit );

        is >> pNote;
        is >> name;

        //early versions of the program (including v1.00) have an
        //explicit blank note in the memorized set.  No more!  So
        //old files need to have the blank note removed.
        if( name != "<Blank Note>" && name != "< Blank Note >" )
            {
            PTMemorizedNote pThing= new TMemorizedNote( pNote, name );

            add( *pThing );
            }

        //MemorizedNote makes copies of the args, so we can dispose originals
        delete pNote;
        }

	}
else
	SYSERR( ERR_VER );

return this;
}


//*****************************************************************************

//*****************************************************************************
void TMemNoteSet::write( Ropstream os )
{

os << (int) 2;  //version

os << getItemsInContainer();

int n= getItemsInContainer();

// errors are caught locally so we can clean up, and signalled upward
CATCHBUF cBuf;
LPCATCHBUF pOldCBuf= TError::SetCatchBuf( cBuf );
TErrCode rc= (TErrCode) Catch( cBuf );
if( rc == 0 )
    {
    for( int i=0; i < n; i++ )
        {
        RTMemorizedNote thing= (RTMemorizedNote) (*this)[i];

        os << &thing;
        }
     }

//restore saved error handler
TError::SetCatchBuf( pOldCBuf );

//signal error to caller
if( rc )
    SYSTHROW( rc );
}

//*****************************************************************************
// HealthCheck
//
//  Make sure all the memorized notes are consistent
//*****************************************************************************
BOOL TMemNoteSet::HealthCheck( void )
{
int n= getItemsInContainer();
for( int i = 0; i < n; i++ )
	{
    RTMemorizedNote note = (RTMemorizedNote) (*this)[i];
    if (!note.HealthCheck())
        return FALSE;
	}

return TRUE;
}

//*****************************************************************************
// ModifyForChangedWorkTime
//
// Check each note in the set to see if it needs
// updating because the user changed the definition
// of worktime (in the holiday or worktime dialog)
//*****************************************************************************
int TMemNoteSet::ModifyForChangedWorkTime( void )
{
    int rc= 0;

    // modify the notes as necessary to match the
    // new work times
    for( int i = 0; i < getItemsInContainer(); i++) {

        RTMemorizedNote note = (RTMemorizedNote) (*this)[i];
        if( note.itsNoteP->ModifyForChangedWorkTime() )
            rc++;
    }

    return rc;
}

//*****************************************************************************

//*****************************************************************************
void TMemNoteSet::UpdateUnitLength( void )
{
    // update the notes
    for( int i = 0; i < getItemsInContainer(); i++) {

        RTMemorizedNote note = (RTMemorizedNote) (*this)[i];
        note.itsNoteP->UpdateUnitLength();
    }
}

