//*****************************************************************************
// internationalized time format routines
// FIXME FormatTime uses am/pm for noon, midnight; should use intl seperators
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __profile__
#include "profile.h"
#endif
#ifndef __utility__
#include "utility.h"
#endif

#include "intlfmt.h"

static char * DayNames[7][3] =
    {
    { "S",    "Sun",  "Sunday",       },
    { "M",    "Mon",  "Monday",       },
    { "T",    "Tue",  "Tuesday",      },
    { "W",    "Wed",  "Wednesday",    },
    { "T",    "Thu",  "Thursday",     },
    { "F",    "Fri",  "Friday",       },
    { "S",    "Sat",  "Saturday",     }
    };

static char * MonthNames[13][3] =
    {
    { "",     "",     "",             },
    { "J",    "Jan",  "January",      },
    { "F",    "Feb",  "February",     },
    { "M",    "Mar",  "March",        },
    { "A",    "Apr",  "April",        },
    { "M",    "May",  "May",          },
    { "J",    "Jun",  "June",         },
    { "J",    "Jul",  "July",         },
    { "A",    "Aug",  "August",       },
    { "S",    "Sep",  "September",    },
    { "O",    "Oct",  "October",      },
    { "N",    "Nov",  "November",     },
    { "D",    "Dec",  "December",     }
    };

TTimeProfile       TIntlFmt::Time = {
																
    FALSE,      // Use24HourClock
    FALSE,      // TwoDigitHours
    "am",       // AMSuffix
    "pm",       // PMSuffix
    ":"         // Separator

    };

TShortDateProfile TIntlFmt::ShortDate = {

    FALSE,           // TwoDigitDays;
    FALSE,           // TwoDigitMonths;
    TRUE,            // TwoDigitYears;
    MDY,             // Order;
    "/"              // Separator[20];

    };

TLongDateProfile TIntlFmt::LongDate = {

    TRUE,            // LongDayName;
    kLongMonthName,  // MonthType;
    FALSE,           // TwoDigitDays;
    FALSE,           // TwoDigitYears;
    ",",             // Separator1[20];
    " ",             // Separator2[20];
    " ",             // Separator3[20];
    MDY              // Order;

    };

TCurrencyProfile TIntlFmt::Currency = {

    "$",            //symbol
    0,              //symbol formatting
    2,              //digits in fractional part
    0               //negative formatting

    };

TNumberProfile TIntlFmt::Number = {

    ",",            //thousands separator
    ".",            //decimal separator
    10,             //decimal digits
    1               //leading zero on small decimals (e.g., 0.34)

    };

char TIntlFmt::itsSavedDateSeparator = '/';
char TIntlFmt::itsSavedTimeSeparator = ':';
char TIntlFmt::itsSavedAMSuffix[20]  = "AM";
char TIntlFmt::itsSavedPMSuffix[20]  = "PM";


#define MAX_TOKEN_LENGTH 20
const int kNumTokenTypes = kError;
typedef struct {

    char       token[20];
    TTokenType type;

    } TLexEntry;


TLexEntry LexEntries[kNumTokenTypes] = {

    { "dddd",     kLongDayName },
    { "mmmm",     kLongMonthName },
    { "yyyy",     kLongYear },
    { "ddd",      kShortDayName },
    { "mmm",      kShortMonthName },
    { "dd",       kLongDay },
    { "mm",       kLongMonth },
    { "yy",       kShortYear },
    { "d",        kShortDay },
    { "m",        kShortMonth },
    { "'",        kSeparator }
    };

//*****************************************************************************

//*****************************************************************************
TIntlFmt::TIntlFmt( void )
{
    ReadTimeAndDate();

    //currency
    GetProfileString( "intl", "sCurrency", "$", Currency.sCurrency, 20 );
    Currency.iCurrency=   GetProfileInt( "intl", "iCurrency", 0 );
    Currency.iCurrDigits= GetProfileInt( "intl", "iCurrDigits", 2 );
    Currency.iNegCurr=    GetProfileInt( "intl", "iNegCurr", 0 );

    //numbers
    GetProfileString( "intl", "sThousand", ",", Number.sThousand, 20 );
    GetProfileString( "intl", "sDecimal", ".", Number.sDecimal, 20 );
    Number.iDigits= GetProfileInt( "intl", "iDigits", 2 );
    Number.iLzero=  GetProfileInt( "intl", "iLzero", 0 );

};

//*****************************************************************************

//*****************************************************************************
TIntlFmt::~TIntlFmt( void )
{
}

// ----------------------------------------------------------------
// ReadTimeAndDate
//
// Read the time and date format from win.ini
// ----------------------------------------------------------------
void TIntlFmt::ReadTimeAndDate( void )
{
    char s[50];

    // time
    Time.Use24HourClock = (BOOL) GetProfileInt( "intl", "iTime", FALSE );

    // FIXME: leading zero's leads to trouble in timeline
    // Time.TwoDigitHours  = (BOOL) GetProfileInt( "intl", "iTLZero", FALSE );
    Time.TwoDigitHours = FALSE;

    GetProfileString( "intl", "s1159", "am", Time.AMSuffix, 20 );
    GetProfileString( "intl", "s2359", "pm", Time.PMSuffix, 20 );
    GetProfileString( "intl", "sTime", ":",  Time.Separator, 20 );

    // short date
    GetProfileString( "intl", "sShortDate", "m/d/yy", s, 50 );
    ParseShortDate( s );

    // long date
    GetProfileString( "intl", "sLongDate", "dddd', 'mmmm' 'd', 'yyyy", s, 50 );
    ParseLongDate( s );

}

// ----------------------------------------------------------------
// ParseShortDate
//
// Parse the short date string from win.ini into the fields used
// by the program.
// ----------------------------------------------------------------
void TIntlFmt::ParseShortDate( char *s )
{
    int         position    = 0;
    char        token[ MAX_TOKEN_LENGTH ];
    char        separator[ MAX_TOKEN_LENGTH ];
    TTokenType  type, dateType, monthType, yearType;

    token[0]= '\0';
    separator[0]= '\0';

    switch( tolower( s[0] ) ) {

        case 'd':

            ShortDate.Order = DMY;
            Lex( s, position, dateType, token );           // date
            Lex( s, position, type, separator );           // separator
            Lex( s, position, monthType, token );          // month
            Lex( s, position, type, token );               // separator
            Lex( s, position, yearType, token );           // year
            break;

        case 'm':

            ShortDate.Order = MDY;
            Lex( s, position, monthType, token );          // month
            Lex( s, position, type, token );               // separator
            Lex( s, position, dateType, token );           // date
            Lex( s, position, type, separator );           // separator
            Lex( s, position, yearType, token );           // year
            break;

        case 'y':

            ShortDate.Order = YMD;
            Lex( s, position, yearType, token );           // year
            Lex( s, position, type, separator );           // separator
            Lex( s, position, monthType, token );          // month
            Lex( s, position, type, token );               // separator
            Lex( s, position, dateType, token );           // date
            break;

        default:

            // error: start again with default
            ParseShortDate( "m/d/yy" );
            return;
    }

    strcpy( ShortDate.Separator, separator );
    ShortDate.TwoDigitDays   = (dateType  == kLongDay);
    ShortDate.TwoDigitMonths = (monthType == kLongMonth);
    ShortDate.TwoDigitYears  = (yearType  == kShortYear);
}

// ----------------------------------------------------------------
// ParseLongDate
//
// Parse the long date string from win.ini into the fields used
// by the program.
// ----------------------------------------------------------------
void TIntlFmt::ParseLongDate( char *s )
{
    int         position    = 0;
    char        token[ MAX_TOKEN_LENGTH ],
                separator1[ MAX_TOKEN_LENGTH ],
                separator2[ MAX_TOKEN_LENGTH ],
                separator3[ MAX_TOKEN_LENGTH ];

    TTokenType  type, dayNameType, dateType, monthType, yearType;

    token[0]=      '\0';
    separator1[0]= '\0';
    separator2[0]= '\0';
    separator3[0]= '\0';

    // get the day name type and the first separator
    Lex( s, position, dayNameType, token );
    Lex( s, position, type, separator1 );

    switch( tolower( s[position] ) ) {

        case 'd':

            LongDate.Order = DMY;
            Lex( s, position, dateType, token );           // date
            Lex( s, position, type, separator2 );          // separator 2
            Lex( s, position, monthType, token );          // month
            Lex( s, position, type, separator3 );          // separator 3
            Lex( s, position, yearType, token );           // year
            break;

        case 'm':

            LongDate.Order = MDY;
            Lex( s, position, monthType, token );          // month
            Lex( s, position, type, separator2 );          // separator 2
            Lex( s, position, dateType, token );           // date
            Lex( s, position, type, separator3 );          // separator 3
            Lex( s, position, yearType, token );           // year
            break;

        case 'y':

            LongDate.Order = YMD;
            Lex( s, position, yearType, token );           // year
            Lex( s, position, type, separator2 );          // separator 2
            Lex( s, position, monthType, token );          // month
            Lex( s, position, type, separator3 );          // separator 3
            Lex( s, position, dateType, token );           // date
            break;

        default:

            // error: try default
            ParseLongDate( "dddd', 'mmmm' 'd', 'yyyy" );
            return;
    }

    strcpy( LongDate.Separator1, separator1 );
    strcpy( LongDate.Separator2, separator2 );
    strcpy( LongDate.Separator3, separator3 );
    LongDate.MonthType      = monthType;
    LongDate.LongDayName    = (dayNameType == kLongDayName);
    LongDate.TwoDigitDays   = (dateType    == kLongDay);
    LongDate.TwoDigitYears  = (yearType    == kShortYear);
}

// ----------------------------------------------------------------
// Lex -- parse time and date tokens
//
//  Read the next token from the string s, starting at position,
//  and return the token type and token (in the case of a kSeparator)
//  Returns kError as the type if string isn't in the token table
// ----------------------------------------------------------------
void TIntlFmt::Lex( char *s, int &position, TTokenType &type, char *token )
{
char *tempStr;
int  i, j, length;

for (i = 0; i < kNumTokenTypes; i++)
    {
    tempStr = LexEntries[i].token;
    length  = strlen( tempStr );
    int n= strlen( &s[position] );
    if ( n >= length && 0 == strnicmp( &s[position], tempStr, length ))  //text matches
        {
        if (LexEntries[i].type == kSeparator)
            {
            type = kSeparator;
            length = strlen( s );
            //read up to next ' mark
            for (j = position + 1; j < length && s[j] != '\''; j++)
                {
                token[j - position - 1] = s[j];
                }
            token[j - position - 1] = '\0';
            position = j + 1;
            }
        else
            {
            type = LexEntries[i].type;
            position += length;
            }
        return;
        } //token matches

    } //next token

//if we didn't match, and there's text left, the assumption is that
//we've got an undelimited separator string
if( s[position] != '\0' )
    {
    // separator that doesn't start with a "'": read
    // up to the next d, m or y
    type = kSeparator;
    int count = 0;
    do
        {
        token[count++] = s[position++];
        } while  (count < MAX_TOKEN_LENGTH &&
                 (s[position] != '\0') &&
                 (tolower(s[position]) != 'd') &&
                 (tolower(s[position]) != 'm') &&
                 (tolower(s[position]) != 'y'));

    token[count] = '\0';
    return;
    }

//couldn't figure it out
type = kError;
}


//*****************************************************************************
// FormatTime
//
//  Formats the time using the formatting options from the International
//  Control Panel.
//*****************************************************************************
void TIntlFmt::FormatTime( const RTTime time, BOOL crush, char *buf, BOOL useNoon )
{
    char hStr[20];

    // bail out?
    if (!time.HasTime()) {

        buf[0] = '\0';
        return;
    }

    int h = time.GetHour();
    int m = time.GetMinute();

    if (Time.Use24HourClock) {

        FormatTwoDigits( h, Time.TwoDigitHours, hStr );
        if (Time.PMSuffix[0]) {

            wsprintf( buf, "%s%s%2.2d %s", hStr, Time.Separator,
                m, Time.PMSuffix );
        }
        else {

            wsprintf( buf, "%s%s%2.2d", hStr, Time.Separator, m );
        }

    }
    else {

        if (h < 12) {

            if ((h == 0) && (m == 0)) {

                if (useNoon) {

                    strcpy( buf, crush ? "mdnt" : "midnight" );
                }
                else {

                    strcpy( buf, "12 am" );
                }
            }
            else {

                h = (h == 0) ? 12 : h;
                if ((crush) && (m == 0)) {

                    FormatTwoDigits( h, Time.TwoDigitHours, hStr );
                    wsprintf( buf, "%s %s", hStr, Time.AMSuffix );
                }
                else {

                    FormatTwoDigits( h, Time.TwoDigitHours, hStr );
                    wsprintf( buf, "%s%s%2.2d %s", hStr, Time.Separator,
                        m, Time.AMSuffix );
                }
            }
        }
        else {

            if ((h == 12) && (m == 0)) {

                if (useNoon) {

                    strcpy( buf, "noon" );
                }
                else {

                    strcpy( buf, "12 pm" );
                }
            }
            else {

                h = (h == 12) ? 12 : h - 12;
                if ((crush) && (m == 0)) {

                    FormatTwoDigits( h, Time.TwoDigitHours, hStr );
                    wsprintf( buf, "%s %s", hStr, Time.PMSuffix );
                }
                else {

                    FormatTwoDigits( h, Time.TwoDigitHours, hStr );
                    wsprintf( buf, "%s%s%2.2d %s", hStr, Time.Separator,
                        m, Time.PMSuffix );
                }
            }
        }
    }
}

//*****************************************************************************
// FormatCompactTime
//
//  Formats the time using the formatting options from the International
//  Control Panel, but keep it short.  Used in month calendar, perhaps elsewhere.
//*****************************************************************************
void TIntlFmt::FormatCompactTime( const RTTime time, RTSafStr buf )
{
char hStr[20];

// bail out?
if (!time.HasTime())
    {
    buf.Clear();
    return;
    }

int h = time.GetHour();
int m = time.GetMinute();

if (Time.Use24HourClock)
    {
    FormatTwoDigits( h, Time.TwoDigitHours, hStr );
    if (Time.PMSuffix[0])
        {
        buf.Printf( "%s%s%2.2d%c ", hStr, Time.Separator,
            m, Time.PMSuffix[0] );
        }
    else
        {
        buf.Printf( "%s%s%2.2d ", hStr, Time.Separator, m );
        }
    }
else
    {
    if (h < 12)
        {
        if ((h == 0) && (m == 0))
            {
            buf.Printf( "12%c ", Time.AMSuffix[0] );
            }
        else
            {
            h = (h == 0) ? 12 : h;
            if (m == 0)
                {
                FormatTwoDigits( h, Time.TwoDigitHours, hStr );
                buf.Printf( "%s%c ", hStr, Time.AMSuffix[0] );
                }
            else
                {
                FormatTwoDigits( h, Time.TwoDigitHours, hStr );
                buf.Printf( "%s%s%2.2d%c ", hStr, Time.Separator,
                    m, Time.AMSuffix[0] );
                }
            }
        }
    else
        {
        if ((h == 12) && (m == 0))
            {
            buf.Printf( "12%c ", Time.PMSuffix[0] );
            }
        else
            {
            h = (h == 12) ? 12 : h - 12;
            if (m == 0)
                {
                FormatTwoDigits( h, Time.TwoDigitHours, hStr );
                buf.Printf( "%s%c ", hStr, Time.PMSuffix[0] );
                }
            else
                {
                FormatTwoDigits( h, Time.TwoDigitHours, hStr );
                buf.Printf( "%s%s%2.2d%c ", hStr, Time.Separator,
                    m, Time.PMSuffix[0] );
                }
            }
        }
    }
}


//*****************************************************************************
// FormatDate
//
//  Formats the date using the formatting options from the International
//  Control Panel.
//*****************************************************************************
void TIntlFmt::FormatDate( const RTTime time, TDateFormat format, char *buf )
{
    char dayName[20], dStr[20], mStr[20], yStr[20];

    // bail out?
    if (!time.HasDate()) {

        buf[0] = '\0';
        return;
    }

    int d = time.GetDay();
    int m = time.GetMonth();
    int y = time.GetYear();

    if (format == kShortDate) {


        FormatTwoDigits( d, ShortDate.TwoDigitDays, dStr );
        FormatTwoDigits( m, ShortDate.TwoDigitMonths, mStr );
        FormatYear(      y, ShortDate.TwoDigitYears, yStr );

        switch (ShortDate.Order) {

            case DMY:

                wsprintf( buf, "%s%s%s%s%s",
                    dStr, ShortDate.Separator, mStr,
                    ShortDate.Separator, yStr );
                break;

            case MDY:

                wsprintf( buf, "%s%s%s%s%s",
                    mStr, ShortDate.Separator, dStr,
                    ShortDate.Separator, yStr );
                break;

            case YMD:

                wsprintf( buf, "%s%s%s%s%s",
                    yStr, ShortDate.Separator, mStr,
                    ShortDate.Separator, dStr );
                break;

            default:

                buf[0] = '\0';
                break;
        }
    }
    else if (format == kLongDate) {

        FormatDayName( time, (TDayNameLength) (LongDate.LongDayName + 1), dayName );
        FormatTwoDigits( d, LongDate.TwoDigitDays, dStr );
        FormatMonthType( time, LongDate.MonthType, mStr );
        FormatYear(      y, LongDate.TwoDigitYears, yStr );

        switch( LongDate.Order ) {

            case DMY:

                wsprintf( buf, "%s%s%s%s%s%s%s",
                    dayName, LongDate.Separator1,
                    dStr,    LongDate.Separator2,
                    mStr,    LongDate.Separator3,
                    yStr );
                break;

            case MDY:

                wsprintf( buf, "%s%s%s%s%s%s%s",
                    dayName, LongDate.Separator1,
                    mStr,    LongDate.Separator2,
                    dStr,    LongDate.Separator3,
                    yStr );
                break;

            case YMD:

                wsprintf( buf, "%s%s%s%s%s%s%s",
                    dayName, LongDate.Separator1,
                    yStr,    LongDate.Separator2,
                    mStr,    LongDate.Separator3,
                    dStr );
                break;

            default:

                buf[0] = '\0';
                break;
        }
    }
    else if (format == kLongDateNoDOW) {

        FormatTwoDigits( d, LongDate.TwoDigitDays, dStr );
        FormatMonthType( time, LongDate.MonthType, mStr );
        FormatYear(      y, LongDate.TwoDigitYears, yStr );

        switch( LongDate.Order ) {

            case DMY:

                wsprintf( buf, "%s%s%s%s%s",
                    dStr,    LongDate.Separator2,
                    mStr,    LongDate.Separator3,
                    yStr );
                break;

            case MDY:

                wsprintf( buf, "%s%s%s%s%s",
                    mStr,    LongDate.Separator2,
                    dStr,    LongDate.Separator3,
                    yStr );
                break;

            case YMD:

                wsprintf( buf, "%s%s%s%s%s",
                    yStr,    LongDate.Separator2,
                    mStr,    LongDate.Separator3,
                    dStr );
                break;

            default:

                buf[0] = '\0';
                break;
        }
    }
}


//*****************************************************************************
// FormatTwoDigits
//
//  Formats the number, prepending zeroes if called for by prependZero
//*****************************************************************************
void TIntlFmt::FormatTwoDigits( int value, BOOL prependZero, char *buf )
{
    if (prependZero) {

        wsprintf( buf, "%2.2d", value );
    }
    else {

        wsprintf( buf, "%d", value );
    }
}

//*****************************************************************************
// FormatYear
//
//  Formats the value, using two digits or four, eg. 92 or 1992
//*****************************************************************************
void TIntlFmt::FormatYear( int value, BOOL twoDigits, char *buf )
{
    if (twoDigits) {

        wsprintf( buf, "%2.2d", value % 100 );
    }
    else {

        wsprintf( buf, "%d", value );
    }
}


//*****************************************************************************
// FormatMonthType
//
//  Formats the time's month name, using the month type
//*****************************************************************************
void TIntlFmt::FormatMonthType( const RTTime time, TTokenType monthType, char *buf )
{
    switch( monthType ) {

        case kShortMonth:

            FormatTwoDigits( time.GetMonth(), FALSE, buf );
            break;

        case kLongMonth:

            FormatTwoDigits( time.GetMonth(), TRUE, buf );
            break;

        case kShortMonthName:

            FormatMonthName( time, kThreeLetter, buf );
            break;

        case kLongMonthName:

            FormatMonthName( time, kFullName, buf );
            break;

        default:

            buf[0] = '\0';
            break;
    }
}




//*****************************************************************************
// FormatDateAndTime
//
//  Uses conventions of international control panel
//*****************************************************************************
void TIntlFmt::FormatDateAndTime( const RTTime time, TDateFormat format, char *buf, BOOL useNoon  )
{
    int type;

    type  = time.HasDate() ? 1:0;
    type += time.HasTime()? 2:0;

    switch (type) {

        default:  // no date, no time

            buf[0] = '\0';
            return;

        case 1:  // just date

            FormatDate( time, format, buf );
            return;

        case 2:  // just time

            FormatTime( time, FALSE, buf, useNoon );
            return;

        case 3:  // both date and time

            char buf1[50];
            char buf2[50];

            FormatDate( time, format, buf1 );
            FormatTime( time, FALSE, buf2,useNoon );
            wsprintf( buf, "%s %s", buf1, buf2 );
            return;
    }
}

//*****************************************************************************
//  DayName
//
//  Return 1-letter, 3-letter or full name of the day specified by the day
//*****************************************************************************
void TIntlFmt::DayName( const int day, TDayNameLength length, char *buf )
{
   wsprintf( buf, DayNames[day][length] );
}

//*****************************************************************************
//  FormatDayName
//
//  Return 1-letter, 3-letter or full name of the day
//*****************************************************************************
void TIntlFmt::FormatDayName( const RTTime time, TDayNameLength length, char *buf )
{
   wsprintf( buf, DayNames[time.GetWeekDay()][length] );
}

//*****************************************************************************
//  FormatMonthName
//
//  Return 1-letter, 3-letter or full name of month
//*****************************************************************************
void TIntlFmt::FormatMonthName( const RTTime time, TDayNameLength length, char *buf )
{
    wsprintf( buf, MonthNames[time.GetMonth()][length] );
}


//*****************************************************************************
//  FormatDayAndMonth
//
//  Return the day and month using the international control panel
//  conventions for the ShortDate
//*****************************************************************************
void TIntlFmt::FormatDayAndMonth( const RTTime time, char *buf )
{
    char   dStr[20], mStr[20];

    if (!time.HasDate()) {

       buf[0] = '\0';
       return;
    }

    FormatTwoDigits( time.GetDay(), ShortDate.TwoDigitDays, dStr );
    FormatTwoDigits( time.GetMonth(), ShortDate.TwoDigitMonths, mStr );

    switch (ShortDate.Order) {

        case DMY:

            wsprintf( buf, "%s%s%s",
                dStr, ShortDate.Separator, mStr );
            break;

        case MDY:

            wsprintf( buf, "%s%s%s",
                mStr, ShortDate.Separator, dStr );
            break;

        case YMD:

            wsprintf( buf, "%s%s%s",
                mStr, ShortDate.Separator, dStr );
            break;

        default:

            buf[0] = '\0';
            break;
    }
}

//*****************************************************************************
//  FormatQuarter
//
//  FIXME: Irrelevant, I think, since we never use quarters
//*****************************************************************************
void TIntlFmt::FormatQuarter( const RTTime time, char *buf )
{
    if (!time.HasDate()) {

        buf[0] = '\0';
        return;
    }

    int m = time.GetMonth();
    if( m <= 3 ) {

        strcpy( buf, "Q1" );
    }
    else if( m <= 6 ){

        strcpy( buf, "Q2" );
    }
    else if( m <= 9 ) {

        strcpy( buf, "Q3" );
    }
    else {

        strcpy( buf, "Q4" );
    }
}

//*****************************************************************************
// FormatYear
//
// Pays no attention to International Control Panel conventions
//*****************************************************************************
void TIntlFmt::FormatYear( const RTTime time, char *buf )
{
    if (!time.HasDate()) {

        buf[0] = '\0';
        return;
    }

    wsprintf( buf, "%d", time.GetYear() );
}


//*****************************************************************************

//*****************************************************************************
void TIntlFmt::FmtDollar( RTValue value, TSafStr &str, BOOL bForceDecimal )
{
long whole, fraction;

value.Get( whole, fraction );

int i,n,skip;
char *p0,*p1;

char buf0[500];
char buf1[500];


//format fractional part
if( fraction != 0 || bForceDecimal )
    {
    long overflow[] = { 0L, 9L, 99L, 999L, 9999L, 99999L, 999999L, 9999999L,
        99999999L, 999999999L };
    // set the fraction to the proper number of digits
    n= Currency.iCurrDigits;
    for (int i = 4; i < n; i++)
        {
        fraction *= 10;
        }
    for (i = 4; i > n; i--)
        {
        fraction = (fraction + 5) / 10;
        }

    // if necessary, round 'whole' up by adding 1
    if (fraction > overflow[n])
        {
        fraction = 0;
        whole++;
        }
    }

//format whole part
wsprintf( buf0, "%ld", whole );

//add thousands separators

//# of leading chars to skip
n= strlen( buf0 );
skip= n % 3;
skip= (skip == 0)? 3: skip;

p0= buf0;
p1= buf1;

//skip leading chars
for( i= 0; i < skip && i < n; i++ )
    {
    *p1++= *p0++;
    }

//copy numbers
i= 0;
while( *p0 != '\0' )
    {
    if( (i % 3) == 0 )
        {
        strcpy( p1, Number.sThousand );
        p1 += strlen( Number.sThousand );
        }

    *p1++= *p0++;
    i++;
    }
*p1 = '\0';

// combine whole and fraction together (iff the number of digits > 0)
if (Currency.iCurrDigits > 0)
    {
    char f[20];
    wsprintf( f, "%%s%%s%%%d.%dld", Currency.iCurrDigits, Currency.iCurrDigits );
    wsprintf( buf0, f, buf1, Number.sDecimal, fraction );
    }
else
    {
    strcpy( buf0, buf1 );
    }


// combine number and currency symbol
char *sym= Currency.sCurrency;
if( whole >= 0 )
    {

    switch( Currency.iCurrency )
        {
        case 0:
        default:
            str.Printf(  "%s%s", sym, buf0 );
            break;

        case 1:
            str.Printf(  "%s%s", buf0, sym );
            break;

        case 2:
            str.Printf(  "%s %s", sym, buf0 );
            break;

        case 3:
            str.Printf(  "%s %s", buf0, sym );
            break;
        }
    }
else
    {
    switch( Currency.iNegCurr )
        {
        case 0:
        default:
            str.Printf(  "(%s%s)", sym, buf0 );
            break;

        case 1:
            str.Printf(  "-%s%s", sym, buf0 );
            break;

        case 2:
            str.Printf(  "%s-%s", sym, buf0 );
            break;

        case 3:
            str.Printf(  "%s%s-", sym, buf0 );
            break;

        case 4:
            str.Printf(  "(%s%s)", buf0, sym );
            break;

        case 5:
            str.Printf(  "-%s%s", buf0, sym );
            break;

        case 6:
            str.Printf(  "%s-%s", buf0, sym );
            break;

        case 7:
            str.Printf(  "%s%s-", buf0, sym );
            break;

        case 8:
            str.Printf(  "-%s %s", sym, buf0 );
            break;

        case 9:
            str.Printf(  "-%s %s", sym, buf0 );
            break;

        case 10:
            str.Printf(  "%s %s-", sym, buf0 );
            break;

        }
    }
}


// -------------------------------------------------------
// SetStandardFormats
//
//  When writing dates and times out as text (for
//  instance in import/export and holidays in the
//  profile) we write a canonical form out which
//  is independent of the international format normally
//  used. Since these canonical formats are always
//  parsed properly, we avoid the problem of not
//  understanding dates and times being written with
//  one intl format and read with another
// -------------------------------------------------------
void TIntlFmt::SetStandardFormats( void )
{
    itsSavedDateSeparator = ShortDate.Separator[0];
    ShortDate.Separator[0] = '/';

    itsSavedTimeSeparator = Time.Separator[0];
    Time.Separator[0] = ':';

    strcpy( itsSavedAMSuffix, Time.AMSuffix );
    strcpy( Time.AMSuffix,"AM" );

    strcpy( itsSavedPMSuffix, Time.PMSuffix );
    strcpy( Time.PMSuffix,"PM" );
}

// -------------------------------------------------------
// ResetFormats
//
//  reverses SetStandardFormats
// -------------------------------------------------------
void TIntlFmt::ResetFormats( void )
{
    ShortDate.Separator[0] = itsSavedDateSeparator;
    Time.Separator[0]      = itsSavedTimeSeparator;
    strcpy( Time.AMSuffix, itsSavedAMSuffix );
    strcpy( Time.PMSuffix, itsSavedPMSuffix );
}
