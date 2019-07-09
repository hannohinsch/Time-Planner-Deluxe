//*****************************************************************************
// import/export field map dialogs
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "fldmapd.h"
#include "address.h"

Str32 noteFieldNames[] =
    {
    "COLOR",
    "TEXT",
    "TIME UNITS",
    "START DATE",
    "START TIME",
    "DURATION",
    "AMOUNT",
    "RATE"
    };

Str32 addressFieldNames[] =
    {
    "FIRST NAME",
    "LAST NAME",
    "COMPANY",
    "WORK ADDRESS 1",
    "WORK ADDRESS 2",
    "WORK ADDRESS 3",
    "WORK CITY",
    "WORK STATE",
    "WORK ZIP",
    "WORK COUNTRY",
    "WORK TELEPHONE",
    "WORK FAX",
    "DESCRIPTION",
    "MR./MS.",
    "TITLE",

    "HOME PHONE",
    "CELLULAR PHONE",
    "ALT 1 PHONE",
    "ALT 2 PHONE",
    "EMAIL ADDRESS",

    "HOME ADDRESS 1",
    "HOME ADDRESS 2",
    "HOME ADDRESS 3",
    "HOME CITY",
    "HOME STATE",
    "HOME ZIP",
    "HOME COUNTRY",

    "ALT ADDRESS 1",
    "ALT ADDRESS 2",
    "ALT ADDRESS 3",
    "ALT CITY",
    "ALT STATE",
    "ALT ZIP",
    "ALT COUNTRY"
    };

// X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X
//
//                  ExportMapDialog
//
// X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TExportMapDialog::TExportMapDialog( PTWindowsObject AParent, WORD id ):
    TOurDialog( AParent, id )
{
    itsSourceCP  = new TListBox( this, IDD_EXPORT_MAP_SOURCE );
    itsTargetCP  = new TListBox( this, IDD_EXPORT_MAP_TARGET );
    itsAddCP     = new TButton(  this, IDD_EXPORT_MAP_ADD );
    itsRemoveCP  = new TButton(  this, IDD_EXPORT_MAP_REMOVE );
    itsClearCP   = new TButton(  this, IDD_EXPORT_MAP_CLEAR );
    itsDefaultCP = new TButton(  this, IDD_EXPORT_MAP_DEFAULT );

    itsSourceCP->DisableTransfer();
    itsTargetCP->DisableTransfer();

    itsHelpTopic= _HelpExportMap;
}


// -------------------------------------------------
// SetupWindow
// -------------------------------------------------
void TExportMapDialog::SetupWindow( void )
{
    // inherited
    TOurDialog::SetupWindow();

    SetupStrings();

    // initialize the source box (manually because of Borland combo box bug)
    for (int i = 0; i < itsNumKeyStrings; i++)
        {
        itsSourceCP->AddString( itsKeyStrings[i] );
        }

    // initialize the destination box (manually because of Borland combo box bug)
    for (i = 0; i < *itsNumKeysP; i++)
        {
        char buf[80];

        KeyToString( itsKeys[i], buf );
        itsTargetCP->AddString( buf );
        }

    // clear button is off if there is nothing in target
    if (i == 0)
        {
        EnableWindow( itsClearCP->HWindow, FALSE );
        }

    // disable the add and remove buttons
    EnableWindow( itsAddCP->HWindow, FALSE );
    EnableWindow( itsRemoveCP->HWindow, FALSE );
}

// -------------------------------------------------
// HandleSource
// -------------------------------------------------
void TExportMapDialog::HandleSource( RTMessage msg )
{
    if (msg.LP.Hi == LBN_DBLCLK)
        {
        int index = itsSourceCP->GetSelIndex();
        itsTargetCP->AddString( itsKeyStrings[index] );

        // deselect anything in the target column
        itsTargetCP->SetSelIndex( -1 );
        EnableWindow( itsClearCP->HWindow, TRUE );
        }
    else if(msg.LP.Hi == LBN_SELCHANGE)
        {
        // turn ADD button on, REMOVE button off
        EnableWindow( itsAddCP->HWindow, TRUE );
        EnableWindow( itsRemoveCP->HWindow, FALSE );

        // deselect anything in the target column
        itsTargetCP->SetSelIndex( -1 );
        }
}

// -------------------------------------------------
// HandleTarget
// -------------------------------------------------
void TExportMapDialog::HandleTarget( RTMessage msg )
{
    if (msg.LP.Hi == LBN_DBLCLK)
        {
        int index = itsTargetCP->GetSelIndex();
        itsTargetCP->DeleteString( index );

        // deselect anything in the source column
        itsSourceCP->SetSelIndex( -1 );
        }
    else if(msg.LP.Hi == LBN_SELCHANGE)
        {
        // turn ADD button off, REMOVE button on
        EnableWindow( itsAddCP->HWindow, FALSE );
        EnableWindow( itsRemoveCP->HWindow, TRUE );

        // deselect anything in the source column
        itsSourceCP->SetSelIndex( -1 );
        }
}

// -------------------------------------------------
// HandleAdd
//
//  Copy the selected source list to the target list
// -------------------------------------------------
void TExportMapDialog::HandleAdd( RTMessage )
{
    int index = itsSourceCP->GetSelIndex();
    if (itsTargetCP->GetCount() < kMaxKeys)
        {
        itsTargetCP->AddString( itsKeyStrings[index] );
        }

    EnableWindow( itsClearCP->HWindow, TRUE );
}

// -------------------------------------------------
// HandleRemove
//
//  Copy the selected source list to the target list
// -------------------------------------------------
void TExportMapDialog::HandleRemove( RTMessage )
{
    int index = itsTargetCP->GetSelIndex();
    itsTargetCP->DeleteString( index );

    // disable the remove button
    EnableWindow( itsRemoveCP->HWindow, FALSE );

    if (itsTargetCP->GetCount() == 0)
        {
        EnableWindow( itsClearCP->HWindow, FALSE );
        }
}

// -------------------------------------------------
// HandleClear
//
//  Clear the list
// -------------------------------------------------
void TExportMapDialog::HandleClear( RTMessage )
{
    int numItems = itsTargetCP->GetCount();
    for (int i = 0; i < numItems; i++)
        {
        itsTargetCP->DeleteString( 0 );
        }


    // disable all buttons
    EnableWindow( itsAddCP->HWindow, FALSE );
    EnableWindow( itsRemoveCP->HWindow, FALSE );
    EnableWindow( itsClearCP->HWindow, FALSE );

    itsSourceCP->SetSelIndex( -1 );
    itsTargetCP->SetSelIndex( -1 );

}

// -------------------------------------------------
// HandleDefault
//
//  clear the fields and stuff the defaults in
// -------------------------------------------------
void TExportMapDialog::HandleDefault( RTMessage )
{
    int numItems = itsTargetCP->GetCount();
    for (int i = 0; i < numItems; i++)
        {
        itsTargetCP->DeleteString( 0 );
        }

    EnableWindow( itsAddCP->HWindow, FALSE );
    EnableWindow( itsRemoveCP->HWindow, FALSE );
    EnableWindow( itsClearCP->HWindow, TRUE );

    itsSourceCP->SetSelIndex( -1 );
    itsTargetCP->SetSelIndex( -1 );

    // each item in the key strings is a default
    for (i = 0; i < itsNumKeyStrings; i++)
        {
        itsTargetCP->AddString( itsKeyStrings[i] );
        }
}

// -------------------------------------------------
// Ok
// -------------------------------------------------
void TExportMapDialog::Ok( RTMessage msg )
{
    *itsNumKeysP = itsTargetCP->GetCount();

    for (int i = 0; i < itsTargetCP->GetCount(); i++)
        {
        // read the i'th string from the control
        char buf[256];
        itsTargetCP->GetString( buf, i );

        // assign it to the map
        itsKeys[i] = StringToKey( buf );
        }

    TOurDialog::Ok( msg );
}

// -------------------------------------------------
// StringToKey
// -------------------------------------------------
int TExportMapDialog::StringToKey( char *buf )
{
    // compare it to each of the possible strings
    for (int i = 0; i < itsNumKeyStrings; i++)
        {
        if (!strcmp( buf, itsKeyStrings[i] ))
            {
            return i;
            }
        }

    return kTextDataKey;
}

// -------------------------------------------------
// KeyToString
// -------------------------------------------------
void TExportMapDialog::KeyToString( int key, char *buf )
{
    strcpy( buf, itsKeyStrings[key] );
}

// -------------------------------------------------
// SetupStrings
// -------------------------------------------------
void TExportMapDialog::SetupStrings( void )
{
    itsKeyStrings    = ::noteFieldNames;
    itsNumKeyStrings = kNumDataKeys - 1;
    itsKeys          = TFieldMap::gFieldMapP->itsExportDataKeys;
    itsNumKeysP      = &TFieldMap::gFieldMapP->itsNumExportDataKeys;
}

// X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X
//
//                  ExportAddressMapDialog
//
// X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TExportAddressMapDialog::TExportAddressMapDialog(
    PTWindowsObject AParent ):
    TExportMapDialog( AParent, IDD_EXPORT_ADDR_MAP )
{
itsHelpTopic= _HelpAddrExportOrder;
}

// -------------------------------------------------
// SetupStrings
// -------------------------------------------------
void TExportAddressMapDialog::SetupStrings( void )
{
    itsKeyStrings    = ::addressFieldNames;
    itsNumKeyStrings = kAllFields;
    itsKeys          = TFieldMap::gFieldMapP->itsExportAddressKeys;
    itsNumKeysP      = &TFieldMap::gFieldMapP->itsNumExportAddressKeys;
}

// X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X
//
//                  ImportMapDialog
//
// X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TImportMapDialog::TImportMapDialog(
    PTWindowsObject AParent, char *fileName, WORD id ):
    TOurDialog( AParent, id )
{
    itsHelpTopic= _HelpImportMap;

    itsSourceCP  = new TListBox( this, IDD_IMPORT_MAP_SOURCE );
    itsTargetCP  = new TListBox( this, IDD_IMPORT_MAP_TARGET );
    itsAddCP     = new TButton(  this, IDD_IMPORT_MAP_ADD );
    itsRemoveCP  = new TButton(  this, IDD_IMPORT_MAP_REMOVE );

    itsSourceCP->DisableTransfer();
    itsTargetCP->DisableTransfer();
    itsFileName = fileName;
}


// -------------------------------------------------
// SetupWindow
// -------------------------------------------------
void TImportMapDialog::SetupWindow( void )
{
    // inherited
    TOurDialog::SetupWindow();

    SetupStrings();

    // setup sources
    TCSV csv;
    BOOL eol;
    // open the input file
    ifstream is( itsFileName, ios::in );
    do {

        TSafStr token;
        csv.ReadString( is, token, eol );
        if( token.IsBlank() )
            token= "<blank>";
        itsSourceCP->AddString( SAF2CP( token ) );

    } while (!eol);

    // close the file
    is.close();

    // initialize the destination box source strings
    for (int i = 0; i < itsNumKeyStrings; i++)
        {
        //problem with saving import field maps is that the number
        //of import data fields might be very different between
        //invocations
        #if 0
        char buf1[200], buf2[200];

        int sourceItem= itsKeys[ i ];
        if (sourceItem == itsIgnoreKey) {

            wsprintf( buf2, "%s", itsKeyStrings[ i ] );
        }
        else {

            itsSourceCP->GetString( buf1, sourceItem );
            wsprintf( buf2, "%s => %s",
                itsKeyStrings[ i ], buf1 );
        }

        itsTargetCP->AddString( buf2 );
        itsMatchKeys[i] = itsKeys[i];
        #endif

        itsTargetCP->AddString( itsKeyStrings[ i ] );
        itsMatchKeys[i] = itsIgnoreKey;
        }

    // disable the remove button
    EnableWindow( itsRemoveCP->HWindow,
        itsMatchKeys[0] != itsIgnoreKey );

    // there is exactly one selected at all
    // times in the source and target controls
    itsSourceCP->SetSelIndex( 0 );
    itsTargetCP->SetSelIndex( 0 );
}

// -------------------------------------------------
// SetupStrings
// -------------------------------------------------
void TImportMapDialog::SetupStrings( void )
{
    itsKeyStrings    = ::noteFieldNames;
    itsNumKeyStrings = kNumDataKeys - 1;
    itsKeys          = TFieldMap::gFieldMapP->itsImportDataKeys;
    itsNumKeysP      = &TFieldMap::gFieldMapP->itsNumImportDataKeys;
    itsIgnoreKey     = kIgnoreDataKey;
}

// -------------------------------------------------
// HandleTarget
//
//  Toggle the remove button
// -------------------------------------------------
void TImportMapDialog::HandleTarget( RTMessage msg )
{
    if (msg.LP.Hi == LBN_SELCHANGE) {

       int index = itsTargetCP->GetSelIndex();
       EnableWindow( itsRemoveCP->HWindow,
           itsMatchKeys[index] != itsIgnoreKey );
    }
}

// -------------------------------------------------
// SetTarget
//
//  Set the target string for item i, given it's
//  key
// -------------------------------------------------
void TImportMapDialog::SetTarget( int sourceItem, int destItem )
{
    char buf1[200], buf2[200];

    if (sourceItem == itsIgnoreKey) {

        wsprintf( buf2, "%s", itsKeyStrings[destItem] );
    }
    else {

        itsSourceCP->GetString( buf1, sourceItem );
        wsprintf( buf2, "%s => %s",
            itsKeyStrings[destItem], buf1 );
    }
    itsTargetCP->DeleteString( destItem );
    itsTargetCP->InsertString( buf2, destItem );
    itsTargetCP->SetSelIndex( destItem );
    itsMatchKeys[destItem] = sourceItem;
}

// -------------------------------------------------
// HandleAdd
//
//  Copy the selected source list to the target list
// -------------------------------------------------
void TImportMapDialog::HandleAdd( RTMessage )
{
    SetTarget( itsSourceCP->GetSelIndex(),
        itsTargetCP->GetSelIndex() );
    EnableWindow( itsRemoveCP->HWindow, TRUE );
}

// -------------------------------------------------
// HandleRemove
//
//  Copy the selected source list to the target list
// -------------------------------------------------
void TImportMapDialog::HandleRemove( RTMessage )
{
    SetTarget( itsIgnoreKey, itsTargetCP->GetSelIndex() );
    EnableWindow( itsRemoveCP->HWindow, FALSE );
}

// -------------------------------------------------
// Ok
// -------------------------------------------------
void TImportMapDialog::Ok( RTMessage msg )
{
    *itsNumKeysP = itsSourceCP->GetCount();

    for(int i = 0; i < *itsNumKeysP; i++) {

        BOOL found = FALSE;
        for (int j = 0; j < itsNumKeyStrings; j++) {

            if (itsMatchKeys[j] == i) {

                itsKeys[i] = j;
                found = TRUE;
                break;
            }
        }
        if (!found) {

            itsKeys[i] = itsIgnoreKey;
        }
    }

    TOurDialog::Ok( msg );
}



// X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X
//
//                  ImportAddressMapDialog
//
// X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X-X

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TImportAddressMapDialog::TImportAddressMapDialog(
    PTWindowsObject AParent, char *fileName ):
    TImportMapDialog( AParent, fileName, IDD_IMPORT_ADDR_MAP )
{
itsHelpTopic= _HelpAddrImportOrder;
}

// -------------------------------------------------
// SetupStrings
// -------------------------------------------------
void TImportAddressMapDialog::SetupStrings( void )
{
    itsKeyStrings    = ::addressFieldNames;
    itsNumKeyStrings = kAllFields;
    itsKeys          = TFieldMap::gFieldMapP->itsImportAddressKeys;
    itsNumKeysP      = &TFieldMap::gFieldMapP->itsNumImportAddressKeys;
    itsIgnoreKey     = kIgnoreAddressField;
}
