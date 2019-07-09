//*****************************************************************************
// filter object
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "filter.h"

#ifndef __note__
#include "note.h"
#endif
#ifndef __bounds__
#include "bounds.h"
#endif
#ifndef __alarmset__
#include "alarmset.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __message__
#include "message.h"
#endif
#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __cattable__
#include "cattable.h"
#endif

//*****************************************************************************
// copy filter object
//*****************************************************************************
TFilter::TFilter( RTFilter filter )
{
itsSelCatsP= new TCatList( *(filter.itsSelCatsP) );

itsFindText.Set( filter.itsFindText );

itsAllSelected= filter.itsAllSelected;
itsMatchStyle= filter.itsMatchStyle;

itsStart= filter.itsStart;
itsEnd= filter.itsEnd;

itsIncludeUncat= filter.itsIncludeUncat;

for (int i = kFirstExclusion; i <= kLastExclusion; i++)
    itsExclusions[i] = filter.itsExclusions[i];

itsDateEffect = filter.itsDateEffect;
}

//------------------------------------------------------------------
// Constructor
//
// create a filter from specified parameters
//------------------------------------------------------------------
TFilter::TFilter(
    PTCatList pCategories, 
    BOOL selection, TMatchStyle match,
    PTSafStr pSearchText, BOOL excludeDatelessNotes )
{

    itsAllSelected = selection;
    itsMatchStyle  = match;

    if (pSearchText) {    itsFindText       = *pSearchText;   }


    if (pCategories)
       itsSelCatsP    = new TCatList( *pCategories );
    else
       itsSelCatsP    = new TCatList;

    for (int i = kFirstExclusion; i <= kLastExclusion; i++)
        itsExclusions[i] = FALSE;

   itsExclusions[ kExcUndated ]= excludeDatelessNotes;
   itsExclusions[ kExcFloat ]= excludeDatelessNotes;

   itsIncludeUncat = FALSE;
   itsDateEffect   = kAnyDateFilter;
}


//*****************************************************************************

//*****************************************************************************
TFilter::~TFilter()
{
delete itsSelCatsP;
}

//*****************************************************************************
// see if a note meets a filter's criteria
//*****************************************************************************
BOOL TFilter::Match( RTNote note )
{
    //check exclusions

    switch( note.GetStyle() )
        {
        case kItemStyle:
            if( itsExclusions[ kExcUndated ] )
                return FALSE;
            break;
        case kFloatStyle:
            if( itsExclusions[ kExcFloat ] )
                return FALSE;
            break;
        case kTaskStyle:
            if( itsExclusions[ kExcTask ] )
                return FALSE;
            break;
        case kMilestoneStyle:
            if( itsExclusions[ kExcMilestone ] )
                return FALSE;
            break;
        default:
            ;
        }

    switch( note.GetColor().Get() )
        {
        case kRed:
            if( itsExclusions[ kExcRed ] )
                return FALSE;
            break;
        case kRedGray:
            if( itsExclusions[ kExcRedGray ] )
                return FALSE;
            break;
        case kGray:
            if( itsExclusions[ kExcGray ] )
                return FALSE;
            break;
        case kBlueGray:
            if( itsExclusions[ kExcBlueGray ] )
                return FALSE;
            break;
        case kBlue:
            if( itsExclusions[ kExcBlue ] )
                return FALSE;
            break;
        default:
            ;
        }

    if( note.IsDone() )
        {
        if( itsExclusions[ kExcDone ] )
            return FALSE;
        }
    else
        {
        if( itsExclusions[ kExcUndone ] )
            return FALSE;
        }

    if( NULL == note.GetFreq() )
        {
        if( itsExclusions[ kExcNoFreq ] )
            return FALSE;
        }
    else
        {
        if( itsExclusions[ kExcFreq ] )
            return FALSE;
        }

    //FIXME this is an unbelievably inefficient call -- needs work
    if( NULL == TSysState::pAlarms->AlarmForNote( note ) )
        {
        if( itsExclusions[ kExcNoAlarm ] )
            return FALSE;
        }
    else
        {
        if( itsExclusions[ kExcAlarm ] )
            return FALSE;
        }

    // one of these conditions must be true for there to be a category match
    //
    //  - any category at all is supposed to match
    //  - one of the selected categories matches a category in the note
    //  - 'uncategorized' is selected, and the note has no categories

    //NB pCatList.Match() is a relatively expensive operation, so we
    //attempt to exhaust all other checks first.

    if( itsMatchStyle == kCatMatchAny ) {

        //category check
        BOOL anyCatMatches   = itsAllSelected && itsMatchStyle == kCatMatchAny;
        BOOL uncatMatches    = itsIncludeUncat && note.GetCatList().getItemsInContainer() == 0;

        if (! (anyCatMatches || uncatMatches ||
                MatchAnyCat( note.GetCatList() ))) {

            return FALSE;
        }


    } else if (itsMatchStyle == kCatMatchAll) {

        // if <uncategorized> is selected, it has to match as well
        if ( itsIncludeUncat &&
            (note.GetCatList().getItemsInContainer() != 0)) {

            return FALSE;
        }

        // the regular categories all have to match
        if( !MatchEveryCat( note.GetCatList())) {

            return FALSE;
        }

    }

    //date check
    if ((note.GetStyle() == kMilestoneStyle) ||
        (note.GetStyle() == kTaskStyle)) {

        if (!DatesMatch( note )) {

            return FALSE;
        }
    }


    //text check ( if text filter exists )
    if( itsFindText.Len() > 0 ) {
        if( ! note.GetText().Contains( itsFindText ) )
            return FALSE;
    }

    return TRUE;
}

//*****************************************************************************
// PrepareCatTable
//
//  Set up a CatTable in advance of a series of calls
//  to Match which uses the cattable.Careful here: the cat data
//  is not an array of CatLists, it's an array of pointers to
//  catlists. 
//*****************************************************************************
void TFilter::PrepareCatTable( void )
{
    itsCatTableSize = itsSelCatsP->getItemsInContainer();
    itsCatTableData = (PTCatList**) new Pvoid[itsCatTableSize + 1];
   for( int i = 0; i < itsCatTableSize; i++) {

        itsCatTableData[i] = new PTCatList;
        *(itsCatTableData[i]) = NULL;
    }
    itsCatTableP = new TCatTable( itsSelCatsP, (Pvoid*) itsCatTableData );

    itsCatTableSetUp = TRUE;
}

//*****************************************************************************
// DestroyCatTable
//
//  Destroy a CatTable after a series of calls
//  to Match which uses the cattable
//*****************************************************************************
void TFilter::DestroyCatTable( void )
{
    for( int i = 0; i < itsCatTableSize; i++) {

         delete itsCatTableData[i];
    }
    delete itsCatTableData;
    delete itsCatTableP;
    itsCatTableSetUp = FALSE;
}

//*****************************************************************************
// MatchAnyCat
//
//  see if any cat in the catlist matches a selected
//  cat. If itsCatTableSetUp is true then hunt for
//  each cat in the CatTable and return TRUE if
//  one is found.
//*****************************************************************************
BOOL TFilter::MatchAnyCat( RTCatList catList )
{
    if (itsCatTableSetUp) {

        for( int i = 0; i < catList.getItemsInContainer(); i++) {

            RTCat cat = catList[i];

            if (((Pvoid) KEYNOTFOUND) != itsCatTableP->BinarySearch( (long) &cat )) {

                return TRUE;
            }
        }
        return FALSE;
    }
    else {

        SYSERR_STR( "MatchAnyCat: Cat Table not set up" );
        return itsSelCatsP->Match( &catList, kCatMatchAny );
    }
}

//*****************************************************************************
// MatchEveryCat
//
//  see if every cat in the filter also appears in the catlist passed
//  in (which is a note's catlist).
//
//  Since the categories can be found quickly in the filter's
//  cat table, we work by running through each of the cat's in the
//  catlist and hunting for it in the cattable. If it's found, we
//  mark it. If, when the catlist is exhausted, every cat in the
//  cattable is marked, there is a match. The mark used is the
//  address of the catlist itself, which will be unique to each note
//*****************************************************************************
BOOL TFilter::MatchEveryCat( RTCatList catList )
{
    if (itsCatTableSetUp) {

        // if the cat in the cat list is in the cattable,
        // then mark that entry in the cattable
        for( int i = 0; i < catList.getItemsInContainer(); i++) {

            RTCat cat = catList[i];
            PTCatList *data= (PTCatList *)
                itsCatTableP->BinarySearch( (long) &cat );
            if (data != (( PTCatList *) KEYNOTFOUND)) {

                // the thing in the cat table is not a catList,
                // it's a pointer to a cat list
                *data = &catList;
            }
        }

        // see if everybody is marked
        for( i = 0; i < itsCatTableSize; i++) {

            if (*(itsCatTableData[i]) != &catList) {

                return FALSE;
            }
        }
        return TRUE;
    }
    else {

        SYSERR_STR( "MatchEveryCat: Cat Table not set up" );
        return itsSelCatsP->Match( &catList, kCatMatchAll );
    }
}

//*****************************************************************************
// DatesMatch
//
//  See if the note falls within the filter's dates
//*****************************************************************************
BOOL TFilter::DatesMatch( RTNote note )
{
    switch( itsDateEffect ) {

        case kAnyDateFilter:

            break;

        case kBeforeFilter:

            return NoteEndsBefore( note, itsEnd );

        case kAfterFilter:

            return !NoteEndsBefore( note, itsStart );

        case kDayFilter:
        case kWeekFilter:
        case kMonthFilter:
        case kNoteDatesFilter:

            TBounds bounds( itsStart, itsEnd );
            BOOL found = FALSE;
            BOOL overlaps = bounds.NoteOverlaps( note );
            do {

                if (overlaps) {

                    found = TRUE;
                    break;
                }

            } while (bounds.NoteOverlapsAgain( note, overlaps ));

            if (!found) {

                return FALSE;
            }
            break;
    }

    return TRUE;
}

// ---------------------------------------------------
// NoteEndsBefore
//
//  Does the note end before endTime? If the note is
//  a milestone, then does it start before endTime?
//  If the note has a frequency then, by definition,
//  it doesn't end before endTime, unless the frequency
//  itself has a final date before endTime
// ---------------------------------------------------
BOOL TFilter::NoteEndsBefore( RTNote note, RTTime endTime )
{
    PTFreq freqP = note.GetFreq();
    if (freqP) {

        TTime last;
        freqP->Last( last );
        if (!last.HasDate()) return FALSE;
        return( last < endTime );
    }

    TTime end( note.GetEnd() );
    if (end.HasDate()) {

        if (!end.HasTime()) {

            // set to 11:59
            end.SetToMidnightTomorrow();
            end.PlusEquals( k1Min, -1 );
        }
        return (end < endTime);
    }
    else {

        TTime start( note.GetStart() );
        if (!start.HasTime()) {

            start.SetToMidnight();
        }
        return (start < endTime);
    }
}

//*****************************************************************************
//
//*****************************************************************************
BOOL TFilter::IsActive( void )
{
BOOL x= FALSE;
for(int i= kFirstExclusion; i <= kLastExclusion && !x; i++ )
    x= itsExclusions[ i ];

return ( x ||
        itsMatchStyle != kCatMatchAny ||
        itsDateEffect != kAnyDateFilter ||
        itsFindText.Len() > 0
       );
}

//*****************************************************************************

//*****************************************************************************
void TFilter::Exclude( TExclusion x, BOOL b )
{
itsExclusions[ x ]= b;
}

//*****************************************************************************

//*****************************************************************************
BOOL TFilter::IsExcluded( TExclusion x )
{
return itsExclusions[ x ];
}

//*****************************************************************************
// count up selected categories
//*****************************************************************************
int TFilter::NumCats( void )
{
int n= itsSelCatsP->getItemsInContainer();

n+= (itsIncludeUncat) ? 1:0;

return n;
}

//*****************************************************************************
//
//*****************************************************************************
classType TFilter::isA( void ) const
{
return IDFilter;
}

char * TFilter::nameOf( void ) const
{
return "TFilter";
}

hashValueType TFilter::hashValue( void ) const
{
return 0; //FIXME
}

int TFilter::isEqual(const Object & obj) const
{
return( this == & obj );
}

void TFilter::printOn(ostream far&) const
{
//later
}

//*****************************************************************************
// stream support
//*****************************************************************************
TFilter::TFilter( StreamableInit s ):
    itsEnd(s), itsStart(s), itsFindText(s)
{
for( int i= kFirstExclusion; i <= kLastExclusion; i++ )
    itsExclusions[i]= FALSE;
};

PTStreamable TFilter::build( void )
{
return( new TFilter( streamableInit ) );
}

TStreamableClass RegTFilter( "TFilter", TFilter::build, __DELTA(TFilter) );

Pvoid TFilter::read( Ripstream is )
{
int version;

is >> version;

if( version == 4)          // keep track of "today"
    {
    is >> itsSelCatsP;
    is >> itsAllSelected;

    int i;
    is >> i;
    itsMatchStyle= (TMatchStyle) i;

    is >> itsStart;
    is >> itsEnd;
    is >> itsFindText;
    is >> itsIncludeUncat;

    for( int x= kFirstExclusion; x <= kLastExclusion; x++ )
        is >> itsExclusions[x];

    is >> i;
    itsDateEffect = (TDateEffect) i;
    }
else if( version == 3 )          //exclusions
    {
    is >> itsSelCatsP;
    is >> itsAllSelected;

    int i;
    is >> i;
    itsMatchStyle= (TMatchStyle) i;

    is >> itsStart;
    is >> itsEnd;
    is >> itsFindText;
    is >> itsIncludeUncat;

    for( int x= kFirstExclusion; x <= kLastExclusion; x++ )
        is >> itsExclusions[x];

    itsDateEffect = kAnyDateFilter;
    }
else if( version == 2 )          //explicit system cats
    {
    is >> itsSelCatsP;
    is >> itsAllSelected;

    int i;
    is >> i;
    itsMatchStyle= (TMatchStyle) i;

    is >> itsStart;
    is >> itsEnd;
    is >> itsFindText;

    int temp;
    is >> temp;             //bDateLessNotes

    is >> temp;             //fSystemCatTypes[ kUserCatType ];
    is >> itsIncludeUncat;  //fSystemCatTypes[ kUncategorizedCatType ];

    is >> temp;
    itsExclusions[ kExcDone ]= !temp;    //fSystemCatTypes[ kDoneCatType ];

    is >> temp;             //fSystemCatTypes[ kAddressCatType ];

    itsDateEffect = kAnyDateFilter;
    }
else if( version == 1 )      //version 1.00
	{
    is >> itsSelCatsP;
    is >> itsAllSelected;

    int i;
    is >> i;
    itsMatchStyle= (TMatchStyle) i;

    is >> itsStart;
    is >> itsEnd;
    is >> itsFindText;

    int temp;
    is >> temp;

    //version 1 wrote out an array of system cat types -- more
    //than are now used.  For now, we just throw them all out.  Not
    //totally compatible, but I doubt anyone will notice. //hmh
    int n;
    is >> n;
    BOOL t;
    for( i= 0; i <= n; i++ )
        is >> t;        //throw away

    itsDateEffect = kAnyDateFilter;
    }
else
	SYSERR( ERR_VER );

// turn on the individual categories in addition
// to the overriding itsAllSelected, so that the
// catboxes know they're all selected
if (itsAllSelected)
    {
    SelectAll();
    }

if (!itsStart.HasDate()) itsStart.SetNow();
if (!itsEnd.HasDate()) itsEnd.SetNow();

return this;
}

void TFilter::write( Ropstream os )
{

os << (int) 4;  //version

os << itsSelCatsP;
os << itsAllSelected;
os << (int) itsMatchStyle;
os << itsStart;
os << itsEnd;
os << itsFindText;
os << itsIncludeUncat;

//exclusions
for( int i= kFirstExclusion; i <= kLastExclusion; i++ )
    os << (int) itsExclusions[i];

os << itsDateEffect;
}


//*****************************************************************************
// SelectAll
//*****************************************************************************
void TFilter::SelectAll( void )
{
    itsAllSelected = TRUE;
    IncludeUncat( TRUE );
    itsSelCatsP->flush();
}

//*****************************************************************************
// SelectNone
//*****************************************************************************
void TFilter::SelectNone( void )
{
    itsAllSelected = FALSE;
    IncludeUncat( FALSE );
    itsSelCatsP->flush();
}

//*****************************************************************************
// SetFilterDates
//
//    Set itsDateFilter and itsStart and itsEnd
//    based on the filter.
//*****************************************************************************
void TFilter::SetFilterDates( TDateEffect effect, RTTime time )
{
itsDateEffect = effect;

switch( effect )
    {
    case kAnyDateFilter:

        itsStart.Clear();
        itsEnd.Clear();
        break;

    case kBeforeFilter:

        itsStart.SetDate( 1, 1, 1900 );
        itsStart.SetToMidnight();
        itsEnd = time;
        itsEnd.SetToMidnight();
        break;

    case kAfterFilter:

        itsStart = time;
        itsStart.SetToMidnightTomorrow();
        itsEnd.SetDate( 1, 1, 2100 );
        itsEnd.SetToMidnight();
        break;

    case kDayFilter:

        itsStart = time;
        itsStart.SetToMidnight();
        itsEnd = itsStart.Plus( SCALE_DAY, 1 );
        break;

    case kWeekFilter:

        itsStart = time;
        itsStart.SetToMidnight();
        itsStart.PlusEquals( SCALE_DAY, -itsStart.GetWeekDay() );
        itsEnd = itsStart.Plus( SCALE_WEEK, 1 );
        break;

    case kMonthFilter:

        itsStart = time;
        itsStart.SetToMidnight();
        itsStart.SetDay( 1 );
        itsEnd = itsStart.Plus( SCALE_MONTH, 1 );
        break;
    }
}

//*****************************************************************************
// SetFilterDates
//
//    Set itsEnd and itsStart from the time
//*****************************************************************************
void TFilter::SetFilterDates( RTTime time )
{
SetFilterDates( itsDateEffect, time );
}

//*****************************************************************************
// SetFilterDates
//
//    Set itsDateFilter and itsStart and itsEnd
//    based on the times.
//*****************************************************************************
void TFilter::SetFilterDates( RTTime start, RTTime end )
{
    itsStart      = start;
    itsEnd        = end;
    itsDateEffect = kNoteDatesFilter;
}

//*****************************************************************************

//*****************************************************************************
void TFilter::SwitchCats( PTCatTree pCatTree )
{
itsSelCatsP->SwitchCats( pCatTree );
}

//*****************************************************************************
// UpdateFloatingDates
//
//  Check to see if the user wants to update the
//  filter dates to use "today" rather than whatever
//  date was used when the filter was saved
//*****************************************************************************
void TFilter::UpdateFloatingDates( void )
{
    char    message[256];
    char    beforeStr[256];
    char    afterStr[256];

    switch( itsDateEffect ) {

        case kAnyDateFilter:

            return;

        case kBeforeFilter:

            TIntlFmt::FormatDate( itsEnd, kShortDate, beforeStr );
            wsprintf( message, "%s %s. %s",
                "The filter in this view selects notes that end before",
                beforeStr,
                "Would you prefer to select only notes which end before today instead?" );
            break;

        case kAfterFilter:

            TIntlFmt::FormatDate( itsStart.Plus( k1Day, -1 ), kShortDate, afterStr );
            wsprintf( message, "%s %s. %s",
                "The filter in this view selects notes that end after",
                afterStr,
                "Would you prefer to select only notes which end after today instead?" );
            break;

        case kDayFilter:

            TIntlFmt::FormatDate( itsStart, kShortDate, afterStr );
            wsprintf( message, "%s %s. %s",
                "The filter in this view selects notes for",
                afterStr,
                "Would you prefer to select only notes for today instead?" );
            break;

        case kWeekFilter:

            TIntlFmt::FormatDate( itsEnd.Plus( k1Day, -1 ), kShortDate, beforeStr );
            TIntlFmt::FormatDate( itsStart, kShortDate, afterStr );
            wsprintf( message, "%s %s and %s. %s",
                "The filter in this view selects notes between ",
                afterStr, beforeStr,
                "Would you prefer to select only notes for this week instead?" );
            break;

        case kMonthFilter:

            TIntlFmt::FormatDate( itsEnd.Plus( k1Day, -1 ), kShortDate, beforeStr );
            TIntlFmt::FormatDate( itsStart, kShortDate, afterStr );
            wsprintf( message, "%s %s and %s. %s",
                "The filter in this view selects notes between ",
                afterStr, beforeStr,
                "Would you prefer to select only notes for this month instead?" );
            break;
    }

    if (IDOK == TMessageBox::OkAndCancel(
        TPimFrame::pFrame, message, "Update Filter Dates" ))
        {
        TTime time;
        time.SetNow();
        SetFilterDates( time );
        }
}
