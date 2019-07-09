//*****************************************************************************
// category dialogs: cat mgr, new cat, delete cat
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "catdlg.h"

#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __note__
#include "note.h"
#endif
#ifndef __noteset__
#include "noteset.h"
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
#ifndef __catbox__
#include "catbox.h"
#endif
#ifndef __utility__
#include "utility.h"
#endif

enum { kCheckDelete, kCheckCreateSubcat, kCheckSelect };

//*****************************************************************************

//*****************************************************************************
TCatNew::TCatNew( PTWindowsObject AParent, int AResID, PTCat pCat ):
    TOurDialog( AParent, AResID )
{
    itsHelpTopic    = _HelpCreateFolder;

    itsCatP         = pCat;
    pNameBox        = new TEdit( this, IDD_CATNEWNAME, MAX_CAT_NAME_LEN );
    pNicknameBox    = new TEdit( this, IDD_SHORT_NEWCATNAME, MAX_CAT_NAME_LEN );
    pKeywordBox     = new TEdit( this, IDD_CATNEWKEY,  MAX_CAT_KEY_LEN );
    pDescriptionBox = new TEdit( this, IDD_CATNEWDESC, MAX_CAT_DESC_LEN );

    pNameBox->DisableTransfer();
    pNicknameBox->DisableTransfer();
    pKeywordBox->DisableTransfer();
    pDescriptionBox->DisableTransfer();
}

//*****************************************************************************
// add new category to global set
//*****************************************************************************
void TCatNew::Ok( RTMessage Msg )
{
    char buf[255];

    // get the name, keywords and description from the controls
    pKeywordBox->GetText( buf, 255 );
    itsCatP->SetKeywords( buf );

    pDescriptionBox->GetText( buf, 255 );
    itsCatP->SetDescription( buf );

    pNicknameBox->GetText( buf, 255 );
    itsCatP->SetNickname( IsBlank( buf ) ? "" : buf );

    // if the name is not empty then add the category
    pNameBox->GetText( buf, 255 );
    itsCatP->Rename( buf );
    if (IsBlank( buf )) {

        TMessageBox::Ok( this,
            "Blank names are not allowed.",
            "Bad Folder Name" );
        return;
    }
    else {

        // check that no category with that name already exists
        TSafStr s( buf );
        RTCat cat = TSysState::pCategories->FindCatInTree( s );
        if (((RObject) cat) != NOOBJECT) {

            TMessageBox::Ok( this,
                "A folder with that name already exists.",
                "Bad Folder Name" );
            return;
        }
    }

    TDialog::Ok( Msg );
}



//*****************************************************************************
// this is the category management dialog
//*****************************************************************************
TCatMgr::TCatMgr( PTWindowsObject AParent, int AResID ):
                  TOurDialog( AParent, AResID )
{

itsHelpTopic= _HelpFolderMan;

pNameBox= new TEdit( this, IDD_CATNAME, MAX_CAT_NAME_LEN );
pNicknameBox= new TEdit( this, IDD_SHORT_CATNAME, MAX_CAT_NAME_LEN );
pKeywordBox= new TEdit( this, IDD_CATKEYW, MAX_CAT_KEY_LEN );
pDescriptionBox= new TEdit( this, IDD_CATDESC, MAX_CAT_DESC_LEN );

PTCatBox catBoxP = new TCatBox( this, IDD_CATDELLIST );
catBoxP->UseTimerWithLButton( FALSE );

trnBuf.pCatBoxData= new TCatBoxData( FALSE, FALSE ); // no system, addr cats
trnBuf.pCatBoxData->SetSingleSelect( TRUE );
trnBuf.pCatBoxData->PrepareNone();    //empty list

//FIXME since there are no cats, these edit fields should be disabled!
pCat = (TCat*) &NOOBJECT;
trnBuf.name[0]= '\0';
trnBuf.nickname[0]= '\0';
trnBuf.keywords[0]= '\0';
trnBuf.description[0]= '\0';


pSubcatButton = new TBButton( this, IDD_CATMGRNEW_SUBCAT );
pDeleteButton = new TBButton( this, IDD_CATDELETE );

SetTransferBuffer( &trnBuf );

itsStartCatP = NULL;
}

//*****************************************************************************
// this is the category management dialog
//*****************************************************************************
TCatMgr::TCatMgr( PTWindowsObject AParent, int AResID, RTCat cat ):
                  TOurDialog( AParent, AResID )
{

itsHelpTopic= _HelpFolderMan;

pNameBox= new TEdit( this, IDD_CATNAME, MAX_CAT_NAME_LEN );
pNicknameBox= new TEdit( this, IDD_SHORT_CATNAME, MAX_CAT_NAME_LEN );
pKeywordBox= new TEdit( this, IDD_CATKEYW, MAX_CAT_KEY_LEN );
pDescriptionBox= new TEdit( this, IDD_CATDESC, MAX_CAT_DESC_LEN );

PTCatBox catBoxP = new TCatBox( this, IDD_CATDELLIST );
catBoxP->UseTimerWithLButton( FALSE );

trnBuf.pCatBoxData= new TCatBoxData( FALSE, FALSE ); // no system, addr cats
trnBuf.pCatBoxData->SetSingleSelect( TRUE );
trnBuf.pCatBoxData->PrepareNone();    //empty list
trnBuf.pCatBoxData->Select( cat );

//FIXME since there are no cats, these edit fields should be disabled!
pCat = (TCat*) &NOOBJECT;
trnBuf.name[0]= '\0';
trnBuf.nickname[0]= '\0';
trnBuf.keywords[0]= '\0';
trnBuf.description[0]= '\0';


pSubcatButton = new TBButton( this, IDD_CATMGRNEW_SUBCAT );
pDeleteButton = new TBButton( this, IDD_CATDELETE );

SetTransferBuffer( &trnBuf );

// select the specified cat
itsStartCatP = &cat;
}


//*****************************************************************************
// ShowFields
//*****************************************************************************
void TCatMgr::ShowFields( RTCat cat )
{
    if (((RObject) cat) == NOOBJECT) {

        EnableWindow(pNameBox->HWindow, FALSE );
        EnableWindow(pNicknameBox->HWindow, FALSE );
        EnableWindow(pKeywordBox->HWindow, FALSE );
        EnableWindow(pDescriptionBox->HWindow, FALSE );
        EnableWindow(pDeleteButton->HWindow, FALSE );
        EnableWindow(pSubcatButton->HWindow, FALSE );
    }
    else {

        EnableWindow( pNameBox->HWindow, TRUE );
        EnableWindow( pNicknameBox->HWindow, TRUE );
        EnableWindow( pKeywordBox->HWindow, TRUE );
        EnableWindow( pDescriptionBox->HWindow, TRUE );
        EnableWindow( pDeleteButton->HWindow, TRUE );
        EnableWindow( pSubcatButton->HWindow, TRUE );
    }
}


//*****************************************************************************
// SetupWindow
//*****************************************************************************
void TCatMgr::SetupWindow( void )
{
    RECT r,parentR;
    GetWindowRect( Parent->HWindow, &parentR );
    GetWindowRect( HWindow, &r );
    int width        = r.right - r.left;
    int height       = r.bottom - r.top;
    int parentWidth  = parentR.right - parentR.left;
    int parentHeight = parentR.bottom - parentR.top;

    int newX = parentR.left + (parentWidth - width) / 2;
    int newY = parentR.top  + (parentHeight - height) / 2;
    SetWindowPos( HWindow, 0, newX, newY, 0, 0,
        SWP_NOZORDER | SWP_NOSIZE );

    TOurDialog::SetupWindow();

    if (itsStartCatP != NULL) {

        SelectCat( *itsStartCatP );
    }

    ShowFields( *pCat );

}

//*****************************************************************************

//*****************************************************************************
TCatMgr::~TCatMgr()
{
delete trnBuf.pCatBoxData;
}

//*****************************************************************************
//
//*****************************************************************************
void TCatMgr::Ok( RTMessage Msg)
{
//store dialog data
TransferData( TF_GETDATA );

if (((RObject) *pCat) != NOOBJECT )
    {
    if (!UpdateCatFields()) return;
    }

Notify( kCatVis, NOOBJECT );
TDialog::Ok( Msg );
}

//*****************************************************************************
// AddCat
//*****************************************************************************
PTCat TCatMgr::AddCat( PTWindowsObject parent, PTCat pParentCat )
{
    if (!TSysState::pCategories->RoomForMore( 1 ))
        return NULL;

    PTCat pCat = new TCat();

    PTOurDialog pDlg= new TCatNew( parent, IDD_CATNEW, pCat );
    if( pParentCat == NULL )
        pDlg->SetCaption( "Create Folder" );
    else
        pDlg->SetCaption( "Create Subfolder" );

    if (IDOK == parent->GetApplication()->ExecDialog( pDlg )) {

        // assign the cat to its parent, or the global list
        if (pParentCat) {

            pParentCat->AddSubCat( *pCat );
            pParentCat->SetVisibleSubCats( TRUE );
        }
        else {

            TSysState::pCategories->add( *pCat );
        }
        return( pCat );
    }
    else {

        delete pCat;
        return( NULL );
    }
}

//*****************************************************************************

//*****************************************************************************
void TCatMgr::CreateCat( RTMessage )
{
//save current dialog info
TransferData( TF_GETDATA );

if( ((RObject) *pCat) != NOOBJECT )
    {
    if (!UpdateCatFields()) return;
    }


// add a new cat?
PTCat tempCatP = AddCat( this, NULL );
if (tempCatP == NULL) {

    return;
}
else {

    pCat = tempCatP;
    Notify( kCatNew, (RObject) *pCat );
}

// make sure Subcat button is now on, since there must be a cat
EnableWindow( pSubcatButton->HWindow, TRUE );

//update dialog box (empty list)
trnBuf.pCatBoxData->PrepareNone();

//set up transfer buffers
if( (pCat != NULL) && (((RObject) *pCat) != NOOBJECT ))
    {
    TSafStr s;
    pCat->GetShortName( s );
    s.Get( trnBuf.name, MAX_CAT_NAME_LEN );
    pCat->GetNickname( trnBuf.nickname, FALSE, MAX_CAT_KEY_LEN );
    pCat->GetKeywords( trnBuf.keywords, MAX_CAT_KEY_LEN );
    pCat->GetDescription( trnBuf.description, MAX_CAT_DESC_LEN );
    trnBuf.pCatBoxData->Select( *pCat );
    }

TransferData( TF_SETDATA );

ShowFields( *pCat );
}

//*****************************************************************************

//*****************************************************************************
void TCatMgr::CreateSubcat( RTMessage )
{
//save current dialog info
TransferData( TF_GETDATA );

if( ((RObject) *pCat) != NOOBJECT )
    {

    // can we create a subcat for the parent cat?
    if (!ValidCatOperation( *pCat, kCheckCreateSubcat )) return;

    // make sure the parent category is ok first
    if (!UpdateCatFields()) return;

    // add a new cat?
    PTCat tempCatP = AddCat( this, pCat );
    if (tempCatP == NULL)
        {
        return;
        }
    else
        {
        pCat = tempCatP;
        Notify( kCatNew, (RObject) *pCat );
        }
    }
else
    {
    // add a new cat?
    PTCat tempCatP = AddCat( this, pCat );
    if (tempCatP == NULL)
        {
        return;
        }
    else
        {
        pCat = tempCatP;
        Notify( kCatNew, (RObject) *pCat );
        }
    }

// update dialog box (empty list)
trnBuf.pCatBoxData->PrepareNone();

//set up transfer buffers
if( (pCat != NULL) && (((RObject) *pCat) != NOOBJECT ))
    {
    TSafStr s;
    pCat->GetShortName( s );
    s.Get( trnBuf.name, MAX_CAT_NAME_LEN );
    pCat->GetNickname( trnBuf.nickname, FALSE, MAX_CAT_KEY_LEN );
    pCat->GetKeywords( trnBuf.keywords, MAX_CAT_KEY_LEN );
    pCat->GetDescription( trnBuf.description, MAX_CAT_DESC_LEN );
    trnBuf.pCatBoxData->Select( *pCat );
    }

TransferData( TF_SETDATA );

SelectCat( *pCat->GetParent() );
}

//*****************************************************************************

//*****************************************************************************
void TCatMgr::NameCat( RTMessage )
{

}

//*****************************************************************************

//*****************************************************************************
void TCatMgr::NicknameCat( RTMessage )
{

}


//*****************************************************************************

//*****************************************************************************
void TCatMgr::DeleteCat( RTMessage )
{
//FIXME the delete button should be grayed (disabled) if there are subcats,
//or there is no current category.. Avoid error messages.

//save current dialog info
TransferData( TF_GETDATA );

RTCat cat= trnBuf.pCatBoxData->GetSelCat();
if( ((RObject) cat) != NOOBJECT )
    {
    if (!ValidCatOperation( cat, kCheckDelete )) return;

    if( !cat.HasSubCats() )
        {
        char message[100];

        TSafStr s;
        cat.GetShortName( s );
        wsprintf( message, "Delete folder '%s'?", s.Addr() );
        if( IDOK == TMessageBox::OkAndCancel( this, message, "Confirm deletion" ))
            {

            // get the cat before this one
            RTCat prevCat = trnBuf.pCatBoxData->GetPrevCat( cat );

            //remove category references from all notes,
            //then delete category from global list
            // this order is critical!
            TSysState::pCategories->detach( cat );  //detach from global list
            Notify( kCatDel, (RObject) cat );     //let observers get rid of references
            SendMessage( Parent->HWindow, WM_CATSELCHG, 0, (DWORD) 0 ); // tell the noteview about it
            delete &cat;                         //and delete it

            //update list box hiliting cat before the deleted one
            trnBuf.pCatBoxData->PrepareNone();
            if (NOOBJECT != (RObject) prevCat)
                {
                trnBuf.pCatBoxData->Select( prevCat );
                }
            else
                {
                trnBuf.pCatBoxData->SelectFirst();
                }
            }
        }
    else
        {
        TMessageBox::Ok( this,
            "Delete all subfolders first",
            "Cannot Delete Folder" );
        }
    }
else
    {
    TMessageBox::Ok( this,
        "No folders to delete.",
        "Can't Delete!" );
    }

pCat= & trnBuf.pCatBoxData->GetSelCat();
if( ((RObject) *pCat) != NOOBJECT )
    {
    //set up new cat fields
    TSafStr s;
    pCat->GetShortName( s );
    s.Get( trnBuf.name, MAX_CAT_NAME_LEN );
    pCat->GetNickname( trnBuf.nickname, FALSE, MAX_CAT_KEY_LEN );
    pCat->GetKeywords( trnBuf.keywords, MAX_CAT_KEY_LEN );
    pCat->GetDescription( trnBuf.description, MAX_CAT_DESC_LEN );
    }
 else
    {
    trnBuf.name[0]= '\0';
    trnBuf.nickname[0]= '\0';
    trnBuf.keywords[0]= '\0';
    trnBuf.description[0]= '\0';
    }

TransferData( TF_SETDATA );

ShowFields( *pCat );

}

//*****************************************************************************
// on selection change, reset the desc and key windows
//*****************************************************************************
void TCatMgr::ListHandler( RTMessage )
{
//store dialog data
TransferData( TF_GETDATA );

if( ((RObject) *pCat) != NOOBJECT )
    {
    if (!UpdateCatFields()) return;
    }

//set up data for newly selected category
RTCat cat= trnBuf.pCatBoxData->GetSelCat();
SelectCat( cat );

}

//*****************************************************************************
// SelectCat
//
//    Select the category and dump it into the text fields
//*****************************************************************************
void TCatMgr::SelectCat( RTCat cat )
{
char buf[ MAX_CAT_BUFFER_LEN ];

if( ((RObject) cat) != NOOBJECT )       //this should never fail, actually
    {
    if (!ValidCatOperation( cat, kCheckSelect )) return;

    TSafStr s;
    cat.GetShortName( s );
    s.Get( buf, MAX_CAT_NAME_LEN );
    pNameBox->Transfer( buf, TF_SETDATA );

    cat.GetNickname( buf, FALSE, MAX_CAT_BUFFER_LEN );
    pNicknameBox->Transfer( buf, TF_SETDATA );

    cat.GetKeywords( buf, MAX_CAT_BUFFER_LEN );
    pKeywordBox->Transfer( buf, TF_SETDATA );

    cat.GetDescription( buf, MAX_CAT_BUFFER_LEN );
    pDescriptionBox->Transfer( buf, TF_SETDATA );

    pCat= &cat;
    }

trnBuf.pCatBoxData->Select( *pCat );
ShowFields( *pCat );
}

//*****************************************************************************
// ValidCatOperation
//
//    Determine if the specified category can undergo the specified operation
//*****************************************************************************
BOOL TCatMgr::ValidCatOperation( RTCat cat, int operation )
{
    switch (operation) {

        case kCheckCreateSubcat:

            if (cat.GetCatType() != kUserCatType) {

                TMessageBox::Ok( this,
                    "Cannot create subfolders of system folders.",
                    "Can't Create Subfolder" );

                pCat = (TCat*) &NOOBJECT;
                ShowFields( *pCat );
                return FALSE;
            }
            return TRUE;

        case kCheckDelete:

		    if (cat.GetCatType() == kAddressCatType) {

                TMessageBox::Ok( this,
                    "Delete addresses from the address dialog.",
                    "Can't Delete Folder" );

                pCat = (TCat*) &NOOBJECT;
                ShowFields( *pCat );
                return FALSE;
            }
            else if (pCat->GetCatType() != kUserCatType) {

                TMessageBox::Ok( this,
                    "Cannot delete system folders.",
                    "Can't Delete Folder" );

                pCat = (TCat*) &NOOBJECT;
                ShowFields( *pCat );
                return FALSE;
            }
            return TRUE;

        case kCheckSelect:

            if (cat.GetCatType() != kUserCatType) {

                TMessageBox::Ok( this,
                    "Cannot select system folders.",
                    "Can't Select Folder" );

                pCat = (TCat*) &NOOBJECT;
                ShowFields( *pCat );
                return FALSE;
            }
            return TRUE;
    }

    pCat = (TCat*) &NOOBJECT;
    ShowFields( *pCat );
    return FALSE;
}

//*****************************************************************************

//*****************************************************************************
void TCatMgr::Listen( TSnoopMsg , RObject  )
{
//for now, assume we already know of all cat creations and deletions
// cause we did them
}


// ------------------------------------------------------------------------
// UpdateCatFields
//
// set the current category fields from the transfer buffer, but make sure
// to pull the cat out of the master list first and then reinsert it later
// in order to make sure that it falls in the right place alphabetically
// ------------------------------------------------------------------------
BOOL TCatMgr::UpdateCatFields( void )
{
BOOL notifyB= FALSE;

    // if the name field changed, then
    // rename the category and move it around in the catTree
    TSafStr s;
    pCat->GetShortName( s );
    if (strcmp( s.Addr(), trnBuf.name ) != 0) {

        if (IsBlank( trnBuf.name )) {

            TMessageBox::Ok( this,
                "Blank folder names are not allowed.",
                "Bad Folder Name" );
            SelectCat( *pCat );
            return FALSE;
        }


        // check that no category with the new name already exists
        TSafStr s( trnBuf.name );
        RTCat cat = TSysState::pCategories->FindCatInTree( s );
        BOOL foundCat = (NOOBJECT != (RObject) cat);
        BOOL sameCatAsThis = (pCat == &cat);
        if (foundCat && !sameCatAsThis) {

            TMessageBox::Ok( this,
                "A folder with that name already exists.",
                "Bad Folder Name" );
            SelectCat( *pCat );
            return FALSE;
        }

        pCat->Rename( trnBuf.name );
        notifyB= TRUE;

        TransferData( TF_GETDATA );
        trnBuf.pCatBoxData->PrepareNone();
        trnBuf.pCatBoxData->SelectFirst();
        TransferData( TF_SETDATA );
    }


    // set other fields
    char buf[ 50 ];
    pCat->GetNickname( buf, FALSE, 50 );
    if( 0 != strcmp( buf, trnBuf.nickname ))
        notifyB= TRUE;

    pCat->SetNickname( IsBlank( trnBuf.nickname ) ? "" : trnBuf.nickname );
    pCat->SetKeywords( trnBuf.keywords );
    pCat->SetDescription( trnBuf.description );

    if( notifyB )
        Notify( kCatNew, NOOBJECT );

    return TRUE;

}
