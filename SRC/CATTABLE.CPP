//*****************************************************************************

//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "cattable.h"
#ifndef __cat__
#include "cat.h"
#endif
#ifndef __catlist__
#include "catlist.h"
#endif
#ifndef __cattree__
#include "cattree.h"
#endif

//*****************************************************************************
// Constructor
//
// A CatTable is KeyedList, where the key is a category
// and the data depends on the use of the CatTable.
// Because the list is sorted by category,
// it is fast to find the data associated with a
// given category by binary search.
//
// The constructor requires a CatList
// and an array of data objects. There must be one
// data object for each cat in the list. The
// method will associate each cat with an
// element in the array of data.
//
// Each subcat of a cat in the CatList is also
// associated with the data for the root cat. That
// way, we can easily look up cat X and its
// subcats X:Y and X:Z, but they all use the same
// data.
//
// The CatTable doesn't own the data passed in and
// will not delete it. 
//*****************************************************************************
TCatTable::TCatTable( PTCatList catListP, Pvoid data[] ):
   TKeyedList( FALSE )
{
    int n = catListP->getItemsInContainer();
    for (int i = 0; i < n; i++) {

        // associate the cat and its subcats
        // with an element of the data array
        RTCat cat = (*catListP)[i];
        AddSubcats( &cat, data[i], catListP );
    }

    // sort the CatTable using the cats as keys
    Sort();
}


//*****************************************************************************
// Destructor
//*****************************************************************************
TCatTable::~TCatTable( void )
{
}

//*****************************************************************************
// AddSubcats
//
//   All subcats of catP get added to the array, but
//   use the same noteset
//*****************************************************************************
void TCatTable::AddSubcats( PTCat catP, Pvoid data, PTCatList catListP )
{
    // stick the cat and data into the tree
    Add( (long) catP, data );

    // add the subcats to the table
    PTCatTree catTreeP = catP->GetSubCats();
    int n = catTreeP->getItemsInContainer();
    for( int i = 0; i < n; i++) {

        RTCat cat = (*catTreeP)[i];

        // if the subcat appears in the main list
        // of cats to be registered, then it will
        // get its own data and shouldn't be added here
        if (!catListP->IsCatInList( cat )) {

            AddSubcats( &cat, data, catListP );
        }
    }
}

