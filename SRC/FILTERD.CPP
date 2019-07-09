//*****************************************************************************
// filter dialog
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop


#ifndef __filterd__
#include "filterd.h"
#endif
#ifndef __filter__
#include "filter.h"
#endif
#ifndef __datepick__
#include "datepick.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif


//*****************************************************************************
// set up source categories and filters
//*****************************************************************************
TFilterDlg::TFilterDlg( PTWindowsObject AParent, int AResID, PTFilter pFilter ):
                  TOurDialog( AParent, AResID )
{
itsFilterP= pFilter;

itsHelpTopic= _HelpFilterOv;

itsFindTextCP= new TEdit( this, IDC_FILTER_TEXT, MAX_SEARCH_LEN );

// Any/Every radio buttons
PTGroupBox pGroupBox = new TGroupBox( this, IDC_FILTER_CAT_GROUP );
itsAnyRadioCP= new TRadioButton( this, IDC_FILTER_CAT_ANY, pGroupBox );
itsEveryRadioCP= new TRadioButton( this, IDC_FILTER_CAT_ALL, pGroupBox );

// Date selection radio buttons
PTGroupBox pDateGroup = new TGroupBox( this, IDC_FILTER_DATE_GROUP );
itsAnyDateCP = new TRadioButton( this, IDC_FILTER_ANY_DATE, pDateGroup );
itsBeforeCP  = new TRadioButton( this, IDC_FILTER_BEFORE,   pDateGroup );
itsAfterCP   = new TRadioButton( this, IDC_FILTER_AFTER,    pDateGroup );
itsDayCP     = new TRadioButton( this, IDC_FILTER_DAY,      pDateGroup );
itsWeekCP    = new TRadioButton( this, IDC_FILTER_WEEK,     pDateGroup );
itsMonthCP   = new TRadioButton( this, IDC_FILTER_MONTH,    pDateGroup );

// Date picker
TTime start;
switch( itsFilterP->itsDateEffect ) {

    case kAnyDateFilter: start = TSysState::selTime;     break;
    case kBeforeFilter:  start = itsFilterP->itsEnd;  break;
    case kAfterFilter:   start = itsFilterP->itsStart.Plus( k1Day, -1 );   break;
    case kDayFilter:     start = itsFilterP->itsStart;   break;
    case kWeekFilter:    start = itsFilterP->itsStart;  break;
    case kMonthFilter:   start = itsFilterP->itsStart;   break;
}

itsDateCP    = new TDatePick( this, IDC_FILTER_DATE, start );

// Exclusions radio buttons
PTGroupBox pExcGroup= new TGroupBox( this, IDC_FILTER_EXC_GROUP );
itsUndatedCP= new TCheckBox( this, IDC_FILTER_UNDATED, pExcGroup );
itsMilestoneCP= new TCheckBox( this, IDC_FILTER_MILESTONE, pExcGroup  );
itsFloatCP= new TCheckBox( this, IDC_FILTER_FLOAT, pExcGroup  );
itsTaskCP= new TCheckBox( this, IDC_FILTER_TASK, pExcGroup  );

itsRedCP= new TCheckBox( this, IDC_FILTER_RED, pExcGroup  );
itsRedGrayCP= new TCheckBox( this, IDC_FILTER_RED_GRAY, pExcGroup  );
itsGrayCP= new TCheckBox( this, IDC_FILTER_GRAY, pExcGroup  );
itsBlueGrayCP= new TCheckBox( this, IDC_FILTER_BLUE_GRAY, pExcGroup  );
itsBlueCP= new TCheckBox( this, IDC_FILTER_BLUE, pExcGroup  );

itsDoneCP= new TCheckBox( this, IDC_FILTER_DONE, pExcGroup  );
itsUndoneCP= new TCheckBox( this, IDC_FILTER_UNDONE, pExcGroup  );
itsAlarmCP= new TCheckBox( this, IDC_FILTER_ALARM, pExcGroup  );
itsNoAlarmCP= new TCheckBox( this, IDC_FILTER_NOALARM, pExcGroup  );
itsFreqCP= new TCheckBox( this, IDC_FILTER_FREQ, pExcGroup  );
itsNoFreqCP= new TCheckBox( this, IDC_FILTER_NOFREQ, pExcGroup  );

}

//*****************************************************************************

//*****************************************************************************
void TFilterDlg::SetupWindow( void )
{
TOurDialog::SetupWindow();

char buf[MAX_SEARCH_LEN];

itsFilterP->itsFindText.Get( buf, MAX_SEARCH_LEN );
itsFindTextCP->SetCaption( buf );

//matchstyle radio buttons
if( itsFilterP->itsMatchStyle == kCatMatchAny )
    {
    itsAnyRadioCP->Check();
    itsEveryRadioCP->Uncheck();
    }
else
    {
    itsAnyRadioCP->Uncheck();
    itsEveryRadioCP->Check();
    }

//initialize exclusion checkboxes
itsUndatedCP->SetCheck( (itsFilterP->itsExclusions[ kExcUndated ]) ? BF_CHECKED: BF_UNCHECKED );
itsMilestoneCP->SetCheck( (itsFilterP->itsExclusions[ kExcMilestone ]) ? BF_CHECKED: BF_UNCHECKED );
itsFloatCP->SetCheck( (itsFilterP->itsExclusions[ kExcFloat ]) ? BF_CHECKED: BF_UNCHECKED );
itsTaskCP->SetCheck( (itsFilterP->itsExclusions[ kExcTask ]) ? BF_CHECKED: BF_UNCHECKED );

itsRedCP->SetCheck( (itsFilterP->itsExclusions[ kExcRed ]) ? BF_CHECKED: BF_UNCHECKED );
itsRedGrayCP->SetCheck( (itsFilterP->itsExclusions[ kExcRedGray ]) ? BF_CHECKED: BF_UNCHECKED );
itsGrayCP->SetCheck( (itsFilterP->itsExclusions[ kExcGray ]) ? BF_CHECKED: BF_UNCHECKED );
itsBlueGrayCP->SetCheck( (itsFilterP->itsExclusions[ kExcBlueGray ]) ? BF_CHECKED: BF_UNCHECKED );
itsBlueCP->SetCheck( (itsFilterP->itsExclusions[ kExcBlue ]) ? BF_CHECKED: BF_UNCHECKED );

itsDoneCP->SetCheck( (itsFilterP->itsExclusions[ kExcDone ]) ? BF_CHECKED: BF_UNCHECKED );
itsUndoneCP->SetCheck( (itsFilterP->itsExclusions[ kExcUndone ]) ? BF_CHECKED: BF_UNCHECKED );
itsAlarmCP->SetCheck( (itsFilterP->itsExclusions[ kExcAlarm ]) ? BF_CHECKED: BF_UNCHECKED );
itsNoAlarmCP->SetCheck( (itsFilterP->itsExclusions[ kExcNoAlarm ]) ? BF_CHECKED: BF_UNCHECKED );
itsFreqCP->SetCheck( (itsFilterP->itsExclusions[ kExcFreq ]) ? BF_CHECKED: BF_UNCHECKED );
itsNoFreqCP->SetCheck( (itsFilterP->itsExclusions[ kExcNoFreq ]) ? BF_CHECKED: BF_UNCHECKED );

itsAnyDateCP->SetCheck( BF_UNCHECKED );
itsBeforeCP-> SetCheck( BF_UNCHECKED );
itsAfterCP->  SetCheck( BF_UNCHECKED );
itsDayCP->    SetCheck( BF_UNCHECKED );
itsWeekCP->   SetCheck( BF_UNCHECKED );
itsMonthCP->  SetCheck( BF_UNCHECKED );

if (itsFilterP->itsDateEffect == kAnyDateFilter)
    itsAnyDateCP->SetCheck( BF_CHECKED );
else if (itsFilterP->itsDateEffect == kBeforeFilter)
    itsBeforeCP->SetCheck( BF_CHECKED );
else if (itsFilterP->itsDateEffect == kAfterFilter)
    itsAfterCP->SetCheck( BF_CHECKED );
else if (itsFilterP->itsDateEffect == kDayFilter)
    itsDayCP->SetCheck( BF_CHECKED );
else if (itsFilterP->itsDateEffect == kWeekFilter)
    itsWeekCP->SetCheck( BF_CHECKED );
else if (itsFilterP->itsDateEffect == kMonthFilter)
    itsMonthCP->SetCheck( BF_CHECKED );
}

//*****************************************************************************

//*****************************************************************************
TFilterDlg::~TFilterDlg()
{
}

//*****************************************************************************
//
//*****************************************************************************
void TFilterDlg::Ok( RTMessage Msg)
{
char        buf[MAX_SEARCH_LEN];
TDateEffect effect;

//store search text
itsFindTextCP->GetText( buf, MAX_SEARCH_LEN );
itsFilterP->itsFindText.Set( buf );

if (itsAnyDateCP->GetCheck() == BF_CHECKED)
    effect = kAnyDateFilter;
else if (itsBeforeCP->GetCheck() == BF_CHECKED)
    effect = kBeforeFilter;
else if (itsAfterCP->GetCheck() == BF_CHECKED)
    effect = kAfterFilter;
else if (itsDayCP->GetCheck() == BF_CHECKED)
    effect = kDayFilter;
else if (itsWeekCP->GetCheck() == BF_CHECKED)
    effect = kWeekFilter;
else if (itsMonthCP->GetCheck() == BF_CHECKED)
    effect = kMonthFilter;
itsFilterP->SetFilterDates( effect, itsDateCP->GetSelTime());

//matchstyle radio buttons
itsFilterP->itsMatchStyle= (itsAnyRadioCP->GetCheck() == BF_CHECKED)? kCatMatchAny: kCatMatchAll;

//exclusions
itsFilterP->itsExclusions[ kExcUndated ]= (itsUndatedCP->GetCheck() == BF_CHECKED);
itsFilterP->itsExclusions[ kExcMilestone ]= (itsMilestoneCP->GetCheck() == BF_CHECKED);
itsFilterP->itsExclusions[ kExcFloat ]= (itsFloatCP->GetCheck() == BF_CHECKED);
itsFilterP->itsExclusions[ kExcTask ]= (itsTaskCP->GetCheck() == BF_CHECKED);

itsFilterP->itsExclusions[ kExcRed ]= (itsRedCP->GetCheck() == BF_CHECKED);
itsFilterP->itsExclusions[ kExcRedGray ]= (itsRedGrayCP->GetCheck() == BF_CHECKED);
itsFilterP->itsExclusions[ kExcGray ]= (itsGrayCP->GetCheck() == BF_CHECKED);
itsFilterP->itsExclusions[ kExcBlueGray ]= (itsBlueGrayCP->GetCheck() == BF_CHECKED);
itsFilterP->itsExclusions[ kExcBlue ]= (itsBlueCP->GetCheck() == BF_CHECKED);

itsFilterP->itsExclusions[ kExcDone ]= (itsDoneCP->GetCheck() == BF_CHECKED);
itsFilterP->itsExclusions[ kExcUndone ]= (itsUndoneCP->GetCheck() == BF_CHECKED);
itsFilterP->itsExclusions[ kExcAlarm ]= (itsAlarmCP->GetCheck() == BF_CHECKED);
itsFilterP->itsExclusions[ kExcNoAlarm ]= (itsNoAlarmCP->GetCheck() == BF_CHECKED);
itsFilterP->itsExclusions[ kExcFreq ]= (itsFreqCP->GetCheck() == BF_CHECKED);
itsFilterP->itsExclusions[ kExcNoFreq ]= (itsNoFreqCP->GetCheck() == BF_CHECKED);


TDialog::Ok( Msg );
}


// -----------------------------------------------------------------
// HandleClearAll
//
// Clear all fields
// -----------------------------------------------------------------
void TFilterDlg::OnClearAll( RTMessage  )
{
    itsFindTextCP->SetText( "" );

    itsAnyRadioCP->Check();
    itsEveryRadioCP->Uncheck();

    //clear exclusion checkboxes
    itsUndatedCP->Uncheck();
    itsMilestoneCP->Uncheck();
    itsFloatCP->Uncheck();
    itsTaskCP->Uncheck();

    itsRedCP->Uncheck();
    itsRedGrayCP->Uncheck();
    itsGrayCP->Uncheck();
    itsBlueGrayCP->Uncheck();
    itsBlueCP->Uncheck();

    itsDoneCP->Uncheck();
    itsUndoneCP->Uncheck();
    itsAlarmCP->Uncheck();
    itsNoAlarmCP->Uncheck();
    itsFreqCP->Uncheck();
    itsNoFreqCP->Uncheck();

    itsAnyDateCP->Check();
    itsBeforeCP->Uncheck();
    itsAfterCP->Uncheck();
    itsDayCP->Uncheck();
    itsWeekCP->Uncheck();
    itsMonthCP->Uncheck();
}

