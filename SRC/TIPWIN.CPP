//*****************************************************************************
// manage "tip" window (helpful messages about internal state)
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "sysstate.h"
#include "noteset.h"
#include "filter.h"
#include "profile.h"
#include "tipwin.h"

//*****************************************************************************

//*****************************************************************************
TTipWin::TTipWin( PTWindowsObject aParent, WORD anID ):
            TDialog( aParent, anID )
{
itsTextCP= new TStatic( this, IDD_TIPMSGTEXT, 255 );

itsActive= FALSE;   //start with window hidden
}

//*****************************************************************************

//*****************************************************************************
TTipWin::~TTipWin( void )
{
}

//*****************************************************************************

//*****************************************************************************
void TTipWin::Update( void )
{
char *pMsg= NULL;

int numSelNotes= TSysState::pSelNotes->getItemsInContainer();

if( itsActive && TProfile::bShowTips && numSelNotes == 0 )
    {
    if( 0 == TSysState::pAllNotes->getItemsInContainer() )
        { //no notes in system
        pMsg= "No notes appear in this view because no notes have been created.";
        }
    else if( !TSysState::pFilter->IsAllSelected() &&
              0 == TSysState::pFilter->NumCats() )
        { //no categories selected
        pMsg= "No notes appear in this view because no folders are selected.";
        }
    else if( TSysState::pFilter->IsActive() )
        { //everything filtered out
        pMsg= "No notes appear in this view because no notes match the filter criteria.";
        }
    else //FIXME is there a reasonable test for this?
        { // no notes in selected categories
        pMsg= "No notes appear in this view because there are no notes in the selected folders.";
        }
    }

if( pMsg )
    {
    itsTextCP->SetText( pMsg );

    SetWindowPos( HWindow, HWND_TOPMOST, 0, 0, 0, 0,
                  SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE );
    }
else
    Show( SW_HIDE );
}

//*****************************************************************************

//*****************************************************************************
BOOL TTipWin::MakeActive( BOOL b )
{
BOOL previous= itsActive;

itsActive= b;

Update();

return previous;
}
