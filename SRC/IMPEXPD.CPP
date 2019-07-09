//*****************************************************************************
// import / export dialogs
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "impexpd.h"
#ifndef __fieldmap__
#include "fieldmap.h"
#endif
#ifndef __fldmapd__
#include "fldmapd.h"
#endif

// -----------------------------------------------------------------------
// Constructor
//
//     Standard file dialog, except that the radio button is also allocated
// -----------------------------------------------------------------------
TExportDialog::TExportDialog( PTWindowsObject AParent,
        LPSTR AFilePath, BOOL *headlinesOnlyP )
     : TOurFileDialog( AParent, IDD_EXPORT, AFilePath, NULL )
{

    itsHelpTopic= _HelpExport;

    strcpy( Extension, ".csv");

    // create radio buttons, but don't let it transfer anything, since
    // presumably TFileDialog has its own transfer buffer already, and
    // will just confuse things if we try to set another transfer buffer
    // for the same dialog
    PTGroupBox pGrp= new TGroupBox( this, IDD_EXPORT_GROUP );

    itsHeadlinesButtonP = new TRadioButton( this, IDD_EXPORT_HEADLINES, pGrp );
    itsHeadlinesButtonP->DisableTransfer();

    itsFullTextButtonP= new TRadioButton( this, IDD_EXPORT_FULLTEXT, pGrp );
    itsFullTextButtonP->DisableTransfer();

    new TButton( this, IDD_EXPORT_FIELD_ORDER );

    itsHeadlinesOnlyP = headlinesOnlyP;
}


// -----------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------
TExportDialog::~TExportDialog( void )
{
}

//*****************************************************************************

//*****************************************************************************
void TExportDialog::SetupWindow( void )
{
TOurFileDialog::SetupWindow();

itsFullTextButtonP->SetCheck( BF_CHECKED );
}

// -----------------------------------------------------------------------
// Ok
// -----------------------------------------------------------------------
void TExportDialog::Ok( RTMessage Msg )
{
    // the two radio buttons are mutually exclusive, so this works
    (*itsHeadlinesOnlyP) = (itsHeadlinesButtonP->GetCheck() == BF_CHECKED);

    TOurFileDialog::Ok( Msg );
}


// -----------------------------------------------------------------------
// HandleFieldMap
// -----------------------------------------------------------------------
void TExportDialog::HandleFieldMap( RTMessage  )
{
    // get the field map
    GetApplication()->ExecDialog(
        new TExportMapDialog( this ));
}


//*****************************************************************************
// Constructor
//*****************************************************************************
TImportDialog::TImportDialog(
    PTWindowsObject AParent, 
    LPSTR AFilePath )
    : TOurFileDialog(AParent, IDD_IMPORT, AFilePath, NULL )
{
    itsHelpTopic= _HelpImport;

    strcpy( Extension, ".csv" );
}

// -----------------------------------------------------------------------
// Constructor
//
//     Standard file dialog, except that the radio button is also allocated
// -----------------------------------------------------------------------
TExportAddressDialog::TExportAddressDialog( PTWindowsObject AParent,
        LPSTR AFilePath, BOOL *markedAddrOnlyP )
     : TOurFileDialog( AParent, IDD_EXPORT_ADDRESS, AFilePath, NULL )
{

    strcpy( Extension, ".csv");
    SetCaption( "Export Addresses" );
    itsHelpTopic= _HelpAddrExport;

    new TButton( this, IDD_EXPORT_FIELD_ORDER );

    itsMarkedAddrOnlyCP = new TCheckBox( this, IDD_EXPORT_MARKED_ADDR_ONLY, NULL );
    itsMarkedAddrOnlyCP->DisableTransfer();

    itsMarkedAddrOnlyP = markedAddrOnlyP;
}


// -----------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------
TExportAddressDialog::~TExportAddressDialog( void )
{
}

//*****************************************************************************

//*****************************************************************************
void TExportAddressDialog::SetupWindow( void )
{
TOurFileDialog::SetupWindow();
}

// -----------------------------------------------------------------------
// HandleFieldMap
// -----------------------------------------------------------------------
void TExportAddressDialog::HandleFieldMap( RTMessage  )
{
    // get the field map
    GetApplication()->ExecDialog(
        new TExportAddressMapDialog( this ));
}


// -----------------------------------------------------------------------
// OK
//
//  sets itsMarkedOnlyP
// -----------------------------------------------------------------------
void TExportAddressDialog::Ok( RTMessage Msg )
{
    *itsMarkedAddrOnlyP = itsMarkedAddrOnlyCP->GetCheck();

    TOurFileDialog::Ok( Msg );
}

//*****************************************************************************
// Constructor
//*****************************************************************************
TImportAddressDialog::TImportAddressDialog(
    PTWindowsObject AParent, LPSTR AFilePath )
    : TOurFileDialog(AParent, IDD_IMPORT, AFilePath, NULL )
{
    strcpy( Extension, ".csv" );
    SetCaption( "Import Addresses" );

    itsHelpTopic= _HelpAddrImport;
}

