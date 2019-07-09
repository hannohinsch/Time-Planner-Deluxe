//*****************************************************************************
// archive dialog
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "archived.h"
#include "oem.h"

// -----------------------------------------------------------------------
// Constructor
//
//     Standard file dialog, except that the radio button is also allocated
// -----------------------------------------------------------------------
TArchiveD::TArchiveD( PTWindowsObject AParent,
        LPSTR AFilePath, BOOL *shouldDeleteP, PTModule AModule )
     : TOurFileDialog( AParent, IDD_ARCHIVE_SAVE, AFilePath, AModule )
{

    itsHelpTopic= _HelpArchive;

    strcpy( Extension, TOEM::fileExt );

    // create checkbox, but don't let it transfer anything, since
    // presumably TFileDialog has its own transfer buffer already, and
    // will just confuse things if we try to set another transfer buffer
    // for the same dialog
    itsDeleteButtonP = new TCheckBox( this, IDD_ARCHIVE_DELETE, NULL );
    itsDeleteButtonP->DisableTransfer();

    itsShouldDeleteP = shouldDeleteP;
}


// -----------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------
TArchiveD::~TArchiveD( void )
{
}


// -----------------------------------------------------------------------
// Ok
// -----------------------------------------------------------------------
void TArchiveD::Ok( RTMessage Msg )
{
    (*itsShouldDeleteP) = (itsDeleteButtonP->GetCheck() == BF_CHECKED);

    TOurFileDialog::Ok( Msg );

}
