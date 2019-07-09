//*****************************************************************************
// day view dialog: pick # of slots per hour
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "dayviewd.h"
#ifndef __profile__
#include "profile.h"
#endif
#ifndef __oem__
#include "oem.h"
#endif

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TDayviewD::TDayviewD( PTWindowsObject AParent, int AResID ):
	TOurDialog( AParent, AResID )
{

    itsHelpTopic= _HelpCalInterval;

    // create the group box
    itsGroupBoxP     = new TGroupBox( this, IDD_DAYVIEW_GROUP_BOX );
    itsGroupBoxP->NotifyParent = TRUE;


    // create the radio buttons and initialize the transfer buffer
    for (int i = 0; i < kNumButtons; i++) {

        itsButtonsP[i] = new TRadioButton( this,
            IDD_DAYVIEW_5_MIN + i, itsGroupBoxP );
        itsTransferBuffer[i] = BF_UNCHECKED;
    }

    // initialize the currently 'on' transfer
    itsTransferBuffer[TProfile::dayviewIncrement] = BF_CHECKED;


    // create the other buttons
    itsOkButtonP     = new TButton( this, IDOK );
    itsCancelButtonP = new TButton( this, IDCANCEL );

    SetTransferBuffer( itsTransferBuffer );
}


// -------------------------------------------------
// Destructor
// -------------------------------------------------
TDayviewD::~TDayviewD( void )
{
    // delete the radio buttons
    for (int i = 0; i < kNumButtons; i++) {

        delete itsButtonsP[i];
    }


    delete itsGroupBoxP;
    delete itsOkButtonP;
    delete itsCancelButtonP;

}


// -------------------------------------------------
// OK
//
//  figure out what was pressed
// -------------------------------------------------
void TDayviewD::Ok( RTMessage msg )
{
    SetValue();
    TOurDialog::Ok( msg );

}

// -------------------------------------------------
// SetDefault
//
//  Store this information into the profile
// -------------------------------------------------
void TDayviewD::SetDefault( RTMessage msg )
{
    char    buf[100];

    SetValue();

    wsprintf( buf, "%d", TProfile::dayviewIncrement );
    WritePrivateProfileString(
        TOEM::profileSection, "DayviewIncrement",
        buf, TOEM::profileName );

    TOurDialog::Ok( msg );
}

//*****************************************************************************

//*****************************************************************************
void TDayviewD::UseDefault( void )
{
}

// -------------------------------------------------
// SetValue
//
//  Use the defaults selected
// -------------------------------------------------
void TDayviewD::SetValue( void )
{
    TransferData( TF_GETDATA );


    for (int i = 0; i < kNumButtons; i++) {

        if (itsTransferBuffer[i] == BF_CHECKED) {

            TProfile::dayviewIncrement = (TDayviewIncrements) i;
            break;
        }
    }
}
