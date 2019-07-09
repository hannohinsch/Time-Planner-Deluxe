//*****************************************************************************
// phone dialer setup
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "dialsetd.h"
#ifndef __profile__
#include "profile.h"
#endif

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TDialSetupDialog::TDialSetupDialog( PTWindowsObject AParent ):
	TOurDialog( AParent, IDD_DIALSETUP )
{
    itsHelpTopic              = _HelpPhoneSetup;

    itsLocalPrefixCP          = new TEdit( this, IDD_DIALSETUP_LOCAL_PRE, 25 ) ;
    itsLocalSuffixCP          = new TEdit( this, IDD_DIALSETUP_LOCAL_POST, 25 ) ;
    itsLongDistancePrefixCP   = new TEdit( this, IDD_DIALSETUP_LONG_PRE, 25 ) ;
    itsLongDistanceSuffixCP   = new TEdit( this, IDD_DIALSETUP_LONG_POST, 25 ) ;
    itsInternationalPrefixCP  = new TEdit( this, IDD_DIALSETUP_INTL_PRE, 25 ) ;
    itsInternationalSuffixCP  = new TEdit( this, IDD_DIALSETUP_INTL_POST, 25 ) ;
    itsAlternatePrefixCP      = new TEdit( this, IDD_DIALSETUP_ALT_PRE, 25 ) ;
    itsAlternateSuffixCP      = new TEdit( this, IDD_DIALSETUP_ALT_POST, 25 ) ;
    itsLocalAreaCodeCP        = new TEdit( this, IDD_DIALSETUP_AREA_CODE, 25 ) ;

    itsBaudCP                 = new TComboBox( this, IDD_DIALSETUP_BAUD, 25 );
    itsPortCP                 = new TComboBox( this, IDD_DIALSETUP_PORT, 25 );

    itsGroupCP                = new TGroupBox( this, IDD_DIALSETUP_GROUP );
    itsPulseCP                = new TRadioButton( this, IDD_DIALSETUP_PULSE, itsGroupCP );
    itsToneCP                 = new TRadioButton( this, IDD_DIALSETUP_TONE, itsGroupCP );

    itsLocalPrefixCP->DisableTransfer();
    itsLocalSuffixCP->DisableTransfer();
    itsLongDistancePrefixCP->DisableTransfer();
    itsLongDistanceSuffixCP->DisableTransfer();
    itsInternationalPrefixCP->DisableTransfer();
    itsInternationalSuffixCP->DisableTransfer();
    itsAlternatePrefixCP->DisableTransfer();
    itsAlternateSuffixCP->DisableTransfer();
    itsLocalAreaCodeCP->DisableTransfer();
    itsBaudCP->DisableTransfer();
    itsPortCP->DisableTransfer();
    itsGroupCP->DisableTransfer();
    itsPulseCP->DisableTransfer();
    itsToneCP->DisableTransfer();
}


// -------------------------------------------------
// Destructor
// -------------------------------------------------
TDialSetupDialog::~TDialSetupDialog( void )
{
}

// -------------------------------------------------
// SetupWindow
// -------------------------------------------------
void TDialSetupDialog::SetupWindow( void )
{
    // inherited
    TOurDialog::SetupWindow();

    itsBaudCP->AddString( "1200" );
    itsBaudCP->AddString( "2400" );
    itsBaudCP->AddString( "9600" );

    itsPortCP->AddString( "COM1" );
    itsPortCP->AddString( "COM2" );
    itsPortCP->AddString( "COM3" );
    itsPortCP->AddString( "COM4" );

    itsLocalPrefixCP->SetText( TProfile::LocalPrefix );
    itsLocalSuffixCP->SetText( TProfile::LocalSuffix );
    itsLongDistancePrefixCP->SetText( TProfile::LongDistancePrefix );
    itsLongDistanceSuffixCP->SetText( TProfile::LongDistanceSuffix );
    itsInternationalPrefixCP->SetText( TProfile::InternationalPrefix );
    itsInternationalSuffixCP->SetText( TProfile::InternationalSuffix );
    itsAlternatePrefixCP->SetText( TProfile::AlternatePrefix );
    itsAlternateSuffixCP->SetText( TProfile::AlternateSuffix );
    itsLocalAreaCodeCP->SetText( TProfile::LocalAreaCode );

    char baud[20], port[20];

    wsprintf( baud, "%d", TProfile::BaudRate );
    itsBaudCP->SetText( baud );

    wsprintf( port, "COM%d", TProfile::CommPort );
    itsPortCP->SetText( port );

    itsToneCP->SetCheck( TProfile::bTone ? BF_CHECKED : BF_UNCHECKED );
    itsPulseCP->SetCheck( TProfile::bTone ? BF_UNCHECKED : BF_CHECKED );
}

// -------------------------------------------------
// Ok
//
//  Dial the number
// -------------------------------------------------
void TDialSetupDialog::Ok( RTMessage msg )
{
    switch( itsBaudCP->GetSelIndex() )
        {
        case 0: TProfile::BaudRate = 1200; break;
        case 1: TProfile::BaudRate = 2400; break;
        case 2: TProfile::BaudRate = 9600; break;
        }

    switch( itsPortCP->GetSelIndex() )
        {
        case 0: TProfile::CommPort = 1; break;
        case 1: TProfile::CommPort = 2; break;
        case 2: TProfile::CommPort = 3; break;
        case 3: TProfile::CommPort = 4; break;
        }

    TProfile::bTone = itsToneCP->GetCheck() == BF_CHECKED;

    itsLocalPrefixCP->GetText( TProfile::LocalPrefix, 25 );
    itsLocalSuffixCP->GetText( TProfile::LocalSuffix, 25 );
    itsLongDistancePrefixCP->GetText( TProfile::LongDistancePrefix, 25 );
    itsLongDistanceSuffixCP->GetText( TProfile::LongDistanceSuffix, 25 );
    itsInternationalPrefixCP->GetText( TProfile::InternationalPrefix, 25 );
    itsInternationalSuffixCP->GetText( TProfile::InternationalSuffix, 25 );
    itsAlternatePrefixCP->GetText( TProfile::AlternatePrefix, 25 );
    itsAlternateSuffixCP->GetText( TProfile::AlternateSuffix, 25 );
    itsLocalAreaCodeCP->GetText( TProfile::LocalAreaCode, 25 );

    TOurDialog::Ok( msg );
}
