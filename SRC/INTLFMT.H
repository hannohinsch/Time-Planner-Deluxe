#ifndef __intlfmt__
#define __intlfmt__

_CLASSDEF( TTime )
_CLASSDEF( TIntlFmt )

#ifndef __profile__
#include "profile.h"
#endif
#ifndef __value__
#include "value.h"
#endif

typedef enum {

    kOneLetter   = 0,
    kThreeLetter = 1,
    kFullName    = 2

} TDayNameLength;

typedef enum {

    kShortDate       = 0,
    kLongDate        = 1,
    kLongDateNoDOW   = 2

} TDateFormat;

typedef struct {

    BOOL               Use24HourClock;
    BOOL               TwoDigitHours;
    char               AMSuffix[20];
    char               PMSuffix[20];
    char               Separator[20];

    } TTimeProfile;

typedef enum { DMY, YMD, MDY } TDateOrder;
typedef enum {

    kLongDayName    = 0,
    kLongMonthName  = 1,
    kLongYear       = 2,
    kShortDayName   = 3,
    kShortMonthName = 4,
    kLongDay        = 5,
    kLongMonth      = 6,
    kShortYear      = 7,
    kShortDay       = 8,
    kShortMonth     = 9,
    kSeparator      = 10,
    kError          = 11

    } TTokenType;

typedef struct {

    BOOL       TwoDigitDays;
    BOOL       TwoDigitMonths;
    BOOL       TwoDigitYears;
    TDateOrder Order;
    char       Separator[20];

    } TShortDateProfile;

typedef struct {

    BOOL       LongDayName;
    TTokenType MonthType;
    BOOL       TwoDigitDays;
    BOOL       TwoDigitYears;
    char       Separator1[20];
    char       Separator2[20];
    char       Separator3[20];
    TDateOrder Order;

    } TLongDateProfile;

typedef struct {

    char sCurrency[20];
    int iCurrency;
    int iCurrDigits;
    int iNegCurr;
    } TCurrencyProfile;

typedef struct {

    char sThousand[20];
    char sDecimal[20];
    int iDigits;
    int iLzero;
    } TNumberProfile;


class _CLASSTYPE TIntlFmt {

public:
    TIntlFmt( void );
    virtual ~TIntlFmt( void );

    static void DayName(           const int day, TDayNameLength length, char *buf );

    static void FormatTime(        const RTTime time, BOOL crush, char *buf, BOOL useNoon = TRUE );
    static void FormatCompactTime( const RTTime time, RTSafStr buf );
    static void FormatDate(        const RTTime time, TDateFormat format, char *buf );
    static void FormatDateAndTime( const RTTime time, TDateFormat format, char *buf, BOOL useNoon = TRUE );

    static void FormatDayName(     const RTTime time, TDayNameLength length, char *buf );
    static void FormatMonthName(   const RTTime time, TDayNameLength length, char *buf );
    static void FormatDayAndMonth( const RTTime time, char *buf );
    static void FormatQuarter(     const RTTime time, char *buf );
    static void FormatYear(        const RTTime time, char *buf );

    static void FmtDollar( RTValue value, TSafStr &str, BOOL bForceDecimal );

    static void ReadTimeAndDate( void );

    static TDateOrder GetDateOrder( void )    { return ShortDate.Order; }
    static char GetDateSeparator( void )      { return ShortDate.Separator[0]; }
    static char GetTimeSeparator( void )      { return Time.Separator[0]; }
    static char GetThousandsSeparator( void ) { return Number.sThousand[0]; }
    static char GetDecimalSeparator( void )   { return Number.sDecimal[0]; }
    static BOOL Is24HourClock( void )         { return Time.Use24HourClock; };

    static void SetStandardFormats( void );
    static void ResetFormats( void );

private:

    static void FormatTwoDigits( int value, BOOL prependZero, char *buf );
    static void FormatYear( int value, BOOL twoDigits, char *buf );
    static void FormatMonthType( const RTTime time, TTokenType monthType, char *buf );

    static void ParseShortDate( char *s );
    static void ParseLongDate( char *s );
    static void Lex( char *s, int &position, TTokenType &type, char *token );


    // formatting
    static TTimeProfile       Time;
    static TShortDateProfile  ShortDate;
    static TLongDateProfile   LongDate;
    static TCurrencyProfile   Currency;
    static TNumberProfile     Number;

    static char               itsSavedDateSeparator;
    static char               itsSavedTimeSeparator;
    static char               itsSavedAMSuffix[20];
    static char               itsSavedPMSuffix[20];


};

#endif
