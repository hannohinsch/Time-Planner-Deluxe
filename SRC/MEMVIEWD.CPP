//*****************************************************************************
// memorized view dialog
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "msg.h"
#include "memviewd.h"

#ifndef __filter__
#include "filter.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __memview__
#include "memview.h"
#endif

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TMemorizedViewDialog::TMemorizedViewDialog(
    PTWindowsObject AParent, int AResID ):
	TOurDialog( AParent, AResID )
{

    itsHelpTopic= _HelpMemView;

    CreateSelectorData();

    // create selector
    itsSelector  = new TSingleSelector( this, IDD_MEMORIZED_SELECTOR,
        itsNumItems, itsSelectorData );

    // create the other interface objects
    itsOkButton     = new TButton( this, IDOK );
    itsCancelButton = new TButton( this, IDCANCEL );
    itsDeleteButton = new TButton( this, IDD_MEMORIZED_DELETE );
}


// -------------------------------------------------
// Destructor
// -------------------------------------------------
TMemorizedViewDialog::~TMemorizedViewDialog( void )
{
    DestroySelectorData();
}

// -------------------------------------------------
// SetupWindow
//
//  dim the the ok and delete buttons if there's
//  nothing in the box
// -------------------------------------------------
void TMemorizedViewDialog::SetupWindow( void )
{
   SetCaption( "Manage Memorized Views" );

   TOurDialog::SetupWindow();

   if (itsNumItems == 0)
        {
        EnableWindow( itsOkButton->HWindow, FALSE );
        EnableWindow( itsDeleteButton->HWindow, FALSE );
        }
 }


// -------------------------------------------------
// Ok
// -------------------------------------------------
void TMemorizedViewDialog::Ok( RTMessage msg)
{
    if (itsNumItems == 0)
        {
        TOurDialog::Ok( msg );
        return;
        }

    int index = (int) itsSelector->GetSelection();
    if( index >= 0 )
        {
        RTMemorizedView memorizedView =
            ((RTMemorizedView) (*TSysState::pMemViews)[ index ]);
        memorizedView.Select();
        }

    TOurDialog::Ok( msg );
}


// -------------------------------------------------
// HandleDelete
//
//  Delete a MemorizedView note
// -------------------------------------------------
void TMemorizedViewDialog::HandleDelete( RTMessage )
{
    if (itsNumItems == 0)
        {
        return;
        }

    int selection;

    // get the selected view
    selection = (int) itsSelector->GetSelection();
    if( selection >= 0 )
        {
        RTMemorizedView memorizedView =
            (RTMemorizedView) (*TSysState::pMemViews)[ selection ];

        if( IDOK == TMessageBox::OkAndCancel( this,
           "Remove this view from the memorized view list?",
           "Confirm Deletion" ))
            {
            // remove the note from the MemorizedView list
            TSysState::pMemViews->detach(
                memorizedView, TXArray::kDelete );

            // remove the selection from the selector data
            DestroySelectorData();
            CreateSelectorData();

            itsSelector->SetData( itsNumItems, itsSelectorData );
            itsSelector->SetSelection( 0 );
            TSysState::bSystemDirty= TRUE;
            }
        }
}

// -------------------------------------------------
//  Close the dialog on a double click
// -------------------------------------------------
void TMemorizedViewDialog::HandlePopup( RTMessage msg )
{
    if (msg.LP.Hi == CN_DBLCLICKED) {

       Ok( msg );
    }
}


// -------------------------------------------------
// CreateSelectorData
//
//  Create itsSelectorData and itsSelector
//  from the memorize view list
// -------------------------------------------------
void TMemorizedViewDialog::CreateSelectorData( void )
{
    // create a list of view titles
    itsNumItems = TSysState::pMemViews->getItemsInContainer();
    itsSelectorData = new TSelectorData[max( 1, itsNumItems)];

    for (int i = 0; i < itsNumItems; i++) {

        TSafStr s;
        ((RTMemorizedView)
            (*TSysState::pMemViews)[i]).GetName( s );
        itsSelectorData[i].pText    = new TSafStr( s );
        itsSelectorData[i].value    = i;
        itsSelectorData[i].selected = FALSE;
    }

    // select the first item
    itsSelectorData[0].selected = TRUE;

}


// -------------------------------------------------
// DestroySelectorData
//
//  Delete itsSelectorData
// -------------------------------------------------
void TMemorizedViewDialog::DestroySelectorData( void )
{
    for (int i = 0; i < itsNumItems; i++) {

        delete itsSelectorData[i].pText;
    }

    delete itsSelectorData;
}

