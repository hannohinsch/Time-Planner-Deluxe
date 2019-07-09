//*****************************************************************
//
// Manage profile (.ini) file.  Read at startup, written at exit.
//
//*****************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "profile.h"
#ifndef __holiday__
#include "holiday.h"
#endif
#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __parse__
#include "parse.h"
#endif
#ifndef __frame__
#include "frame.h"
#endif
#ifndef __oem__
#include "oem.h"
#endif

// initialize the profiles
TDayviewIncrements TProfile::dayviewIncrement  = kDayview_30_Minutes;
TTimeConflict      TProfile::TimeConflict      = kTimeConflict_Never;
TTimeFmtUnits      TProfile::baseTimeFmtUnits  = kTimeFmt_Hours;
BOOL               TProfile::bMixTimeFmtUnits  = FALSE;

double             TProfile::HoursPerDay       = 8;
double             TProfile::DaysPerWeek       = 5;
double             TProfile::HoursPerWeek      = HoursPerDay * DaysPerWeek;

BOOL               TProfile::IsWorkDay[7]      = { 0, 1, 1, 1, 1, 1, 0 };

TSafStr			   TProfile::MRUFile1;
TSafStr			   TProfile::MRUFile2;
TSafStr			   TProfile::MRUFile3;
TSafStr			   TProfile::MRUFile4;

int                TProfile::debugLevel= 0;
BOOL               TProfile::bAutoLoad= FALSE;
BOOL               TProfile::bShowTips= TRUE;

TNoteOrder        TProfile::NoteOrder[kNumNoteOrders]  = {

    kNoteOrder_Date,   // sort first by start date
    kNoteOrder_Color,  // then by color
    kNoteOrder_Type   // then by shape
    };


TGDISet TProfile::colorSet= kStdColors;

int    TProfile::BaudRate                 = 2400;
int    TProfile::CommPort                 = 1;
BOOL   TProfile::bTone                    = TRUE;
char   TProfile::LocalPrefix[25]          = "";
char   TProfile::LocalSuffix[25]          = "";
char   TProfile::LongDistancePrefix[25]   = "";
char   TProfile::LongDistanceSuffix[25]   = "";
char   TProfile::InternationalPrefix[25]  = "";
char   TProfile::InternationalSuffix[25]  = "";
char   TProfile::AlternatePrefix[25]      = "";
char   TProfile::AlternateSuffix[25]      = "";
char   TProfile::LocalAreaCode[25]        = "";

int    TProfile::WorkDayStartMinute       = 540;
int    TProfile::WorkDayEndMinute         = 1020;
// ----------------------------------------------------------------
// Constructor
//
// Read in the profile values
// ----------------------------------------------------------------
TProfile::TProfile( void )
{
    char    s[50];

    dayviewIncrement = (TDayviewIncrements) GetPrivateProfileInt(
        TOEM::profileSection, "DayviewIncrement", kDayview_30_Minutes, TOEM::profileName );
    TimeConflict = (TTimeConflict) GetPrivateProfileInt(
        TOEM::profileSection, "TimeConflict", kTimeConflict_Never, TOEM::profileName );
    baseTimeFmtUnits = (TTimeFmtUnits) GetPrivateProfileInt(
        TOEM::profileSection, "TimeFmtUnits", kTimeFmt_Hours, TOEM::profileName );
    bMixTimeFmtUnits = (BOOL) GetPrivateProfileInt(
        TOEM::profileSection, "MixTimeFmtUnits", FALSE, TOEM::profileName );

    colorSet= (TGDISet) GetPrivateProfileInt(
        TOEM::profileSection, "ColorSet", kStdColors, TOEM::profileName );

    IsWorkDay[0] =  GetPrivateProfileInt(
        TOEM::profileSection, "IsWorkDaySun", FALSE, TOEM::profileName );
    IsWorkDay[1] =  GetPrivateProfileInt(
        TOEM::profileSection, "IsWorkDayMon", TRUE, TOEM::profileName );
    IsWorkDay[2] =  GetPrivateProfileInt(
        TOEM::profileSection, "IsWorkDayTue", TRUE, TOEM::profileName );
    IsWorkDay[3] =  GetPrivateProfileInt(
        TOEM::profileSection, "IsWorkDayWed", TRUE, TOEM::profileName );
    IsWorkDay[4] =  GetPrivateProfileInt(
        TOEM::profileSection, "IsWorkDayThu", TRUE, TOEM::profileName );
    IsWorkDay[5] =  GetPrivateProfileInt(
        TOEM::profileSection, "IsWorkDayFri", TRUE, TOEM::profileName );
    IsWorkDay[6] =  GetPrivateProfileInt(
        TOEM::profileSection, "IsWorkDaySat", FALSE, TOEM::profileName );

    NoteOrder[0] = (TNoteOrder) GetPrivateProfileInt(
        TOEM::profileSection, "noteorder1", kNoteOrder_Date, TOEM::profileName );
    NoteOrder[1] = (TNoteOrder) GetPrivateProfileInt(
        TOEM::profileSection, "noteorder2", kNoteOrder_Color, TOEM::profileName );
    NoteOrder[2] = (TNoteOrder) GetPrivateProfileInt(
        TOEM::profileSection, "noteorder3", kNoteOrder_Type, TOEM::profileName );

    GetPrivateProfileString( TOEM::profileSection, "File1", "", s, 50, TOEM::profileName );
	MRUFile1.Set( s );
    GetPrivateProfileString( TOEM::profileSection, "File2", "",  s, 50, TOEM::profileName );
	MRUFile2.Set( s );
    GetPrivateProfileString( TOEM::profileSection, "File3", "", s, 50, TOEM::profileName );
	MRUFile3.Set( s );
    GetPrivateProfileString( TOEM::profileSection, "File4", "", s, 50, TOEM::profileName );
	MRUFile4.Set( s );

    debugLevel = GetPrivateProfileInt(
        TOEM::profileSection, "DebugLevel", 0, TOEM::profileName );

    bShowTips = (BOOL) GetPrivateProfileInt(
        TOEM::profileSection, "ShowTips", TRUE, TOEM::profileName );

    bAutoLoad = (BOOL) GetPrivateProfileInt(
        TOEM::profileSection, "AutoLoad", FALSE, TOEM::profileName );

    // Read in Holidays
    char    buf[255];

    THolidaySet::SetCountry( (TCountry) GetPrivateProfileInt(
        "holidays", "Country", kUSA, TOEM::profileName ));

    int numHolidays = GetPrivateProfileInt(
        "holidays", "NumHolidays", 0, TOEM::profileName );
    for (int i = 0; i < numHolidays; i++) {

        char     holidayBuf[10];
        wsprintf( holidayBuf, "h%d", i );
        GetPrivateProfileString( "holidays", holidayBuf, "", buf, 255, TOEM::profileName );

        if (strlen( buf ) == 0) {

            TMessageBox::Ok( TPimFrame::pFrame,
                "Couldn't read holiday from profile",
                "Profile Error" );
            continue;
        }

        char *dateBuf = strtok( buf, "[]" );
        TTime date;
        TParse::ChkDate( dateBuf, date );
        if (date.HasDate()) {

            TSafStr name( strtok( NULL, "[]" ));
            if (name.Len() > 0) {

                PTHoliday holidayP = new THoliday( date, name );
                THolidaySet::Add( *holidayP );
            }
            else {

                TMessageBox::Ok( TPimFrame::pFrame,
                    "Couldn't read holiday from profile",
                    "Profile Error" );
            }
        }
        else {

            TMessageBox::Ok( TPimFrame::pFrame,
                "Couldn't read holiday from profile",
                "Profile Error" );
        }
    }



    BaudRate =  GetPrivateProfileInt(
        TOEM::profileSection, "BaudRate", 2400, TOEM::profileName );
    CommPort =  GetPrivateProfileInt(
        TOEM::profileSection, "CommPort", 1, TOEM::profileName );
    bTone =  GetPrivateProfileInt(
        TOEM::profileSection, "Tone", TRUE, TOEM::profileName );
    
    GetPrivateProfileString( TOEM::profileSection, "LocalPrefix", "", LocalPrefix, 25, TOEM::profileName );
    GetPrivateProfileString( TOEM::profileSection, "LocalSuffix", "", LocalSuffix, 25, TOEM::profileName );
    GetPrivateProfileString( TOEM::profileSection, "LongDistancePrefix", "", LongDistancePrefix, 25, TOEM::profileName );
    GetPrivateProfileString( TOEM::profileSection, "LongDistanceSuffix", "", LongDistanceSuffix, 25, TOEM::profileName );
    GetPrivateProfileString( TOEM::profileSection, "InternationalPrefix", "", InternationalPrefix, 25, TOEM::profileName );
    GetPrivateProfileString( TOEM::profileSection, "InternationalSuffix", "", InternationalSuffix, 25, TOEM::profileName );
    GetPrivateProfileString( TOEM::profileSection, "AlternatePrefix", "", AlternatePrefix, 25, TOEM::profileName );
    GetPrivateProfileString( TOEM::profileSection, "AlternateSuffix", "", AlternateSuffix, 25, TOEM::profileName );
    GetPrivateProfileString( TOEM::profileSection, "LocalAreaCode", "", LocalAreaCode, 25, TOEM::profileName );

    WorkDayStartMinute = GetPrivateProfileInt(
        TOEM::profileSection, "StartMinute",
        540, TOEM::profileName );
    WorkDayEndMinute = GetPrivateProfileInt(
        TOEM::profileSection, "EndMinute",
        1020, TOEM::profileName );

    CalculateHours();
}



// ----------------------------------------------------------------
// Destructor
//
// Write out the profile values
// ----------------------------------------------------------------
TProfile::~TProfile( void )
{
    char buf[300];

    wsprintf( buf, "%d", TimeConflict );
    WritePrivateProfileString( TOEM::profileSection, "TimeConflict", buf, TOEM::profileName );
    wsprintf( buf, "%d", baseTimeFmtUnits );
    WritePrivateProfileString( TOEM::profileSection, "TimeFmtUnits", buf, TOEM::profileName );
    wsprintf( buf, "%d", bMixTimeFmtUnits );
    WritePrivateProfileString( TOEM::profileSection, "MixTimeFmtUnits", buf, TOEM::profileName );

    wsprintf( buf, "%d", colorSet );
    WritePrivateProfileString( TOEM::profileSection, "ColorSet", buf, TOEM::profileName );

    wsprintf( buf, "%d", IsWorkDay[0] );
    WritePrivateProfileString( TOEM::profileSection, "IsWorkDaySun", buf, TOEM::profileName );
    wsprintf( buf, "%d", IsWorkDay[1] );
    WritePrivateProfileString( TOEM::profileSection, "IsWorkDayMon", buf, TOEM::profileName );
    wsprintf( buf, "%d", IsWorkDay[2] );
    WritePrivateProfileString( TOEM::profileSection, "IsWorkDayTue", buf, TOEM::profileName );
    wsprintf( buf, "%d", IsWorkDay[3] );
    WritePrivateProfileString( TOEM::profileSection, "IsWorkDayWed", buf, TOEM::profileName );
    wsprintf( buf, "%d", IsWorkDay[4] );
    WritePrivateProfileString( TOEM::profileSection, "IsWorkDayThu", buf, TOEM::profileName );
    wsprintf( buf, "%d", IsWorkDay[5] );
    WritePrivateProfileString( TOEM::profileSection, "IsWorkDayFri", buf, TOEM::profileName );
    wsprintf( buf, "%d", IsWorkDay[6] );
    WritePrivateProfileString( TOEM::profileSection, "IsWorkDaySat", buf, TOEM::profileName );

    WritePrivateProfileString( TOEM::profileSection, "File1", MRUFile1.Addr(), TOEM::profileName );
    WritePrivateProfileString( TOEM::profileSection, "File2", MRUFile2.Addr(), TOEM::profileName );
    WritePrivateProfileString( TOEM::profileSection, "File3", MRUFile3.Addr(), TOEM::profileName );
    WritePrivateProfileString( TOEM::profileSection, "File4", MRUFile4.Addr(), TOEM::profileName );

    //FIXME this is to prevent mem leak -- must be better way
    MRUFile1.Clear();
    MRUFile2.Clear();
    MRUFile3.Clear();
    MRUFile4.Clear();

    wsprintf( buf, "%d", bShowTips );
    WritePrivateProfileString( TOEM::profileSection, "ShowTips", buf, TOEM::profileName );

    wsprintf( buf, "%d", bAutoLoad );
    WritePrivateProfileString( TOEM::profileSection, "AutoLoad", buf, TOEM::profileName );

    // Write Out Holidays
    wsprintf( buf, "%d", THolidaySet::Country() );
    WritePrivateProfileString( "holidays", "Country", buf, TOEM::profileName );

    wsprintf( buf, "%d", THolidaySet::GetNumHolidays() );
    WritePrivateProfileString( "holidays", "NumHolidays", buf, TOEM::profileName );

    // always write out the holidays with a '/'
    // as a separator, no matter what intlfmt says
    TIntlFmt::SetStandardFormats();

    int numHolidays = THolidaySet::GetNumHolidays();
    for (int i = 0; i < numHolidays; i++) {

        THoliday holiday;
        TSafStr  name;
        TTime    date;
        char     dateBuf[50];
        char     holidayBuf[10];

        THolidaySet::GetHoliday( i, holiday );
        holiday.GetDate( date );
        holiday.GetName( name );

        TIntlFmt::FormatDate( date, kShortDate, dateBuf );
        wsprintf( buf, "[%s][%s]", dateBuf, name.Addr() );
        wsprintf( holidayBuf, "h%d", i );

        WritePrivateProfileString( "holidays", holidayBuf, buf, TOEM::profileName );
    }

    // reset the date/time formats
    TIntlFmt::ResetFormats();


    wsprintf( buf, "%d", BaudRate );
    WritePrivateProfileString( TOEM::profileSection, "BaudRate", buf, TOEM::profileName );
    wsprintf( buf, "%d", CommPort );
    WritePrivateProfileString( TOEM::profileSection, "CommPort", buf, TOEM::profileName );
    wsprintf( buf, "%d", bTone );
    WritePrivateProfileString( TOEM::profileSection, "Tone", buf, TOEM::profileName );

    WritePrivateProfileString( TOEM::profileSection, "LocalPrefix", LocalPrefix, TOEM::profileName );
    WritePrivateProfileString( TOEM::profileSection, "LocalSuffix", LocalSuffix, TOEM::profileName );
    WritePrivateProfileString( TOEM::profileSection, "LongDistancePrefix", LongDistancePrefix, TOEM::profileName );
    WritePrivateProfileString( TOEM::profileSection, "LongDistanceSuffix", LongDistanceSuffix, TOEM::profileName );
    WritePrivateProfileString( TOEM::profileSection, "InternationalPrefix", InternationalPrefix, TOEM::profileName );
    WritePrivateProfileString( TOEM::profileSection, "InternationalSuffix", InternationalSuffix, TOEM::profileName );
    WritePrivateProfileString( TOEM::profileSection, "AlternatePrefix", AlternatePrefix, TOEM::profileName );
    WritePrivateProfileString( TOEM::profileSection, "AlternateSuffix", AlternateSuffix, TOEM::profileName );
    WritePrivateProfileString( TOEM::profileSection, "LocalAreaCode", LocalAreaCode, TOEM::profileName );

    wsprintf( buf, "%d", WorkDayStartMinute );
    WritePrivateProfileString(
        TOEM::profileSection, "StartMinute",
        buf, TOEM::profileName );
    wsprintf( buf, "%d", WorkDayEndMinute );
    WritePrivateProfileString(
        TOEM::profileSection, "EndMinute",
        buf, TOEM::profileName );
}



// ----------------------------------------------------------------
// StringToDouble
//
// Convert a string to a double. Stops at the first non-digit or
// second decimal character
// ----------------------------------------------------------------
void TProfile::StringToDouble( char *s, double &d )
{
    BOOL   foundDecimal   = FALSE;
    double decimalDivider = 10;
    int    length         = strlen( s );

    d = 0;
    for (int i = 0; i < length; i++) {

        if (('0' <= s[i]) && (s[i] <= '9')) {

            if (foundDecimal) {

                d += (s[i] - '0') / decimalDivider;
                decimalDivider *= 10;
            }
            else {

                d = d * 10 + (s[i] - '0');
            }
        }
        else if (s[i] == '.') {

            if (foundDecimal) {

                return;
            }
            else {

                foundDecimal = TRUE;
            }
        }
        else {

            return;
        }
    }
}

// ----------------------------------------------------------------
// DoubleToString
//
// Convert a double to a string
// ----------------------------------------------------------------
void TProfile::DoubleToString( double d, char *s )
{
    long    whole    = d;
    long    fraction = ((d - whole) * 1000005L + 5) / 10;
    char    wholeStr[25];
    char    fracStr[25];

    //handle the whole part
    wsprintf( wholeStr, "%ld", whole );

    // handle the fractional part
    wsprintf( fracStr, "%0.5ld", fraction );


    // delete trailing zeros
    for (int i = strlen( fracStr ) - 1; i >= 0; i--) {

        if (fracStr[i] == '0') {

            fracStr[i] = '\0';
        }
        else {

            break;
        }
    }

    // put them together
    if (strlen( fracStr ) == 0) {

        strcpy( s, wholeStr );
    }
    else {

        wsprintf( s, "%s.%s", wholeStr, fracStr );
    }
}


// ----------------------------------------------------------------
// CalculateHours
//
// Calculate the number of hours after something changed
// ----------------------------------------------------------------
void TProfile::CalculateHours( void )
{

    HoursPerDay= (WorkDayEndMinute - WorkDayStartMinute) / 60.0;

    DaysPerWeek= 0;
    for( int i=0; i < 7; i++ )
        DaysPerWeek+= (IsWorkDay[ i ] > 0)? 1: 0;

    HoursPerWeek    = HoursPerDay * DaysPerWeek;

}

//*****************************************************************************

//*****************************************************************************
void TProfile::GetNoteOrderName( TNoteOrder order, RTSafStr str )
{
switch( order )
    {
    case kNoteOrder_Color:
        str.KSET( "Color" );
        break;
    case kNoteOrder_Date:
        str.KSET( "StartDate" );
        break;
    case kNoteOrder_Duration:
        str.KSET( "Duration" );
        break;
    case kNoteOrder_Type:
        str.KSET( "Shape" );
        break;
    case kNoteOrder_Alpha:
        str.KSET( "Alpha" );
        break;

    default:
        SYSERR( ERR_GEN );
    }
}

//*****************************************************************************

//*****************************************************************************
void TProfile::RestoreSort( void )
{
    NoteOrder[0] = (TNoteOrder) GetPrivateProfileInt(
        TOEM::profileSection, "noteorder1", kNoteOrder_Date, TOEM::profileName );
    NoteOrder[1] = (TNoteOrder) GetPrivateProfileInt(
        TOEM::profileSection, "noteorder2", kNoteOrder_Color, TOEM::profileName );
    NoteOrder[2] = (TNoteOrder) GetPrivateProfileInt(
        TOEM::profileSection, "noteorder3", kNoteOrder_Type, TOEM::profileName );

}

//*****************************************************************************

//*****************************************************************************
void TProfile::RestoreInterval( void )
{
    dayviewIncrement = (TDayviewIncrements) GetPrivateProfileInt(
        TOEM::profileSection, "DayviewIncrement", kDayview_30_Minutes, TOEM::profileName );
}

