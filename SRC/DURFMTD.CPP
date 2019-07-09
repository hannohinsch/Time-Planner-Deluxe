//*****************************************************************************
// duration format dialog
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "durfmtd.h"

#ifndef __profile__
#include "profile.h"
#endif

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TDurFmtD::TDurFmtD( PTWindowsObject AParent, int AResID ):
	TOurDialog( AParent, AResID )
{

    itsHelpTopic= _HelpWorkFormat;

    // create the hour/day/week group and radio buttons
    PTGroupBox group1P = new TGroupBox( this, IDD_TABVIEW_GROUP_BOX_1 );
    for (int i = 0; i < kNumTabButtons; i++) {

        itsButtonsP[i] = new TRadioButton(
            this, IDD_TABVIEW_HOURS + 10 * i, group1P );
        itsTransferBuffer.buttonValues[i] = BF_UNCHECKED;
    }

    // create the mixed/decimal group and radio buttons
    PTGroupBox group2P = new TGroupBox( this, IDD_TABVIEW_GROUP_BOX_2 );
    itsMixButtonP      = new TRadioButton( this, IDD_TABVIEW_MIX, group2P );
    itsDecimalButtonP  = new TRadioButton( this, IDD_TABVIEW_DECIMAL, group2P );
    itsTransferBuffer.mixValues[0] = BF_UNCHECKED;
    itsTransferBuffer.mixValues[1] = BF_UNCHECKED;


    // initialize the currently 'on' radio buttons
    itsTransferBuffer.buttonValues[TProfile::baseTimeFmtUnits] = BF_CHECKED;
    if (TProfile::bMixTimeFmtUnits) {

        itsTransferBuffer.mixValues[0]   = BF_CHECKED;
    }
    else {

        itsTransferBuffer.mixValues[1]   = BF_CHECKED;
    }


    SetTransferBuffer( &itsTransferBuffer );
}


// -------------------------------------------------
// Destructor
// -------------------------------------------------
TDurFmtD::~TDurFmtD( void )
{
}

// -------------------------------------------------
// OK
//
//  figure out what was pressed
// -------------------------------------------------
void TDurFmtD::Ok( RTMessage msg )
{
    TransferData( TF_GETDATA );

    for (int i = 0; i < kNumTabButtons; i++) {

        if (itsTransferBuffer.buttonValues[i] == BF_CHECKED) {

            TProfile::baseTimeFmtUnits = (TTimeFmtUnits) i;
            break;
        }
    }

    TProfile::bMixTimeFmtUnits = itsTransferBuffer.mixValues[0];

    Notify( kNoteSetChg, NOOBJECT );
    TDialog::Ok( msg );
}
