//*****************************************************************************
// standard input dialog
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "ourinpd.h"

// -----------------------------------------------------
// Constructor
// -----------------------------------------------------
TOurInputDialog::TOurInputDialog(
    PTWindowsObject parent,
    const char *title,
    const char *prompt,
    RTSafStr buffer,
    DWORD helpID ):
    TOurDialog( parent, IDD_INPUT, NULL )
{
    itsHelpTopic= helpID;

    itsTitle.Set( title );
    itsPrompt.Set( prompt );
    itsBufferP= &buffer;

    itsPromptCP= new TStatic( this, IDC_PROMPT, 250 );
    itsBufferCP= new TEdit( this, IDC_INPUT, FALSE );
}

// -----------------------------------------------------
// SetupWindow
//
// -----------------------------------------------------
void TOurInputDialog::SetupWindow( void )
{
    TOurDialog::SetupWindow();

    SetCaption( SAF2CP( itsTitle ) );
    itsPromptCP->SetText( SAF2CP( itsPrompt ) );
}

//*****************************************************************************

//*****************************************************************************
void TOurInputDialog::Ok( RTMessage msg )
{
    char buf[255];

    itsBufferCP->GetText( buf, 255 );
    itsBufferP->Set( buf );

    TOurDialog::Ok( msg );
}
