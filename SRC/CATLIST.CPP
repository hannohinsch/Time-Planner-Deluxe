//*****************************************************************************
// TCatList
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "catlist.h"

#ifndef __cat__
#include "cat.h"
#endif
#ifndef __cattree__
#include "cattree.h"
#endif
#ifndef __classid__
#include "classid.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __error__
#include "error.h"
#endif



//**************************************************************************
//
// Constructors
//
//**************************************************************************


//--------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------
TCatList::TCatList( void )
{
pCatArray= new TXArray(10, 10, TXArray::kNoSort);
}

//--------------------------------------------------------------------------
// copy constructor
//--------------------------------------------------------------------------
TCatList::TCatList( RTCatList catList )
{

int n= catList.getItemsInContainer();
pCatArray= new TXArray(n,10, TXArray::kNoSort);

for( int i=0; i < n; i++ )
    {
	RTCat cat= catList[i];
	CHKBUG( ERR_CAT,  cat.HealthCheck() );

    pCatArray->add( cat );
    }

}

//--------------------------------------------------------------------------
// Constructor/Address Selector
//
// Copy those categories in a catTree whose addresses are marked
// into a new catlist
//--------------------------------------------------------------------------
TCatList::TCatList( RTCatTree catTree, TAddressInit )
{

pCatArray= new TXArray(10,10, TXArray::kNoSort);

RTCat cat= catTree.FindCatInTopLevel( "<addresses>" );

if( cat == NOOBJECT )
    return;

RTCatTree addrCats= *cat.itsSubCatsP;

int n= addrCats.getItemsInContainer();
for( int i=0; i < n; i++ )
    {
    RTCat cat= addrCats[i];
	CHKBUG( ERR_CAT,  cat.HealthCheck() );

    // is there a marked address?
    if (cat.itsAddressP != NULL)
        {
        if( cat.itsAddressP->GetMark() )
            pCatArray->add( cat );
        }
    }
}

//--------------------------------------------------------------------------
// constructor
// make a flat catlist containing all of the cats in catTree
//--------------------------------------------------------------------------
TCatList::TCatList( RTCatTree catTree, BOOL doOnlyTopLevel )
{
pCatArray= new TXArray(10, 10, TXArray::kNoSort);

AddRecurse( catTree, doOnlyTopLevel );

}

//--------------------------------------------------------------------------
// given text, create a catlist based on keyword matches
// check each global cat for match, include if appropriate
// recurse! down subcats
//--------------------------------------------------------------------------
TCatList::TCatList( const char *text )
{
pCatArray= new TXArray(10, 10, TXArray::kNoSort);

int n= (TSysState::pCategories)->getItemsInContainer();
for( int i=0; i < n; i++ )
    {
    RTCat cat= (*TSysState::pCategories)[i];
	CHKBUG( ERR_CAT,  cat.HealthCheck() );
    KeyRecurse( cat, text );
    }

}

//--------------------------------------------------------------------------
// Destructor
//--------------------------------------------------------------------------
TCatList:: ~TCatList( void )
{
delete pCatArray;
}

//--------------------------------------------------------------------------
// AddRecurse
//
// Add all the cats in this level of the tree to the list and then
// recurse down the tree
//--------------------------------------------------------------------------
void TCatList::AddRecurse( RTCatTree catTree, BOOL doOnlyTopLevel )
{
int n= catTree.getItemsInContainer();
for( int i=0; i < n; i++ )
    {
    RTCat cat= (RTCat) catTree[i];
	CHKBUG( ERR_CAT,  cat.HealthCheck() );

    pCatArray->add( cat );
    if( !doOnlyTopLevel && 0 < cat.itsSubCatsP->getItemsInContainer() )
        {
        AddRecurse( *cat.itsSubCatsP, FALSE );
        }
    }
}

//--------------------------------------------------------------------------
// KeyRecurse
//
// Check to see if the text is a keyword for the category or the cat's
// children
//--------------------------------------------------------------------------
void TCatList::KeyRecurse( RTCat cat, const char *text)
{

if( cat.KeyMatch( text ) )
    {
    pCatArray->add( cat );
    }

// now check each sub
int n= cat.itsSubCatsP->getItemsInContainer();
for( int i=0; i < n; i++ )
    {
    RTCat subCat= (*cat.itsSubCatsP) [i];
	CHKBUG( ERR_CAT,  cat.HealthCheck() );

    KeyRecurse( subCat, text );
    }

}


//**************************************************************************
//
// Miscellaneous Operators
//
//**************************************************************************

//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
BOOL TCatList::HealthCheck( void )
{
BOOL rc= TRUE;

int n= pCatArray->getItemsInContainer();
for( int i=0; i < n; i++ )
	{
    RTCat cat= (RTCat) (*pCatArray)[i];

    rc= cat.HealthCheck();

    if( !rc )
        break;
    }


return rc;
}

//--------------------------------------------------------------------------
// figure out if two catlists intersect
// 1. if kCatMatchAny, then any intersection returns TRUE
// 2. if kCatMatchAll, then all categories in *this must also be in NoteList
// 3. the cats in *this are recursively descended in a search for intersections
//--------------------------------------------------------------------------
BOOL TCatList::Match( PTCatList pNoteList, TMatchStyle matchStyle )
{
BOOL orFound= FALSE;
BOOL andFound= TRUE;
BOOL goOn= TRUE;

int n= pCatArray->getItemsInContainer();
for( int i=0; i < n && goOn; i++ )
    {
    RTCat catA= (RTCat) (*pCatArray)[i];

    int m= pNoteList->pCatArray->getItemsInContainer();
    BOOL foundJ= FALSE;
    for( int j=0; j < m && !foundJ; j++)
        {
        RTCat catB= (RTCat) (*pNoteList->pCatArray)[j];

        foundJ= (catA == catB) || catA.itsSubCatsP->IsCatInTree( catB );
        }

    andFound &= foundJ;
    orFound |= foundJ;

    //short circuit
    if( (matchStyle == kCatMatchAny && orFound) ||
        (matchStyle == kCatMatchAll && !andFound) )
            goOn= FALSE;

    }


if( matchStyle == kCatMatchAny )
    return orFound;
else if( matchStyle == kCatMatchAll )
    return andFound;
else
    return FALSE;
}


//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
BOOL TCatList::IsCatInList( RTCat cat )
{
BOOL found= FALSE;

int n= pCatArray->getItemsInContainer();

for( int i= 0; i < n && !found; i++ )
    {
    RTCat listCat= (RTCat) (*pCatArray)[i];

    found= (cat == listCat);
    }

return found;
}

//*****************************************************************************
// IsCatOrChildInList
//
// Check to see if any category in the catTree is in the catlist
//*****************************************************************************
BOOL TCatList::IsCatOrChildInList( RTCatTree catTree )
{
    int n = catTree.getItemsInContainer();
    for (int i = 0; i < n; i++) {

        RTCat cat = catTree[i];

        if (IsCatInList( cat )) {

            return( TRUE );
        }
        else {

            if (IsCatOrChildInList( *cat.itsSubCatsP )) {

                return( TRUE );
            }
        }
    }

    return( FALSE );
}

//*****************************************************************************
// IsCatOrChildInList
//
// Check to see if this category or any of its descendents is in the catlist
//*****************************************************************************
BOOL TCatList::IsCatOrChildInList( RTCat cat )
{
    if (IsCatInList( cat )) {

        return TRUE;
    }
    else {

        return IsCatOrChildInList( *cat.itsSubCatsP );
    }
}

//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
void TCatList::FmtCatNames( char *pBuf, int bufLen, char sepChar )
{
const char *s;
const char *pTail= ", ...";
char sepStr[]=" + ";

sepStr[ 1 ]= sepChar;

*pBuf='\0';

int n= pCatArray->getItemsInContainer();

for( int i=0; i < n; i++ )
    {
    RTCat cat= (RTCat) (*pCatArray)[i];

    s= cat.itsShortName.Addr();
    if( (strlen( pBuf ) + strlen(sepStr) + strlen( s ) + strlen( pTail ) ) < bufLen )
        {
        if( strlen( pBuf ) > 0 )
            {
            strcat( pBuf, sepStr );
            strcat( pBuf, s );
            }
        else //guaranteed to have room
            strcat( pBuf, s );
        }
    else
        {
        strcat( pBuf, pTail);
        break;
        }
    }

}


//*****************************************************************************
// AddVisibleCats
//
// Add to the CatList all categories from the CatTree which are visible
//*****************************************************************************
void TCatList::AddVisibleCats( PTCatTree catTreeP )
{

    int n = catTreeP->getItemsInContainer();
    for (int i = 0; i < n; i++) {

        RTCat cat = (*catTreeP)[i];

        add( cat );
        if (cat.itsShowingSubs) {

            AddVisibleCats( cat.itsSubCatsP );
        }
    }
}

//*****************************************************************************
// replace all the cats in this list with the equivalent cats
// from the new tree.  Used in merges.
//*****************************************************************************
void TCatList::SwitchCats( PTCatTree catTreeP )
{
int n= pCatArray->GetCount();

PTXArray newArrayP= new TXArray( n, 10 );

for (int j = 0; j < n; j++)
    {
    TSafStr s;
    RTCat oldCat = (RTCat) (*pCatArray)[j];
    oldCat.GetShortName( s );
    RTCat newCat = catTreeP->FindCatInTree( s );
    if (((RObject) newCat) != NOOBJECT)
        {
        newArrayP->add( newCat );
        }
    else
        {
        TMessageBox::Ok( TPimFrame::pFrame,
            "Can't find new folder.",
            "Merge Error" );
        }
    }

//now switch arrays
delete pCatArray;
pCatArray= newArrayP;

CHKBUG( ERR_GEN, HealthCheck() );
}

// --------------------------------------------------------------
// Union
//
// Add every category from the newCatList that isn't already
// in this catlist to this catlist.
// --------------------------------------------------------------
void TCatList::Union( PTCatList newCatList )
{
    // loop through all categories in the newCatList
    int numCats = newCatList->getItemsInContainer();
    for (int i = 0; i < numCats; i++) {

        RTCat cat = (*newCatList)[i];

        // if this category isn't already in 'this' then add it
        if (!IsCatInList( cat )) {

           add( cat );
        }
    }
}

//--------------------------------------------------------------------------
// THIS IS IMPORTANT: before any category is deleted, all catlists in the system
// need to be notified by sending a kCatDel message!
//--------------------------------------------------------------------------
void TCatList::Listen( TSnoopMsg aMsg, RObject aObject )
{
switch( aMsg )
    {
    case kCatDel:
        CHKBUG( ERR_GEN, aObject != NOOBJECT );
        CHKBUG( ERR_GEN, aObject.isA() == IDCat );
        detach( (RTCat) aObject );
        break;

    default:
        break;
    }

}

//**************************************************************************
//
// Array Operators
//
//**************************************************************************


//--------------------------------------------------------------------------
// operator []
//--------------------------------------------------------------------------
RTCat TCatList::operator []( int index )
{
return (RTCat) (*pCatArray) [index] ;
}

//-----------------------------------------------------------------------
// Find
//-----------------------------------------------------------------------
int TCatList::find( const RTCat cat )
{
    return( pCatArray->getIndex( cat ));
}


//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
void TCatList::detach( RTCat cat )
{
    pCatArray->detach( cat );
}

//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
void TCatList::add( RTCat cat )
{
    pCatArray->add( cat );
}

//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
void TCatList::flush( TXArray::TDeleteStyle dt )
{
    pCatArray->flush( dt );
}

//**************************************************************************
//
// Object Nonsense
//
//**************************************************************************
classType TCatList::isA( void ) const           { return IDCatList;       }
char * TCatList::nameOf( void ) const           { return "TCatList";      }
hashValueType TCatList::hashValue( void ) const { return 0;               } //FIXME
int TCatList::isEqual(const Object & obj) const { return( this == & obj );}
void TCatList::printOn(ostream far&) const      {                         } // FIXME

//**************************************************************************
//
// I/O
//
//**************************************************************************

//--------------------------------------------------------------------------
// stream support
//--------------------------------------------------------------------------
PTStreamable TCatList::build( void )
{
return( new TCatList( streamableInit ) );
}

TStreamableClass RegTCatList( "TCatList", TCatList::build, __DELTA(TCatList) );

//--------------------------------------------------------------------------
// Read
//--------------------------------------------------------------------------
Pvoid TCatList::read( Ripstream is )
{
int catSize;
int version;
int i;

is >> version;

//catlist
is >> catSize;

if( version == 2 ) //category objects
    {
    pCatArray= new TXArray( catSize, 10, TXArray::kNoSort );

    for( i= 0; i < catSize; i++ )
        {
        PTCat pCat;

        is >> pCat;
        pCatArray->add( *pCat );

        }
    }
else
	SYSERR( ERR_VER );

return this;
}

//--------------------------------------------------------------------------
// Write
//--------------------------------------------------------------------------
void TCatList::write( Ropstream os )
{

os << (int) 2;      //version

//catList
os << pCatArray->getItemsInContainer();


int n= pCatArray->getItemsInContainer();

// errors are caught locally so we can clean up, and signalled upward
CATCHBUF cBuf;
LPCATCHBUF pOldCBuf= TError::SetCatchBuf( cBuf );
TErrCode rc= (TErrCode) Catch( cBuf );
if( rc == 0 )
    {
    for( int i=0; i < n; i++ )
        {
        RTCat cat= (RTCat) (*pCatArray)[i];

        CHKBUG( ERR_CAT, cat.HealthCheck() );       //check for tree membership

        os << &cat;
        }
     }

//restore saved error handler
TError::SetCatchBuf( pOldCBuf );

//signal error to caller
if( rc )
    SYSTHROW( rc );
}

