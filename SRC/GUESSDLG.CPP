//*****************************************************************************
// check note text for foder keyword matches
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "guessdlg.h"
#include "msg.h"
#ifndef __cat__
#include "cat.h"
#endif
#ifndef __catlist__
#include "catlist.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif


// -------------------------------------------------
// Constructor
// -------------------------------------------------
TGuessDialog::TGuessDialog( PTWindowsObject AParent, int AResID,
    PTCatList guessListP, PTCatList realListP ):
	TOurDialog( AParent, AResID )
{
    itsGuessListP = guessListP ;
    itsRealListP  = realListP ;

    itsHelpTopic= _HelpKeyword;

    // make up data for the selector
    int           n = guessListP->getItemsInContainer();
    itsSelectorData = new TSelectorData[n];
    for (int i = 0; i < n; i++)
        {
        TSafStr temp;
        RTCat cat                   = (*guessListP)[i];
        cat.GetShortName( temp );
        itsSelectorData[i].pText    = new TSafStr( temp );
        itsSelectorData[i].value    = (long) &cat;
        itsSelectorData[i].selected = TRUE;
        }

    // create selector
    itsSelector  = new TSelector( this, IDD_CHKLIST,
        n, itsSelectorData );

    // create the other interface objects
    itsOkButton = new TButton( this, IDOK );
    itsCancelButton = new TButton( this, IDCANCEL );
}


// -------------------------------------------------
// Destructor
// -------------------------------------------------
TGuessDialog::~TGuessDialog( void )
{
    int           n = itsGuessListP->getItemsInContainer();
    for (int i = 0; i < n; i++)
        {
        delete itsSelectorData[i].pText;
        }

    delete itsSelectorData;
}

// -------------------------------------------------
// Ok
//
//  Add whatever is selected in the selector data
//  to the catlist
// -------------------------------------------------
void TGuessDialog::Ok( RTMessage msg )
{
    int           n = itsGuessListP->getItemsInContainer();
    for (int i = 0; i < n; i++)
        {
        RTCat cat = *(PTCat) itsSelectorData[i].value;
        BOOL selected = itsSelectorData[i].selected;
        if (selected && (!itsRealListP->IsCatInList( cat )))
            {
            itsRealListP->add( cat );
            }
        if (!selected && itsRealListP->IsCatInList( cat ))
            {
            itsRealListP->detach( cat );
            }
        }

    TOurDialog::Ok( msg );

}

