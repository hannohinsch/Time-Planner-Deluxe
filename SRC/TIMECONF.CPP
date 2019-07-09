//*****************************************************************************
// time conflict dialog
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "timeconf.h"


// -------------------------------------------------
// Constructor
// -------------------------------------------------
TTimeConflictDialog::TTimeConflictDialog( PTWindowsObject AParent, int AResID, TNoteSet &noteSet ):
	TOurDialog( AParent, AResID )
{
    // create the interface objects
    itsSelectorP  = new TNoteListGrid( this, IDD_NOTELIST_SELECTOR,
        FALSE, &noteSet );
}


// -------------------------------------------------
// Destructor
// -------------------------------------------------
TTimeConflictDialog::~TTimeConflictDialog( void )
{
    delete itsSelectorP;
}

