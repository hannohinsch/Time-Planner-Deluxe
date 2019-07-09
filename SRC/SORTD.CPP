//*****************************************************************************
// note sort order dialog
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "sortd.h"

#ifndef __profile__
#include "profile.h"
#endif
#ifndef __oem__
#include "oem.h"
#endif


// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------
TSortD::TSortD( PTWindowsObject AParent, int resId )
     : TOurDialog( AParent, resId )
{
    itsHelpTopic= _HelpSort;

    for (int g = 0; g < kNumGroups; g++) {

        // create the group box
        int groupId = IDD_ORDER_GROUP_1 + 20 * g;
        itsGroupBoxes[g]  = new TGroupBox( this, groupId );
        itsGroupBoxes[g]->NotifyParent = TRUE;


        // create the buttons (unchecked)
        for (int b = 0; b < kNumButtonsInGroup; b++) {

            itsButtons[g][b] = new TRadioButton( this, groupId + b + 1, itsGroupBoxes[g] );
            itsTransferBuffer[g][b] = BF_UNCHECKED;
        }
    }


    // turn on some radio buttons
    itsTransferBuffer[0][TProfile::NoteOrder[0]] = BF_CHECKED;
    itsTransferBuffer[1][TProfile::NoteOrder[1]] = BF_CHECKED;
    itsTransferBuffer[2][TProfile::NoteOrder[2]] = BF_CHECKED;

    // create the other buttons
    itsOkButtonP     = new TButton( this, IDOK );
    itsCancelButtonP = new TButton( this, IDCANCEL );

    SetTransferBuffer( &itsTransferBuffer );
}


// -----------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------
TSortD::~TSortD( void )
{
}


// -----------------------------------------------------------------------
// HandleGroup
// -----------------------------------------------------------------------
void TSortD::HandleGroup( RTMessage msg, int group, TNoteOrder order )
{
    if (msg.LP.Hi == BN_CLICKED) {

        for (int b = 0; b < kNumButtonsInGroup; b++) {

            itsTransferBuffer[group][b] = BF_UNCHECKED;
        }

        itsTransferBuffer[group][order] = BF_CHECKED;

        TransferData( TF_SETDATA );
    }
}



// -----------------------------------------------------------------------
// Ok
// -----------------------------------------------------------------------
void TSortD::Ok( RTMessage msg )
{
    SetValues();
    TDialog::Ok( msg );
}

// -------------------------------------------------
// SetDefault
//
//  Close the dialog and store this information
//  into the profile
// -------------------------------------------------
void TSortD::SetDefault( RTMessage msg )
{
    char    buf[100];

    SetValues();

    wsprintf( buf, "%d", TProfile::NoteOrder[0] );
    WritePrivateProfileString( TOEM::profileSection, "noteorder1", buf, TOEM::profileName );
    wsprintf( buf, "%d", TProfile::NoteOrder[1] );
    WritePrivateProfileString( TOEM::profileSection, "noteorder2", buf, TOEM::profileName );
    wsprintf( buf, "%d", TProfile::NoteOrder[2] );
    WritePrivateProfileString( TOEM::profileSection, "noteorder3", buf, TOEM::profileName );

    TDialog::Ok( msg );
}

// -------------------------------------------------
// SetValues
//
//  Use the values selected
// -------------------------------------------------
void TSortD::SetValues( void )
{

    TransferData( TF_GETDATA );

    for (int g = 0; g < kNumGroups; g++) {

        for (int b = 0; b < kNumButtonsInGroup; b++) {

            if (itsTransferBuffer[g][b] == BF_CHECKED) {

                TProfile::NoteOrder[g] = (TNoteOrder) b;
                break;
            }
        }
    }
}

