//*****************************************************************************
// preferences dialog: pick colors, auto-start, show tips, conflicts
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "frame.h"
#include "gdimap.h"
#include "preferd.h"
#include "profile.h"
#include "tipwin.h"

//*****************************************************************************

//*****************************************************************************
TPreferD::TPreferD( PTWindowsObject aParent, WORD anID ): TOurDialog( aParent, anID )
{
itsHelpTopic= _HelpPreferences;

itsDspGroupCP=  new TGroupBox( this, IDC_DSPGROUP );
itsStdButtonCP= new TRadioButton( this, IDC_DSPNORMAL, itsDspGroupCP );
itsLCDButtonCP= new TRadioButton( this, IDC_DSPLCD, itsDspGroupCP );

itsAutoLoadCP= new TCheckBox( this, IDC_AUTOLOAD, NULL );
itsShowTipsCP= new TCheckBox( this, IDC_SHOWTIPS, NULL );

itsChkGroupCP   = new TGroupBox( this, IDC_TIMECONF_GROUP );
itsChkNoneCP    = new TRadioButton( this, IDC_TIMECONF_NEVER, itsChkGroupCP );
itsChkAllCP     = new TRadioButton( this, IDC_TIMECONF_ALWAYS, itsChkGroupCP );
itsChkSelectCP  = new TRadioButton( this, IDC_TIMECONF_CAT, itsChkGroupCP );

}

//*****************************************************************************

//*****************************************************************************
void TPreferD::SetupWindow()
{
TOurDialog::SetupWindow();

switch( TProfile::colorSet )
    {
    case kStdColors:
        itsStdButtonCP->SetCheck( BF_CHECKED );
        break;
    case kLCDColors:
        itsLCDButtonCP->SetCheck( BF_CHECKED );
        break;
    default:
        break;
    };

itsAutoLoadCP->SetCheck( (TProfile::bAutoLoad) ? BF_CHECKED: BF_UNCHECKED );
itsShowTipsCP->SetCheck( (TProfile::bShowTips) ? BF_CHECKED: BF_UNCHECKED );

switch( TProfile::TimeConflict )
    {
    case kTimeConflict_Never:
        itsChkNoneCP->SetCheck( BF_CHECKED );
        break;
    case kTimeConflict_Cat:
        itsChkSelectCP->SetCheck( BF_CHECKED );
        break;
    case kTimeConflict_Always:
        itsChkAllCP->SetCheck( BF_CHECKED );
        break;
    default:
        break;
    }
}

//*****************************************************************************

//*****************************************************************************
TPreferD::~TPreferD()
{
}

//*****************************************************************************

//*****************************************************************************
void TPreferD::Ok( RTMessage msg )
{

if( itsStdButtonCP->GetCheck() == BF_CHECKED && TProfile::colorSet != kStdColors )
    TGDIMap::SetGDIMap( kStdColors );
else if( itsLCDButtonCP->GetCheck() == BF_CHECKED && TProfile::colorSet != kLCDColors )
    TGDIMap::SetGDIMap( kLCDColors );

TProfile::bAutoLoad= (itsAutoLoadCP->GetCheck() == BF_CHECKED);

TProfile::bShowTips= (itsShowTipsCP->GetCheck() == BF_CHECKED);
TPimFrame::pTipWin->Update();

if( itsChkAllCP->GetCheck() == BF_CHECKED )
    TProfile::TimeConflict= kTimeConflict_Always;
else if( itsChkSelectCP->GetCheck() == BF_CHECKED )
    TProfile::TimeConflict= kTimeConflict_Cat;
else if( itsChkNoneCP->GetCheck() == BF_CHECKED )
    TProfile::TimeConflict= kTimeConflict_Never;

TDialog::Ok( msg );
}

