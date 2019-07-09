//*****************************************************************************
// frequency (recurring event)
//
// FIXME The iteration mechanism should not keep its state in the freq object;
// it should use an iterator object.
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "freq.h"

#ifndef __note__
#include "note.h"
#endif
#ifndef __xarray__
#include "xarray.h"
#endif
#ifndef __message__
#include "message.h"
#endif
#ifndef __frame__
#include "frame.h"
#endif

enum {

     kSun  = 0,
     kMon  = 1,
     kTue  = 2,
     kWed  = 3,
     kThu  = 4,
     kFri  = 5,
     kSat  = 6
     };

//  ----------------------------------------------------------------------------
//  Constructor
// 	----------------------------------------------------------------------------
TFreq::TFreq( void )
{
    // frequency information
    itsFrequencyType = kOnce;
    itsFrequency     = 1;
    itsFrequencyWeek = k1st;
    itsFrequencyDay  = 1;

    itsAdditionsP    = new TXArray( 10, 10 );
    itsExclusionsP   = new TXArray( 10, 10 );
}

//  ----------------------------------------------------------------------------
//  Constructor
// 	----------------------------------------------------------------------------
TFreq::TFreq( RTTime date )
{
    // frequency information
    itsFrequencyType = kOnce;
    itsFrequency     = 1;
    itsFrequencyWeek = k1st;
    itsFrequencyDay  = 1;

    itsAdditionsP    = new TXArray( 10, 10 );
    itsExclusionsP   = new TXArray( 10, 10 );

    itsStartDate     = date;
}

//  ----------------------------------------------------------------------------
//  Constructor
// 	----------------------------------------------------------------------------
TFreq::TFreq( RTFreq freq )
{
    itsAdditionsP    = new TXArray( 10, 10 );
    itsExclusionsP   = new TXArray( 10, 10 );

    (*this) = freq;
}

//  ----------------------------------------------------------------------------
//  Destructor
// 	----------------------------------------------------------------------------
TFreq::~TFreq( void )
{
    itsAdditionsP->flush(  TXArray::kDelete );
    itsExclusionsP->flush( TXArray::kDelete );

    delete itsAdditionsP;
    delete itsExclusionsP;
}

//  ----------------------------------------------------------------------------
//  Clear
// 	----------------------------------------------------------------------------
void TFreq::Clear( void )
{
    itsStartDate.Clear();
    itsEndDate.Clear();
    itsCurrentDate.Clear();
    itsLastAdditionDate.Clear();

    itsFrequencyType = kOnce;
    itsFrequency     = 1;
    itsFrequencyWeek = k1st;
    itsFrequencyDay  = 1;

    itsAdditionsP->flush(  TXArray::kDelete );
    itsExclusionsP->flush( TXArray::kDelete );
}

//---------------------------------------------------------------------------
// GetStart
//---------------------------------------------------------------------------
void TFreq::GetStart( RTTime start )
{
    start = itsStartDate;
}

//---------------------------------------------------------------------------
// SetTime
//
// Set the various time in the frequency to some other time of day
//---------------------------------------------------------------------------
void TFreq::SetTime( RTTime time )
{
    if (itsStartDate.HasTime())         itsStartDate.CopyTime( time );
    if (itsEndDate.HasTime())           itsEndDate.CopyTime( time );
    if (itsCurrentDate.HasTime())       itsCurrentDate.CopyTime( time );
    if (itsLastAdditionDate.HasTime())  itsLastAdditionDate.CopyTime( time );
}

//  ----------------------------------------------------------------------------
//  operator =
// 	----------------------------------------------------------------------------
RTFreq TFreq::operator =( RTFreq rhs )
{

    itsStartDate     = rhs.itsStartDate;
    itsEndDate       = rhs.itsEndDate;

    itsFrequencyType = rhs.itsFrequencyType;
    itsFrequency     = rhs.itsFrequency;
    itsFrequencyWeek = rhs.itsFrequencyWeek;
    itsFrequencyDay  = rhs.itsFrequencyDay;

    itsAdditionsP->flush( TXArray::kDelete );
    itsExclusionsP->flush( TXArray::kDelete );

    int n = rhs.itsAdditionsP->getItemsInContainer();
    for (int i = 0; i < n; i++) {

        RTTime time = (RTTime) (*rhs.itsAdditionsP)[i];
        PTTime timeP = new TTime( time );
        itsAdditionsP->add( *timeP );
    }

    n = rhs.itsExclusionsP->getItemsInContainer();
    for (i = 0; i < n; i++) {

        RTTime time = (RTTime) (*rhs.itsExclusionsP)[i];
        PTTime timeP = new TTime( time );
        itsExclusionsP->add( *timeP );
    }


    return *this;
}

//  ----------------------------------------------------------------------------
//  operator ==
//
//  it's actually hard to compare two frequencies, so just put up a warning,
//  since we (so far) we don't even need to compare frequencies
// 	----------------------------------------------------------------------------
BOOL TFreq::operator ==( RTFreq  )
{
    SYSERR_STR( "Don't use == for frequencies" );
    return FALSE;
}

//  ----------------------------------------------------------------------------
//  InferFrequency
//
//	Calculate the frequency based on the start and end times.
// 	----------------------------------------------------------------------------
void TFreq::InferFrequency( PTWindowsObject parent, RTTime date1, RTTime date2 )
{
    TWeekSpecifier  week1;
	TWeekSpecifier 	week2;
	int 			dayInWeek1;
	int 			dayInWeek2;



    // collect information about dates
    date1.GetDayOfMonth( week1, dayInWeek1 );
    date2.GetDayOfMonth( week2, dayInWeek2 );

    // if one of the weeks is kLast and the other k4th,
    // then get the week again, not allowing kLast this time
    if ((week1 == kLast) && (week2 == k4th)) {

        date1.GetDayOfMonth( week1, dayInWeek1, FALSE );
    }
    else if ((week1 == k4th) && (week2 == kLast)) {

        date2.GetDayOfMonth( week2, dayInWeek2, FALSE );
    }



    // same date
    if (date1.IsSameDay( date2, FALSE )) {

        itsFrequencyType  = kOnce;
	}

    // last day of different months
    else if (date1.IsLastDayOfMonth() && date2.IsLastDayOfMonth()) {

        itsFrequency      = date1.MonthDelta( date2 );
        itsFrequencyType  = kMonthsApartOnLastDay;
	}

    // same day of different months
    else if (date1.itsDay == date2.itsDay) {

        itsFrequency      = date1.MonthDelta( date2 );
        itsFrequencyType  = kMonthsApartOnDayNumber;
	}

    // same week within month and day within week
	else if ((week1 == week2) && (dayInWeek1 == dayInWeek2)) {

        itsFrequencyType  = kMonthsApartOnDayName;
        itsFrequency      = date1.MonthDelta( date2 );
        itsFrequencyWeek  = week1;
        itsFrequencyDay   = dayInWeek1;
	}

    // same day in different weeks
	else if (dayInWeek1 == dayInWeek2) {

        itsFrequency      = date1.WeekDelta( date2 );
        itsFrequencyDay   = dayInWeek1;
        itsFrequencyType  = kWeeksApart;
	}

    else if (
        ((dayInWeek1 == kMon) && (dayInWeek2 == kWed)) ||
        ((dayInWeek1 == kWed) && (dayInWeek2 == kFri)) ||
        ((dayInWeek1 == kFri) && (dayInWeek2 == kMon))) {

        int numDays = date1.DayDelta( date2 );
        char buf[20];
        wsprintf( buf, "Every %d Days", numDays );

        switch( TMessageBox::ThreeButton(
            parent, IDD_MWF_TTH,
           "Every Monday, Wednesday and Friday?", "Frequency",
           "Mon/Wed/Fri", buf )) {

            case IDYES:

                itsFrequencyType = kMonWedFri;
                break;

            case IDNO:

                itsFrequency      = numDays;
                itsFrequencyType  = kDaysApart;
                break;

            case IDCANCEL:

                break;
        }
    }

    else if (
       ((dayInWeek1 == kTue) && (dayInWeek2 == kThu)) ||
       ((dayInWeek1 == kThu) && (dayInWeek2 == kTue))) {

        int numDays = date1.DayDelta( date2 );
        char buf[20];
        wsprintf( buf, "Every %d Days", numDays );

        switch( TMessageBox::ThreeButton(
            parent, IDD_MWF_TTH,
           "Every Tuesday and Thursday?", "Frequency",
           "Tue/Thu", buf )) {

            case IDYES:

                itsFrequencyType  = kTueThu;
                break;

            case IDNO:

                itsFrequency      = numDays;
                itsFrequencyType  = kDaysApart;
                break;

            case IDCANCEL:

                break;
        }
    }

    // some number of days apart
	else {

        itsFrequency      = date1.DayDelta( date2 );
        itsFrequencyType  = kDaysApart;
	}
}

//  ----------------------------------------------------------------------------
//  AddDate
//
// If the date was previously excluded, then
// remove it from the exclusion list, otherwise add it to the additions list
// 	----------------------------------------------------------------------------
void TFreq::AddDate( RTTime addDate )
{
    int n = itsExclusionsP->getItemsInContainer();
    for (int i = 0; i < n; i++) {

        RTTime date = (RTTime) (*itsExclusionsP)[i];
        if (date.IsSameDay( addDate, FALSE )) {

            itsExclusionsP->detach( date, TXArray::kDelete );
            return;
        }
    }


    // if there was no exclusion to reinstate, then add the date
    // to the additions list
    PTTime d = new TTime( addDate );
    itsAdditionsP->add( *d );
}


//  ----------------------------------------------------------------------------
//  ExcludeDate
//
// If the date was previously in the additions list, then
// remove it, otherwise add it to the exclusions list
// 	----------------------------------------------------------------------------
void TFreq::ExcludeDate( RTTime addDate )
{
    int n = itsAdditionsP->getItemsInContainer();
    for (int i = 0; i < n; i++) {

        RTTime date = (RTTime) (*itsAdditionsP)[i];
        if (date.IsSameDay( addDate, FALSE )) {

            itsAdditionsP->detach( date, TXArray::kDelete );
            return;
        }
    }


    // if there was no addition to remove then add the date the
    // the list of exclusions
    PTTime d = new TTime( addDate );
    itsExclusionsP->add( *d );
}


//---------------------------------------------------------------------------
// GetFreqText
//---------------------------------------------------------------------------
void TFreq::GetFreqText( char *s )
{
	switch (itsFrequencyType) {

	    case kOnce:

            strcpy( s, "once" );
            break;

        case kDaysApart:

            if (itsFrequency == 1) {

                strcpy( s, "every day" );
            }
            else {

                wsprintf( s, "%d days apart", itsFrequency );
            }
            break;

        case kMonWedFri:

            strcpy( s, "every Mon/Wed/Fri" );
            break;

        case kTueThu:

            strcpy( s, "every Tue/Thu" );
            break;

	    case kWeeksApart:

            if (itsFrequency == 1) {

                strcpy( s, "once a week" );
            }
            else {

                wsprintf( s, "%d weeks apart", itsFrequency );
            }
            break;

	    case kMonthsApartOnLastDay:
	    case kMonthsApartOnDayNumber:
	    case kMonthsApartOnDayName:

            if (itsFrequency == 1) {

                strcpy( s, "once a month" );
            }
            else {

                wsprintf( s, "%d months apart", itsFrequency );
            }
            break;

        default:

            SYSERR_STR( "GetFreqText: unexpected case" );
            break;
    }
}

//---------------------------------------------------------------------------
// First
//---------------------------------------------------------------------------
void TFreq::First( RTTime first )
{
    itsCurrentDate      = itsStartDate;
    itsLastAdditionDate = itsStartDate;
    first               = itsStartDate;
}

//---------------------------------------------------------------------------
// Next
//---------------------------------------------------------------------------
BOOL TFreq::Next( RTTime next )
{
    // try to calculate the next date
    if (!NextSub( next )) {

        // (wouldn't work only if the frequency is kOnce)
        return FALSE;
    }


    // repeat the operation as many times as necessary to make sure
    // that the calculated next date is not on the list of exclusions
    while (IsExclusion( next )) {

        NextSub( next );
    }

    // is 'next' really valid?
    if (itsEndDate.HasDate()) {

        // if it's less than the end date, then it's valid
        return (next <= itsEndDate);
    }
    else {

        // if there is no end date at all, it's valid
        return TRUE;
    }
}


//---------------------------------------------------------------------------
// NextDisplay
//---------------------------------------------------------------------------
BOOL TFreq::NextDisplay( RTTime next, TCellType &cellType )
{
    // try to calculate the next date
    if (!NextSub( next )) {

        // (wouldn't work only if the frequency is kOnce)
        return FALSE;
    }


    // figure out what kind of cell it is
    if (IsExclusion( next )) {

        cellType = kExcludeCell;
    }
    else if (IsAddition( next )) {

        cellType = kAdditionCell;
    }
    else {

        cellType = kFrequencyCell;
    }


    // is 'next' really valid?
    if (itsEndDate.HasDate()) {

        // if it's less than the end date, then it's valid
        return (next <= itsEndDate);
    }
    else {

        // if there is no end date at all, it's valid
        return TRUE;
    }
}

//---------------------------------------------------------------------------
// NextSub
//---------------------------------------------------------------------------
BOOL TFreq::NextSub( RTTime next )
{
    next = itsCurrentDate;

    switch (itsFrequencyType) {

	    case kOnce:

            return FALSE;

        case kDaysApart:

            next.PlusEquals( SCALE_DAY, itsFrequency );
            break;

        case kMonWedFri:

            switch( next.GetWeekDay() ) {

                case kMon:
                case kWed:

                    next.PlusEquals( SCALE_DAY, 2 );
                    break;

                case kFri:

                    next.PlusEquals( SCALE_DAY, 3 );
                    break;
            }
            break;

        case kTueThu:

            switch( next.GetWeekDay() ) {

                case kTue:

                    next.PlusEquals( SCALE_DAY, 2 );
                    break;

                case kThu:

                    next.PlusEquals( SCALE_DAY, 5 );
                    break;
            }
            break;

	    case kWeeksApart:

            next.PlusEquals( SCALE_DAY, itsFrequency * 7 );
            break;

	    case kMonthsApartOnDayNumber:

            next.PlusEquals( SCALE_MONTH, itsFrequency );
            break;

	    case kMonthsApartOnLastDay:

            next.PlusEquals( SCALE_MONTH, itsFrequency );
            next.SetLastDayOfMonth();
            break;

	    case kMonthsApartOnDayName:

            next.PlusEquals( SCALE_MONTH, itsFrequency );
            next.SetDayOfMonth( itsFrequencyWeek, itsFrequencyDay );
            break;

        default:

            SYSERR_STR( "NextSub: unexpected case" );
            return FALSE;
    }


    // having calculated the next date from the present one, modify
    // the next date if the user specified an additional date between
    // the start date and the newly calculated next date
    ModifyForAddition( next );
    return TRUE;
}

//---------------------------------------------------------------------------
// IsExclusion
//---------------------------------------------------------------------------
BOOL TFreq::IsExclusion( RTTime date )
{
    int n = itsExclusionsP->getItemsInContainer();
    for (int i = 0; i < n; i++) {

        RTTime exclusion = (RTTime) (*itsExclusionsP)[i];
        if (date.IsSameDay( exclusion, FALSE )) {

            return TRUE;
        }
    }

    return FALSE;
}

//---------------------------------------------------------------------------
// IsAddition
//---------------------------------------------------------------------------
BOOL TFreq::IsAddition( RTTime date )
{
    int n = itsAdditionsP->getItemsInContainer();
    for (int i = 0; i < n; i++) {

        RTTime addition = (RTTime) (*itsAdditionsP)[i];
        if (date.IsSameDay( addition, FALSE )) {

            return TRUE;
        }
    }

    return FALSE;
}

//---------------------------------------------------------------------------
// ModifyForAddition
//
// If there is an addition between lastDate and date, then modify date
// to be the new addition
//---------------------------------------------------------------------------
void TFreq::ModifyForAddition( RTTime date )
{
    // the last date is the later of the last regular date or the
    // last additional date
    TTime lastDate = max( itsLastAdditionDate, itsCurrentDate );

    int n = itsAdditionsP->getItemsInContainer();
    for (int i = 0; i < n; i++) {

        RTTime addition = (RTTime) (*itsAdditionsP)[i];


        // make sure addition has same time as lastDate and date,
        // so that comparisons work properly
        addition.CopyTime( itsStartDate );

        // if the addition falls between the last date and the
        // new regular date, then return the additional date and
        // update itsLastAdditionalDate
        if ((lastDate < addition) && (addition <= date)) {

            date                = addition;
            itsLastAdditionDate = addition;
            //FIXME freq shouldn't have times!
            date.CopyTime( itsStartDate );
            itsLastAdditionDate.CopyTime( itsStartDate );
            return;
        }
    }

    // if we made it this far, then no additions were found and the
    // itsCurrentDate should be updated
    itsCurrentDate = date;
}


TStreamableClass RegTFreq( "TFreq", TFreq::build, __DELTA(TFreq) );

//---------------------------------------------------------------------------
// read
//---------------------------------------------------------------------------
Pvoid TFreq::read( Ripstream is )
{
    int   version;
    short x;

    is >> version;

    if( version == 1 ) {

        is >> itsStartDate;
        is >> itsEndDate;
        is >> itsCurrentDate;
        is >> itsLastAdditionDate;

        is >> x;  itsFrequencyType = (TFrequencyType) x;
        is >>     itsFrequency;
        is >> x;  itsFrequencyWeek = (TWeekSpecifier) x;
        is >>     itsFrequencyDay;

        // read in the objects in the arrays, since we can't easily
        // read in the arrays themselves
        int   n;
        TTime date;

        is >> n;
        itsAdditionsP = new TXArray( n, 10 );

        for (int i = 0; i < n; i++) {

            is >> date;
            PTTime dateP = new TTime( date );
            itsAdditionsP->add( *dateP );
        }

        is >> n;
        itsExclusionsP = new TXArray( n, 10 );

        for (i = 0; i < n; i++) {

            is >> date;
            PTTime dateP = new TTime( date );
            itsExclusionsP->add( *dateP );
        }
    }
    else {

	    SYSERR( ERR_VER );
    }

    return this;
}


// -----------------------------------------------------------------------
// Write
// -----------------------------------------------------------------------
void TFreq::write( Ropstream os )
{
    os << 1;

    os << itsStartDate;
    os << itsEndDate;
    os << itsCurrentDate;
    os << itsLastAdditionDate;

    os << itsFrequencyType;
    os << itsFrequency;
    os << itsFrequencyWeek;
    os << itsFrequencyDay;

    // write out the objects in the arrays, since we can't easily
    // write out the arrays themselves
    int n = itsAdditionsP->getItemsInContainer();
    os << n;

    for (int i = 0; i < n; i++) {

        RTTime date = (RTTime) (*itsAdditionsP)[i];
        if (date != NOOBJECT) {

            os << date;
        }
    }

    n = itsExclusionsP->getItemsInContainer();
    os << n;

    for (i = 0; i < n; i++) {

        RTTime date = (RTTime) (*itsExclusionsP)[i];
        if (date != NOOBJECT) {

            os << date;
        }
    }
}

