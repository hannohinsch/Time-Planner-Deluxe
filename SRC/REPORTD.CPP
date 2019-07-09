//*****************************************************************************
// report view columns
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "profile.h"
#include "reportd.h"
#include "oem.h"

//*****************************************************************************

//*****************************************************************************
void TReportD::TReportD( PTWindowsObject parent, WORD anID ):
        TOurDialog( parent, anID )
{
itsHelpTopic= _HelpColumns;

itsFieldNames[ kStartField ]= new KSTR( "Start date/time" );
itsFieldNames[ kEndField ] = new KSTR( "End date/time" );
itsFieldNames[ kElapsedField ] = new KSTR( "Elapsed time" );
itsFieldNames[ kWorktimeField ] = new KSTR( "Work time" );
itsFieldNames[ kFreqField ] = new KSTR( "Frequency" );
itsFieldNames[ kRateField ] = new KSTR( "Rate" );
itsFieldNames[ kAmtField ] = new KSTR( "Amount" );
itsFieldNames[ kAlarmField ] = new KSTR( "Alarm time" );
itsFieldNames[ kMarkField ] = new KSTR( "Shape/color" );
itsFieldNames[ kTextField ] = new KSTR( "Note text" );
itsFieldNames[ kDontUseField ] = new KSTR( "<Don't Use>" );
itsFieldNames[ kTimeUnitField ] = new KSTR( "Time units" );

for( int i= 0; i < kMaxReportCols; i++ )
    itsFieldBoxCP[i]= new TComboBox( this, IDC_REPORT_COL1 + i, 100 );

}

//*****************************************************************************

//*****************************************************************************
void TReportD::~TReportD()
{
for( int i= kFirstField; i <= kLastField; i++ )
    {
    delete itsFieldNames[ i ];
    }
}

//*****************************************************************************

//*****************************************************************************
void TReportD::SetupWindow( void )
{
TOurDialog::SetupWindow();

for( int i= 0; i < kMaxReportCols; i++ )
    {
    for( int j= kFirstField; j <= kLastField; j++ )
        itsFieldBoxCP[i]->AddString( (char *)(const char *) (*itsFieldNames[j]) );
    }

//fix first two columns
EnableWindow( itsFieldBoxCP[0]->HWindow, FALSE );
EnableWindow( itsFieldBoxCP[1]->HWindow, FALSE );

for( i= 0; i < kMaxReportCols; i++ )
    itsFieldBoxCP[i]->SetSelIndex( (int) TReportGrid::GetColumnField( i ) );

}

//*****************************************************************************

//*****************************************************************************
void TReportD::Ok( RTMessage msg )
{
    SetValues();
    TOurDialog::Ok( msg );
}


// -------------------------------------------------
// SetDefault
//
//  Store this information into the profile
// -------------------------------------------------
void TReportD::SetDefault( RTMessage msg )
{
    SetValues();
    TReportGrid::SaveProfile();
    TOurDialog::Ok( msg );
}

//*****************************************************************************
// get values from profile
//*****************************************************************************
void TReportD::UseDefault( RTMessage )
{
}

// -------------------------------------------------
// SetValues
//
//  Set the values from the dialog box
// -------------------------------------------------
void TReportD::SetValues( void  )
{
//set column fields
for( int i= 0; i < kMaxReportCols; i++ )
    TReportGrid::SetColumnField( i,
                      (TReportField) itsFieldBoxCP[i]->GetSelIndex() );
}
