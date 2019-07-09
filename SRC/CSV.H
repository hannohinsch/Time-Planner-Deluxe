#ifndef __csv__
#define __csv__

_CLASSDEF( TCSV )

#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __value__
#include "value.h"
#endif
#ifndef __duration__
#include "duration.h"
#endif


typedef enum { kCSVOk, kCSVErrorSkip, kCSVErrorCancel, kCSVOverflow } TCSVError;


class _CLASSTYPE TCSV {

public:

    typedef enum {

        kStart,
        kState1,
        kState2,
        kState3,
        kSuccessWithComma,
        kSuccessWithEOL,
        kEOFError,
        kUnknownStateError,
        kBadCharacterError,
        kUnexpectedQuoteError,
        kReadIntError,
        kReadEnumError,
        kReadTimeError,
        kReadDateError,
        kReadValueError,
        kReadDurationError

        } TState;

    // -------------------------------------------------------------------
    TCSV( void );
    virtual ~TCSV( void );

    void       WriteString( ofstream &os, char *s, BOOL eol );
    void       WriteString( ofstream &os, RTSafStr s, BOOL eol );
    void       WriteInt( ofstream &os, int x, BOOL eol );

    TCSVError  ReadString( ifstream &is, RTSafStr token, BOOL &eol );
    TCSVError  ReadInt( ifstream &is, int &x, BOOL &eol );
    TCSVError  ReadEnum( ifstream &is, int low, int high, int &x, BOOL &eol );
    TCSVError  ReadTime( ifstream &is, RTTime time, BOOL &eol );
    TCSVError  ReadDate( ifstream &is, RTTime time, BOOL &eol );
    TCSVError  ReadValue( ifstream &is, RTValue value, BOOL &eol );
    TCSVError  ReadDuration( ifstream &is, RTDuration duration, BOOL &eol );

    TCSVError  ShouldBeEOL( ifstream &is, BOOL eol );
    TCSVError  ShouldntBeEOL( ifstream &is, BOOL eol );

private:

    TCSVError  ReadError( TState error, ifstream &is );
    istream &  Get( ifstream &is, char &c );

    TState     StartState( ifstream &is, RTSafStr token );
    TState     State1( ifstream &is, RTSafStr token );
    TState     State2( ifstream &is, RTSafStr token );
    TState     State3( ifstream &is, RTSafStr token );

    int        itsLine;
    int        itsCharacter;

};




#endif
