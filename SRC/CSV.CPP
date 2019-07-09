//*****************************************************************************
// comma separated value file - import & export
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "csv.h"
#ifndef __frame__
#include "frame.h"
#endif
#ifndef __parse__
#include "parse.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif

const int MaxStringLength = 1024;

class _CLASSTYPE TCSVDialog: public TOurDialog {

    public:

        TCSVDialog( PTWindowsObject AParent,
            char *message, BOOL allowContinue,
            TCSVError *actionP );
        virtual ~TCSVDialog( void );

    private:

        void SetupWindow();

        // handle buttons
        void HandleSkip( RTMessage Msg )    = [ ID_FIRST + IDD_CSV_SKIP ];
        void HandleContinue( RTMessage Msg )= [ ID_FIRST + IDD_CSV_CONTINUE ];
        void Cancel( RTMessage Msg )        = [ ID_FIRST + IDCANCEL ];

        PTBButton    itsSkipButtonP;
        PTBButton    itsContinueButtonP;
        PTBButton    itsCancelButtonP;
        PTStatic     itsTextP;

        char         *itsMessage;
        TCSVError    *itsActionP;
        BOOL         itsContinueAllowed;
};


// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------
TCSV::TCSV( void )
{
    itsLine      = 1;
    itsCharacter = 0;
}

// -----------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------
TCSV::~TCSV( void )
{
}


// -----------------------------------------------------------------------
// ReadError
// -----------------------------------------------------------------------
TCSVError TCSV::ReadError( TState error, ifstream &is )
{
    char       buf[80];
    char       buf2[80];
    TCSVError  action;

    switch( error ) {

        case kEOFError:

            wsprintf( buf, "Found unexpected end of file." );
            break;

        case kUnknownStateError:

            wsprintf( buf, "Internal Error (unknown state)." );
            break;

        case kBadCharacterError:

            wsprintf( buf, "Don't understand character after quote." );
            break;

        case kUnexpectedQuoteError:

            wsprintf( buf, "Found an unexpected quote." );
            break;

        case kReadIntError:

            wsprintf( buf, "Expected an integer." );
            break;

        case kReadEnumError:

            wsprintf( buf, "Integer out of range for enumerated type." );
            break;

        case kReadDateError:

            wsprintf( buf, "Expected a date." );
            break;

        case kReadTimeError:

            wsprintf( buf, "Expected a time." );
            break;

        case kReadValueError:

            wsprintf( buf, "Expected a value." );
            break;

        case kReadDurationError:

            wsprintf( buf, "Expected a duration." );
            break;

        case kStart:
        case kState1:
        case kState2:
        case kState3:
        case kSuccessWithComma:
        case kSuccessWithEOL:

            return kCSVOk;

        default:

            wsprintf( buf, "Internal Error (missed a state)." );
            break;
    }


    wsprintf( buf2, "%s\nLine %d character %d.", buf, itsLine, itsCharacter );
    GetApplicationObject()->ExecDialog(
        new TCSVDialog( TPimFrame::pFrame, buf2, TRUE, &action));


    // skip to the end of the line
	if (action == kCSVErrorSkip) {

        char c;
        int line = itsLine;

		while (Get( is, c ) && (itsLine == line));
    }

    return( action );

}

// -----------------------------------------------------------------------
// ShouldBeEOL
//
//   We're expecting to be at an eol, if we're not then
//   ask what to do
// -----------------------------------------------------------------------
TCSVError TCSV::ShouldBeEOL( ifstream &is, BOOL eol )
{
    char       buf[80];
    TCSVError  action;

    if (eol) {

        return kCSVOk;
    }

    wsprintf( buf, "There are too many fields on this line.\nLine %d character %d.", itsLine, itsCharacter );
    GetApplicationObject()->ExecDialog(
        new TCSVDialog( TPimFrame::pFrame, buf, FALSE, &action));


    // skip to the end of the line
	if (action == kCSVErrorSkip) {

        char c;
        int line = itsLine;

		while (Get( is, c ) && (itsLine == line));
    }

    return( action );

}

// -----------------------------------------------------------------------
// ShouldntBeEOL
//
//   We're not expecting to be at an eol, if we are then
//   ask what to do
// -----------------------------------------------------------------------
TCSVError TCSV::ShouldntBeEOL( ifstream &/* is */, BOOL eol )
{
    char       buf[80];
    TCSVError  action;

    if (!eol) {

        return kCSVOk;
    }

    wsprintf( buf, "Not enough fields on this line.\nLine %d character %d.", itsLine, itsCharacter );
    GetApplicationObject()->ExecDialog(
        new TCSVDialog( TPimFrame::pFrame, buf, FALSE, &action));

    return( action );

}

// -----------------------------------------------------------------------
// WriteString
//
//     write the string out as a "comma separated value"
// -----------------------------------------------------------------------
void TCSV::WriteString( ofstream &os, char *s, BOOL eol )
{
    int strLength= strlen( s );
    long maxStringLength= 2 * strLength + 1;
    char *output= new char[ (WORD) maxStringLength ];

    // copy the source string s
    strcpy( output, s );

    // double up quotes by pushing characters to the right
    for (int i = 0; i < maxStringLength && output[i] != '\0' ; i++)
    {

        if (output[i] == '"')
        {
            memmove( &output[i + 1], &output[i], strlen( &output[i] ) );
            i++;
        }
    }

    // write out the output string surrounded by quotes, and followed
    // by a new-line or comma character as appropriate
    os << "\"" << output << "\"";

    if (eol) 
        os << "\n";
    else
        os << ",";

    delete output;
}


// -----------------------------------------------------------------------
// WriteString
//
//     write the string out as a "comma separated value"
// -----------------------------------------------------------------------
void TCSV::WriteString( ofstream &os, RTSafStr s, BOOL eol )
{
    WriteString( os, (char*) s.Addr(), eol );
}


// -----------------------------------------------------------------------
// WriteInt
//
//     write the integer out as a "comma separated value"
// -----------------------------------------------------------------------
void TCSV::WriteInt( ofstream &os, int x, BOOL eol )
{
    char buf[80];
    wsprintf( buf, "%d", x );

    os << (char*) "\"";
    os << buf;
    os << "\"";

    if (eol) {

        os << "\n";
    }
    else {

        os << ",";
    }
}


// -----------------------------------------------------------------------
// ReadInt
//
//    Read a string and convert it to an int
// -----------------------------------------------------------------------
TCSVError TCSV::ReadInt( ifstream &is, int &x, BOOL &eol )
{
    TSafStr s;
    TCSVError err;

    err = ReadString( is, s, eol );
    if (err != kCSVOk) {

        return( err );
    }

    if (s.Len() == 0) {

        return( ReadError( kReadIntError, is ));
    }


    x = 0;
    TSafStrIter p( s );
    while( *p ) {

        char c = *p++;

        if (('0' <= c) && (c <= '9')) {

            x = 10 * x + c - '0';
        }
        else {

            return( ReadError( kReadIntError, is ));
        }
    }

    return( kCSVOk );
}

// -----------------------------------------------------------------------
// ReadEnum
//
//    Read an integer and make sure that within the specified range
// -----------------------------------------------------------------------
TCSVError TCSV::ReadEnum( ifstream &is, int low, int high, int &x, BOOL &eol )
{
    TCSVError err;

    err = ReadInt( is, x, eol );
    if (err != kCSVOk) {

        return( err );
    }

    if ((low <= x) && (x <= high)) {

        return( kCSVOk );
    }
    else {

        return( ReadError( kReadEnumError, is ));
    }

}

// -----------------------------------------------------------------------
// ReadDate
//
//    Read a string and convert it to a date
// -----------------------------------------------------------------------
TCSVError TCSV::ReadDate( ifstream &is, RTTime time, BOOL &eol )
{
    TSafStr s;
    TCSVError err;

    err = ReadString( is, s, eol );
    if (err != kCSVOk) {

        return( err );
    }

    TParse::ChkDate( (char*) s.Addr(), time );
    if ((s.Len() > 0) && !time.HasDate()) {

        return( ReadError( kReadDateError,is ));
    }
    else {

        return( kCSVOk );
    }
}

// -----------------------------------------------------------------------
// ReadTime
//
//    Read a string and convert it to a time
// -----------------------------------------------------------------------
TCSVError TCSV::ReadTime( ifstream &is, RTTime time, BOOL &eol )
{
    TSafStr s;
    TCSVError err;

    err = ReadString( is, s, eol );
    if (err != kCSVOk) {

        return( err );
    }

    TParse::ChkTime( (char*) s.Addr(), time );
    if ((s.Len() > 0) && !time.HasTime()) {

        return( ReadError( kReadTimeError,is ));
    }
    else {

        return( kCSVOk );
    }
}

// -----------------------------------------------------------------------
// ReadValue
//
//    Read a string and convert it to a value
// -----------------------------------------------------------------------
TCSVError TCSV::ReadValue( ifstream &is, RTValue value, BOOL &eol )
{
    TSafStr s;
    TCSVError err;

    err = ReadString( is, s, eol );
    if (err != kCSVOk) {

        return( err );
    }


    TParse::ChkValue( (char*) s.Addr(), value );
    if ((s.Len() > 0) && !value.HasValue()) {

        return( ReadError( kReadValueError, is ));
    }
    else {

        return( kCSVOk );
    }
}

// -----------------------------------------------------------------------
// ReadDuration
//
//    Read a string and convert it to a duration
// -----------------------------------------------------------------------
TCSVError TCSV::ReadDuration( ifstream &is,  RTDuration duration, BOOL &eol )
{
    TSafStr   s;
    TCSVError err;

    err = ReadString( is, s, eol );
    if (err != kCSVOk) {

        return( err );
    }

    if (s.Len() == 0) {

        return( ReadError( kReadDurationError, is ));
    }


    long x = 0;
    TSafStrIter p( s );
    while( *p ) {

        char c = *p++;

        if (('0' <= c) && (c <= '9')) {

            x = 10 * x + c - '0';
        }
        else {

            return( ReadError( kReadDurationError, is ));
        }
    }

    duration.Set( k1Min, x );
    return( kCSVOk );
}

// -----------------------------------------------------------------------
// ReadString
//
//    The main control of a state machine to parse comma separated values
// -----------------------------------------------------------------------
TCSVError TCSV::ReadString( ifstream &is, RTSafStr token, BOOL &eol )
{

    TState state = kStart;
    token.Clear();

    while (TRUE) {

        switch( state ) {

            case kStart:

                state = StartState( is, token );
                break;

            case kState1:

                state = State1( is, token );
                break;

            case kState2:

                state = State2( is, token );
                break;

            case kState3:

                state = State3( is, token );
                break;

            case kSuccessWithComma:

                eol = FALSE;
                return( kCSVOk );

            case kSuccessWithEOL:

                eol =TRUE;
                return( kCSVOk );

            default:

                return( ReadError( state, is ));
        }
    }
}

// -----------------------------------------------------------------------
// Get
//
//     Get a character and set the position
// -----------------------------------------------------------------------
istream & TCSV::Get( ifstream &is, char &c )
{
    char c1;

    istream &stream1 = is.get( c );
    if (!stream1) {

        return( stream1 );
    }

    if (c == '\n') {

		itsLine++;
        itsCharacter = 0;

        if (is.peek() == '\r') {

            // if it's really a \r then go get it
			return( is.get( c1 ));
        }
    }
    else if (c == '\r') {

		itsLine++;
        itsCharacter = 0;

        if (is.peek() == '\n') {

            // if it's really a \n then go get it
			return( is.get( c1 ));
        }
    }
    else {

        itsCharacter++;
    }


    return( stream1 );
}

// -----------------------------------------------------------------------
// StartState
//
//     Success if char is comma or eol; move to state 1 on quote; move
//     to state 3 on anything else
// -----------------------------------------------------------------------
TCSV::TState TCSV::StartState( ifstream &is, RTSafStr token )
{
    char c;

    if (!Get( is, c )) {

        return kEOFError;
    }

    switch( c ) {

        case '"':                          return kState1;
        case ',':                          return kSuccessWithComma;
        case '\n':                         return kSuccessWithEOL;
        default:   token.Cat( c );         return kState3;
    }
}

// -----------------------------------------------------------------------
// State1
//
//     Collects characters until it finds a potential close quote
// -----------------------------------------------------------------------
TCSV::TState TCSV::State1( ifstream &is, RTSafStr token )
{
    char c;

    if (!Get( is, c )) {

        return kEOFError;
    }

    switch( c ) {

        case '"':                          return kState2;
        default:  token.Cat( c );          return kState1;
    }
}

// -----------------------------------------------------------------------
// State2
//
//     Checks to see if a quote is really a close quote or not and returns
//     to state 1 if it's not
// -----------------------------------------------------------------------
TCSV::TState TCSV::State2( ifstream &is, RTSafStr token )
{
    char c;

    if (!Get( is, c )) {

        return kEOFError;
    }

    switch( c ) {

        case '\n':                             return kSuccessWithEOL;
        case ',':                              return kSuccessWithComma;
        case '"':    token.Cat( c );           return kState1;
        default:                               return kBadCharacterError;
    }
}

// -----------------------------------------------------------------------
// State3
//
//     Collects characters until it finds a comma or eol which marks
//     the end. A quote would be illegal here.
// -----------------------------------------------------------------------
TCSV::TState TCSV::State3( ifstream &is, RTSafStr token )
{
    char c;

    if (!Get( is, c )) {

        return kEOFError;
    }

    switch( c ) {

        case '\n':                         return kSuccessWithEOL;
        case ',':                          return kSuccessWithComma;
        case '"':                          return kUnexpectedQuoteError;
        default:   token.Cat( c );         return kState3;

    }
}


// -----------------------------------------------------------------
// -----------------------------------------------------------------
//
//                       CSV Error Dialog
//
// -----------------------------------------------------------------
// -----------------------------------------------------------------

// -----------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------
TCSVDialog::TCSVDialog( PTWindowsObject AParent,
    char *message, BOOL allowContinue, TCSVError *actionP )
    : TOurDialog( AParent, IDD_CSV_ERROR )
{

    // create the buttons
    itsSkipButtonP     = new TBButton( this, IDD_CSV_SKIP );
    itsContinueButtonP = new TBButton( this, IDD_CSV_CONTINUE );
    itsCancelButtonP   = new TBButton( this, IDCANCEL );

    // create the text
    itsTextP           = new TStatic( this, IDD_CSV_TEXT, 256 );
    itsMessage         = message;
    itsActionP         = actionP;
    itsContinueAllowed = allowContinue;
}


// -----------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------
TCSVDialog::~TCSVDialog( void )
{
}

// -----------------------------------------------------------------
// SetupWindow
// -----------------------------------------------------------------
void TCSVDialog::SetupWindow( void )
{
    TOurDialog::SetupWindow();

    if (!itsContinueAllowed) {

        itsContinueButtonP->Show( SW_HIDE );
    }

    itsTextP->SetText( itsMessage );
}

// -----------------------------------------------------------------
// HandleSkip
// -----------------------------------------------------------------
void TCSVDialog::HandleSkip( RTMessage msg )
{
    (*itsActionP) = kCSVErrorSkip;
    TDialog::Cancel( msg );
}

// -----------------------------------------------------------------
// HandleContinue
// -----------------------------------------------------------------
void TCSVDialog::HandleContinue( RTMessage msg )
{
    (*itsActionP) = kCSVOk;
    TDialog::Cancel( msg );
}

// -----------------------------------------------------------------
// Cancel
// -----------------------------------------------------------------
void TCSVDialog::Cancel( RTMessage msg )
{
    (*itsActionP) = kCSVErrorCancel;
    TDialog::Cancel( msg );
}

