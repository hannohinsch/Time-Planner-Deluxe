//*****************************************************************************
// holiday object and holiday set
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "holiday.h"
#include "xarray.h"

// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------
THoliday::THoliday( void )
{
}

// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------
THoliday::THoliday( RTHoliday holiday )
{
    itsName = holiday.itsName;
    itsDate = holiday.itsDate;
}

// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------
THoliday::THoliday( RTTime date, RTSafStr name )
{
    itsDate = date;
    itsDate.ClearTime();

    itsName = name;
}

TStreamableClass RegTHoliday( "THoliday", THoliday::build, __DELTA(THoliday) );

// -----------------------------------------------------------------------
// Read
// -----------------------------------------------------------------------
Pvoid THoliday::read( Ripstream is )
{
    int version;

    is >> version;

    if( version == 1 ) {

        is >> itsDate;
        is >> itsName;
    }
    else {

	    SYSERR( ERR_VER );
    }

    return this;
}

// -----------------------------------------------------------------------
// Write
// -----------------------------------------------------------------------
void THoliday::write( Ropstream os )
{
    os << 1;

    os << itsDate;
    os << itsName;
}

// ***********************************************************************
//
//                     HolidaySet
//
// ***********************************************************************

static const int kJan = 1;
static const int kFeb = 2;
static const int kMar = 3;
static const int kApr = 4;
static const int kMay = 5;
static const int kJun = 6;
static const int kJul = 7;
static const int kAug = 8;
static const int kSep = 9;
static const int kOct = 10;
static const int kNov = 11;
static const int kDec = 12;

static const int kSun = 0;
static const int kMon = 1;
static const int kTue = 2;
static const int kWed = 3;
static const int kThu = 4;
static const int kFri = 5;
static const int kSat = 6;

TFixedHoliday USAJan[] ={{ kDate,    1,  k1st, "New Years Day"        }};
TFixedHoliday USAMay[] ={{  kDay, kMon, kLast, "Memorial Day"         }};
TFixedHoliday USAJul[] ={{ kDate,    4,  k1st, "Independence Day"     }};
TFixedHoliday USASep[] ={{  kDay, kMon,  k1st, "Labor Day"            }};
TFixedHoliday USANov[] ={{  kDay, kThu, kLast, "Thanksgiving"         }};
TFixedHoliday USADec[] ={{ kDate,   25,  k1st, "Christmas Day"        }};

THolidayTable THolidaySet::itsFixedHolidays = {

    // kNone
    {{ 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              }},

    // kUSA
    {{ 0, NULL                                              },
     { sizeof( USAJan ) / sizeof( TFixedHoliday ), USAJan   },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { sizeof( USAMay ) / sizeof( TFixedHoliday ), USAMay   },
     { 0, NULL                                              },
     { sizeof( USAJul ) / sizeof( TFixedHoliday ), USAJul   },
     { 0, NULL                                              },
     { sizeof( USASep ) / sizeof( TFixedHoliday ), USASep   },
     { 0, NULL                                              },
     { sizeof( USANov ) / sizeof( TFixedHoliday ), USANov   },
     { sizeof( USADec ) / sizeof( TFixedHoliday ), USADec   }},

    // kCanada
    {{ 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              },
     { 0, NULL                                              }}
};

TTime        THolidaySet::itsCurrentDate;
TCountry     THolidaySet::itsCountry      = kUSA;
PTXArray     THolidaySet::itsHolidaysP    = NULL;

// -----------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------
THolidaySet::THolidaySet( void )
{
    itsHolidaysP = new TXArray( 10, 10 );
}

// -----------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------
THolidaySet::~THolidaySet( void )
{
    if (itsHolidaysP != NULL) {

        itsHolidaysP->flush( TXArray::kDelete );
        delete itsHolidaysP;
    }
}

// -----------------------------------------------------------------------
// IsHoliday
// -----------------------------------------------------------------------
BOOL THolidaySet::IsHoliday( RTTime date )
{
    TTime day( date );
    day.ClearTime();

    int numHolidays = itsHolidaysP->getItemsInContainer();
    for (int i = 0; i < numHolidays; i++) {

        RTHoliday holiday = (RTHoliday) (*itsHolidaysP)[i];
        if (holiday.itsDate == day) {

            return TRUE;
        }
    }

    return IsFixedHoliday( date );
}

// -----------------------------------------------------------------------
// IsFixedHoliday
// -----------------------------------------------------------------------
BOOL THolidaySet::IsFixedHoliday( RTTime date )
{
    int n = itsFixedHolidays[itsCountry][date.GetMonth()].holidaysInMonth;
    for (int i = 0; i < n; i++) {

        if (MatchNthFixedHoliday( date, i )) {

            return TRUE;
        }
    }

    return FALSE;
}




// -----------------------------------------------------------------------
// GetNthFixedHoliday
//
//  return the nth fixed holiday in the given month and year
// -----------------------------------------------------------------------
void THolidaySet::GetNthFixedHoliday( int month, int year, int n, RTTime date )
{
    date.SetDate( 1, month, year );

    TFixedHoliday fixedHoliday =
        itsFixedHolidays[itsCountry][date.GetMonth()].holidays[n];

    if (fixedHoliday.type == kDate) {

        date.SetDay( fixedHoliday.day );
    }
    else {

	    date.SetDayOfMonth( fixedHoliday.week, fixedHoliday.day );
    }
}

// -----------------------------------------------------------------------
// MatchNthFixedHoliday
//
//  does the date match the nth holiday for the date's month?
// -----------------------------------------------------------------------
BOOL THolidaySet::MatchNthFixedHoliday( RTTime date, int n )
{
    TFixedHoliday fixedHoliday =
            itsFixedHolidays[itsCountry][date.GetMonth()].holidays[n];
    if (fixedHoliday.type == kDate) {

        return fixedHoliday.day == date.GetDay();
    }
    else {

        TWeekSpecifier week;
        int            dayInWeek;

        date.GetDayOfMonth( week, dayInWeek );
        return (fixedHoliday.week == week) && (fixedHoliday.day == dayInWeek);
    }
}

// -----------------------------------------------------------------------
// GetNumHolidays
//
//  return the number of holidays in the set
// -----------------------------------------------------------------------
int THolidaySet::GetNumHolidays( void )
{
    return itsHolidaysP->getItemsInContainer();
}

// -----------------------------------------------------------------------
// GetHoliday
//
//  return the nth holiday
// -----------------------------------------------------------------------
BOOL THolidaySet::GetHoliday( int n, RTHoliday holiday )
{
    if ((n < 0) || (n >= itsHolidaysP->getItemsInContainer())) {

        return FALSE;
    }

    holiday = (RTHoliday) (*itsHolidaysP)[n];
    return holiday == NOOBJECT;
}

// -----------------------------------------------------------------------
// GetHoliday
//
//  return the holiday for the specified day
// -----------------------------------------------------------------------
BOOL THolidaySet::GetHoliday( RTTime date, RTHoliday holiday )
{
    TTime day( date );
    day.ClearTime();

            // find out what holiday is on this day, anyway
    int n = itsHolidaysP->getItemsInContainer();
    for (int i = 0; i < n; i++) {

        RTHoliday h = (RTHoliday) (*itsHolidaysP)[i];
        if (h == NOOBJECT) {

            continue;
        }

        TTime holidate;
        h.GetDate( holidate );
        if (holidate == day) {

            holiday = h;
            return TRUE;
        }
    }

    return FALSE;
}

// -----------------------------------------------------------------------
// GetFixedHoliday
//
//  return the fixed holiday for the specified day
// -----------------------------------------------------------------------
BOOL THolidaySet::GetFixedHoliday( RTTime date, RTHoliday holiday )
{
    int n = itsFixedHolidays[itsCountry][date.GetMonth()].holidaysInMonth;
    for (int i = 0; i < n; i++) {

        if (MatchNthFixedHoliday( date, i )) {

            holiday.itsDate = date;
            holiday.itsName = itsFixedHolidays[itsCountry][date.GetMonth()].holidays[i].name;
            return TRUE;
        }
    }

    return FALSE;

}

// -----------------------------------------------------------------------
// Add
//
//  add the holiday
// -----------------------------------------------------------------------
void THolidaySet::Add( RTHoliday holiday )
{
    itsHolidaysP->add( holiday );
}

// -----------------------------------------------------------------------
// Flush
//
//  clear out all of the holidays and delete them all
// -----------------------------------------------------------------------
void THolidaySet::Flush( void )
{
    itsHolidaysP->flush( TXArray::kDelete );
}

// -----------------------------------------------------------------------
// GetFirstFixedHoliday
//
//  get the first fixed holiday after the start date. return false
//  if it's also after the end date
// -----------------------------------------------------------------------
BOOL THolidaySet::GetFirstFixedHoliday( RTTime start, RTTime end, RTTime holidate )
{
    int monthDelta = start.MonthDelta( end );
    TTime date     = start;

    for (int i = 0; i <= monthDelta; i++) {

        int month = date.GetMonth();
        int n = itsFixedHolidays[itsCountry][month].holidaysInMonth;
        for (int j = 0; j < n; j++) {

            GetNthFixedHoliday( month, date.GetYear(), j, holidate );
            if (start <= holidate) {

                itsCurrentDate = holidate.Plus( SCALE_DAY, 1 );
                return (holidate <= end);
            }
        }

        date.PlusEquals( SCALE_MONTH, 1 );
    }

    return FALSE;
}

// -----------------------------------------------------------------------
// GetNextFixedHoliday
//
//  get the first fixed holiday after the start date. return false
//  if it's also after the end date
// -----------------------------------------------------------------------
BOOL THolidaySet::GetNextFixedHoliday( RTTime end, RTTime holidate )
{
    return GetFirstFixedHoliday( itsCurrentDate, end, holidate );
}
