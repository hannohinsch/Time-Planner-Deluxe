//*****************************************************************************
// cover class for TNSCollection and TNSSortedXollection that emulates
// the (sluggish) containers in the classlib
//
// The TNS collections are a bit tricky to use, because of the handling
// of 'duplicates' and the compare function.  If there are objects which
// compare says are equal to one another, the 'duplicate' flag must be set.
// Also, unlike the classlib containers, the compare function must return all
// three relations between two objects.
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "xarray.h"
#include "error.h"

TXArray * TXArray::itsCompP= NULL;  //used by qsort comp() routine

//*****************************************************************************

//*****************************************************************************
TXArray::TXArray( int limit, int delta, TSortStyle style ):
    TNSSortedCollection( limit, delta )
{
duplicates= True;
shouldDelete= False;
itsSortStyle= style;
};

//*****************************************************************************

//*****************************************************************************
TXArray::~TXArray()
{
}

//*****************************************************************************

//*****************************************************************************
BOOL TXArray::HasMember( Object &obj, TSortStyle style )
{
if( style == kNoSort || (itsSortStyle == kNoSort && style == kDefSort) )
    return ccNotFound != TNSCollection::indexOf( &obj );       //linear search
else
    {
    CHKBUG( ERR_GEN, IsSorted() );
    return ccNotFound != indexOf( &obj );                      //binary search
    }
}


//*****************************************************************************

//*****************************************************************************
int TXArray::GetIndex( Object &obj, TSortStyle style )
{
if( style == kNoSort || (itsSortStyle == kNoSort && style == kDefSort) )
    return TNSCollection::indexOf( &obj );       //linear search
else
    {
    CHKBUG( ERR_GEN, IsSorted() );
    return indexOf( &obj );                      //binary search
    }
}

//*****************************************************************************

//*****************************************************************************
void TXArray::Add( Object &toAdd, TSortStyle style )
{
if( style == kNoSort || (itsSortStyle == kNoSort && style == kDefSort) )
    TNSCollection::insert( &toAdd );
else
    {
    CHKBUG( ERR_GEN, IsSorted() );
    insert( &toAdd );
    }
};

//*****************************************************************************

//*****************************************************************************
void TXArray::Detach( Object &obj, TDeleteStyle dt )
{
// the array may or  may not be sorted, so we
// do a linear search (from TNSCollection), rather than the
// default binary search
int index= TNSCollection::indexOf( &obj );
if( index == ccNotFound )
    return;

CHKBUG( ERR_GEN, &obj == at( index ) );

if( dt == kDelete )
    atFree( index );
else
    atRemove( index );
};

//*****************************************************************************

//*****************************************************************************
void TXArray::Flush( TDeleteStyle dt )
{
if( dt == kDelete )
    freeAll();
else
    removeAll();
};

//*****************************************************************************
// if delete doesn't know it's an Object, it doesn't call the
// right destructors!
//*****************************************************************************
void TXArray::freeItem( void *item )
{
delete (Object *) item;
}


//*****************************************************************************

//*****************************************************************************
Object & TXArray::operator [] (int idx)
{
//putting two separate assertions here makes it easier to track
//down problems, because we can tell which one failed from line number
ASSERT( ERR_GEN, idx >= 0  );

//FIXME this guarded assertion makes no sense, but since I discovered it
// on release day I don't want to mess with it.  //hmh
if( idx >= count )
    {
    ASSERT( ERR_GEN, idx < count );
    }

return *(Object *)at( idx );
};

//*****************************************************************************

//*****************************************************************************
BOOL TXArray::IsSorted( void )
{
for( int i= 0; i < count - 1; i++ )
    {
    if( ((Sortable *) items[ i+1 ] )->isLessThan( *((Sortable *) items[ i ] )))
        return FALSE;
    }

return TRUE;
}

//*****************************************************************************
// NB by convention, objects in sorted array are derived from class Sortable
// Return -1, 0, 1 depending on relation between objects
//*****************************************************************************
int TXArray::compare( Pvoid key1, Pvoid key2 )
{
CHKBUG( ERR_GEN,
    ((Object *)key1)->isSortable() && ((Object *)key2)->isSortable() );

// limited to homogenous collections
CHKBUG( ERR_GEN,
    ((Object *)key1)->isA() == ((Object *)key2)->isA() );

// FIXME set up objects to have complete compare fn, instead of having
// to do this twice
if( ((Sortable *)key1)->isLessThan( *((Sortable *)key2) ))
    return -1;
else if( ((Sortable *)key2)->isLessThan( *((Sortable *)key1) ))
    return 1;
else
    return 0;


}

//*****************************************************************************

//*****************************************************************************
int TXArray::Comp( const void *a, const void *b)
{
//we are given pointers to the table entries...
return itsCompP->compare( *(void **)a, *(void **)b );
}

//*****************************************************************************
//
//*****************************************************************************
void TXArray::Sort( void )
{

itsCompP= this;
qsort( items, count, sizeof( void * ), TXArray::Comp );

};
