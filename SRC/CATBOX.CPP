/***************************************************************************
 *                                                                         *
 *  MODULE     : catbox
 *               implement category selection control
 *
 *
 ***************************************************************************/
#include "standard.h"
#pragma hdrstop

#include "catbox.h"

#ifndef __classid__
#include "classid.h"
#endif
#ifndef __msg__
#include "msg.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __cat__
#include "cat.h"
#endif
#ifndef __catdlg__
#include "catdlg.h"
#endif
#ifndef __catlist__
#include "catlist.h"
#endif
#ifndef __cattree__
#include "cattree.h"
#endif
#ifndef __error__
#include "error.h"
#endif
#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __paint__
#include "paint.h"
#endif
#ifndef  __cursor__
#include "cursor.h"
#endif
#ifndef __xarray__
#include "xarray.h"
#endif
#ifndef __moushelp__
#include "moushelp.h"
#endif
#ifndef __addressd__
#include "addressd.h"
#endif
#ifndef __catdlg__
#include "catdlg.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __filter__
#include "filter.h"
#endif
#ifndef __gridutil__
#include "gridutil.h"
#endif
#ifndef __dialdlg__
#include "dialdlg.h"
#endif

PTCat TCatBoxData::unCatP= NULL;

//*****************************************************************************
// box; size from resId
//*****************************************************************************
TCatBox::TCatBox( PTWindowsObject AParent, WORD resID )
    : TMousGrid( AParent, resID )
{
    scrPainter= new TPaint3D;

    pData= NULL;
    SetRowCnt( 0 );     //reset by transfer
    SetColCnt( 2 );
    SetColFreeze( 2 );
}

//*****************************************************************************
// box; explicit size
//*****************************************************************************
TCatBox::TCatBox( PTWindowsObject AParent, WORD anID, int x, int y,
    int dx, int dy )
    : TMousGrid( AParent, anID, x, y, dx, dy )
{
    scrPainter= new TPaint3D;

    pData= NULL;
    SetRowCnt( 0 );     //reset by transfer
    SetColCnt( 2 );
    SetColFreeze( 2 );
}

//*****************************************************************************

//*****************************************************************************
TCatBox::~TCatBox( void )
{
delete scrPainter;
}

//*****************************************************************************

//*****************************************************************************
int  TCatBox::CellWidth( PTPaint, int col )
{
int w;

if( col == 0 )
    w= 20;
else
    {
    RECT r;
    GetClientRect( HWindow, &r );
    w= r.right - r.left - 20;
    }

return w;
}

//*****************************************************************************

//*****************************************************************************
int TCatBox::CellHeight( PTPaint, int  )
{
return 20;
}

//*****************************************************************************
// RowItem
//*****************************************************************************
RTVisiCat TCatBox::RowItem( int row )
{
    return (RTVisiCat) (*(pData->pVisiCatList))[row];
}

//*****************************************************************************

//*****************************************************************************
void TCatBox::PaintCell( RTDrawInfo di )
{
if( di.col == 0 )
    PaintPlusMinus( di );
 else
    PaintName( di );
}


//*****************************************************************************

//*****************************************************************************
void  TCatBox::PaintPlusMinus( RTDrawInfo di  )
{
char buf[100];

RTVisiCat visiCat= RowItem( di.row );
if(visiCat.pCat->HasSubCats() )
    buf[0]= (char) ( visiCat.pCat->IsShowingSubCats() )?'-':'+';
else
    buf[0]='\0';
buf[1]= '\0';

THilite hilite= (visiCat.bHiddenSels)? kInvertHilite: kNoHilite;
WORD tStyle= DT_SINGLELINE | DT_NOPREFIX | DT_EXTERNALLEADING | DT_CENTER | DT_VCENTER;
di.pPainter->Text( di, buf, hilite, tStyle, kBigBoldFont );
}

//*****************************************************************************

//*****************************************************************************
void  TCatBox::PaintName( RTDrawInfo di )
{
char buf[100];
TSafStr s;

RTVisiCat visiCat= RowItem( di.row );
visiCat.pCat->GetShortName( s );
s.Get( buf, sizeof( buf ) );

THilite hilite= (visiCat.bSelected)? kInvertHilite: kNoHilite;

// shift start of text right 10% for each level down in hierarchy
RECT r= di.cellRect;
int step= (di.cellRect.right - di.cellRect.left)/10;
r.left+= 2 + step * min( 5, max( 0, visiCat.level - 1 ) );

di.pPainter->Text( di, buf, hilite, kSingleLine | DT_LEFT | DT_VCENTER,
                   kNormalFont, &r );
}

//*****************************************************************************
// select cat name; or, in col 0, expand/collapse hierarchy
//*****************************************************************************
void TCatBox::MousClick( RTMousInfo mi )
{
if( mi.style != kCellPoint )
    return;

int row= mi.row;
int col= mi.col;

if( row >= rowCnt || col >= colCnt )
    return;

TCursor::Busy( TRUE );

if( col == 0 )
    {
    RTVisiCat visiCat= RowItem( row );
    RTCat cat= *(visiCat.pCat);

    if( cat.HasSubCats() )
        {
        // flip visibility
        cat.ToggleVisibleSubCats();

        // rebuild display list
        {
        TCatList tempList;
        pData->Store( &tempList );
        pData->Prepare( &tempList );
        }

        //update control
        SetRowCnt( pData->pVisiCatList->getItemsInContainer() );

        InvalidateCell( -1, -1 );       //redisplay all
        UpdateWindow( HWindow );

        Notify( kCatVis, NOOBJECT );
        }
    }
else if( col == 1 )     //this may be kind of slow....
    {
    SelectCat( row, col );
    }
TCursor::Busy( FALSE );
}

//*****************************************************************************
// SelectCat
//
//  Select or deselect the cat in the visicat
//*****************************************************************************
void TCatBox::SelectCat( int row, int col )
{
    RTVisiCat visiCat= RowItem( row );
    RTCat cat= *(visiCat.pCat);

    if( !(pData->bSingleSelect) && visiCat.bSelected )
        visiCat.bSelected= FALSE;
    else
        pData->Select( cat );

    if( pData->bSingleSelect )
        InvalidateCell( -1, -1 );       //crude but guaranteed
    else
        InvalidateCell( row, col );     //easy to be accurate

    // handle the uncategorized category
    if (cat.GetCatType() == kUncategorizedCatType) {

        pData->SelectUncat( visiCat.bSelected );
    }

    UpdateWindow( HWindow );

    //notify parent
    SendMessage( Parent->HWindow, WM_CATSELCHG, 0, (DWORD) 0 );
}

//*****************************************************************************
// if click on +/-,  expand/collapse subcategories
//*****************************************************************************
void TCatBox::MousOver( RTMousInfo mi )
{
    // bad row?
    if (mi.row == -1) {

        TCursor::Set( kArrowCursor );
    }

    // +/- column?
    else if( mi.col == 0 ) {

        // check to see if the specific cat has subcats or not
        RTVisiCat visiCat = RowItem( mi.row );
        if (visiCat.pCat->HasSubCats()) {

            TCursor::Set( kOutlineCursor );
        }
        else {

            TCursor::Set( kArrowCursor );
        }
    }

    // cat name column?
    else if( mi.col == 1 ) {

        TCursor::Set( kCatSelCursor );
    }

    // bad column?
    else {

        TCursor::Set( kArrowCursor );
    }


}

//*****************************************************************************
// MousRtClick
//
//  If we've got a funny cursor, put up a message
//*****************************************************************************
void TCatBox::MousRtClick( RTMousInfo mi )
{
    if( mi.style != kCellPoint )
        return;

    if( mi.row >= rowCnt || mi.col >= colCnt )
        return;

    RTVisiCat visiCat = RowItem( mi.row );
    PTCat catP = visiCat.pCat;
    char nickname[256], buf[256];
    catP->GetNickname( nickname, TRUE );

    // +/- column?
    if (mi.col == 0) {

        // check to see if the specific cat has subcats or not
        if (catP->HasSubCats()) {

            if (catP->IsShowingSubCats()) {

                wsprintf( buf,
                    "Click left button to hide subfolders of '%s'.",
                    nickname );
            }
            else {
                wsprintf( buf,
                    "Click left button to show subfolders of '%s'.",
                    nickname );
            }
            TMousHelp::MousHelp( Parent, buf );
        }
    }

    // cat name column?
    else if( mi.col == 1 ) {

        // it's in the frame or noteview,
        // put up the catbox menu
        if (pData->itsHasAddressCats &&
               (
               kUserCatType == catP->GetCatType() ||
               kAddressCatType == catP->GetCatType()
               )
            ){

            //load the proper menu resource
            int id = catP->IsAddress() ? IDM_CATBOX_A : IDM_CATBOX_F;

            HMENU hMenuRes = LoadMenu( GetApplicationObject()->hInstance,
                MAKEINTRESOURCE( id ));
            HMENU hMenu = GetSubMenu( hMenuRes, 0 );
            ClientToScreen( HWindow, &mi.xy );

            itsMenuRow = mi.row;
            itsMenuCol = mi.col;

            //if it's in the noteview, delete "create note"
            if (!pData->itsHasSystemCats)
                {
                EnableMenuItem( hMenu, IDM_CATBOX_NEW_NOTE,
                    MF_BYCOMMAND | MF_GRAYED);
                }

            TGridUtil::OurTrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                mi.xy.x, mi.xy.y, 0, HWindow, NULL );

            DestroyMenu( hMenuRes );
        }

        // it's in the catmgr, or a system cat, so just put up a message
        else {

            wsprintf( buf,
                "Click left button to select folder '%s'.", nickname );
            TMousHelp::MousHelp( this, buf );
        }
    }
}


//*****************************************************************************
// HandleCategory
//
//  Put up the address or category manager with
//  the proper category selected
//*****************************************************************************
void TCatBox::HandleCategory( RTMessage )
{
    RTVisiCat visiCat= RowItem( itsMenuRow );
    RTCat cat= *(visiCat.pCat);

    GetApplication()->ExecDialog(
        new TCatMgr(this, IDD_CATMGR, cat ));
}

//*****************************************************************************
// HandleAddress
//
//  Put up the address manager with
//  the proper category selected
//*****************************************************************************
void TCatBox::HandleAddress( RTMessage )
{
    RTVisiCat visiCat= RowItem( itsMenuRow );
    RTCat cat= *(visiCat.pCat);

    GetApplicationObject()->ExecDialog(
        new TAddressDialog( this, IDD_ADDRESS, cat ));
}

//*****************************************************************************
// HandleNewNote
//
//  Put up the note manager with
//  the proper category selected
//*****************************************************************************
void TCatBox::HandleNewNote( RTMessage )
{
    RTVisiCat visiCat= RowItem( itsMenuRow );
    RTCat cat= *(visiCat.pCat);

    PTNote noteP = TSysState::pAllNotes->CreateNote( this );
    if( noteP != NULL )
        {
        if (&cat != TCatBoxData::unCatP) {

            RTCatList catList = noteP->GetCatList();
            catList.add( cat );
        }
        GetApplication()->ExecDialog(new TNoteView( this, IDD_NOTE, noteP ));
        }
}

//*****************************************************************************

//*****************************************************************************
void TCatBox::HandleNewFolder( RTMessage )
{
PTCat pCat= TCatMgr::AddCat( this, NULL );

//since we call Notify(), our Listen() isn't called automatically
if( pCat != NULL )
    {
    Notify( kCatNew, (RObject) *pCat );
    Listen( kCatNew, (RObject) *pCat );
    }
}

//*****************************************************************************

//*****************************************************************************
void TCatBox::HandleNewSubfolder( RTMessage )
{
RTVisiCat visiCat= RowItem( itsMenuRow );

PTCat pCat= TCatMgr::AddCat( this, visiCat.pCat );

//since we call Notify(), our Listen() isn't called automatically
if( pCat != NULL )
    {
    Notify( kCatNew, (RObject) *pCat );
    Listen( kCatNew, (RObject) *pCat );
    }
}

//*****************************************************************************

//*****************************************************************************
void TCatBox::HandleDelete( RTMessage )
{
RTVisiCat visiCat= RowItem( itsMenuRow );
RTCat cat= *(visiCat.pCat);

if( ((RObject) cat) != NOOBJECT )
    {
    // if (!ValidCatOperation( cat, kCheckDelete )) return;

    if( !cat.HasSubCats() )
        {
        char message[100];

        TSafStr s;
        cat.GetShortName( s );
        wsprintf( message, "Delete folder '%s'?", s.Addr() );
        if( IDOK == TMessageBox::OkAndCancel( this, message, "Confirm deletion" ))
            {
            //remove category references from all notes,
            //then delete category from global list
            // this order is critical!
            TSysState::pCategories->detach( cat );  //detach from global list

            Notify( kCatDel, (RObject) cat );     //let observers get rid of references
            // SendMessage( Parent->HWindow, WM_CATSELCHG, 0, (DWORD) 0 ); // tell the noteview about it

            //since we call Notify(), our Listen() isn't called automatically
            Listen( kCatDel, (RObject) cat );

            delete &cat;                         //and delete it
            }
        }
    else
        {
        TMessageBox::Ok( this,
            "Delete all subfolders first",
            "Cannot Delete Folder" );
        }
    }

}

//*****************************************************************************

//*****************************************************************************
void TCatBox::HandleDial( RTMessage )
{
RTVisiCat visiCat= RowItem( itsMenuRow );

if (TDialDialog::CurrentDialDialog)
    {
    BringWindowToTop(
        TDialDialog::CurrentDialDialog->HWindow );
    }
else
    {
    PTDialDialog pDlg= new TDialDialog(
        TPimFrame::pFrame, visiCat.pCat );
    GetApplicationObject()->MakeWindow( pDlg );
    ShowWindow( pDlg->HWindow, TRUE );
    }

}

//*****************************************************************************
// SelectNone
//
//  Deselect everything in the catbox
//*****************************************************************************
void TCatBox::SelectNone( void )
{
    TCursor::Busy( TRUE );

    // deselect everything in the catbox and redraw
    pData->PrepareNone();
    InvalidateCell( -1, -1 );
    UpdateWindow( HWindow );

    //notify parent
    //SendMessage( Parent->HWindow, WM_CATSELCHG, 0, (DWORD) 0 );

    TCursor::Busy( FALSE );
}

//*****************************************************************************
// SelectAll
//
//  Select everything in the catbox
//*****************************************************************************
void TCatBox::SelectAll( void )
{
    TCursor::Busy( TRUE );

    // select everything in the catbox and redraw
    pData->PrepareAll();
    InvalidateCell( -1, -1 );
    UpdateWindow( HWindow );

    //notify parent
    //SendMessage( Parent->HWindow, WM_CATSELCHG, 0, (DWORD) 0 );

    TCursor::Busy( FALSE );
}

//*****************************************************************************

//*****************************************************************************
void TCatBox::GetDefaultFocusCell( int &row, int &col )
{
row= vScrollPos + frozenRows;
col= 1;
}

//*****************************************************************************

//*****************************************************************************
void TCatBox::PaintFocus( HDC hdc, RECT &cellR )
{
RECT r= cellR;

//make our rectangle smaller
InflateRect( &r, -2, -2 );

TGDIMap::SetColorEnvironment( hdc, kEnv3D );

TMousGrid::PaintFocus( hdc, r );
}


//*****************************************************************************

//*****************************************************************************
void TCatBox::Listen( TSnoopMsg aMsg, RObject aObject )
{
switch( aMsg )
    {
    case kCatDel:
        ASSERT( ERR_GEN, aObject != NOOBJECT );
        CHKBUG( ERR_GEN, aObject.isA() == IDCat );

        if( pData != NULL )  //we might get this before we have a data block
            {
            InvalidateFocus();

            TCatList temp;
            pData->Store( &temp );
            temp.detach( (RTCat) aObject );
            pData->Prepare( &temp );

            SetRowCnt( pData->pVisiCatList->getItemsInContainer() );
            InvalidateCell( -1, -1 );       //redisplay all
            }
        break;

    case kCatNew:
    case kCatVis:           //clean up visible cats

        if( pData != NULL )  //we might get this before we have a data block
            {
            InvalidateFocus();

            TCatList temp;
            pData->Store( &temp );
            pData->Prepare( &temp );

            SetRowCnt( pData->pVisiCatList->getItemsInContainer() );
            InvalidateCell( -1, -1 );       //redisplay all
            }
        break;

    default:
        break;
    }
}

//*****************************************************************************

//*****************************************************************************
void TCatBox::GetWindowClass( WNDCLASS & AWndClass )
{
TMousGrid::GetWindowClass( AWndClass );

AWndClass.hbrBackground= TGDIMap::GetScreenBrush( kHbrBkCtl );
AWndClass.style &= ~CS_DBLCLKS;
}


//*****************************************************************************
//  Transfers state information for a TCatBox. The TransferFlag passed
//  specifies whether data is to be read from or written to the passed
//  buffer, or whether the data element size is simply to be returned.
//  The return value is the size (in bytes) of the transfer data.
//*****************************************************************************
WORD TCatBox::Transfer(Pvoid DataPtr, WORD TransferFlag)
{

if ( TransferFlag == TF_GETDATA )
    {
    //buffer is always maintained
    }
else if ( TransferFlag == TF_SETDATA )
    {
    //CatBox control works directly on buffer; needs only address pointer
    pData = *(PTCatBoxData *)DataPtr;
    SetRowCnt( pData->pVisiCatList->getItemsInContainer() );

    // scroll to the first hilighted cat
    int first = 0;
    for (PTVisiCat vCatP = pData->FirstVisiCat();
        vCatP != NULL;
        vCatP = pData->NextVisiCat() )
        {
        if( vCatP->bSelected )
            break;
        else
            first++;
        }

    if( vCatP == NULL )
        first= 0;       //no cats selected

    GotoRC( first, 0 );

    InvalidateRect( HWindow, NULL, TRUE );        //force repaint
    }

return sizeof(PTCatBoxData);
}

//*****************************************************************************
// update window background brush
//*****************************************************************************
void TCatBox::WMLocalColor( RTMessage )
{
SetClassWord( HWindow, GCW_HBRBACKGROUND, TGDIMap::GetScreenBrush( kHbrBkCtl ) );
InvalidateRect( HWindow, NULL, TRUE );
}

//*****************************************************************************
//////////////////////////// TCatBoxData /////////////////////////////////
//*****************************************************************************

//*****************************************************************************
// create system cats
//*****************************************************************************
void TCatBoxData::AppInit( void )
{
KSTR uncat( "<unfiled>");

unCatP= new TCat( kUncategorizedCatType, uncat );
}

//*****************************************************************************

//*****************************************************************************
void TCatBoxData::AppCleanup( void )
{
delete unCatP;
}

//*****************************************************************************

//*****************************************************************************
void TCatBoxData::TCatBoxData( BOOL hasSystemCats,
    BOOL hasAddressCats )
{
    pVisiCatList       = new TXArray(10, 10, TXArray::kNoSort);
    pHiddenSelections  = new TCatList;
    bSingleSelect      = FALSE;
    itsHasSystemCats   = hasSystemCats;
    itsHasAddressCats  = hasAddressCats;
}

//*****************************************************************************

//*****************************************************************************
TCatBoxData::~TCatBoxData()
{
    pVisiCatList->flush( TXArray::kDelete );
    delete pVisiCatList;

    delete pHiddenSelections;
}

//*****************************************************************************
// default is multiple select
//*****************************************************************************
void TCatBoxData::SetSingleSelect( BOOL b)
{
    bSingleSelect= b;
}

//*****************************************************************************
//iterate through global catlist, selecting only those in catlist arg
//*****************************************************************************
void TCatBoxData::Prepare( PTCatList pACatList )
{
    pHiddenSelections->flush();
    pVisiCatList->flush( TXArray::kDelete );
    BuildVisiList( FALSE );
    AddSystemCats();

    // now select all cats that appear in pACatList
    Select( *pACatList );
}

//*****************************************************************************

//*****************************************************************************
void TCatBoxData::PrepareAll( void )
{

    pHiddenSelections->flush();
    pVisiCatList->flush( TXArray::kDelete );
    BuildVisiList( TRUE );
    AddSystemCats();

    // now select ALL cats
    //TCatList catList( *TSysState::pCategories );      //flatten tree
    //Select( catList );
    SelectAll();
}


//*****************************************************************************

//*****************************************************************************
void TCatBoxData::PrepareNone( void )
{
    pHiddenSelections->flush();
    pVisiCatList->flush( TXArray::kDelete );
    BuildVisiList( FALSE );
    AddSystemCats();
    ClearSelections();
}

//*****************************************************************************
// BuildVisiList
//
// this guy builds the hierarchical representation (the VisiList) of visible
// items in a category box.
//*****************************************************************************
void TCatBoxData::BuildVisiList( BOOL bSelect )
{
    TCatList catList;
    catList.AddVisibleCats( TSysState::pCategories );

    int n = catList.getItemsInContainer();
    for (int i = 0; i < n; i++) {

        RTCat cat = catList[i];

        TCatType type= cat.GetCatType();
        if ( type == kUserCatType ||
             (type == kUncategorizedCatType && itsHasSystemCats) ||
             (type == kAddressCatType && itsHasAddressCats ) ||
             (type == kMasterAddressCatType && itsHasAddressCats )
             ) {

		    PTVisiCat pVisiCat= new TVisiCat( cat, cat.GetDepth(), bSelect );
		    if( pVisiCat == 0 ) SYSERR( ERR_NEW );
		    pVisiCatList->add( *pVisiCat );
        }
    }
}



//*****************************************************************************
// AddSystemCats
//
// Add system cats to the visiCatList
//*****************************************************************************
void TCatBoxData::AddSystemCats( void )
{
    if (itsHasSystemCats) {

        PTVisiCat fUnVisiCatP = new TVisiCat( *unCatP,
            0, TSysState::pFilter->IsUncatIncluded() );
        pVisiCatList->add( *fUnVisiCatP );

    }
}

//*****************************************************************************
//
//*****************************************************************************
void TCatBoxData::Store( PTCatList pACatList )
{
    pACatList->flush();

    for (PTVisiCat vCatP = FirstVisiCat(); vCatP != NULL; vCatP = NextVisiCat()) {

        if (vCatP->bSelected) {

		    RTCat cat= *(vCatP->pCat);
            CHKBUG( ERR_CAT, cat.HealthCheck() );

            // the master address type is treated like any other
            // folder
            if (cat.GetCatType() == kUserCatType ||
                cat.GetCatType() == kAddressCatType ||
                cat.GetCatType() == kMasterAddressCatType ) {

                pACatList->add( cat );
            }
        }
    }


    // add hidden cats
    int n = pHiddenSelections->getItemsInContainer();
    for( int j= 0; j < n; j++ ) {

        RTCat cat= (*pHiddenSelections)[j];
	    CHKBUG( ERR_CAT, cat.HealthCheck() );

        pACatList->add( cat );
    }

}

//*****************************************************************************

//*****************************************************************************
int TCatBoxData::GetSelCount()
{
    int count = 0;

    for (PTVisiCat vCatP = FirstVisiCat(); vCatP != NULL; vCatP = NextVisiCat()) {

        if (vCatP->bSelected) {

            count++;
        }
    }

    return count;
}

//*****************************************************************************
//
//*****************************************************************************
void TCatBoxData::SelectAll( void )
{
    pHiddenSelections->flush();

    for (PTVisiCat vCatP = FirstVisiCat(); vCatP != NULL; vCatP = NextVisiCat()) {

        vCatP->bSelected   = TRUE;
        vCatP->bHiddenSels = FALSE;
    }
}

//*****************************************************************************
// select
//
// Select every category in the list
//*****************************************************************************
void TCatBoxData::Select( RTCatList catList )
{
    int n = catList.getItemsInContainer();
    for (int i= 0; i < n; i++ ) {

        RTCat cat= catList[i];
        CHKBUG( ERR_CAT, cat.HealthCheck() );

        Select( cat );
    }
}

//*****************************************************************************
// select category
// if cat isn't visible (is therefore a subcat), the highest visible cat is
// marked with "bHiddenSels"
//*****************************************************************************
void TCatBoxData::Select( RTCat cat )
{
    // only one selection at a time?
    if (bSingleSelect) {

        ClearSelections();
    }


    // if we find the cat in among the visible cats, then mark it
    PTVisiCat visiCatP = FindVisiCat( &cat );
    if (visiCatP != NULL) {

        visiCatP->bSelected = TRUE;
    }

    // otherwise, mark the cat's nearest visible ancestor
    // and add the category to the hidden list
    else {

        visiCatP = FindParentVisiCat( &cat );
        visiCatP->bHiddenSels = TRUE;
        pHiddenSelections->add( cat );
    }
}

//*****************************************************************************
// ClearSelections
//*****************************************************************************
void TCatBoxData::ClearSelections( void )
{
    pHiddenSelections->flush();

    for (PTVisiCat vCatP = FirstVisiCat(); vCatP != NULL; vCatP = NextVisiCat()) {

        vCatP->bSelected   = FALSE;
        vCatP->bHiddenSels = FALSE;
    }
}

//*****************************************************************************
// FindCat
//
// Find the visiCat that contains the category
//*****************************************************************************
PTVisiCat TCatBoxData::FindVisiCat( PTCat catP )
{
    for (PTVisiCat vCatP = FirstVisiCat(); vCatP != NULL; vCatP = NextVisiCat()) {

        if (vCatP->pCat == catP) {

            return vCatP;
        }
    }

    return NULL;
}

//*****************************************************************************
// FindParentVisiCat
//
// given a selected but hidden category, find the first ancestor which
// is not hidden and set its bHiddenSels field, so that the CatBox's know
// to hilite the + sign
//*****************************************************************************
PTVisiCat TCatBoxData::FindParentVisiCat( PTCat catP )
{
    catP = catP->GetParent();
    while (catP != NULL)  {

        PTVisiCat vCatP = FindVisiCat( catP );
        if (vCatP != NULL) {

            return vCatP;
        }

        // move up the tree
        catP = catP->GetParent();
    }

    // we should always find one, since all top-level cats ARE visible
    SYSERR( ERR_GEN );
    return NULL;
}


//*****************************************************************************
// SelectFirst
//
// select first item in set; should only happen with SingleSelect on
//*****************************************************************************
void TCatBoxData::SelectFirst( void )
{
    ASSERT( ERR_GEN, bSingleSelect );

    ClearSelections();

    PTVisiCat visiCatP = FirstVisiCat();
    if (visiCatP != NULL) {

        visiCatP->bSelected = TRUE;
    }
}

//*****************************************************************************
// SelectUncat
//*****************************************************************************
void TCatBoxData::SelectUncat( BOOL select )
{
    TSysState::pFilter->IncludeUncat( select );
}


//*****************************************************************************
// GetSelCat
//
//for single select only -- return selection
//*****************************************************************************
RTCat TCatBoxData::GetSelCat( void )
{
    ASSERT( ERR_GEN, bSingleSelect );

    for (PTVisiCat vCatP = FirstVisiCat(); vCatP != NULL; vCatP = NextVisiCat()) {

        if (vCatP->bSelected) {

            return *vCatP->pCat;
        }
    }

    return (RTCat) NOOBJECT;
}


//*****************************************************************************
// GetPrevCat
//
// for single select only -- return cat before selection (for use after
// deleting)
//*****************************************************************************
RTCat TCatBoxData::GetPrevCat( RTCat cat )
{
    PTCat prevCatP = (PTCat) &NOOBJECT;

    for (PTVisiCat vCatP = FirstVisiCat(); vCatP != NULL; vCatP = NextVisiCat()) {

        if (vCatP->pCat == &cat) {

            return *prevCatP;
        }

        prevCatP = vCatP->pCat;
    }

    return (RTCat) NOOBJECT;
}

//*****************************************************************************
// FirstVisiCat
//*****************************************************************************
PTVisiCat TCatBoxData::FirstVisiCat( void )
{
    itsCurrentVisiCat = 0;
    return NextVisiCat();
}

//*****************************************************************************
// NextVisiCat
//*****************************************************************************
PTVisiCat TCatBoxData::NextVisiCat( void )
{
    // out of range?
    if (itsCurrentVisiCat >= pVisiCatList->getItemsInContainer()) {

        return NULL;
    }

    RTVisiCat visiCat = (RTVisiCat) (*pVisiCatList)[itsCurrentVisiCat++];

    return (PTVisiCat) &visiCat;
}


//*****************************************************************************
/////////////////////// TVisiCat ///////////////////////
//*****************************************************************************

//*****************************************************************************

//*****************************************************************************
TVisiCat::TVisiCat( RTCat aCat, int aLevel, BOOL b )
{
pCat= &aCat;
bSelected= b;
bHiddenSels= FALSE;
level= aLevel;
}

//*****************************************************************************

//*****************************************************************************
TVisiCat::~TVisiCat( void )
{
}

//*****************************************************************************
// basic object fns
//*****************************************************************************
classType TVisiCat::isA( void ) const
{
return IDVisiCat;
}

char * TVisiCat::nameOf( void ) const
{
return "TVisiCat";
}

hashValueType TVisiCat::hashValue( void ) const
{
return 0; //FIXME
}

int TVisiCat::isEqual(const Object & obj) const
{
return( this == & obj );
}

void TVisiCat::printOn(ostream far&) const
{
//later
}


