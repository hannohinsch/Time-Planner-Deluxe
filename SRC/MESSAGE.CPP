//*****************************************************************************
// message box dialogs
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#ifndef __ourdlg__
#include "ourdlg.h"
#endif

#include "message.h"

_CLASSDEF( TOk )
_CLASSDEF( TOkCancel )
_CLASSDEF( TThreeButton )


// #########################################################################
//
//                           TOk dialog
//
// #########################################################################

class _CLASSTYPE TOk: public TOurDialog {

    public:

    TOk( PTWindowsObject parent, const char *message, const char *title );
    ~TOk( void );

    private:

    void SetupWindow( void );

    PTStatic      itsMessageP;
    PTBButton     itsOkP;

    char          itsMessage[256];
    char          itsTitle[256];
};

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
TOk::TOk( PTWindowsObject parent, const char *message, const char *title ):
	TOurDialog( parent, IDD_OK )
{
    itsMessageP = new TStatic( this, IDD_MESSAGE, 256 );
    itsOkP      = new TBButton( this, IDOK );

    strcpy( itsTitle, title );
    strcpy( itsMessage, message );
}

// -------------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------------
TOk::~TOk( void )
{
}

// -------------------------------------------------
// Setup
// -------------------------------------------------
void TOk::SetupWindow( void )
{
    // inherited
    TOurDialog::SetupWindow();

    itsMessageP->SetText( itsMessage );
    SetCaption( itsTitle );

}

// #########################################################################
//
//                           TOkCancel dialog
//
// #########################################################################

class _CLASSTYPE TOkCancel: public TOurDialog {

    public:

    TOkCancel( PTWindowsObject parent, const char *message, const char *title );
    ~TOkCancel( void );

    private:

    void SetupWindow( void );

    PTStatic      itsMessageP;
    PTBButton     itsOkP;
    PTBButton     itsCancelP;

    char          itsMessage[256];
    char          itsTitle[256];
};

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
TOkCancel::TOkCancel( PTWindowsObject parent, const char *message, const char *title ):
	TOurDialog( parent, IDD_OK_CANCEL )
{
    itsMessageP = new TStatic( this, IDD_MESSAGE, 256 );
    itsOkP      = new TBButton( this, IDOK );
    itsCancelP  = new TBButton( this, IDCANCEL );

    strcpy( itsTitle, title );
    strcpy( itsMessage, message );
}

// -------------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------------
TOkCancel::~TOkCancel( void )
{
}

// -------------------------------------------------
// Setup
// -------------------------------------------------
void TOkCancel::SetupWindow( void )
{
    // inherited
    TOurDialog::SetupWindow();

    itsMessageP->SetText( itsMessage );
    SetCaption( itsTitle );

}

// #########################################################################
//
//                           TThreeButton dialog
//
// #########################################################################


class _CLASSTYPE TThreeButton: public TOurDialog {

    public:

    TThreeButton( PTWindowsObject parent, const int resId,
        const char *message, const char *title,
        const char *yesText, const char *noText );
    ~TThreeButton( void );

    void Yes(    RTMessage Msg ) = [ ID_FIRST + IDYES    ];
    void No(     RTMessage Msg ) = [ ID_FIRST + IDNO     ];

    private:

    void SetupWindow( void );

    PTStatic      itsMessageP;
    PTButton     itsYesP;
    PTButton     itsNoP;
    PTButton     itsCancelP;

    char          itsMessage[256];
    char          itsTitle[256];
    char          itsYesText[256];
    char          itsNoText[256];
};

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
TThreeButton::TThreeButton( PTWindowsObject parent,
    const int resId,
    const char *message, const char *title,
    const char *yesText, const char *noText ):
	TOurDialog( parent, resId )
{
    itsMessageP = new TStatic( this, IDD_MESSAGE, 256 );
    itsYesP     = new TButton( this, IDYES );
    itsNoP      = new TButton( this, IDNO );
    itsCancelP  = new TButton( this, IDCANCEL );

    strcpy( itsTitle, title );
    strcpy( itsMessage, message );
    strcpy( itsYesText, yesText );
    strcpy( itsNoText, noText ); 
}

// -------------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------------
TThreeButton::~TThreeButton( void )
{
}

// -------------------------------------------------
// Setup
// -------------------------------------------------
void TThreeButton::SetupWindow( void )
{
    // inherited
    TOurDialog::SetupWindow();

    itsMessageP->SetText( itsMessage );
    SetCaption( itsTitle );
    itsYesP->SetCaption( itsYesText );
    itsNoP->SetCaption( itsNoText );
}

// -------------------------------------------------------------------------
// Yes
// -------------------------------------------------------------------------
void TThreeButton::Yes( RTMessage  )
{
    CloseWindow( IDYES );
}

// -------------------------------------------------------------------------
// No
// -------------------------------------------------------------------------
void TThreeButton::No( RTMessage  )
{
    CloseWindow( IDNO );
}
// #########################################################################
//
//                           TMessageBox
//
// #########################################################################

// -------------------------------------------------------------------------
// Ok
//
// Puts up a message box with the message with an ok button
// -------------------------------------------------------------------------
void TMessageBox::Ok( PTWindowsObject parent, const char *message, const char *title )
{
    PTOk dialog = new TOk( parent, message, title );
    GetApplicationObject()->ExecDialog( dialog );
}

// -------------------------------------------------------------------------
// OkAndCancel
//
// Puts up a message box with the message with an ok and cancel button
// -------------------------------------------------------------------------
int TMessageBox::OkAndCancel( PTWindowsObject parent, const char *message, const char *title )
{
    PTOkCancel dialog = new TOkCancel( parent, message, title );
    return GetApplicationObject()->ExecDialog( dialog );
}

// -------------------------------------------------------------------------
// ThreeButton
//
// Puts up a message box with the message with yes, no and cancel buttons
// -------------------------------------------------------------------------
int TMessageBox::YesNoCancel( PTWindowsObject parent, const char *message, const char *title )
{
    PTThreeButton dialog =
        new TThreeButton( parent, IDD_YES_NO_CANCEL, message, title, "Yes", "No" );
    return GetApplicationObject()->ExecDialog( dialog );
}

// -------------------------------------------------------------------------
// ThreeButton
//
// Same as Yes/No/Cancel, but the text for 'yes' and
// 'no' are replaced with something else
// -------------------------------------------------------------------------
int TMessageBox::ThreeButton( PTWindowsObject parent,
    const int resId, const char *message, const char *title,
    const char *yesText, const char *noText )
{
    PTThreeButton dialog =
        new TThreeButton( parent, resId, message, title,
        yesText, noText );
    return GetApplicationObject()->ExecDialog( dialog );
}

