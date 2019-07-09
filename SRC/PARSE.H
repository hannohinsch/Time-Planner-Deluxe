#ifndef __parse__
#define __parse__

_CLASSDEF( TTime )
_CLASSDEF( TValue)
_CLASSDEF( TParse )

const kMaxTokenString= 20;
const kMaxToken= 200;

typedef enum
    {
    kCheckDate,
    kCheckTime,
    kCheckValue
    } TParseContext;

typedef enum
    {
    DOW, MONTH, AMPM, NUM, NOON, MIDNIGHT,
    DOLLAR, COLON, SLASH, DASH, COMMA, DOT,
    DATE_SEPARATOR, TIME_SEPARATOR,
    THOUSANDS_SEPARATOR, DECIMAL_SEPARATOR,
    SPACE, JUNK, TEXTEND,
    DATE1, DATE2, DATE3, DATE4, DATE5, DATE6, DATE7,
    TIME1, TIME2, TIME3, TIME4, TIME5, TIME6,
    VALUE1, VALUE2, VALUE3, VALUE4, VALUE5,
    VALUE6, VALUE7, VALUE8, VALUE9, VALUE10,
    PATEND, NOACTION, ACTION
    } TParseTokenType;

typedef struct
    {
    TParseTokenType  Type;
    WORD        Num;
    char        Text[255];
    } TToken, &RTToken;

typedef TParseTokenType TTokenPatternTableEntry[kMaxTokenString];
typedef TTokenPatternTableEntry TTokenPatternTable[];

class _CLASSTYPE TParse
{
public:

    static void AppInit();
    static void AppCleanup();

    static void ChkDate( char *text, TTime &date );
    static void ChkTime( char *text, TTime &time );
    static void ChkValue( char *text, TValue &value );
    static long MkNum( char *p );

    static BOOL ParseTest( void );

private:

    static void   AnalyseDate(        RTTime date );
    static void   AnalyseTime(        RTTime time );
    static void   AnalyseValue(       RTValue value );
    static void   MakeDate(           int first, int second, int third, RTTime time );
    static void   MakeDate(           int first, int second, RTTime time );
    static void   MakeDateWithMonth(  int month, int second, int third, RTTime time );
    static void   MakeDateWithMonth(  int month, int second, RTTime time );
    static void   MakeDateWithDOW(    int dow, RTTime time );
    static void   MakeTime(           int first, int second, BOOL isAm, RTTime time );
    static void   MakeTime(           int first, int second, RTTime time );

    static int    GuessYear(          int day, int mon );

    static void   BuildTokenTable(    char *text );
    static int    IsMatch(            int minCnt, char *startP, char *endP, char *tab[] );
    static void   GetToken(           char *p, RTToken token );
    static int    Compare(            char *p, char *s );
    static void   ComparePattern(     int startToken, int patIndex, TParseTokenType *pType, int *cnt, TTokenPatternTable patterns );
    static int    MakeYear(           int raw );
    static long   MakeFraction(       RTToken token );

    static void FillToken(            RTToken token, TParseTokenType type,
        WORD num, char *pStart, char *pEnd );

    static TTokenPatternTable itsDatePatterns;
    static TTokenPatternTable itsTimePatterns;
    static TTokenPatternTable itsValuePatterns;
    static TToken             itsTokenTab[kMaxToken];
    static WORD               itsTokenIndex;
    static TParseContext      itsParseContext;      
};

#endif
