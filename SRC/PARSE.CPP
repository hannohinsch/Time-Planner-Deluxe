//*****************************************************************************
// find, in a text string, the two date/times and the one value
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "parse.h"

#ifndef __classid__
#include "classid.h"
#endif
#ifndef __value__
#include "value.h"
#endif
#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __intlfmt__
#include "intlfmt.h"
#endif

#include "error.h"
void TestValue( char *text, long w, long f );
void TestDate( char *text, int year, int month, int day );
void TestTime( char *text, int hour, int min );


TParseTokenType TParse::itsDatePatterns[] [kMaxTokenString]=
    {
    { NUM, DATE_SEPARATOR, NUM, DATE_SEPARATOR, NUM,
                                      ACTION, DATE1 },     //  12?31?91
    { NUM, SLASH, NUM, SLASH, NUM,    ACTION, DATE1 },     //  12/31/91
    { NUM, DASH, NUM, DASH, NUM,      ACTION, DATE1 },     //  12-31-91
    { NUM, DOT, NUM, DOT, NUM,        ACTION, DATE1 },     //  12.31.91
    { NUM, DATE_SEPARATOR, NUM,       ACTION, DATE2 },     //  12?31
    { NUM, SLASH, NUM,                ACTION, DATE2 },     //  12/31
    { NUM, DASH, NUM,                 ACTION, DATE2 },     //  12-31
    { NUM, DOT, NUM,                  ACTION, DATE2 },     //  12.31
    { NUM, MONTH, NUM,                ACTION, DATE3 },     //  12 June 91
    { NUM, MONTH,                     ACTION, DATE4 },     //  12 June
    { MONTH, NUM, COMMA, NUM,         ACTION, DATE5 },     //  June 12, 1991
    { MONTH, NUM,                     ACTION, DATE6 },     //  June 12
    { DOW,                            ACTION, DATE7 },     //  Monday

    { PATEND }
    };

TParseTokenType TParse::itsTimePatterns[] [kMaxTokenString]=
    {
    { NUM, TIME_SEPARATOR, NUM, AMPM, ACTION, TIME1 },     //  12?30 pm
    { NUM, COLON, NUM, AMPM,          ACTION, TIME1 },     //  12:30 pm
    { NUM, TIME_SEPARATOR, NUM,       ACTION, TIME2 },     //  12?30
    { NUM, COLON, NUM,                ACTION, TIME2 },     //  12:30
    { NUM, AMPM,                      ACTION, TIME3 },     //  12 am
    { NUM,                            ACTION, TIME4 },     //  12
    { NOON,                           ACTION, TIME5 },     //  12
    { MIDNIGHT,                       ACTION, TIME6 },     //  12

    { PATEND }
    };

TParseTokenType TParse::itsValuePatterns[] [kMaxTokenString]=
    {

    { NUM, THOUSANDS_SEPARATOR,
      NUM, THOUSANDS_SEPARATOR,
      NUM, DECIMAL_SEPARATOR,
      NUM,                            ACTION, VALUE1 },    //  123,456,789.789
    { NUM, THOUSANDS_SEPARATOR,
      NUM, THOUSANDS_SEPARATOR,
      NUM, DECIMAL_SEPARATOR,         ACTION, VALUE2 },    //  123,456,789.
    { NUM, THOUSANDS_SEPARATOR,
      NUM, THOUSANDS_SEPARATOR,
      NUM,                            ACTION, VALUE3 },    //  123,456,789
    { NUM, THOUSANDS_SEPARATOR,
      NUM, DECIMAL_SEPARATOR,
      NUM,                            ACTION, VALUE4 },    //  456,789.789
    { NUM, THOUSANDS_SEPARATOR,
      NUM, DECIMAL_SEPARATOR,         ACTION, VALUE5 },    //  456,789.
    { NUM, THOUSANDS_SEPARATOR, NUM,  ACTION, VALUE6 },    //  456,789
    { NUM, DECIMAL_SEPARATOR, NUM,    ACTION, VALUE7 },    //  789.789
    { NUM, DECIMAL_SEPARATOR,         ACTION, VALUE8 },    //  789.
    { DECIMAL_SEPARATOR, NUM,         ACTION, VALUE9 },    //  .789
    { NUM,                            ACTION, VALUE10 },    //  789
    { PATEND }
    };


char *days[]=
    {
    "sunday",
    "monday",
    "tuesday",
    "wednesday",
    "thursday",
    "friday",
    "saturday",
    ""
    };

char *months[]=
    {
    "january",
    "february",
    "march",
    "april",
    "may",
    "june",
    "july",
    "august",
    "september",
    "october",
    "november",
    "december",
    ""
    };

char *ampm[]=
    {
    "am",
    "pm",
    ""
    };

char *noons[]=
    {
    "noon",
    ""
    };

char *midnights[]=
    {
    "midnight",
    "mdnt",
    ""
    };

TToken TParse::itsTokenTab[ kMaxToken ];

WORD TParse::itsTokenIndex= 0;     //last token in table
TParseContext TParse::itsParseContext= kCheckTime;

#define GETVAL( INDEX ) itsTokenTab[ startToken + INDEX ].Num

//*****************************************************************************
//
//*****************************************************************************
void TParse::AppInit( void )
{
    CHKBUG( ERR_GEN, ParseTest() );
}

//*****************************************************************************
//
//*****************************************************************************
void TParse::AppCleanup( void )
{
}

//*****************************************************************************
//
//*****************************************************************************
void TParse::ChkDate( char *pText, TTime &date )
{
itsParseContext = kCheckDate;

date.Clear();
BuildTokenTable( pText );
AnalyseDate( date );
}

//*****************************************************************************
//
//*****************************************************************************
void TParse::ChkTime( char *pText, TTime &time )
{
itsParseContext = kCheckTime;

time.Clear();
BuildTokenTable( pText );
AnalyseTime( time );
}

//*****************************************************************************
//
//*****************************************************************************
void TParse::ChkValue( char *pText, TValue &value )
{
itsParseContext = kCheckValue;

value.Clear();
BuildTokenTable( pText );
AnalyseValue( value );
}



//*****************************************************************************

//*****************************************************************************
long TParse::MkNum( char *p )
{
long num=0;

while( *p )
    {
    if( isdigit( *p ) )
        num= (10L * num) + (long) (*p - '0' );
    p++;
    }

return num;
}

//*****************************************************************************

//*****************************************************************************
int TParse::MakeYear( int raw )
{
if( raw > 100 )
    return raw;
else
    if( raw < 30 )
        return 2000 + raw;
    else
        return 1900 + raw;
}



//----------------------------------------------------------
// AnalyseDate
//
//  Parse a date
//----------------------------------------------------------
void TParse::AnalyseDate( RTTime time )
{
TParseTokenType action;
int skipCount;

for( int startToken= 0; startToken < itsTokenIndex; startToken += skipCount )
    {
    int numPats = sizeof( itsDatePatterns ) / sizeof( TTokenPatternTableEntry );
    for( int i= 0; i < numPats; i++ )
        {
        ComparePattern( startToken, i, &action, &skipCount, itsDatePatterns );
        if (action != NOACTION) break;
        }

    switch( action )
        {
        case DATE1:    // 12/31/93

             MakeDate( GETVAL( 0 ), GETVAL( 2 ), GETVAL( 4 ), time );
             break;

        case DATE2:    // 12-31

             MakeDate( GETVAL( 0 ), GETVAL( 2 ), time );
             break;

        case DATE3:   //  12 June 91

             MakeDateWithMonth( 1 + GETVAL( 1 ), GETVAL( 0 ), GETVAL( 2 ), time );
             break;

        case DATE4:   // 12 June

             MakeDateWithMonth( 1 + GETVAL( 1 ), GETVAL( 0 ), time );
             break;

        case DATE5:   //  June 12, 1991

             MakeDateWithMonth( 1 + GETVAL( 0 ), GETVAL( 1 ), GETVAL( 3 ), time );
             break;

        case DATE6:   //  June 12

             MakeDateWithMonth( 1 + GETVAL( 0 ), GETVAL( 1 ), time );
             break;

        case DATE7:   //  Monday

             MakeDateWithDOW( GETVAL( 0 ), time );
             break;

        case NOACTION:  //pattern didn't match

             break;

        case PATEND:

             skipCount = 1;
             break;

        default:

             break;

        } //switch
    } //for startToken
}

//----------------------------------------------------------
// AnalyseTime
//
//  Parse a time
//----------------------------------------------------------
void TParse::AnalyseTime( RTTime time )
{
TParseTokenType action;
int skipCount;

for( int startToken= 0; startToken < itsTokenIndex; startToken += skipCount )
    {
    int numPats = sizeof( itsTimePatterns ) / sizeof( TTokenPatternTableEntry );
    for( int i= 0; i < numPats; i++ )
        {
        ComparePattern( startToken, i, &action, &skipCount, itsTimePatterns );
        if (action != NOACTION) break;
        }

    switch( action )
        {
        case TIME1:   //  12:30 pm

             MakeTime( GETVAL( 0 ), GETVAL( 2 ), GETVAL( 3 ), time );
             break;

        case TIME2:   //  12:30

             MakeTime( GETVAL( 0 ), GETVAL( 2 ), time );
             break;

        case TIME3:   //  12 am

             MakeTime( GETVAL( 0 ), 0, GETVAL( 1 ), time );
             break;

        case TIME4:   //  12

             MakeTime( GETVAL( 0 ), 0, time );
             break;

        case TIME5:   //  noon

             time.itsTimeValid = TRUE;
             time.itsHour      = 12;
             time.itsMinute    = 0;
             time.Adjust();
             break;

        case TIME6:   //  midnight

             time.itsTimeValid = TRUE;
             time.itsHour      = 0;
             time.itsMinute    = 0;
             time.Adjust();
             break;


        case NOACTION:  //pattern didn't match

             break;

        case PATEND:

             skipCount = 1;
             break;

        default:

             break;

        } //switch
    } //for startToken
}

//----------------------------------------------------------
// AnalyseValue
//
//  Parse a value
//----------------------------------------------------------
void TParse::AnalyseValue( RTValue value )
{
TParseTokenType action;
int        skipCount;
TValue     tmpValue;
long       w, f;

for( int startToken= 0; startToken < itsTokenIndex; startToken += skipCount )
    {
    int numPats = sizeof( itsValuePatterns ) / sizeof( TTokenPatternTableEntry );
    for( int i= 0; i < numPats; i++ )
        {
        ComparePattern( startToken, i, &action, &skipCount, itsValuePatterns );
        if (action != NOACTION) break;
        }

    switch( action )
        {
        case VALUE1:  //  123,456,789.789

            w = GETVAL( 0 ) * 1000000L + GETVAL( 2 ) * 1000L + GETVAL( 4 );
            f = MakeFraction( itsTokenTab[ startToken + 6 ]);
            value.Set( w, f );
            break;

        case VALUE2:  //  123,456,789.
        case VALUE3:  //  123,456,789

            w = GETVAL( 0 ) * 1000000L + GETVAL( 2 ) * 1000L + GETVAL( 4 );
            value.Set( w, 0 );
            break;

        case VALUE4:  //  456,789.789

            w = GETVAL( 0 ) * 1000L + GETVAL( 2 );
            f = MakeFraction( itsTokenTab[ startToken + 4 ]);
            value.Set( w, f );
            break;

        case VALUE5:  //  456,789.
        case VALUE6:  //  456,789

            w = GETVAL( 0 ) * 1000L + GETVAL( 2 );
            value.Set( w, 0 );
            break;

        case VALUE7:  //  789.789

            w = GETVAL( 0 );
            f = MakeFraction( itsTokenTab[ startToken + 2 ]);
            value.Set( w, f );
            break;

        case VALUE8:  //  789.
        case VALUE10: //  789

            w = GETVAL( 0 );
            value.Set( w, 0 );
            break;

        case VALUE9:  //  .789

            f = MakeFraction( itsTokenTab[ startToken + 1 ]);
            value.Set( 0, f );
            break;

        case NOACTION:  //pattern didn't match

             break;

        case PATEND:

             skipCount = 1;
             break;

        default:

             break;

        } //switch
    } //for startToken
}

//*****************************************************************************
// MakeFraction
//
//  The fractional part of a TValue is supposed to have
//  four digits. This method makes sure it does
//*****************************************************************************
long TParse::MakeFraction( RTToken token )
{
long fraction = token.Num;

if (fraction > 10000)
    for (; fraction > 10000; fraction /= 10);
else if (fraction != 0)
    {
    // multiply the value by 10 for each trailing zero
    // that should be there in order to make the decimal
    // four digits
    for (int i = strlen( token.Text ); i < 4; i++)
        fraction *= 10;
    }

return fraction;
}

//*****************************************************************************
// return appropriate action token if strings match, else NOACTION token
//*****************************************************************************
void TParse::ComparePattern( int startToken,
    int patIndex, TParseTokenType *pType, int *cnt,
    TTokenPatternTable tokPatterns )
{
int i;
TParseTokenType patType;
TToken thisToken;

for( i=0; ; i++ )
    {
    patType= tokPatterns[ patIndex ][i];

    thisToken= itsTokenTab[ startToken + i];

    if( patType == PATEND )
        {
        *pType= PATEND;
        *cnt= 0;
        return;          //end of table
        }

    if( patType == ACTION )
        {
        *pType= ( tokPatterns[ patIndex ] [i+1] ); //action token follows ACTION
        *cnt= i;   //skip these tokens
        return;
        }

    if( patType != thisToken.Type )
        {
        *pType= NOACTION;
        *cnt= 0;
        return;
        }
    }
}

//*****************************************************************************
// build a string of tokens on itsTokenTab
//*****************************************************************************
void TParse::BuildTokenTable( char *pText )
{
TToken token, lastToken;

itsTokenIndex= 0;

lastToken.Type= JUNK;

GetToken( pText, token );    //initialize text string

while( token.Type != TEXTEND )
    {
    if( token.Type == SPACE )
        ;  //ignore
    else if( token.Type == JUNK && lastToken.Type == JUNK )
        ;  //skip forward
    else
        {
        itsTokenTab[ itsTokenIndex++ ]= token;
		if( itsTokenIndex > kMaxToken ) SYSERR( ERR_GEN );
        }

    lastToken= token;
    GetToken( NULL, token );
    }

itsTokenTab[ itsTokenIndex ]= token;      //append TEXTEND
}

//*****************************************************************************
// if all chars in p match some string in tab, return index of tab
// at least 3 chars must match for success
// return -1 for failure
//*****************************************************************************
int TParse::IsMatch( int minCnt, char *startP, char *endP, char *tab[] )
{
int i, length;
char  p[255];

length = (int) (endP - startP);
strncpy( p, startP, length );
p[length] = '\0';

for( i=0; *tab[i]; i++ )
    {
    if( minCnt <= Compare( tab[i], p ) )
        { //we found word
        return i;
        }
    }

return -1;
}

//*****************************************************************************
// do case insensitive compare; return # of chars that matched
//*****************************************************************************
int TParse::Compare( char *p, char *s )
{
int i;

for( i= 0; p[i] && s[i]; i++ )
    {
    if( !( p[i] == s[i] || (toupper( p[i] ) == s[i] ) ) )
        return 0; //some char didn't match
    }

return i;
}

// ------------------------------------------------------------
// FillToken
//
//  Fill the fields of the token
// ------------------------------------------------------------
void TParse::FillToken( RTToken token, TParseTokenType type,
    WORD num, char *pStart, char *pEnd)
{
    char    text[255];
    int     length;

    length = (int) (pEnd - pStart);
    strncpy( text, pStart, length );
    text[length] = '\0';

    // see if a 1-character token is really a separator
    if (length == 1)
        {
        switch( itsParseContext )
            {
            case kCheckDate:

                if (text[0] == TIntlFmt::GetDateSeparator())
                    {
                    type = DATE_SEPARATOR;
                    }
                break;

            case kCheckTime:

                if (text[0] == TIntlFmt::GetTimeSeparator())
                    {
                    type = TIME_SEPARATOR;
                    }
                break;

            case kCheckValue:

                if (text[0] == TIntlFmt::GetThousandsSeparator())
                    {
                    type = THOUSANDS_SEPARATOR;
                    }
                else if (text[0] == TIntlFmt::GetDecimalSeparator())
                    {
                    type = DECIMAL_SEPARATOR;
                    }
                break;
            }
        }

    token.Type   = type;
    token.Num    = num;
    strcpy( token.Text, text );
}

// ------------------------------------------------------------
// GetToken
//
//  Retrieve a single token from the string pointed
//  at by sourceText. Subsequent calls to GetToken
//  after the first should pass a NULL for SourceText,
//  although the text passed in initially will be used
// ------------------------------------------------------------
void TParse::GetToken( char *sourceText, RTToken token )
{
static char     *pStart;
char            *pCurrentChar;
char            c;

// if not initial call, patch up string
if( sourceText )
    {
    pStart = sourceText;
    }

// we'll march through the characters by incrementing
// pCurrentChar, so that pStart maintains its position
// at the start of the token
pCurrentChar= pStart;

c= *pCurrentChar;
switch( c )
    {
    case '\0':

        FillToken( token, TEXTEND, 0, pStart, pCurrentChar );
        break;

    case '$':

        pCurrentChar++;
        FillToken( token, DOLLAR, 0, pStart, pCurrentChar );
        break;

    case ':':

        pCurrentChar++;
        FillToken( token, COLON, 0, pStart, pCurrentChar );
        break;

    case '\\':
    case '/':

        pCurrentChar++;
        FillToken( token, SLASH, 0, pStart, pCurrentChar );
        break;

    case '-':

        pCurrentChar++;
        FillToken( token, DASH, 0, pStart, pCurrentChar );
        break;

    case '.':

        pCurrentChar++;
        FillToken( token, DOT, 0, pStart, pCurrentChar );
        break;

    case ',':

        pCurrentChar++;
        FillToken( token, COMMA, 0, pStart, pCurrentChar );
        break;

    default:

        if (isdigit( c ))
            {
            while( isdigit( *pCurrentChar ) ) pCurrentChar++;
            char tempC = *pCurrentChar;
            *pCurrentChar = '\0';
            FillToken( token, NUM, (int) MkNum( pStart ), pStart, pCurrentChar );
            *pCurrentChar = tempC;
            }
        else if (isalpha( c ))
            {
            while( isalpha( *pCurrentChar ) ) pCurrentChar++;

            int i;
            if( -1 != (i= IsMatch( 3, pStart, pCurrentChar, days )))
                FillToken( token, DOW, i, pStart, pCurrentChar );
            else if( -1 != (i= IsMatch( 3, pStart, pCurrentChar, months )))
                FillToken( token, MONTH, i, pStart, pCurrentChar );
            else if( -1 != (i= IsMatch( 2, pStart, pCurrentChar, ampm )))
                FillToken( token, AMPM, i, pStart, pCurrentChar );
            else if( -1 != (i= IsMatch( 2, pStart, pCurrentChar, noons )))
                FillToken( token, NOON, i, pStart, pCurrentChar );
            else if( -1 != (i= IsMatch( 2, pStart, pCurrentChar, midnights )))
                FillToken( token, MIDNIGHT, i, pStart, pCurrentChar );
            }
        else if( isspace( c ) )
            {
            while( isspace( *pCurrentChar )) pCurrentChar++;
            FillToken( token, SPACE, 0, pStart, pCurrentChar );
            }
        else
            {
            pCurrentChar++;
            FillToken( token, JUNK, 0, pStart, pCurrentChar );
            }
    }

pStart = pCurrentChar;
}


// 	----------------------------------------------------------------------------
// MakeDate
// 	----------------------------------------------------------------------------
void TParse::MakeDate( int first, int second, int third, RTTime time )
{
    switch( TIntlFmt::GetDateOrder() )
        {
        case DMY:

            time.itsDay       = first;
            time.itsMonth     = second;
            time.itsYear      = MakeYear( third );
            break;

        case YMD:

            time.itsYear      = MakeYear( first );
            time.itsMonth     = second;
            time.itsDay       = third;
            break;

        case MDY:

            time.itsMonth     = first;
            time.itsDay       = second;
            time.itsYear      = MakeYear( third );
            break;
        }

    if ((time.itsDay   < 1) || (time.itsDay   > 31) ||
        (time.itsMonth < 1) || (time.itsMonth > 12))
        {
        time.Clear();
        }
    else
        {
        time.itsDateValid = TRUE;
        time.Adjust();
        }
}

// 	----------------------------------------------------------------------------
// MakeDate
// 	----------------------------------------------------------------------------
void TParse::MakeDate( int first, int second, RTTime time )
{
    switch( TIntlFmt::GetDateOrder() )
        {
        case DMY:

            time.itsDay       = first;
            time.itsMonth     = second;
            time.itsYear      = GuessYear( first, second );
            break;

        case YMD:

            time.itsYear      = GuessYear( second, first );
            time.itsMonth     = first;
            time.itsDay       = second;
            break;

        case MDY:

            time.itsMonth     = first;
            time.itsDay       = second;
            time.itsYear      = GuessYear( second, first );
            break;
        }


    if ((time.itsDay   < 1) || (time.itsDay   > 31) ||
        (time.itsMonth < 1) || (time.itsMonth > 12))
        {
        time.Clear();
        }
    else
        {
        time.itsDateValid = TRUE;
        time.Adjust();
        }
}

// 	----------------------------------------------------------------------------
// MakeDateWithMonth
// 	----------------------------------------------------------------------------
void TParse::MakeDateWithMonth( int month, int first, int second, RTTime time )
{
    switch( TIntlFmt::GetDateOrder() )
        {
        case DMY:

            time.itsDay       = first;
            time.itsMonth     = month;
            time.itsYear      = MakeYear( second );
            break;

        case YMD:

            time.itsYear      = MakeYear( first );
            time.itsMonth     = month;
            time.itsDay       = second;
            break;

        case MDY:

            time.itsMonth     = month;
            time.itsDay       = first;
            time.itsYear      = MakeYear( second );
            break;

        }

    if ((time.itsDay   < 1) || (time.itsDay   > 31) ||
        (time.itsMonth < 1) || (time.itsMonth > 12))
        {
        time.Clear();
        }
    else
        {
        time.itsDateValid = TRUE;
        time.Adjust();
        }
}

// 	----------------------------------------------------------------------------
// MakeDateWithMonth
//
//  There is no ambiguity about the second parameter:
//  it must be the day.
// 	----------------------------------------------------------------------------
void TParse::MakeDateWithMonth( int month, int day, RTTime time )
{
    time.itsDay       = day;
    time.itsMonth     = month;
    time.itsYear      = GuessYear( day, month );

    if ((time.itsDay   < 1) || (time.itsDay   > 31) ||
        (time.itsMonth < 1) || (time.itsMonth > 12))
        {
        time.Clear();
        }
    else
        {
        time.itsDateValid = TRUE;
        time.Adjust();
        }
}

// -----------------------------------------------------------------------
// MakeDateWithDOW
//
// day of week supplied, find next appropriate date
// -----------------------------------------------------------------------
void TParse::MakeDateWithDOW( int wday, RTTime time )
{
    time.SetNow();
    time.itsDateValid = TRUE;
    time.itsTimeValid = FALSE;       // was set by SetNow()

    int today = time.GetWeekDay();

    if( wday > today ) {

        time.itsDay += wday - today;
    }
    else {

        time.itsDay += 7 + wday - today;
    }

    time.Adjust();
}

// -----------------------------------------------------------------------
// GuessYear
//
// we didn't find a year, so guess it from month and mday
// last 6 mos are history, next 6 mos are future
// find the year s.t. date is within 6 mos of now
// -----------------------------------------------------------------------
int TParse::GuessYear( int day, int month )
{
    // initialize the easy parts
    TTime time;
    time.itsDateValid = TRUE;
    time.itsDay       = day;
    time.itsMonth     = month;

    TTime now;
    now.SetNow();

    // see if Day/Month/NextYear is within 6
    // months of today
    time.itsYear = now.itsYear + 1;
    time.Adjust();
    TTime sixMonthsFromNow = now.Plus( SCALE_MONTH, 6 );
    if ( time < sixMonthsFromNow ) {

        return time.itsYear;
    }

    // see if Day/Month/LastYear is within 6
    // months of today
    time.itsYear = now.itsYear - 1;
    time.Adjust();
    if( time.Plus( SCALE_MONTH, 6) > now ) {

        return time.itsYear;
    }

    // use ThisYear
    return now.itsYear;
}


// 	----------------------------------------------------------------------------
// MakeTime
// 	----------------------------------------------------------------------------
void TParse::MakeTime( int hour, int min, BOOL isPm, RTTime time )
{
    time.itsTimeValid = TRUE;
    time.itsHour      = hour;
    time.itsMinute    = min;

    if (isPm && hour < 12) {

        time.itsHour += 12;
    }
    else if(!isPm && hour == 12 ) {  //special 12:30 am processing

        time.itsHour= 0;
    }

    if ((time.itsHour     < 0) || (time.itsHour    > 23) ||
        (time.itsMinute   < 0) || (time.itsMinute  > 59))
        {
        time.Clear();
        }
    else
        {
        time.itsTimeValid = TRUE;
        time.Adjust();
        }
}


// -----------------------------------------------------------------------
// MakeTime
//
// given hour, guess am/pm
// -----------------------------------------------------------------------
void TParse::MakeTime( int hour, int minute, RTTime time )
{

    time.itsTimeValid = TRUE;
    time.itsHour      = hour;
    time.itsMinute    = minute;

    if( !TIntlFmt::Is24HourClock() && hour < 8 ) {

        time.itsHour += 12;
    }


    if ((time.itsHour   < 0) || (time.itsHour   > 23) ||
        (time.itsMinute < 0) || (time.itsMinute > 59))
        {
        time.Clear();
        }
    else
        {
        time.itsTimeValid = TRUE;
        time.Adjust();
        }
}



BOOL TParse::ParseTest( void )
{
    #if 0
    TestValue( "123,456,789.001", 123456789, 10 );
    TestValue( "456,789.001",     456789,    10 );
    TestValue( "789.001",         789,       10 );
    TestValue( "0.001",           0,         10 );
    TestValue( ".001",            0,         10 );

    TestValue( "123,456,789.80",  123456789, 8000 );
    TestValue( "456,789.80",      456789,    8000 );
    TestValue( "789.80",          789,       8000 );
    TestValue( "0.80",            0,         8000 );
    TestValue( ".80",             0,         8000 );

    TestValue( "123,456,789.123", 123456789, 1230 );
    TestValue( "456,789.123",     456789,    1230 );
    TestValue( "789.123",         789,       1230 );
    TestValue( "0.123",           0,         1230 );
    TestValue( ".123",            0,         1230 );

    TestValue( "123,456,789.12345", 123456789, 1234 );
    TestValue( "456,789.12345",     456789,    1234 );
    TestValue( "789.12345",         789,       1234 );
    TestValue( "0.12345",           0,         1234 );
    TestValue( ".12345",            0,         1234 );

    TestValue( "123,456,789.",  123456789, 0 );
    TestValue( "456,789.",      456789,    0 );
    TestValue( "789.",          789,       0 );
    TestValue( "0.",            0,         0 );

    TestValue( "123,456,789",  123456789, 0 );
    TestValue( "456,789",      456789,    0 );
    TestValue( "789",          789,       0 );
    TestValue( "0",            0,         0 );

    TestDate( "1/2/93", 1993, 1, 2 );
    TestDate( "1-2-93", 1993, 1, 2 );
    TestDate( "1.2.93", 1993, 1, 2 );
    TestDate( "1?2?93", 1993, 1, 2 );

    TestDate( "1/2",    1994, 1, 2 );
    TestDate( "1-2",    1994, 1, 2 );
    TestDate( "1.2",    1994, 1, 2 );
    TestDate( "1?2",    1994, 1, 2 );

    TestDate( "6/20",   1993, 6, 20 );
    TestDate( "6-20",   1993, 6, 20 );
    TestDate( "6.20",   1993, 6, 20 );
    TestDate( "6?20",   1993, 6, 20 );

    TestDate( "jun 20, 93", 1993, 6, 20 );
    TestDate( "20 jun 93",  1993, 6, 20 );
    TestDate( "jun 20",     1993, 6, 20);
    TestDate( "20 jun",     1993, 6, 20 );
    TestDate( "2 jan",      1994, 1, 2 );
    TestDate( "jan 2",      1994, 1, 2 );

    TestTime( "12:30am",    0, 30 );
    TestTime( "12:30pm",   12, 30 );
    TestTime( "12?30am",    0, 30 );
    TestTime( "12?30pm",   12, 30 );
    TestTime( "12am",       0,  0 );
    TestTime( "12pm",      12, 0 );
    TestTime( "12:30",     12, 30 );
    TestTime( "12?30",     12, 30 );
    TestTime( "8:30",      8, 30 );
    TestTime( "8?30",      8, 30 );
    TestTime( "12",        12, 0 );
    TestTime( "8",          8, 0 );
    TestTime( "noon",      12, 0 );
    TestTime( "no",        12, 0 );
    TestTime( "midnight",  0, 0 );
    TestTime( "mi",        0, 0 );
    TestTime( "mdnt",      0, 0 );
    TestTime( "md",        0, 0 );

    #endif

    return TRUE;
}

void TestValue( char *text, long w, long f )
{
    TValue value;
    TValue compValue;
    compValue.Set( w, f );

    TParse::ChkValue( text, value );

    ASSERT_STR( text, value == compValue );
}

void TestDate( char *text, int year, int month, int day )
{
    TTime   time;
    TTime   compTime;

    compTime.SetNow();
    compTime.ClearTime();
    compTime.SetYear(  year );
    compTime.SetMonth(  month );
    compTime.SetDay(  day );

    TParse::ChkDate( text, time );

    ASSERT_STR( text, time == compTime );
}

void TestTime( char *text, int hour, int min )
{
    TTime   time;
    TTime   compTime;

    compTime.SetNow();
    compTime.SetHour(  hour );
    compTime.SetMinute(  min );

    TParse::ChkTime( text, time );
    time.CopyDate( compTime );

    ASSERT_STR( text, time == compTime );
}
