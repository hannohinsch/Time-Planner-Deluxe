//*****************************************************************************
// memorized View object and set
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "safstr.h"
#include "note.h"
#include "memView.h"
#include "sysstate.h"
#include "filter.h"

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TMemorizedView::TMemorizedView( void )
{
    itsView             = TSysState::activeView;
    itsTimeScale        = TSysState::timeScale;
    itsFilterP          = new TFilter( *TSysState::pFilter );
    itsTime             = TSysState::selTime;
    itsBreakLevel       = TReportGrid::GetBreakLevel();
    itsDayviewIncrement = TProfile::dayviewIncrement;
    for (int i = 0; i < kMaxReportCols; i++)
        {
        itsColumnFields[i] = TReportGrid::GetColumnField( i );
        }

    for (i = 0; i < kNumNoteOrders; i++)
        {
        itsNoteOrders[i] = TProfile::NoteOrder[i];
        }

    itsName.Set( "" );

    // category grouping
    itsCatGrouping = TSysState::CatGrouping;
    itsGroupCatListP = new TCatList( *TSysState::GroupCatListP );
}

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TMemorizedView::TMemorizedView( const char *name )
{
    itsView         = TSysState::activeView;
    itsTimeScale    = TSysState::timeScale;
    itsFilterP      = new TFilter( *TSysState::pFilter );
    itsTime         = TSysState::selTime;
    itsBreakLevel       = TReportGrid::GetBreakLevel();
    itsDayviewIncrement = TProfile::dayviewIncrement;
    for (int i = 0; i < kMaxReportCols; i++)
        {
        itsColumnFields[i] = TReportGrid::GetColumnField( i );
        }

    for (i = 0; i < kNumNoteOrders; i++)
        {
        itsNoteOrders[i] = TProfile::NoteOrder[i];
        }

    itsName.Set( name );

    // category grouping
    itsCatGrouping = TSysState::CatGrouping;
    itsGroupCatListP = new TCatList( *TSysState::GroupCatListP );
}

// -------------------------------------------------
// Destructor
// -------------------------------------------------
TMemorizedView::~TMemorizedView( void )
{
    if (itsFilterP != NULL) {

       delete itsFilterP;
       itsFilterP = NULL;
    }

    if (itsGroupCatListP != NULL) {

       delete itsGroupCatListP;
       itsGroupCatListP = NULL;
    }
}

// -------------------------------------------------
// IsEqual
// -------------------------------------------------
int TMemorizedView::isEqual( const Object &other ) const
{
    return( ((RTMemorizedView) *this).itsName ==
            ((RTMemorizedView) other).itsName );
}

// -------------------------------------------------
// isLessThan
// -------------------------------------------------
int TMemorizedView::isLessThan( const Object &other ) const
{
    return( ((RTMemorizedView) *this).itsName <
            ((RTMemorizedView) other).itsName );
}


// -------------------------------------------------
// Select
//
//  Set up the system based on the contents of the view
// -------------------------------------------------
void TMemorizedView::Select( void )
{
    BOOL useDayIncrement = FALSE;
    BOOL useBreakLevel   = FALSE;
    BOOL useColumnNames  = FALSE;
    BOOL useNoteOrder    = FALSE;
    BOOL useTimeScale    = FALSE;
    BOOL useCatGrouping  = FALSE;

    switch( itsView ) {

        case kCalView:

            useDayIncrement = (itsTimeScale < SCALE_WEEK);
            useTimeScale    = TRUE;
            break;

        case kReportView:

            useBreakLevel   = TRUE;
            useColumnNames  = TRUE;
            useNoteOrder    = TRUE;
            useCatGrouping  = TRUE;
            break;

        case kTimeView:

            useNoteOrder    = TRUE;
            useTimeScale    = TRUE;
            useCatGrouping  = TRUE;
            break;

        case kBrowView:

            useNoteOrder    = TRUE;
            useCatGrouping  = TRUE;
            break;

        case kGrafView:

            useTimeScale    = TRUE;
            break;

        default:
            SYSERR( ERR_GEN );
            break;
    }


    // set the time units for the calendars
    if (useDayIncrement)
        {
        TProfile::dayviewIncrement =
            itsDayviewIncrement;
        }

    // set the report grid defaults
    if (useBreakLevel) 
        {
        TReportGrid::SetBreakLevel( itsBreakLevel );
        }

    if (useColumnNames)
        {
        for( int i= 0; i < kMaxReportCols; i++ )
            {
            TReportGrid::SetColumnField( i,
                itsColumnFields[i] );
            }
        }

    // restore the note order
    if (useNoteOrder)
        {
        for (int g = 0; g < kNumNoteOrders; g++)
            {
            TProfile::NoteOrder[g] = itsNoteOrders[g];
            }
        }

    // category grouping
    if (useCatGrouping)
        {
        TSysState::CatGrouping = itsCatGrouping;
        delete TSysState::GroupCatListP;
        TSysState::GroupCatListP = new TCatList(
            *itsGroupCatListP );
        }

    // change the filter and thereafter the noteset
    delete TSysState::pFilter;
    TSysState::pFilter = new TFilter( *itsFilterP );
    TSysState::pFilter->UpdateFloatingDates();

    delete TSysState::pSelNotes;
    TSysState::pSelNotes= new TNoteSet( *(TSysState::pFilter) );

    // switch views
    TTimeScale timeScale = (useTimeScale) ? itsTimeScale : TSysState::timeScale;

    TPimFrame::pFrame->UpdateView(
        timeScale, TSysState::selTime, itsView );
}

//*****************************************************************************

//*****************************************************************************
void TMemorizedView::SwitchCats( PTCatTree pTree )
{
itsGroupCatListP->SwitchCats( pTree );
itsFilterP->SwitchCats( pTree );
}

//*****************************************************************************
// stream support
//*****************************************************************************
PTStreamable TMemorizedView::build( void )
{
    return( new TMemorizedView( streamableInit ) );
}

TStreamableClass RegTMemorizedView( "TMemorizedView", TMemorizedView::build, __DELTA(TMemorizedView) );

//*****************************************************************************

//*****************************************************************************
Pvoid TMemorizedView::read( Ripstream is )
{
    int version;
    int temp;

    is >> version;

    // version 2 added category grouping
    if( version == 2 )
    	{
        is >> temp;
        itsView = (TActiveView) temp;
        is >> temp;
        itsTimeScale = (TTimeScale) temp;
        is >> itsTime;
        is >> itsName;
        is >> itsFilterP;
        is >> itsBreakLevel;
        is >> (int) itsDayviewIncrement;

        for (int i = 0; i < kMaxReportCols; i++)
            {
            is >> (int) itsColumnFields[i];
            }

        for (i = 0; i < kNumNoteOrders; i++)
            {
            is >> (int) itsNoteOrders[i];
            }

        is >> (int) itsCatGrouping;
        is >> itsGroupCatListP;

	    }
    else if( version == 1 )
    	{
        is >> temp;
        itsView = (TActiveView) temp;
        is >> temp;
        itsTimeScale = (TTimeScale) temp;
        is >> itsTime;
        is >> itsName;
        is >> itsFilterP;
        is >> itsBreakLevel;
        is >> (int) itsDayviewIncrement;

        for (int i = 0; i < kMaxReportCols; i++)
            {
            is >> (int) itsColumnFields[i];
            }

        for (i = 0; i < kNumNoteOrders; i++)
            {
            is >> (int) itsNoteOrders[i];
            }

        itsCatGrouping = kAutomaticCatGrouping;
        itsGroupCatListP = new TCatList;
	    }
    else
    	SYSERR( ERR_VER );

    return this;
}


//*****************************************************************************

//*****************************************************************************
void TMemorizedView::write( Ropstream os )
{

    // errors are caught locally so we can clean up, and signalled upward
    CATCHBUF cBuf;
    LPCATCHBUF pOldCBuf= TError::SetCatchBuf( cBuf );
    TErrCode rc= (TErrCode) Catch( cBuf );
    if( rc == 0 )
        {
        os << (int) 2;  //version

        os << itsView;
        os << itsTimeScale;
        os << itsTime;
        os << itsName;
        os << itsFilterP;

        os << itsBreakLevel;
        os << itsDayviewIncrement;

        for (int i = 0; i < kMaxReportCols; i++)
            {
            os << itsColumnFields[i];
            }

        for (i = 0; i < kNumNoteOrders; i++)
            {
            os << itsNoteOrders[i];
            }

        os << itsCatGrouping;
        os << itsGroupCatListP;
        }

    //restore saved error handler
    TError::SetCatchBuf( pOldCBuf );

    //if error, signal to caller
    if( rc )
        SYSTHROW( rc );
}

//*****************************************************************************
// HealthCheck
//
//  Makes sure the view has a valid name and filter
//*****************************************************************************
BOOL TMemorizedView::HealthCheck( void )
{
    if (!itsName.HealthCheck())
        return FALSE;

    if ((itsFilterP == NULL) ||
        (itsFilterP->isA() != IDFilter))
        return FALSE;

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//////////////////////// TMemViewSet ////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//*****************************************************************************

//*****************************************************************************
RTMemorizedView TMemViewSet::FindView( RTMemorizedView view )
{
int n= getItemsInContainer();
for( int i= 0; i < n; i++ )
    {
    if( view.isEqual( (RTMemorizedView) (*this)[i] ))
        return (RTMemorizedView) (*this)[i];
    }

return (RTMemorizedView) NOOBJECT;
}

//*****************************************************************************
// stream support
//*****************************************************************************
PTStreamable TMemViewSet::build( void )
{
return( new TMemViewSet( streamableInit ) );
}

TStreamableClass RegTMemViewSet( "TMemViewSet", TMemViewSet::build, __DELTA(TMemViewSet) );

//*****************************************************************************

//*****************************************************************************
Pvoid TMemViewSet::read( Ripstream is )
{
int version, num;

is >> version;

if( version == 1 )
    {
    is >> num;

    for( int i= 0; i < num; i++ )
		{
        PTMemorizedView pView;

        is >> pView;
        add( *pView );
		}
    }
else
	SYSERR( ERR_VER );

return this;
}


//*****************************************************************************

//*****************************************************************************
void TMemViewSet::write( Ropstream os )
{

os << (int) 1;  //version

os << getItemsInContainer();

int n= getItemsInContainer();

// errors are caught locally so we can clean up, and signalled upward
CATCHBUF cBuf;
LPCATCHBUF pOldCBuf= TError::SetCatchBuf( cBuf );
TErrCode rc= (TErrCode) Catch( cBuf );
if( rc == 0 )
    {
    for( int i=0; i < n; i++ )
        {
        RTMemorizedView thing= (RTMemorizedView) (*this)[i];

        os << &thing;
        }
     }

//restore saved error handler
TError::SetCatchBuf( pOldCBuf );

//signal error to caller
if( rc )
    SYSTHROW( rc );
}


//*****************************************************************************
// HealthCheck
//
//  Make sure all the memorized views are consistent
//*****************************************************************************
BOOL TMemViewSet::HealthCheck( void )
{
int n= getItemsInContainer();
for( int i = 0; i < n; i++ )
	{
    RTMemorizedView view = (RTMemorizedView) (*this)[i];
    if (!view.HealthCheck())
        return FALSE;
	}

return TRUE;
}
