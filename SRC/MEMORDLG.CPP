//*****************************************************************************
// memorized note dialog
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "memordlg.h"

#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __note__
#include "note.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __memnote__
#include "memnote.h"
#endif
#ifndef __paint__
#include "paint.h"
#endif


// -------------------------------------------------
// Constructor
// -------------------------------------------------
TMemorizedDialog::TMemorizedDialog( PTWindowsObject AParent, int AResID, TNote **ppNote ):
	TOurDialog( AParent, AResID )
{

    itsHelpTopic= _HelpTemplate;

    SetCaption( "Note Templates" );

    CreateSelectorData();

    // create selector
    itsSelector  = new TSingleSelector( this, IDD_MEMORIZED_SELECTOR,
        itsNumItems, itsSelectorData );

    // create the other interface objects
    itsOkButton = new TButton( this, IDOK );
    itsCancelButton = new TButton( this, IDCANCEL );
    itsDeleteButton = new TButton( this, IDD_MEMORIZED_DELETE );
    itsNotePP= ppNote;

    *itsNotePP= NULL;
}


// -------------------------------------------------
// Destructor
// -------------------------------------------------
TMemorizedDialog::~TMemorizedDialog( void )
{

    DestroySelectorData();

}


// -------------------------------------------------
// Ok
// -------------------------------------------------
void TMemorizedDialog::Ok( RTMessage msg)
{
    PTNote pNote;

    // create the note from the memorized note; or blank
    int idx= (int) itsSelector->GetSelection() - 1;
    if( idx >= 0 )
        {
        RTMemorizedNote memorizedNote = ((RTMemorizedNote) (*TSysState::pMemNotes)[ idx ]);
        pNote= new TNote( *memorizedNote.itsNoteP );
        }
    else
        {
        pNote= new TNote();
        }

    //tell caller
    *itsNotePP= pNote;

    TDialog::Ok( msg );
}


// -------------------------------------------------
// HandleDelete
//
//  Delete a memorized note
// -------------------------------------------------
void TMemorizedDialog::HandleDelete( RTMessage )
{
    int selection;

    // get the selected note
    selection = (int) itsSelector->GetSelection() - 1;
    if( selection >= 0 )
        {
        RTMemorizedNote memorizedNote = (RTMemorizedNote) (*TSysState::pMemNotes)[ selection ];

        if( IDOK == TMessageBox::OkAndCancel( this,
           "Remove this note from the memorized note list?",
           "Confirm Deletion" ))
            {
            // remove the note from the memorized list
            TSysState::pMemNotes->detach( memorizedNote, TXArray::kDelete );


            // remove the selection from the selector data
            DestroySelectorData();
            CreateSelectorData();

            itsSelector->SetData( itsNumItems, itsSelectorData );
            itsSelector->SetSelection( 0 );
            }
        }
    else
        {
        TMessageBox::Ok( this, "<Blank Note> can't be deleted.",
                                "Can't Delete" );
        }
}

// -------------------------------------------------
// HandlePopup
//
//  Close the dialog on a double click
// -------------------------------------------------
void TMemorizedDialog::HandlePopup( RTMessage msg )
{
    if (msg.LP.Hi == CN_DBLCLICKED) {

       Ok( msg );
    }
}


// -------------------------------------------------
// CreateSelectorData
//
//  Create itsSelectorData and itsSelector
//  from the memorized note list
// -------------------------------------------------
void TMemorizedDialog::CreateSelectorData( void )
{
    // create a list of note titles
    itsNumItems = 1 + TSysState::pMemNotes->getItemsInContainer();
    itsSelectorData = new TSelectorData[itsNumItems];

    itsSelectorData[0].pText    = new KSTR( "<Blank Note>" );
    itsSelectorData[0].value    = 0;
    itsSelectorData[0].selected = TRUE;

    for (int i = 1; i < itsNumItems; i++) {

        RTSafStr s=((RTMemorizedNote) (*TSysState::pMemNotes)[i-1]).itsName;
        itsSelectorData[i].pText    = new TSafStr( s );
        itsSelectorData[i].value    = i;
        itsSelectorData[i].selected = FALSE;
    }


}


// -------------------------------------------------
// DestroySelectorData
//
//  Delete itsSelectorData
// -------------------------------------------------
void TMemorizedDialog::DestroySelectorData( void )
{
    for (int i = 0; i < itsNumItems; i++) {

        delete itsSelectorData[i].pText;
    }

    delete itsSelectorData;
}

