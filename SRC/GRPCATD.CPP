//*****************************************************************************
// dialog: control grouping of notes in views
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "grpcatd.h"
#ifndef __catbox__
#include "catbox.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __filter__
#include "filter.h"
#endif
#ifndef __rptgrid__
#include "rptgrid.h"
#endif


// -------------------------------------------------
// Constructor
// -------------------------------------------------
TGroupCatDialog::TGroupCatDialog( PTWindowsObject AParent ):
	TOurDialog( AParent, IDD_GROUPCAT )
{
    itsHelpTopic= _HelpGroup;

    itsSubtotalCP   = new TCheckBox( this, IDD_GROUPCAT_SUBTOTALS, NULL );
    itsSubtotalCP->DisableTransfer();

    itsCatBoxDataP  = new TCatBoxData;
    itsCatBoxDataP->Prepare( TSysState::GroupCatListP );
    itsCatBoxCP     = new TCatBox( this, IDD_GROUPCAT_CATBOX );
    itsCatBoxCP->DisableTransfer();

    itsRadioGroup   = new TGroupBox( this, IDD_GROUPCAT_GROUP );
    itsRadioGroup->NotifyParent = TRUE;

    itsAutomaticCP  = new TRadioButton( this, IDD_GROUPCAT_AUTOMATIC,   itsRadioGroup );
    itsNoGroupingCP = new TRadioButton( this, IDD_GROUPCAT_NO_GROUPING, itsRadioGroup );
    itsSelectedCP   = new TRadioButton( this, IDD_GROUPCAT_SELECTED,    itsRadioGroup );

    for (int i = 0; i < 3; i++)
        {
        itsTransferBuffer[i] = BF_UNCHECKED;
        }
    itsTransferBuffer[(int)TSysState::CatGrouping] = BF_CHECKED;
    SetTransferBuffer( itsTransferBuffer );
}


// -------------------------------------------------
// Destructor
// -------------------------------------------------
TGroupCatDialog::~TGroupCatDialog( void )
{
    delete itsCatBoxDataP;
    delete itsCatBoxCP;
}

// -------------------------------------------------
// SetupWindow
// -------------------------------------------------
void TGroupCatDialog::SetupWindow( void )
{
    // inherited
    TOurDialog::SetupWindow();

    itsCatBoxCP->Transfer( &itsCatBoxDataP, TF_SETDATA );
    itsSubtotalCP->SetCheck(
        TReportGrid::GetBreakLevel() ?
        BF_CHECKED : BF_UNCHECKED );
}

// -------------------------------------------------
// HandleAutomatic
//
//  Enable the subtotal button
// -------------------------------------------------
void TGroupCatDialog::HandleAutomatic( RTMessage )
{
    EnableWindow( itsSubtotalCP->HWindow, TRUE );
}

// -------------------------------------------------
// HandleSelected
//
//  Enable the subtotal button
// -------------------------------------------------
void TGroupCatDialog::HandleSelected( RTMessage )
{
    EnableWindow( itsSubtotalCP->HWindow, TRUE );
}

// -------------------------------------------------
// HandleNoGrouping
//
//  turn off and disable the subtotal button
// -------------------------------------------------
void TGroupCatDialog::HandleNoGrouping( RTMessage )
{
    itsSubtotalCP->SetCheck( BF_UNCHECKED );
    EnableWindow( itsSubtotalCP->HWindow, FALSE );
}

// -------------------------------------------------
// HandleCatBox
//
//  Select the appropriate radio button
// -------------------------------------------------
void TGroupCatDialog::HandleCatBox( RTMessage )
{
    itsTransferBuffer[0] = BF_UNCHECKED;
    itsTransferBuffer[1] = BF_UNCHECKED;
    itsTransferBuffer[2] = BF_CHECKED;
    TransferData( TF_SETDATA );

    // enable subtotal 
    EnableWindow( itsSubtotalCP->HWindow, TRUE );
}

// -------------------------------------------------
// Ok
// -------------------------------------------------
void TGroupCatDialog::Ok( RTMessage msg)
{
    TransferData( TF_GETDATA );

    if (itsTransferBuffer[0] == BF_CHECKED)
        {
        TSysState::GroupCatListP->flush( TXArray::kNoDelete );
        TSysState::CatGrouping = kAutomaticCatGrouping;
        }
    else if (itsTransferBuffer[1] == BF_CHECKED)
        {
        TSysState::GroupCatListP->flush( TXArray::kNoDelete );
        TSysState::CatGrouping = kNoCatGrouping;
        }
    else
        {
        TSysState::GroupCatListP->flush( TXArray::kNoDelete );
        TSysState::CatGrouping = kSelectedCatGrouping;
        itsCatBoxCP->Transfer( &itsCatBoxDataP, TF_GETDATA );
        itsCatBoxDataP->Store( TSysState::GroupCatListP );
        }

    int breakLevel =
        itsSubtotalCP->GetCheck() == BF_CHECKED;

    TReportGrid::SetBreakLevel( breakLevel );

    TDialog::Ok( msg );
}

