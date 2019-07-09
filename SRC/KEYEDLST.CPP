//*****************************************************************************

//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "keyedlst.h"
#ifndef __error__
#include "error.h"
#endif

//*****************************************************************************
// Constructor
//*****************************************************************************
TKeyedList::TKeyedList( BOOL ownsData, CompareFunc func  )
{
    // arbitrarily create a table with 10 entries
    // Add will make sure that the array has
    // enough entries before inserting items
    itsCompareFunc = func;
    itsOwnsData    = ownsData;
    itsSorted      = FALSE;
    itsNumEntries  = 0;
    itsMaxEntries  = 10;
    itsDataP       = new TKeyedPair[itsMaxEntries];
}

//*****************************************************************************
// Destructor
//*****************************************************************************
TKeyedList::~TKeyedList( void )
{
    if (itsOwnsData) {

        for( int i = 0; i < itsNumEntries; i++) {

            if (itsDataP[i].DataP) {

                 delete itsDataP[i].DataP;
            }
        }
    }

    delete itsDataP;
}

//*****************************************************************************
// Add
//*****************************************************************************
void TKeyedList::Add( long key, Pvoid dataP )
{
    CHKBUG( ERR_GEN, HealthCheck());

    if (itsNumEntries == itsMaxEntries) {

        // double the size of the table and
        // copy the old data into the new table
        PTKeyedPair newTable = new TKeyedPair[itsMaxEntries * 2];
        memcpy( newTable, itsDataP, itsMaxEntries * sizeof( TKeyedPair ));
        delete itsDataP;
        itsDataP = newTable;
        itsMaxEntries *= 2;
    }

    // throw the new keyed pair into the table
    itsDataP[itsNumEntries].Key = key;
    itsDataP[itsNumEntries].DataP = dataP;
    itsNumEntries++;

    // it's no longer sorted since we just dumped
    // the new entry into the end of the list
    itsSorted = FALSE;

    CHKBUG( ERR_GEN, HealthCheck());
}

//*****************************************************************************
// Detach
//
//  Remove the key from the list
//*****************************************************************************
void TKeyedList::Detach( long key )
{
    CHKBUG( ERR_GEN, HealthCheck());

    int index = (itsSorted) ?
        BinaryFind( key ) : LinearFind( key );

    if ((0 <= index) && (index < itsNumEntries)) {

        // delete the data if required
        if (itsOwnsData && itsDataP[index].DataP) {

            delete itsDataP[index].DataP;
        }

        // crush the indexed item out of existence
        int remaining = itsNumEntries - index - 1;
        memcpy( &itsDataP[index], &itsDataP[index + 1],
           remaining * sizeof( TKeyedPair ));
        itsNumEntries--;
    }

    else {

        SYSERR_STR( "Detach: bad index" );
    }

    CHKBUG( ERR_GEN, HealthCheck());
}
//*****************************************************************************
// SetData
//
//  Set the data for the key
//*****************************************************************************
void TKeyedList::SetData( long key, Pvoid dataP )
{
    CHKBUG( ERR_GEN, HealthCheck());

    int index = (itsSorted) ?
        BinaryFind( key ) : LinearFind( key );

    if ((0 <= index) && (index < itsNumEntries)) {

        itsDataP[index].DataP = dataP;
    }

    else {

        SYSERR_STR( "SetData: bad index" );
    }

    CHKBUG( ERR_GEN, HealthCheck());
}

//*****************************************************************************
// Sort
//
//  Sort the entries
//*****************************************************************************
void TKeyedList::Sort( void )
{
    CHKBUG( ERR_GEN, HealthCheck());

    if (!itsSorted) {

        qsort( itsDataP, itsNumEntries,
            sizeof( TKeyedPair ), itsCompareFunc );
        itsSorted = TRUE;

    CHKBUG( ERR_GEN, HealthCheck());
    }
}

//*****************************************************************************
// LinearSearch
//
//  return the data that goes with the key, searching
//  linearly through the list
//*****************************************************************************
Pvoid TKeyedList::LinearSearch( long key )
{
    CHKBUG( ERR_GEN, HealthCheck());

    int index = LinearFind( key );
    if ((0 <= index) && (index < itsNumEntries)) {

        return itsDataP[index].DataP;
    }
    else {

        return (Pvoid) KEYNOTFOUND;
    }
}

//*****************************************************************************
// BinarySearch
//
//  return the data that goes with the key, using a
//  binary search if the data is sorted
//*****************************************************************************
Pvoid TKeyedList::BinarySearch( long key )
{
    CHKBUG( ERR_GEN, HealthCheck());

    if (!itsSorted) {

        SYSERR_STR( "BinarySearch: not sorted" );
        return (Pvoid) KEYNOTFOUND;
    }

    int index = BinaryFind( key );
    if ((0 <= index) && (index < itsNumEntries)) {

        return itsDataP[index].DataP;
    }
    else {

        return (Pvoid) KEYNOTFOUND;
    }
}

//*****************************************************************************
// LinearFind
//
//  return the index of the specified key
//*****************************************************************************
int TKeyedList::LinearFind( long key )
{
    TKeyedPair pair;
    pair.Key = key;

    PTKeyedPair foundP = (PTKeyedPair) lfind( &pair, itsDataP,
        &itsNumEntries, sizeof( TKeyedPair ), itsCompareFunc );

    if (foundP) {

        // return the index
        return (int) (foundP - itsDataP);
    }
    else {

        return KEYNOTFOUND;
    }
}

//*****************************************************************************
// BinaryFind
//
//  return the index of the specified key
//*****************************************************************************
int TKeyedList::BinaryFind( long key )
{
    if (!itsSorted) {

        SYSERR_STR( "BinaryFind: not sorted" );
        return KEYNOTFOUND;
    }

    TKeyedPair pair;
    pair.Key = key;

    PTKeyedPair foundP = (PTKeyedPair) bsearch( &pair, itsDataP,
        itsNumEntries, sizeof( TKeyedPair ), itsCompareFunc );

    if (foundP) {

        // return the index
        return (int) (foundP - itsDataP);
    }
    else {

        return KEYNOTFOUND;
    }
}

//*****************************************************************************
// Compare
//
//  Compare the two keys by just comparing the Key fields directly.
//
//  Imagine that you're using category pointers as keys.
//  This function will just compare the ADDRESSES of
//  the categories to order them. If you want to
//  compare the categories themselves, you'll have
//  to use a comparison function that casts the keys
//  into categories and then compares them using
//  methods in the cat class.
//*****************************************************************************
int TKeyedList::Compare( const void* key1, const void* key2 )
{
    PTKeyedPair a = (PTKeyedPair) key1;
    PTKeyedPair b = (PTKeyedPair) key2;

    if (a->Key < b->Key) return -1;
    else if (a->Key > b->Key) return 1;
    else return 0;
}

//*****************************************************************************
// HealthCheck
//*****************************************************************************
BOOL TKeyedList::HealthCheck( void )
{
   BOOL ok = ((itsDataP != NULL) &&
        (itsNumEntries <= itsMaxEntries));

   if (itsSorted) {

       for( int i = 0; i < ((int) itsNumEntries) - 1; i++) {

          if (itsCompareFunc( &itsDataP[i], &itsDataP[i + 1] ) == 1) {

              SYSERR_STR( "HealthCheck: not sorted properly" );
              return FALSE;
          }
       }
   }

   return ok; 
}

//*****************************************************************************
// Test
//
//  Exercise the methods
//*****************************************************************************
    int TestCompare( const void *key1, const void *key2 );
    int TestCompare( const void *key1, const void *key2 )
    {
    long *a = (long *) key1;
    long *b = (long *) key2;

    if (*a < *b) return 1;
    else if (*a > *b) return -1;
    else return 0;
    }

void TKeyedList::Test( void )
{
    int i;

    // add enough items to force the array to be expanded
    // check that it finds everything properly
    // check that it deletes everything
    PTKeyedList listP = new TKeyedList( FALSE );
    for (i = 0; i < 15; i++) listP->Add( i, (void*) i );
    for (i = 0; i < 15; i++) ASSERT_STR( "Test", i == (int) listP->LinearSearch( i ));
    for (i = 0; i < 15; i++) listP->Detach( i );
    delete listP;

    // repeat the same tests using a new compare func
    listP = new TKeyedList( FALSE, TestCompare );
    for (i = 0; i < 15; i++) listP->Add( i, (void*) i );
    for (i = 0; i < 15; i++) ASSERT_STR( "Test", i == (int) listP->LinearSearch( i ));

    // sort and see if it still finds everything
    listP->Sort();
    for (i = 0; i < 15; i++) ASSERT_STR( "Test", i == (int) listP->LinearSearch( i ));
    for (i = 0; i < 15; i++) ASSERT_STR( "Test", i == (int) listP->BinarySearch( i ));

    // delete some and see if it still finds everything
    for (i = 10; i < 15; i++) listP->Detach( i );
    for (i = 0; i < 10; i++) ASSERT_STR( "Test", i == (int) listP->LinearSearch( i ));
    for (i = 0; i < 10; i++) ASSERT_STR( "Test", i == (int) listP->BinarySearch( i ));

    // add some more in a different order and see if
    // it still finds everything 
    for (i = 14; i >= 10; i--) listP->Add( i, (void*) i );
    for (i = 0; i < 15; i++) ASSERT_STR( "Test", i == (int) listP->LinearSearch( i ));
    listP->Sort();
    for (i = 0; i < 15; i++) ASSERT_STR( "Test", i == (int) listP->BinarySearch( i ));
    delete listP;


}

