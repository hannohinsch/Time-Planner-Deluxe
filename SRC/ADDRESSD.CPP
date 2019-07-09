//*****************************************************************************
// address dialog
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "addressd.h"

#ifndef __addrbook__
#include "addrbook.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __cat__
#include "cat.h"
#endif
#ifndef __catlist__
#include "catlist.h"
#endif
#ifndef __cattree__
#include "cattree.h"
#endif
#ifndef __cursor__
#include "cursor.h"
#endif
#ifndef __csv__
#include "csv.h"
#endif
#ifndef __print__
#include "print.h"
#endif
#ifndef __oem__
#include "oem.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __noteview__
#include "noteview.h"
#endif
#ifndef __ourfiled__
#include "ourfiled.h"
#endif
#ifndef  __dialdlg__
#include "dialdlg.h"
#endif
#ifndef  __impexpd__
#include "impexpd.h"
#endif
#ifndef  __fldmapd__
#include "fldmapd.h"
#endif

//needed for file access() function
#include "io.h"

//static variables
HANDLE TAddressDialog::itsWindowHandle = NULL;
char TAddressDialog::itsSavedSearchText[ kMaxLen ]= { '\0' };
int TAddressDialog::itsSavedIndex= 0;

// -----------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------
TAddressDialog::TAddressDialog( PTWindowsObject AParent, WORD AResID )
    : TOurDialog( AParent, AResID )
{
    IAddressDialog();
    itsAddressP = NthAddress( itsSavedIndex );
}

// -----------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------
TAddressDialog::TAddressDialog( PTWindowsObject AParent, WORD AResID, RTCat cat )
    : TOurDialog( AParent, AResID )
{
    IAddressDialog();
    itsAddressP = &cat;
}

// -----------------------------------------------------------------
// IAddressDialog
//
//  Handles the common features of the two constructors
// -----------------------------------------------------------------
void TAddressDialog::IAddressDialog( void )
{
    // create the data controls
    for (int i = 0; i < kNumEditFields; i++) {

        itsDataFields[i] = new TEdit( this,
            IDD_ADD_FIRST_DATA_FIELD + i, 256 );
        trnBuf.dataFields[i][0] = '\0';
    }

    // create the search field
    itsSearchFieldCP = new TEdit( this, IDD_ADD_SEARCH_FIELD, 256 );
    strcpy( trnBuf.searchField, itsSavedSearchText );

    SetTransferBuffer( (LPSTR) &trnBuf );

    // create the buttons
    itsAddButton        = new TButton( this, IDD_ADD_ADD_BUTTON );
    itsDeleteButton     = new TButton( this, IDD_ADD_DEL_BUTTON );
    itsImportButton     = new TButton( this, IDD_ADD_IMPORT );
    itsExportButton     = new TButton( this, IDD_ADD_EXPORT );
    itsOkButton         = new TButton( this, IDOK );
    itsNextButton       = new TButton( this, IDD_ADD_NEXT_BUTTON );
    itsPreviousButton   = new TButton( this, IDD_ADD_LAST_BUTTON );
    itsPrintButton      = new TButton( this, IDD_ADD_PRINT );
    itsClpbrdButton     = new TButton( this, IDD_ADD_CLPBRD );
    itsNewNoteButton    = new TButton( this, IDD_ADD_NEW_NOTE );
    itsDialButton       = new TButton( this, IDD_ADD_DIAL );
    itsNextMarkedButton = new TButton( this, IDD_ADD_NEXT_MARKED );
    itsMarkMatchingButton = new TButton( this, IDD_ADD_MARK_MATCHES );
    itsUnmarkAllButton  = new TButton( this, IDD_ADD_UNMARK_ALL );
    itsFindButton       = new TButton( this, IDD_ADD_FIND );

    for (i = 0; i < 26; i++) {

        itsLetterButtons[i] = new TButton(
            this, IDD_ADD_A_TO_Z + i );
    }

    itsMarkCP           = new TCheckBox( this, IDD_ADD_MARK, NULL );
    itsMarkCP->DisableTransfer();

    // initialize the count field
    itsCountField   = new TStatic( this, IDD_ADD_COUNT, kMaxLen );

    itsDataFieldsShowing = TRUE;
    itsWHAMode= kWorkAddress;
    itsAddressP= NULL;

    // make sure the root category tree has a
    // master <addresses> cat
    if ((RObject) TSysState::pCategories->FindCatInTopLevel( "<addresses>" ) == NOOBJECT) {

        PTCat addressCatP = new TCat( kMasterAddressCatType );
        addressCatP->Rename( "<addresses>" );
        addressCatP->SetNickname( "<addr>" );
        TSysState::pCategories->add( *addressCatP );
        Notify( kCatNew, (RObject) *addressCatP );
    }

    // set itsAddressesP to point to the addresses
    RTCat cat =
        TSysState::pCategories->FindCatInTopLevel(  "<addresses>" );
    itsAddressesP = cat.GetSubCats();

    // set help
    itsHelpTopic= _HelpAddress;
}


// -----------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------
TAddressDialog::~TAddressDialog( void )
{
    //save index, search string
    itsSavedIndex= itsAddressIndex;

    strcpy( itsSavedSearchText, trnBuf.searchField );

    // remove the <addresses> category if there
    // are no addresses in it
    if (itsAddressesP->NumCatsInTree() == 0) {

        RTCat cat = TSysState::pCategories->FindCatInTopLevel( "<addresses>" );
        TSysState::pCategories->detach( cat );
        Notify( kCatDel, (RObject) cat );
        delete &cat;
    }
}

// -----------------------------------------------------------------
// SetupWindow
//
// Initialize the on-screen items, once they have been created
// -----------------------------------------------------------------
void TAddressDialog::SetupWindow()
{
    TOurDialog::SetupWindow();

    //start with work address
    CheckDlgButton( HWindow, IDD_ADD_ADDR_BUS, 1 );

    //put up address fields
    SelectAddress( itsAddressP );

    // enable the appropriate letter buttons
    TurnOnLetters();

    itsWindowHandle = HWindow;
}

// -----------------------------------------------------------------
// TurnOnLetters
//
// Turn on the letter buttons as needed for the
// addresses in the set
// -----------------------------------------------------------------
void TAddressDialog::TurnOnLetters( void )
{
    BOOL letterUsed[26];

    for (int i = 0; i < 26; i++) {

        letterUsed[i] = FALSE;
    }

    for (i = 0; i < NumAddresses(); i++) {

        PTCat addressP = NthAddress( i );
        TSafStr name;

        addressP->GetShortName( name );
        name.ToUpper();

        letterUsed[(name.Addr())[0] - 'A'] = TRUE;
    }

    for (i = 0; i < 26; i++) {

        HANDLE h = itsLetterButtons[i]->HWindow;
        if (letterUsed[i] && !IsWindowEnabled( h )) {

            EnableWindow( h, TRUE );
        }
        else if (IsWindowEnabled( h ) && !letterUsed[i]) {

            EnableWindow( h, FALSE );
        }
    }

}

// -----------------------------------------------------------------
// SelectAddress
//
// Select the address catP, or if it's NULL,
// then select the first address
// Sets itsAddressP and itsAddressIndex
//*****************************************************************************
void TAddressDialog::SelectAddress( PTCat catP )
{
    // show the new address
    if (NumAddresses() == 0) {

        SetMode( kEmptySetMode );
        itsAddressP = NULL;
        itsAddressIndex = -1;
    }
    else {

        itsAddressIndex = (catP == NULL) ?
            0 : itsAddressesP->find( *catP );
        itsAddressP = NthAddress( itsAddressIndex );

        SetMode( kNormalMode );
    }

    // enable new note?
    EnableWindow( itsNewNoteButton->HWindow,
        itsAddressP && HasName() );
    EnableWindow( itsDialButton->HWindow,
        itsAddressP && HasName() );

    // display information
    ShowAddress();
    UpdateCount();
}

// -----------------------------------------------------------------
// ShowAddress
//
// Transfer the address from a category to the dialog box
// -----------------------------------------------------------------
void TAddressDialog::ShowAddress( void )
{

    if (itsAddressP == NULL) {

        // clear out the screen fields
        for (int i = 0; i < kNumEditFields; i++) {

            trnBuf.dataFields[i][0] = '\0';
        }
    }
    else {

        // copy the address onto the screen
        for (int i = 0; i < kNumEditFields; i++) {

            itsAddressP->GetField( (TAddressField) i,
                trnBuf.dataFields[i], kMaxLen, itsWHAMode );
        }

        itsMarkCP->SetCheck( itsAddressP->GetAddrMark());

    }

    // transfer
    TransferData( TF_SETDATA );
}


// -----------------------------------------------------------------
// UpdateCount
//
// Update the count field
// -----------------------------------------------------------------
void TAddressDialog::UpdateCount( void  )
{
    char buf[256];

    wsprintf( buf, "%d of %d",
        itsAddressIndex + 1, NumAddresses());
    itsCountField->SetText( buf );
}

//*****************************************************************************
// an easy place to intercept letter and field messages...
//*****************************************************************************
void TAddressDialog::DefChildProc( RTMessage msg )
{
WORD id= msg.WParam;

if( id >= IDD_ADD_A_TO_Z && id < IDD_ADD_A_TO_Z + 26 )
    HandleLetterButton( 'A' + (id - IDD_ADD_A_TO_Z) );
else if( id >= IDD_ADD_FIRST_DATA_FIELD && id < IDD_ADD_FIRST_DATA_FIELD + kNumEditFields )
    HandleDataField( msg, (id - IDD_ADD_FIRST_DATA_FIELD) );
else
    TOurDialog::DefChildProc( msg );
}


// -----------------------------------------------------------------
// HandleDataField
//
// If the focus is lost in a data field then modify that field
// of the current address
// -----------------------------------------------------------------
void TAddressDialog::HandleDataField( RTMessage msg, int field )
{

    if (msg.LP.Hi == EN_KILLFOCUS && itsAddressP != NULL )
        {
        char buf[256], buf2[256];

        itsDataFields[field]->GetText( buf, 256 );
        itsAddressP->GetField( (TAddressField) field, buf2, sizeof( buf2), itsWHAMode );

        if( 0 != strcmp( buf, buf2 ) )
            {
            itsAddressP->SetField( (TAddressField) field, buf, itsWHAMode );

            TSysState::bSystemDirty= TRUE;

            // name field is a special case
            if (field == kLastNameField ||
                field == kFirstNameField ||
                field == kCompanyField )
                {
                SetName( itsAddressP );
                }
            }
        }
}

// -----------------------------------------------------------------
// HandleAdd
//
// Handle the Add button: transfer the address from the dialog
// box into a new category
// -----------------------------------------------------------------
void TAddressDialog::HandleAdd( RTMessage  )
{
    if (!TSysState::pCategories->RoomForMore( 2, TRUE, TRUE,
        "Can't add a new address. You have reached the 3000 folder limit." ))
        return;

    // create an empty address and put it in the lists
    PTCat catP = new TCat( kAddressCatType );
    if (catP == NULL)
        return;

    itsAddressesP->add( *catP );
    Notify( kCatNew, (RObject) *catP );
    SelectAddress( catP );

    SetFocus( itsDataFields[kFirstNameField]->HWindow );
}


//*****************************************************************************
// transfer current address to system clipboard
//*****************************************************************************
void TAddressDialog::HandleClpbrd( RTMessage )
{
//prepare formatted address (name, address, no more)
TSafStr addr;
itsAddressP->FormatAddr( addr, TRUE, TRUE, FALSE, itsWHAMode );

//
//put into clipboard
//
OpenClipboard( HWindow );
EmptyClipboard();

//put note text into clipboard
// for other applications
int n= 1 + addr.Len();
HANDLE hText= GlobalAlloc( GMEM_SHARE, n );
char *p= GlobalLock( hText );
strcpy( p, addr );
GlobalUnlock( hText );

SetClipboardData( CF_TEXT, hText );

CloseClipboard();
}

//*****************************************************************************
// HandleNewNote
//
//  Put up a note dialog
//*****************************************************************************
void TAddressDialog::HandleNewNote( RTMessage )
{
// if there's already a note open, then pop
// that note to the top
if (TNoteView::CurrentNoteViewP != NULL)
    {
    SetWindowPos( TNoteView::CurrentNoteViewP->HWindow,
        HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
    }
else        //  make a new note
    {
    PTNote pNote= TSysState::pAllNotes->CreateNote( this );
    if( pNote != NULL )
        {
        RTCatList catList = pNote->GetCatList();
        catList.add( *itsAddressP );
        GetApplication()->ExecDialog(new TNoteView(
            TPimFrame::pFrame, IDD_NOTE, pNote ));
        }
    }
}

//*****************************************************************************
// HandleDial
//*****************************************************************************
void TAddressDialog::HandleDial( RTMessage )
{
    if (TDialDialog::CurrentDialDialog)
        {
        BringWindowToTop(
            TDialDialog::CurrentDialDialog->HWindow );
        }
    else
        {
        PTDialDialog pDlg= new TDialDialog(
            TPimFrame::pFrame, itsAddressP );
        GetApplicationObject()->MakeWindow( pDlg );
        ShowWindow( pDlg->HWindow, TRUE );
        }
} 

//*****************************************************************************
// print address book
//*****************************************************************************
void TAddressDialog::HandlePrint( RTMessage )
{
TCursor::Busy( TRUE );

PTCatList catsP = new TCatList( *itsAddressesP );
PTRawGrid addrGridP= new TAddrGrid( catsP );

TPrintView printObj( this );
printObj.PrepareToPrint( addrGridP, IDD_ADDR_PRTDLG );

delete addrGridP;
delete catsP;

TCursor::Busy( FALSE );
}


// -----------------------------------------------------------------
// HandleLast
//
// Handle the Previous button: show the previous address
// -----------------------------------------------------------------
void TAddressDialog::HandleLast( RTMessage  )
{
    if (itsAddressIndex > 0) {

        PTCat catP= NthAddress( itsAddressIndex - 1 );
        SelectAddress( catP );
    }
}

// -----------------------------------------------------------------
// HandleNext
//
// Handle the Next button: show the next address
// -----------------------------------------------------------------
void TAddressDialog::HandleNext( RTMessage  )
{
    if (itsAddressIndex < NumAddresses() - 1) {

        PTCat catP = NthAddress( itsAddressIndex + 1 );
        SelectAddress( catP );
    }
}

// -----------------------------------------------------------------
// HandleNextMarked
//
// Handle the NextMarked button: show the next
// marked address
// -----------------------------------------------------------------
void TAddressDialog::HandleNextMarked( RTMessage  )
{
    int index = itsAddressIndex;
    while (index < NumAddresses() - 1) {

        PTCat addressP = NthAddress( ++index );
        if (addressP->GetAddrMark()) {

            SelectAddress( addressP );
            return;
        }
    }

    MessageBeep( -1 );
}



// -----------------------------------------------------------------
// HandleSearchField
//
// Handle changes in a search field
// -----------------------------------------------------------------
void TAddressDialog::HandleSearchField( RTMessage msg )
{
    // make sure we save the search criteria
    // TransferData( TF_GETDATA );

    //make FindNext the default button while control has focus
    // (otherwise default is OK button)
    if( msg.LP.Hi == EN_KILLFOCUS )
        SendMessage( HWindow, DM_SETDEFID, (WPARAM) IDOK, 0L );
    else if( msg.LP.Hi == EN_SETFOCUS )
        SendMessage( HWindow, DM_SETDEFID, (WPARAM) IDD_ADD_FIND, 0L );

}

// -----------------------------------------------------------------
// HandleDelete
//
// Handle delete button
// -----------------------------------------------------------------
void TAddressDialog::HandleDelete( RTMessage  )
{

    if( IDOK == TMessageBox::OkAndCancel( this,
        "Delete this address?",
        "Delete Address?" )) {

        if (itsAddressP == NULL)
            return;

        TSysState::pCategories->detach( *itsAddressP );
        Notify( kCatDel, (RObject) *itsAddressP );

        delete itsAddressP;
        itsAddressP= NULL;

        SelectAddress( NULL );

        TurnOnLetters();
    }
}

//*****************************************************************************
// show first address starting with clicked letter
//*****************************************************************************
void TAddressDialog::HandleLetterButton( char letter )
{
    int index;
    PTCat addrP;
    TSafStr name;
    for( index= 0; index < NumAddresses(); index++ )
        {
        addrP = NthAddress( index );
        addrP->GetShortName( name );
        name.ToUpper();
        if( *name.Addr() >= letter )
            break;
        }

    if( *name.Addr() >= letter )
        SelectAddress( addrP );
    else
        MessageBeep( -1 );
}

//*****************************************************************************

//*****************************************************************************
void TAddressDialog::HandleHome( RTMessage msg )
{
if( msg.LP.Hi == BN_CLICKED )
    {
    itsWHAMode= kHomeAddress;
    ShowAddress();
    }
}

//*****************************************************************************

//*****************************************************************************
void TAddressDialog::HandleWork( RTMessage msg )
{
if( msg.LP.Hi == BN_CLICKED )
    {
    itsWHAMode= kWorkAddress;
    ShowAddress();
    }
}

//*****************************************************************************

//*****************************************************************************
void TAddressDialog::HandleAlt( RTMessage msg )
{
if( msg.LP.Hi == BN_CLICKED )
    {
    itsWHAMode= kAltAddress;
    ShowAddress();
    }
}

//*****************************************************************************

//*****************************************************************************
void TAddressDialog::HandleUnmarkAll( RTMessage )
{
    for (int i = 0; i < NumAddresses(); i++) {

        PTCat catP = NthAddress( i );
        catP->SetAddrMark( FALSE );
    }

    if (itsAddressP) {

        itsMarkCP->SetCheck( FALSE );
    }
}

//*****************************************************************************

//*****************************************************************************
void TAddressDialog::HandleFindNext( RTMessage )
{
    // make sure we have the latest search criteria
    TransferData( TF_GETDATA );

    // loop through the notes from here to the end
    // of the set
    for (int i = itsAddressIndex + 1; i < NumAddresses(); i++)
        {
        PTCat addressP = NthAddress( i );
        if (addressP->Matches( trnBuf.searchField ))
            {
            SelectAddress( addressP );
            return;
            }
        }

    // nothing was found up to the end of the list,
    // should we start again from the beginning?
    if (itsAddressIndex > 0)
        {
        if( IDOK == TMessageBox::OkAndCancel( this,
            "Nothing matched. Start from the beginning?", "Find Next" ))
            {

            // look from the start of the list
            // up to the current position
            for (i = 0; i < itsAddressIndex; i++)
                {
                PTCat addressP = NthAddress( i );
                if (addressP->Matches( trnBuf.searchField ))
                    {
                    SelectAddress( addressP );
                    return;
                    }
                }

            // nothing was found
            MessageBeep( -1 );
            }
        }
    else
        {
        // nothing was found
        MessageBeep( -1 );
        }
}

//*****************************************************************************

//*****************************************************************************
void TAddressDialog::HandleMarkMatches( RTMessage )
{
    for( int i = 0; i < NumAddresses(); i++)
    {

         PTCat addressP = NthAddress( i );
         if (addressP->Matches( trnBuf.searchField ))
         {

             addressP->SetAddrMark( TRUE );
         }
    }

    if (itsAddressP)
    {

        itsMarkCP->SetCheck( itsAddressP->GetAddrMark());
    }
}

//*****************************************************************************

//*****************************************************************************
void TAddressDialog::HandleMark( RTMessage )
{
    if (itsAddressP)
    {
        itsAddressP->SetAddrMark( itsMarkCP->GetCheck() );
    }
}

// -----------------------------------------------------------------
// Ok
//
// Handle ok button
// -----------------------------------------------------------------
void TAddressDialog::Ok( RTMessage Msg )
{
    // make sure the last input field gets stored properly
    HWND focusWindow = GetFocus();
    if (focusWindow)
        {
        SendMessage( focusWindow, WM_KILLFOCUS, NULL, 0 );
        }

    // let the noteview know that we're
    // finished. Can't do this in dispose
    // because dispose wouldn't get called
    // until after the dispose for the noteview
    // dialog underneath
    itsWindowHandle = NULL;

    TDialog::Ok( Msg );
}

// -----------------------------------------------------------------
// ShutDownWindow
//
// If the user double clicked on the close box then
// make sure itsWindowHandle is NULL, because the
// noteview modal dialog might be up and call
// GetWindowHandle for the address dialog.
//
// The obvious way to deal with this (ie. to make
// itsWindowHandle null in the ~ method) doesn't
// work because the dispose method isn't called until
// AFTER the noteview dialog underneath is closed too.
// -----------------------------------------------------------------
void TAddressDialog::ShutDownWindow( void )
{
   itsWindowHandle = NULL;

   TDialog::ShutDownWindow();
   ShowNoteView();
}

void TAddressDialog::ShutDownWindow( int returnValue )
{
   itsWindowHandle = NULL;

   TDialog::ShutDownWindow( returnValue );
   ShowNoteView();
}

// -----------------------------------------------------------------
//  ShowNoteView
//
//  Hilite the note view window if there is one 
// -----------------------------------------------------------------
void TAddressDialog::ShowNoteView( void )
{
    // enable the note view if there is one
    if (TNoteView::CurrentNoteViewP)
        {
        // enable the note view, disable the frame
        // make the note view active
        EnableWindow(
            TNoteView::CurrentNoteViewP->HWindow,TRUE );
        EnableWindow(
            TPimFrame::pFrame->HWindow, FALSE );
        SetActiveWindow(
            TNoteView::CurrentNoteViewP->HWindow );
        }
    else
        {
        // enable the frame (it might have been
        // disabled when closing a note view)
        EnableWindow(
            TPimFrame::pFrame->HWindow, TRUE );
        }
}

// ----------------------------------------------------------------
// HasName
//
// ----------------------------------------------------------------
BOOL TAddressDialog::HasName( void )
{
    BOOL rc= TRUE;

    //FIXME check name fields of address

    return( rc );
}


// ----------------------------------------------------------------
// SetName
//
// Set the name of the current address, repositioning it in the
// various category lists
// ----------------------------------------------------------------
void TAddressDialog::SetName( PTCat catP )
{

    //figure out new name from fields
    TSafStr first, last, comp;
    catP->GetField( kFirstNameField, first, itsWHAMode );
    catP->GetField( kLastNameField, last, itsWHAMode );
    catP->GetField( kCompanyField, comp, itsWHAMode );

    TSafStr name;
    if( last.Len() > 0 && first.Len() > 0 )
        {
        //name is last, first
        name.Printf( "%s, %s", (const char *) last,
                                (const char *) first );
        }
    else if( first.Len() > 0 )
        {
        //name is first only
        name= first;
        }
    else if( last.Len() > 0 )
        {
        //name is last only
        name= last;
        }
    else if( comp.Len() > 0 )
        {
        //name is company
        name= comp;
        }

    // change the name, if it changed
    TSafStr str;
    catP->GetShortName( str );
    if( str != name )
        {
        catP->Rename( name );
        Notify( kCatNew, (RObject) *catP );
        TSysState::bSystemDirty= TRUE;
        }

    // check to see if we need to enable the new note button
    EnableWindow( itsNewNoteButton->HWindow,
        itsAddressP && HasName() );
    EnableWindow( itsDialButton->HWindow,
        itsAddressP && HasName() );

    // update the window
    itsAddressIndex = itsAddressesP->find( *catP );
    TurnOnLetters();
    UpdateCount();
}


// ----------------------------------------------------------------
// SetMode
//
// Put the dialog into a mode: kEmptySetMode hides the data fields;
// the others show the data fields
// ----------------------------------------------------------------
void TAddressDialog::SetMode( TAddressMode mode )
{
    int i;


    switch( mode ) {

        case kEmptySetMode:

            // disable the data fields
            if (itsDataFieldsShowing) {

                for (i = 0; i < kNumEditFields; i++) {

                    EnableWindow( itsDataFields[i]->HWindow, FALSE );
                }
                EnableWindow( itsNextButton->HWindow, FALSE );
                EnableWindow( itsPreviousButton->HWindow, FALSE );
                EnableWindow( itsDeleteButton->HWindow, FALSE );
                EnableWindow( itsExportButton->HWindow, FALSE );
                EnableWindow( itsPrintButton->HWindow, FALSE );
                EnableWindow( itsClpbrdButton->HWindow,FALSE );
                EnableWindow( itsNextMarkedButton->HWindow,FALSE );
                EnableWindow( itsMarkMatchingButton->HWindow,FALSE );
                EnableWindow( itsUnmarkAllButton->HWindow,FALSE );
                EnableWindow( itsFindButton->HWindow,FALSE );
                itsDataFieldsShowing = FALSE;
            }
            break;

        case kNormalMode:

            // show the data fields
            if (!itsDataFieldsShowing) {

                for (i = 0; i < kNumEditFields; i++) {

                    EnableWindow( itsDataFields[i]->HWindow, TRUE );
                }
                EnableWindow( itsNextButton->HWindow, TRUE );
                EnableWindow( itsPreviousButton->HWindow, TRUE );
                EnableWindow( itsDeleteButton->HWindow, TRUE );
                EnableWindow( itsExportButton->HWindow, TRUE );
                EnableWindow( itsPrintButton->HWindow, TRUE );
                EnableWindow( itsClpbrdButton->HWindow, TRUE );
                EnableWindow( itsNextMarkedButton->HWindow, TRUE );
                EnableWindow( itsMarkMatchingButton->HWindow, TRUE );
                EnableWindow( itsUnmarkAllButton->HWindow, TRUE );
                EnableWindow( itsFindButton->HWindow, TRUE );
                itsDataFieldsShowing = TRUE;
            }
            ShowAddress();
            break;
    }


    // save the mode
    itsMode = mode;
}


// -----------------------------------------------------------------------
// HandleExport
//
//  Export selected addresses
// -----------------------------------------------------------------------
void TAddressDialog::HandleExport( RTMessage )
{
    // get the path name
    char fName[MAXPATH];

    strcpy( fName, "*.csv" );
    BOOL markedNotesOnly;
    if ( IDOK != GetApplication()->ExecDialog(
        new TExportAddressDialog( this, fName, &markedNotesOnly ))) {

        return;
    }

    //if file already exists, ask for confirmation
    if( 0 == access( fName, 0 )) {

        char buf[200];
        wsprintf( buf, "File '%s'\nalready exists.\n\nOverwrite it?", fName );
        if( IDOK != TMessageBox::OkAndCancel( this,
            buf, "Confirm File Overwrite" ) )
            return;
        }

    UpdateWindow( HWindow );

    // open the output file
    ofstream os( fName, ios::trunc );

    TCursor::Busy( TRUE );

    // loop through the addresses writing them out
    int count = 0;
    for (int i = 0; i < NumAddresses(); i++) {

        PTCat catP = NthAddress( i );
	    if (catP != NULL) {

            //start with field names
            if( i == 0 )
                {
                if( 0 != catP->ExportAddress( os, TRUE ) )
                    break;
                }

            // either we export all notes, or this address
            // is marked
            BOOL exportAllNotes = !markedNotesOnly;
            if (exportAllNotes || catP->GetAddrMark()) {

                count++;
                if (catP->ExportAddress( os ) != 0) {

                    break;
                }
            }
        }
    }

    TCursor::Busy( FALSE );

    char s[50];
	wsprintf( s, "%d addresses written.", count );
    TMessageBox::Ok( this, s, "Export" );

    // close the file
    os.close();

}


// -----------------------------------------------------------------------
// HandleImport
// -----------------------------------------------------------------------
void TAddressDialog::HandleImport( RTMessage  )
{
    TCSV csv;

    // get the path name
    char fName[MAXPATH];

    strcpy( fName, "*.csv" );
    if ( IDOK != GetApplication()->ExecDialog(
        new TImportAddressDialog( this, fName ) )) {

        return;
    }

    // read the import map for the addresses
    if ( IDOK != GetApplication()->ExecDialog(
       new TImportAddressMapDialog( this, fName ))) {

       return;
    }

    UpdateWindow( HWindow );

    // open the input file
    ifstream is( fName, ios::in );

    TCursor::Busy( TRUE );

    // loop through the notes reading them in
    char c;
    int count = 0;
    while (is.get( c )) {

        is.putback( c );

        PTCat catP = new TCat( kAddressCatType );
        TCSVError err = catP->ImportAddress( is, &csv );
        if (err == kCSVOk) {

            if (!TSysState::pCategories->RoomForMore( 2, TRUE, FALSE,
                "Can't add a new address. You have reached the 3000 folder limit." ))
                {
                delete catP;
                break;
                }

            //figure out new name from fields
            TSafStr first, last, comp;
            catP->GetField( kFirstNameField, first, itsWHAMode );
            catP->GetField( kLastNameField, last, itsWHAMode );
            catP->GetField( kCompanyField, comp, itsWHAMode );

            TSafStr name;
            if( last.Len() > 0 && first.Len() > 0 )
                {
                //name is last, first
                name.Printf( "%s, %s", (const char *) last,
                                        (const char *) first );
                }
            else if( first.Len() > 0 )
                {
                //name is first only
                name= first;
                }
            else if( last.Len() > 0 )
                {
                //name is last only
                name= last;
                }
            else if( comp.Len() > 0 )
                {
                //name is company
                name= comp;
                }

            TSafStr str;
            catP->GetShortName( str );
            if( str != name )
                {
                catP->Rename( name );
                }

            itsAddressesP->add( *catP );
            Notify( kCatNew, (RObject) *catP );

            count++;
        }
        else if (err == kCSVErrorCancel) {

            delete catP;
            break;
        }
        else if (err == kCSVErrorSkip) {

            delete catP;
        }
    }

    TurnOnLetters();
    UpdateCount();
    SetMode( kNormalMode );
    SelectAddress( itsAddressP );

    TCursor::Busy( FALSE );

    if( count > 0 )
        TSysState::bSystemDirty= TRUE;

    char s[50];
	wsprintf( s, "%d addresses read.", count );
    TMessageBox::Ok( this, s, "Import" );

    // close the file
    is.close();
}


// -----------------------------------------------------------------------
// NumAddresses
// -----------------------------------------------------------------------
int TAddressDialog::NumAddresses( void )
{
    return itsAddressesP->getItemsInContainer();
}

// -----------------------------------------------------------------------
// NthAddress
// -----------------------------------------------------------------------
PTCat TAddressDialog::NthAddress( int n )
{
    if ((0 <= n) && (n < NumAddresses())) {

        return &(*itsAddressesP)[n];
    }
    else {

        return NULL;
    }
}
