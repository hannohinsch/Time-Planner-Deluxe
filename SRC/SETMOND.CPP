//*****************************************************************************
// set month dialog: called by center button of calmon
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "setmond.h"

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
TSetMonthD::TSetMonthD( PTWindowsObject parent, PTTime dateP ):
	TOurDialog( parent, IDD_CALMON_MONTHS )
{
    itsDateP = dateP;

    new TBButton( this, IDD_CALMON_JAN );
    new TBButton( this, IDD_CALMON_FEB );
    new TBButton( this, IDD_CALMON_MAR );
    new TBButton( this, IDD_CALMON_APR );
    new TBButton( this, IDD_CALMON_MAY );
    new TBButton( this, IDD_CALMON_JUN );
    new TBButton( this, IDD_CALMON_JUL );
    new TBButton( this, IDD_CALMON_AUG );
    new TBButton( this, IDD_CALMON_SEP );
    new TBButton( this, IDD_CALMON_OCT );
    new TBButton( this, IDD_CALMON_NOV );
    new TBButton( this, IDD_CALMON_DEC );
}

// -------------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------------
TSetMonthD::~TSetMonthD( void )
{
}

void TSetMonthD::Jan( RTMessage msg ) { itsDateP->SetMonth(  1 ); TDialog::Ok( msg ); };
void TSetMonthD::Feb( RTMessage msg ) { itsDateP->SetMonth(  2 ); TDialog::Ok( msg ); };
void TSetMonthD::Mar( RTMessage msg ) { itsDateP->SetMonth(  3 ); TDialog::Ok( msg ); };
void TSetMonthD::Apr( RTMessage msg ) { itsDateP->SetMonth(  4 ); TDialog::Ok( msg ); };
void TSetMonthD::May( RTMessage msg ) { itsDateP->SetMonth(  5 ); TDialog::Ok( msg ); };
void TSetMonthD::Jun( RTMessage msg ) { itsDateP->SetMonth(  6 ); TDialog::Ok( msg ); };
void TSetMonthD::Jul( RTMessage msg ) { itsDateP->SetMonth(  7 ); TDialog::Ok( msg ); };
void TSetMonthD::Aug( RTMessage msg ) { itsDateP->SetMonth(  8 ); TDialog::Ok( msg ); };
void TSetMonthD::Sep( RTMessage msg ) { itsDateP->SetMonth(  9 ); TDialog::Ok( msg ); };
void TSetMonthD::Oct( RTMessage msg ) { itsDateP->SetMonth( 10 ); TDialog::Ok( msg ); };
void TSetMonthD::Nov( RTMessage msg ) { itsDateP->SetMonth( 11 ); TDialog::Ok( msg ); };
void TSetMonthD::Dec( RTMessage msg ) { itsDateP->SetMonth( 12 ); TDialog::Ok( msg ); };
