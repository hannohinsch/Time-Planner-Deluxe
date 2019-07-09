//*****************************************************************************
// TCatTree
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "cattree.h"

#ifndef __cat__
#include "cat.h"
#endif
#ifndef __catlist__
#include "catlist.h"
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
#ifndef __snoop__
#include "snoop.h"
#endif
#ifndef __xarray__
#include "xarray.h"
#endif

//**************************************************************************
//
// Constructors
//
//**************************************************************************
//--------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------
TCatTree::TCatTree( PTCat parentP )
{
pCatArray= NULL;
itsParentP = parentP;
}

//--------------------------------------------------------------------------
// Constructor
//
//     Create a cat tree using only the marked categories in the source tree
//     If all subcats is false, then only marked cats are duplicated
//
//      NB This DUPLICATES categories, which have to be deleted at some time!
//--------------------------------------------------------------------------
TCatTree::TCatTree( PTCatTree pSourceTree, BOOL allSubcats )
{

    itsParentP= NULL;
    pCatArray = NULL;

    if( pSourceTree->pCatArray != NULL )
        {
        pCatArray = new TXArray( 10, 10 );

        int n= pSourceTree->pCatArray->getItemsInContainer();
        for (int i=0; i < n; i++)
            {

            RTCat cat = (RTCat) (*pSourceTree->pCatArray)[i];
            if (( cat != NOOBJECT ) && (allSubcats || cat.itsMarked))
                {

                // copy the source cat and put it into the destination array
                // Each cat will create its own tree of subcats, consisting (again)
                // only of marked categories
                PTCat pNewCat = new TCat( cat, allSubcats );
                pCatArray->add( *pNewCat, TXArray::kNoSort );

                }
            }

        // calculate the itsParentP fields
        FixupParents( NULL );

        // The sort order relies on the parents, so this has to be done
        // after the parent fixup!
        pCatArray->sort();

        }


}


//--------------------------------------------------------------------------
// destructor
//--------------------------------------------------------------------------
TCatTree:: ~TCatTree( void )
{
if( pCatArray )
    delete pCatArray;
}

//**************************************************************************
//
// Finding a category in the tree
//
//**************************************************************************
//--------------------------------------------------------------------------
// NB finds at first level only (doesn't recurse)
//--------------------------------------------------------------------------
RTCat TCatTree::FindCatInTopLevel( const char *name )
{
if( pCatArray == NULL )
    return (RTCat) NOOBJECT;

int n= pCatArray->getItemsInContainer();
for( int i= 0; i < n; i ++ )
    {
    RTCat cat= (RTCat) (*pCatArray)[i];

	if( cat != NOOBJECT )
        {
		if( cat.itsShortName == name  )
            {
			return cat;
            }
        }
    }

return (RTCat) NOOBJECT;
}

//--------------------------------------------------------------------------
// NB finds at first level only (doesn't recurse)
//--------------------------------------------------------------------------
BOOL TCatTree::IsCatInTopLevel( RTCat searchCat )
{
if( pCatArray == NULL )
    return FALSE;

int n= pCatArray->getItemsInContainer();
for( int i= 0; i < n; i ++ )
    {
    RTCat cat= (RTCat) (*pCatArray)[i];

    if( cat == searchCat )
        return TRUE;
    }

return FALSE;
}

//--------------------------------------------------------------------------
// FindCatInTree
//
//  Same as FindCat, but DOES recurse
//--------------------------------------------------------------------------
RTCat TCatTree::FindCatInTree( RTSafStr name )
{
    if( pCatArray == NULL )
        return (RTCat) NOOBJECT;

    int n = pCatArray->getItemsInContainer();
    for (int i = 0; i < n; i++) {

	    RTCat cat= (RTCat) (*pCatArray)[i];
	    if (cat != NOOBJECT) {

            if (cat.itsShortName == name) {

                return cat;
            }
            else {

                RTCat answer = cat.itsSubCatsP->FindCatInTree( name );
                if (answer != NOOBJECT) {

                    return( answer );
                }
            }
        }
    }

    return (RTCat) NOOBJECT;
}

//--------------------------------------------------------------------------
// IsCatInTree
//--------------------------------------------------------------------------
BOOL TCatTree::IsCatInTree( RTCat searchCat )
{
BOOL found= FALSE;

if( pCatArray == NULL )
    return FALSE;

int n= pCatArray->getItemsInContainer();
for( int i= 0; i < n && !found; i ++ )
    {
    RTCat cat= (RTCat) (*pCatArray)[i];

	if( cat != NOOBJECT )
        {
        if( cat == searchCat )
            found= TRUE;
        else if( 0 < cat.itsSubCatsP->getItemsInContainer() )
            found= cat.itsSubCatsP->IsCatInTree( searchCat );
        }
    }

return found;
}

//**************************************************************************
//
// Array operations
//
//**************************************************************************

//*****************************************************************************

//*****************************************************************************
int TCatTree::getItemsInContainer( void )
{
if( pCatArray == NULL )
    return 0;
else
    return pCatArray->getItemsInContainer();
}

//--------------------------------------------------------------------------
// detach
//--------------------------------------------------------------------------
void TCatTree::detach( RTCat deadCat )
{
    CHKBUG( ERR_GEN, IsCatInTree( deadCat ));

    // detach the category from its parent's subcat list
    // if the cat has no parent cat, then it must be a top-level
    // element of *this
    if (deadCat.itsParentP == NULL) {

         if( pCatArray == NULL )
            return;
         else
            pCatArray->detach( deadCat );
    }
    else {

         ASSERT( ERR_GEN, deadCat.itsParentP->itsSubCatsP->pCatArray != NULL );
         deadCat.itsParentP->itsSubCatsP->pCatArray->detach( deadCat );
    }


    deadCat.itsParentP = NULL;
}

//--------------------------------------------------------------------------
// add
//
// set the parent pointer too
//--------------------------------------------------------------------------
void TCatTree::add( RTCat cat )
{
    if( pCatArray == NULL )
        pCatArray= new TXArray( 10, 10 );

    cat.itsParentP = itsParentP;
    //always fix parent before adding to array
    pCatArray->add( cat );
}

//--------------------------------------------------------------------------
// flush
//
// recurses!
//--------------------------------------------------------------------------
void TCatTree::flush( TXArray::TDeleteStyle dt )
{
if( pCatArray == NULL )
    return;

int n= pCatArray->getItemsInContainer();
for(int i=0; i < n; i++ )
    {
    RTCat cat= (RTCat) (*pCatArray) [i];
	CHKBUG( ERR_CAT, cat.HealthCheck() );
    cat.itsSubCatsP->flush( dt );
    }

pCatArray->flush( dt );
}


//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
RTCat TCatTree::operator []( int index )
{
ASSERT( ERR_GEN, pCatArray != NULL );

return (RTCat) (*pCatArray) [index];
}

//--------------------------------------------------------------------------
// find
//--------------------------------------------------------------------------
int TCatTree::find( RTCat cat )
{
    if( pCatArray == NULL )
        return -1;

    for (int i = 0; i < pCatArray->getItemsInContainer(); i++) {

        if (cat == (RTCat)(*pCatArray)[i]) return i;
    }

    return -1;
}

//--------------------------------------------------------------------------
// NumCatsInTree
//
// recurses
//--------------------------------------------------------------------------
int TCatTree::NumCatsInTree( void )
{
int num= 0;

if( pCatArray == NULL )
    return 0;

int n= pCatArray->getItemsInContainer();
for( int i= 0; i < n; i ++ )
    {
    RTCat cat= (RTCat) (*pCatArray)[i];

    CHKBUG( ERR_CAT, cat.HealthCheck() );

    num++;
    num+= cat.itsSubCatsP->NumCatsInTree();
    }

return num;
}

//**************************************************************************
//
// Other operations
//
//**************************************************************************


// -----------------------------------------------------------------
// Merge
//
// Merge the other CatTree into this one.
// -----------------------------------------------------------------
void TCatTree::Merge( PTCat parentP, RTCatTree otherTree )
{
    if( otherTree.pCatArray == NULL )
        return;

    // loop through categories on this level of the other tree
    int n= otherTree.pCatArray->getItemsInContainer();
    for( int i= 0; i < n; i ++ )
        {
        RTCat otherCat= (RTCat) (*otherTree.pCatArray)[i];

        // look for the category at this level of this tree
        // and create a new one if it's not there
        RTCat ourCat = FindCatInTopLevel( otherCat.itsShortName );
        if (ourCat == NOOBJECT) {

            PTCat pCat        = new TCat();
            pCat->itsShortName.Set( otherCat.itsShortName );
            pCat->itsDescription.Set( otherCat.itsDescription );
            pCat->itsNickname.Set( otherCat.itsNickname );
            pCat->itsKeywords.Set( otherCat.itsKeywords );
            pCat->itsCatType    = otherCat.itsCatType;
            pCat->itsParentP    = parentP;

            if( pCatArray == NULL )
                pCatArray= new TXArray( 10, 10 );

            pCatArray->add( *pCat, TXArray::kNoSort );

            if (otherCat.itsAddressP != NULL) {

                pCat->itsAddressP = new TAddress( *otherCat.itsAddressP );
            }

            pCat->itsSubCatsP->Merge( pCat, *otherCat.itsSubCatsP );
        }
        else {

            ourCat.itsSubCatsP->Merge( &ourCat, *otherCat.itsSubCatsP );
        }
    }

    if( pCatArray != NULL )
        pCatArray->sort();

    CHKBUG( ERR_GEN, HealthCheck() );
}

//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
BOOL TCatTree::HealthCheck( void )
{
BOOL rc= TRUE;

//check parent pointer
if( itsParentP != NULL )
    {
    if( itsParentP->itsSubCatsP != this )
        return FALSE;
    }

if( pCatArray == NULL )
    return TRUE;

int n= pCatArray->getItemsInContainer();
for( int i= 0; i < n; i ++ )
    {
    RTCat cat= (RTCat) (*pCatArray)[i];

    rc= cat.HealthCheck();

    if( !rc )
        break;
    }

return rc;
}


//**************************************************************************
//
// Marking categories
//
//**************************************************************************


//--------------------------------------------------------------------------
// ClearMarks
//
//     Clear marks in each category in the tree.
//--------------------------------------------------------------------------
void TCatTree::ClearMarks( void )
{
    if( pCatArray == NULL )
        return;

    int n= pCatArray->getItemsInContainer();
    for (int i=0; i < n; i++) {

        RTCat cat = (RTCat) (*pCatArray)[i];

        cat.itsMarked = FALSE;
        cat.itsSubCatsP->ClearMarks();
    }
}

//**************************************************************************
//
// Object Nonsense
//
//**************************************************************************
//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
classType TCatTree::isA( void )           const { return IDCatTree;       }
char * TCatTree::nameOf( void )           const { return "TCatTree";      }
hashValueType TCatTree::hashValue( void ) const { return 0;               }        //FIXME
int TCatTree::isEqual(const Object & obj) const { return( this == & obj );}
void TCatTree::printOn(ostream far&)      const {                         }    // FIXME

//**************************************************************************
//
// I/O
//
//**************************************************************************

//--------------------------------------------------------------------------
// stream support
//--------------------------------------------------------------------------
PTStreamable TCatTree::build( void )
{
return( new TCatTree( streamableInit ) );
}

TStreamableClass RegTCatTree( "TCatTree", TCatTree::build, __DELTA(TCatTree) );

//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
Pvoid TCatTree::read( Ripstream is )
{
int catSize;
int version;
int i;

is >> version;

itsParentP= NULL;

if( version == 2 )
    {
    is >> catSize;

    if( catSize == 0 )
        pCatArray= NULL;
    else
        {
        pCatArray= new TXArray( catSize, 10 );

        for( i= 0; i < catSize; i++ )
            {
            PTCat pCat;

            is >> pCat;

            pCatArray->add( *pCat, TXArray::kNoSort );
            }

        // Now that we read all the categories in, fix up itsParentP
        FixupParents( NULL );

        //always set parents before sorting!
        pCatArray->sort();
        }
    }
else if( version == 1 )
    {
    is >> catSize;

    pCatArray= new TXArray( catSize, 10 );

    for( i= 0; i < catSize; i++ )
        {
        PTCat pCat;

        is >> pCat;

        pCatArray->add( *pCat, TXArray::kNoSort );
        }

    // Now that we read all the categories in, fix up itsParentP
    FixupParents( NULL );

    //always set parents before sorting!
    pCatArray->sort();

    }
else
	SYSERR( ERR_VER );

return this;
}

//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
void TCatTree::write( Ropstream os )
{
int n= 0;

os << (int) 2;      //version 2: no pCatArray if it's empty

if( pCatArray )
    n= pCatArray->getItemsInContainer();
else
    n= 0;

os << n;

// errors are caught locally so we can clean up, then throw up
CATCHBUF cBuf;
LPCATCHBUF pOldCBuf= TError::SetCatchBuf( cBuf );
TErrCode rc= (TErrCode) Catch( cBuf );
if( rc == 0 && n > 0 )
    {
    for( int i= 0; i < n; i ++ )
        {
        RTCat cat= (RTCat) (*pCatArray)[i];

        //want to check that we didn't introduce spurious copy here
        CHKBUG( ERR_CAT, cat.HealthCheck() );

        os << &cat;
        }
    }
//restore saved error handler
TError::SetCatchBuf( pOldCBuf );

//signal error to caller
if( rc )
    SYSTHROW( rc );
}

//--------------------------------------------------------------------------
// FixupParents
//
// Once the entire CatTree is read in, fill in all of the itsParentP fields
//--------------------------------------------------------------------------
void TCatTree::FixupParents( PTCat parentP )
{
itsParentP = parentP;

if( pCatArray == NULL )
    return;

int n= pCatArray->getItemsInContainer();
for( int i= 0; i < n; i ++ )
    {
    RTCat cat= (RTCat) (*pCatArray)[i];

    cat.itsParentP = parentP;
    cat.itsSubCatsP->FixupParents( &cat );
    }
}

//*****************************************************************************
// RoomForMore
//
// Is there room for howManyMore categories in the cattree?
//*****************************************************************************
BOOL TCatTree::RoomForMore( int howManyMore,
    BOOL showError, BOOL showWarning,
    char *altError, char *altWarning )
{
    int numCats = NumCatsInTree();

    // still plenty of room
    if (numCats + howManyMore <= 2980 )
        {
        return TRUE;
        }

    // close to the edge but still ok
    if (numCats + howManyMore <= 3000)
        {
        if (showWarning)
            {
            KSTR warning( "You are near the 3000 folder limit." );

            if (altWarning != NULL)
                {
                warning.Set( altWarning );
                }

            TMessageBox::Ok( TPimFrame::pFrame,
                (char*) (const char*) warning,
                "Too Many Folders" );
            }

        return TRUE;
        }
    

    // over the edge
    if (showError)
        {
        KSTR error( "You are over the 3000 folder limit." );

        if (altError != NULL)
            {
            error.Set( altError );
            }

        TMessageBox::Ok( TPimFrame::pFrame,
            (char*) (const char*) error,
            "Too Many Folders" );
        }

    return FALSE;

}
