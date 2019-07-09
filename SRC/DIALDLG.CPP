//*****************************************************************************
// dial a phone number
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "dialdlg.h"
#ifndef __cursor__
#include "cursor.h"
#endif
#ifndef  __dialsetd__
#include "dialsetd.h"
#endif
#ifndef __frame__
#include "frame.h"
#endif
#ifndef __oem__
#include "oem.h"
#endif
#ifndef  __profile__
#include "profile.h"
#endif
#ifndef  __cat__
#include "cat.h"
#endif
#ifndef  __intlfmt__
#include "intlfmt.h"
#endif
#ifndef  __noteview__
#include "noteview.h"
#endif
#ifndef __tipwin__
#include "tipwin.h"
#endif


//static variables
PTDialDialog TDialDialog::CurrentDialDialog = NULL;
int TDialDialog::itsSendTicks;
int TDialDialog::itsDialTicks;

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TDialDialog::TDialDialog( PTWindowsObject AParent, PTCat catP ):
	TOurDialog( AParent, IDD_DIAL ),
    itsStartTime()
{
    EnableKBHandler();

    itsHelpTopic= _HelpDial;

    //milliseconds between characters
    itsSendTicks =  GetPrivateProfileInt(
        TOEM::profileSection, "SendTicks", 0, TOEM::profileName );

    //milliseconds after init, before dial
    itsDialTicks =  GetPrivateProfileInt(
        TOEM::profileSection, "DialTicks", 1000, TOEM::profileName );

    itsRawStartTime= 0;
    itsRawEndTime= 0;

    itsPhoneStringCP  = new TEdit( this, IDD_DIAL_TEXT, 256 );

    itsNameCP = new TStatic( this, IDD_DIAL_NAME, 256 );
    itsStartTimeCP= new TStatic( this, IDD_DIAL_START_TIME, 256 );
    itsDurationCP= new TStatic( this, IDD_DIAL_DURATION, 256 );

    itsAccessCP = new TComboBox( this, IDD_DIAL_ACCESS, 256 );
    itsPhoneCP = new TComboBox( this, IDD_DIAL_PHONE, 256 );

    itsStartStopCP  = new TButton(  this, IDD_DIAL_START );
    itsDialHangupCP  = new TButton(  this, IDD_DIAL_DIAL );
    itsSetupCP  = new TButton(  this, IDD_DIAL_SETUP );
    itsCopyCP = new TButton(  this, IDD_DIAL_COPY );

    itsCatP= catP;
    itsTimerOn = FALSE;
    itsCallActive= FALSE;

    // keep a global copy of this dialog
    // so that only one can be open at a time,
    // and it gets UpdateTime messages
    CurrentDialDialog = this;
}


// -------------------------------------------------
// Destructor
// -------------------------------------------------
TDialDialog::~TDialDialog( void )
{
    CurrentDialDialog = NULL;
}

// -------------------------------------------------
// SetupWindow
// -------------------------------------------------
void TDialDialog::SetupWindow( void )
{
    // inherited
    TOurDialog::SetupWindow();

    // initialize listboxes (manually because of Borland combo box bug)
    itsPhoneCP->AddString( "Work" );
    itsPhoneCP->AddString( "Home" );
    itsPhoneCP->AddString( "Fax" );
    itsPhoneCP->AddString( "Cell" );
    itsPhoneCP->AddString( "Alt 1" );
    itsPhoneCP->AddString( "Alt 2" );
    itsPhoneCP->SetSelIndex( 0 );

    itsAccessCP->AddString( "Local" );
    itsAccessCP->AddString( "Long Distance" );
    itsAccessCP->AddString( "International" );
    itsAccessCP->AddString( "Alternate" );
    SetDefaultAccess();

    char phone[100];
    GetPhone( phone );
    itsPhoneStringCP->SetText( phone );

    TSafStr  name;
    itsCatP->GetShortName( name );
    itsNameCP->SetText( SAF2CP( name ));

    itsTipWinState= TPimFrame::pTipWin->MakeActive( FALSE );
}

// -------------------------------------------------
// HandleAccess
// -------------------------------------------------
void TDialDialog::HandleAccess( RTMessage )
{
    if( itsPhoneStringCP->HWindow != NULL )
        {
        char phone[100];
        GetPhone( phone );
        itsPhoneStringCP->SetText( phone );
        }
}

// -------------------------------------------------
// HandleSetup
// -------------------------------------------------
void TDialDialog::HandleSetup( RTMessage )
{
    PTDialSetupDialog dlgP = new TDialSetupDialog( this );
    GetApplication()->ExecDialog( dlgP );

    char phone[100];
    GetPhone( phone );
    itsPhoneStringCP->SetText( phone );
}

//*****************************************************************************

//*****************************************************************************
void TDialDialog::HandleDial( RTMessage )
{
    DCB   dcb;
    int   port, err;
    char  phone[100];
    char  phoneCmd[100];
    char  commPort[10];

    TCursor::Busy( TRUE );

    if( itsCallActive )
        {
        Hangup( this );
        itsCallActive= FALSE;
        itsDialHangupCP->SetCaption( "&Dial" );
        }
    else
        {
        // set up the phone number cmd
        itsPhoneStringCP->GetText( phone, 100 );

        if (TProfile::bTone)
            wsprintf( phoneCmd, "ATDT %s;\r", phone );
        else
            wsprintf( phoneCmd, "ATDP %s;\r", phone );

        // open the port
        wsprintf( commPort, "COM%d", TProfile::CommPort );
        port   = OpenComm( commPort, 2000, 2000 );

        // modify the baud rate
        err    = GetCommState( port, &dcb );
        dcb.BaudRate = TProfile::BaudRate;
        err    = SetCommState( &dcb );

        // write out the strings
        err = WriteString( port, "ATQ0V1E1S0=0\r" );
        if (!err)
            {
            // wait some time
            long ticks = GetTickCount();
            while (ticks + itsDialTicks > GetTickCount());

            err = WriteString( port, phoneCmd );
            }

        if (err)
            {
            TMessageBox::Ok( this, "There's a problem with the modem.",
                "Dial Error" );
            }
        else
            {
            itsCallActive= TRUE;
            itsDialHangupCP->SetCaption( "&Hangup" );
            }

        // close the port
        err    = CloseComm( port );
        }

    TCursor::Busy( FALSE );
}

//*****************************************************************************

//*****************************************************************************
void TDialDialog::HandlePhone( RTMessage )
{
    if( itsPhoneStringCP->HWindow != NULL )
        {
        char phone[100];

        SetDefaultAccess();

        GetPhone( phone );
        itsPhoneStringCP->SetText( phone );
        }
}

//*****************************************************************************

//*****************************************************************************
void TDialDialog::HandleStart( RTMessage )
{
    if (!itsTimerOn)
        {
        itsRawStartTime = time( NULL );
        itsStartTime.SetNow();
        itsTimerOn = TRUE;

        // tell the frame to send us UpdateTime
        // messages
        TDialDialog::CurrentDialDialog = this;

        // fill in the text fields
        char  text[256];
        TIntlFmt::FormatTime( itsStartTime, TRUE, text );
        itsStartTimeCP->SetText( text );
        itsDurationCP->SetText( "" );
        itsStartStopCP->SetCaption( "&Stop" );
        }
    else
        {
        itsTimerOn = FALSE;

        #if 0
        // tell the frame to stop sending
        // UpdateTime messages
        TDialDialog::CurrentDialDialog = NULL;
        #endif

        itsStartStopCP->SetCaption( "&Start" );
        }
}


//*****************************************************************************

//*****************************************************************************
void TDialDialog::UpdateTime( void )
{
    // toggle the copy to note button
    EnableWindow( itsCopyCP->HWindow,
        TNoteView::CurrentNoteViewP != NULL &&
        itsRawEndTime != itsRawStartTime );

    // don't do anything if the timer's not on
    if (!itsTimerOn) return;

    itsRawEndTime = time( NULL );
    long elapsed = difftime( itsRawEndTime, itsRawStartTime );

    // fill in the text fields
    char  text[256];
    wsprintf( text, "%ld:%02ld", elapsed / 60, elapsed % 60 );
    itsDurationCP->SetText( text );
}

//*****************************************************************************

//*****************************************************************************
void TDialDialog::HandleCopy( RTMessage )
{
    // find the note and bring it to the top
    if (TNoteView::CurrentNoteViewP && itsRawEndTime != itsRawStartTime )
        {
        #if 0
        BringWindowToTop(
            TNoteView::CurrentNoteViewP->HWindow );
        #endif

        UpdateTime();
        long min = (itsRawEndTime - itsRawStartTime) / 60;
        long sec = (itsRawEndTime - itsRawStartTime) % 60;
        if (sec) min++;

        TDuration duration( k1Min, min );
        TNoteView::CurrentNoteViewP->SetFromDialDialog(
            itsStartTime, duration );
        }
}

// -------------------------------------------------
//
//  Close the dialog
// -------------------------------------------------
void TDialDialog::Cancel( RTMessage msg )
{

    TPimFrame::pTipWin->MakeActive( itsTipWinState );
    TOurDialog::Cancel( msg );

}

// -------------------------------------------------
// StripAreaCode
//
//  Checks for '(abc)' where abc is
//  TProfile::LocalAreaCode and removes it if it
//  finds it
// -------------------------------------------------
void TDialDialog::StripAreaCode( char *string )
{
    char *p= string;

    //move past starting 0s and 1s, and starting punctuation
    while( *p )
        {
        if( isdigit( *p ) && (*p != '0' && *p != '1') )
            break;
        p++;
        }

    if( strlen( p ) >= 3 )
        {
        if ((*p == TProfile::LocalAreaCode[0]) &&
           (*(p+1) == TProfile::LocalAreaCode[1]) &&
           (*(p+2) == TProfile::LocalAreaCode[2])
           )
            {
            //point past area code
            p+= 3;

            //move to next digit
            while( *p && !isdigit( *p ))
                p++;

            char result[50];
            strcpy( result, p );
            strcpy( string, result );

            return;
            }
        }
}

//*****************************************************************************
//FIXME this doesn't work yet
//*****************************************************************************
BOOL TDialDialog::IsLocalNumber( const char* number )
{
    BOOL rc= TRUE;
#if 0
    char *p= (char *) number;

    //move past starting 0s and 1s, and starting punctuation
    while( *p )
        {
        if( isdigit( *p ) )
            {
            if( *p == '0' || *p == '1' )
                return FALSE;                  //not local
            else
                break;
            }
        p++;
        }

    if( strlen( p ) >= 3 )
        {
        if ((*p == TProfile::LocalAreaCode[0]) &&
           (*(p+1) == TProfile::LocalAreaCode[1]) &&
           (*(p+2) == TProfile::LocalAreaCode[2])
           )
            {
            rc= TRUE;
            }
        }
#endif

    return rc;
}


// -------------------------------------------------
// WriteString
//
//  Write a string and wait for the acknowledging ok
// -------------------------------------------------
int TDialDialog::WriteString( int port, char *stringP )
{
    int     err, length;
    char    buf[200];
    COMSTAT stat;
    long    ticks;
    BOOL    timeOut;

    TCursor::Busy( TRUE );

    while( *stringP )
        {
        //slow down the sending
        ticks= GetTickCount();
        while( ticks + itsSendTicks > GetTickCount() );

        err= WriteComm( port, stringP, 1 );
        stringP++;
        }

    //wait for a response
    ticks  = GetTickCount();
    do
    {
        length = ReadComm( port, buf, 200 );
        err    = GetCommError( port, &stat );
        timeOut = ticks + 3 * 1000 < GetTickCount();
    } while ((length == 0) && (err == 0) && !timeOut);

    TCursor::Busy( FALSE );

    if (timeOut || err)
        return -1;
    else
        return 0;
}

// -------------------------------------------------
// GetPhone
//
//  Return the full phone number
// -------------------------------------------------
void TDialDialog::GetPhone( char *phone )
{
    char  prefix[25];
    char  suffix[25];

    // build the phone number
    TAddressField field;
    switch( itsPhoneCP->GetSelIndex())
        {
        default:
        case 0:  field =  kWorkPhoneField;  break;
        case 1:  field =  kHomePhoneField;  break;
        case 2:  field =  kFaxField;        break;
        case 3:  field =  kCellPhoneField;  break;
        case 4:  field =  kAlt1PhoneField;  break;
        case 5:  field =  kAlt2PhoneField;  break;
        }

    TSafStr number;
    itsCatP->GetField( field, number );

    switch( itsAccessCP->GetSelIndex())
        {
        default:
        case 0:

            strcpy( prefix, TProfile::LocalPrefix );
            strcpy( suffix, TProfile::LocalSuffix );
            break;

        case 1:

            strcpy( prefix, TProfile::LongDistancePrefix );
            strcpy( suffix, TProfile::LongDistanceSuffix );
            break;

        case 2:

            strcpy( prefix, TProfile::InternationalPrefix );
            strcpy( suffix, TProfile::InternationalSuffix );
            break;

        case 3:

            strcpy( prefix, TProfile::AlternatePrefix );
            strcpy( suffix, TProfile::AlternateSuffix );
            break;
        }

    char numStr[50];
    number.Get( numStr, 50 );
    StripAreaCode( numStr );
    wsprintf( phone, "%s%s%s", prefix, numStr, suffix );
}

//*****************************************************************************
//  hangup
//*****************************************************************************
void TDialDialog::Hangup( PTWindowsObject parent )
{
    DCB   dcb;
    int   port, err;
    char  commPort[10];

    TCursor::Busy( TRUE );

    // open the port
    wsprintf( commPort, "COM%d", TProfile::CommPort );
    port   = OpenComm( commPort, 2000, 2000 );

    // modify the baud rate
    err    = GetCommState( port, &dcb );
    dcb.BaudRate = TProfile::BaudRate;
    err    = SetCommState( &dcb );

    err = WriteString( port, "ATH\r" );

    if (err)
        {
        TMessageBox::Ok( parent, "There is a problem with the modem.",
            "Hangup Error" );
        }

    // close the port
    err    = CloseComm( port );
    err;

    if( CurrentDialDialog )
        CurrentDialDialog->itsCallActive= FALSE;

    TCursor::Busy( FALSE );
}

//*****************************************************************************
// check out the phone number, pick proper access based on
// whether it's a local number
//*****************************************************************************
void TDialDialog::SetDefaultAccess( void )
{
    // get the phone number
    TAddressField field;
    switch( itsPhoneCP->GetSelIndex())
        {
        default:
        case 0:  field =  kWorkPhoneField;  break;
        case 1:  field =  kHomePhoneField;  break;
        case 2:  field =  kFaxField;        break;
        case 3:  field =  kCellPhoneField;  break;
        case 4:  field =  kAlt1PhoneField;  break;
        case 5:  field =  kAlt2PhoneField;  break;
        }

    TSafStr number;
    itsCatP->GetField( field, number );

    if( IsLocalNumber( number ) )
        itsAccessCP->SetSelIndex( 0 );
    else
        itsAccessCP->SetSelIndex( 1 );

}
