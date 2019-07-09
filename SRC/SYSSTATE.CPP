//*****************************************************************************
// system state: esp. the global data containers
// and the reading and writing thereof
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

//
//paranoid error checking?
//
#define SAFE_READ       FALSE
#define SAFE_WRITE      FALSE

#include "sysstate.h"
#include "error.h"
#include "frame.h"

#ifndef __memnote__
#include "memnote.h"
#endif
#ifndef __memview__
#include "memview.h"
#endif
#ifndef __cat__
#include "cat.h"
#endif
#ifndef __catlist__
#include "catlist.h"
#endif
#ifndef __cattree__
#include "cattree.h"
#endif
#ifndef __note__
#include "note.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __alarmset__
#include "alarmset.h"
#endif
#ifndef __filter__
#include "filter.h"
#endif
#ifndef __cursor__
#include "cursor.h"
#endif
#ifndef __oem__
#include "oem.h"
#endif

//needed for file rename() function
#include "stdio.h"

//this string has to appear at the beginning of every .htp file
#define FILE_HEADER "Hinsch Time Planner data file"
#define FILE_HEADER_LEN 50

PTCatTree       TSysState::pCategories= NULL;
PTNoteSet       TSysState::pAllNotes= NULL;
PTMemNoteSet    TSysState::pMemNotes= NULL;
PTMemViewSet    TSysState::pMemViews= NULL;
PTAlarmSet      TSysState::pAlarms= NULL;
PTFilter        TSysState::pFilter= NULL;
PTNoteSet       TSysState::pSelNotes= NULL;
BOOL            TSysState::bSystemDirty= FALSE;
int             TSysState::nextNoteID= 0;
TTime           TSysState::selTime;
TTimeScale      TSysState::timeScale= SCALE_DAY;
int             TSysState::currentNoteIndex= 0;
BOOL            TSysState::bShowFullText= TRUE;
BOOL            TSysState::bShowBars= FALSE;
TActiveView     TSysState::activeView;
TCatGrouping    TSysState::CatGrouping= kAutomaticCatGrouping;
PTCatList       TSysState::GroupCatListP = NULL;

//*****************************************************************************
// this is used for object's containers, not global containers
//*****************************************************************************
TSysState::TSysState( void)
{
its_pFilter= NULL;
}


//*****************************************************************************
// delete global objects
//*****************************************************************************
TSysState::~TSysState( void )
{
if( its_pFilter != NULL )
    delete its_pFilter;
}


//*****************************************************************************

//*****************************************************************************
void TSysState::CreateContainers( void )
{
pCategories= new TCatTree( (PTCat) NULL );
//if mem error ?

pAllNotes= new TNoteSet;
//if mem error ?

pMemNotes= new TMemNoteSet();
pMemViews= new TMemViewSet();
pAlarms = new TAlarmSet;

//set null filter
pFilter= new TFilter();

//calculate active note set
pSelNotes= new TNoteSet( *pFilter );

bSystemDirty= FALSE;

// category grouping
CatGrouping = kAutomaticCatGrouping;
GroupCatListP = new TCatList;
}

//*****************************************************************************

//*****************************************************************************
void TSysState::DeleteContainers( void )
{

delete pSelNotes;
pSelNotes= NULL;

if( pFilter )
    delete pFilter;

pFilter= NULL;

if( pAllNotes )
    pAllNotes->flush( TXArray::kDelete );
delete pAllNotes;
pAllNotes= NULL;

if( pMemNotes )
    pMemNotes->flush( TXArray::kDelete );
delete pMemNotes;
pMemNotes= NULL;

if( pMemViews )
    {
    pMemViews->flush( TXArray::kDelete );
    }

delete pMemViews;
pMemViews= NULL;

// (categories will be deleted with pCategories)
if( GroupCatListP )
    GroupCatListP->flush( TXArray::kNoDelete );
delete GroupCatListP;
GroupCatListP= NULL;

if( pCategories )
    pCategories->flush( TXArray::kDelete );
delete pCategories;
pCategories= NULL;

if( pAlarms )
    pAlarms->flush( TXArray::kDelete );
delete pAlarms;
pAlarms= NULL;
}

//*****************************************************************************
// clean out containers
//*****************************************************************************
void TSysState::FlushSystem( void )
{
DeleteContainers();
CreateContainers();
}

//*****************************************************************************
// object paraphenalia
//*****************************************************************************
classType TSysState::isA( void ) const
{
return IDSysState;
}

char * TSysState::nameOf( void ) const
{
return "TSysState";
}

hashValueType TSysState::hashValue( void ) const
{
return 0; //FIXME
}

int TSysState::isEqual(const Object & obj) const
{
return( this == & obj );
}

void TSysState::printOn(ostream far&) const
{
//later
}

//*****************************************************************************
// stream support
//*****************************************************************************
TSysState::TSysState( StreamableInit s ): its_selTime( s )
{
}

PTStreamable TSysState::build( void )
{
return( new TSysState( streamableInit ) );
}

TStreamableClass RegTSysState( "TSysState", TSysState::build, __DELTA(TSysState) );

//*****************************************************************************
// read state values from file into object
//*****************************************************************************
Pvoid TSysState::read( Ripstream is )
{
int version;

is >> version;

if( version == 2 )
	{
    is >> its_pFilter;
    is >> its_selTime;

    int i;
    is >> i;
    its_timeScale= (TTimeScale) i;

    is >> its_currentNoteIndex;
    is >> bShowFullText;

    is >> i;
    its_activeView= (TActiveView) i;

    is >> bShowBars;
	}
else if( version == 1 )
	{
    is >> its_pFilter;
    is >> its_selTime;

    int i;
    is >> i;
    its_timeScale= (TTimeScale) i;

    is >> its_currentNoteIndex;
    is >> bShowFullText;

    is >> i;
    its_activeView= (TActiveView) i;

    bShowBars = FALSE;
	}
else
	SYSERR( ERR_VER );

return this;
}

//*****************************************************************************
// the tricky thing here is that sysstate actually writes out the PUBLIC values
// When read in, though, they stay private to the SysState object unless
// explicitly moved to the public variables
//*****************************************************************************
void TSysState::write( Ropstream os )
{

os << (int) 2;  //version

os << pFilter;
os << selTime;

os << (int) timeScale;
os << currentNoteIndex;
os << bShowFullText;

os << (int) activeView;

os << bShowBars;
}

//*****************************************************************************

//*****************************************************************************
BOOL TSysState::ArchiveFile( char *fName )
{
BOOL rc;

TCursor::Busy( TRUE );

// create dummies to keep WriteContainers happy,
PTMemNoteSet    pTempMemNotes = new TMemNoteSet();
PTMemViewSet    pTempMemViews = new TMemViewSet();
PTCatList       pTempGroupCatList = new TCatList;

// create temporary category tree which contains only those
// categories that are used by selected notes
PTNoteSet pTempNotes = new TNoteSet( *pFilter, pSelNotes );
MarkSelectedCats( pCategories, *pTempNotes );
PTCatTree pTempCatTree = new TCatTree( pCategories, FALSE );

SwitchCats( pTempCatTree, *pTempNotes );

// make up a dummy filter to write out
PTFilter filterP = pFilter;
pFilter = new TFilter;

// create an alarm set for all notes in pTempNotes that have alarms
//FIXME this might be quite a bit faster if it iterated through the
//alarm set, looking for notes that were in pTempNotes /hmh
PTAlarmSet pTempAlarms = new TAlarmSet();
for (int i = 0; i < pTempNotes->getItemsInContainer(); i++) {

    RTNote note = (*pTempNotes)[i];
    if (note == NOOBJECT) break;

    PTAlarm alarmP =  pAlarms->AlarmForNote( note );
    if (alarmP != NULL) {

        pTempAlarms->add( *alarmP );
    }
}

//write selected notes
rc= WriteContainers( fName, pTempCatTree, pTempNotes,
    pTempMemNotes, pTempMemViews, pTempAlarms,
    pTempGroupCatList  );

//switch notes back to standard categories
SwitchCats( pCategories, *pTempNotes );

// restore the real filter
delete pFilter;
pFilter = filterP;


// delete our temporary containers
delete pTempMemNotes;
delete pTempMemViews;
delete pTempNotes;
delete pTempGroupCatList;

pTempCatTree->flush( TXArray::kDelete );
delete pTempCatTree;

delete pTempAlarms;

TCursor::Busy( FALSE );

return rc;
}

//*****************************************************************************
// write all stuff in containers
// return TRUE iff ok
//
//FIXME the read/write routines inconsistently return 0 or TRUE on success
//*****************************************************************************
BOOL TSysState::WriteFile( char *fName )
{
BOOL rc= WriteContainers( fName, pCategories,
            pAllNotes, pMemNotes,
            pMemViews, pAlarms,
            GroupCatListP );
if( rc )
    bSystemDirty= FALSE;

return rc;
}

//*****************************************************************************
// write object stream; return TRUE if ok
//*****************************************************************************
BOOL TSysState::WriteContainers(
    char *aFName, PTCatTree aCatTreeP,
    PTNoteSet aNoteSetP, PTMemNoteSet aMemNoteP,
    PTMemViewSet aMemViewP, PTAlarmSet aAlarmSetP,
    PTCatList aGroupCatListP )
{
char bakPath[ MAXPATH ];
char bakDrive[ MAXPATH ];
char bakDir[ MAXPATH ];
char bakName[ MAXPATH ];
char bakExt[ MAXPATH ];

TCursor::Busy( TRUE );

TErrCode rc= (TErrCode) 0;  //FIXME make equate for ERR_NONE !

//create temp file name
char tempFile[ MAXPATH ];
fnsplit( aFName, bakDrive, bakDir, bakName, bakExt );
fnmerge( tempFile, bakDrive, bakDir, "TEMP", TOEM::fileExt );

//open temp file and write data
if( rc == 0 )
    {
    ofpstream os( tempFile, ios::trunc );

    if( os.good() )
        {
        CATCHBUF cBuf;
        LPCATCHBUF pOldCBuf= TError::SetCatchBuf( cBuf );
        rc= (TErrCode) Catch( cBuf );
        if( rc == 0 )
            {
            os.writeString( FILE_HEADER );

            // version 7 adds category grouping
            os << (int) 7;      //version number

            os << aCatTreeP;
            os << aNoteSetP;
            os << aMemNoteP;
            os << aAlarmSetP;

            TSysState sysState;     //create on the fly
            os << &sysState;        //...write as ptr
            os << aMemViewP;
            os << aGroupCatListP;
            os << CatGrouping;
            }
        //restore saved error handler
        TError::SetCatchBuf( pOldCBuf );

        os.close();

        if( !os.good() )
            rc=  ERR_OBJSTRM;
        }
    }

#if SAFE_WRITE
// now read temp file to see if it's ok
if( rc == 0 )
    {
    ifpstream is( tempFile );
    BOOL tempCatGrouping;

    if( is.good() )
        {
        PTCatTree pTempCats= NULL;
        PTNoteSet pTempNotes= NULL;
        PTMemNoteSet pTempMemNotes= NULL;
        PTMemViewSet pTempMemViews= NULL;
        PTAlarmSet pTempAlarms= NULL;
        PTSysState pTempSysState= NULL;
        PTCatList  pTempGroupCatList= NULL;

        CATCHBUF cBuf;
        LPCATCHBUF pOldCBuf= TError::SetCatchBuf( cBuf );
        rc= (TErrCode) Catch( cBuf );
        if( rc == 0 )
            {
            char buf[ FILE_HEADER_LEN ];
            //FIXME oughta check this...
            is.freadString( buf, FILE_HEADER_LEN );

            int version;
            is >> version;
            // version 7: added category grouping
            if (version == 7)
                {
                is >> pTempCats;
                is >> pTempNotes;
                is >> pTempMemNotes;
                is >> pTempAlarms;
                is >> pTempSysState;
                is >> pTempMemViews;
                is >> pTempGroupCatList;
                is >> tempCatGrouping;
                }
            else if (version == 6)
                {
                is >> pTempCats;
                is >> pTempNotes;
                is >> pTempMemNotes;
                is >> pTempAlarms;
                is >> pTempSysState;
                is >> pTempMemViews;
                pTempGroupCatList = new TCatList;
                tempCatGrouping = kAutomaticCatGrouping;
                }
            else if( version == 5 )
                {
                is >> pTempCats;
                is >> pTempNotes;
                is >> pTempMemNotes;
                is >> pTempAlarms;
                is >> pTempSysState;
                pTempMemViews = new TMemViewSet;
                pTempGroupCatList = new TCatList;
                tempCatGrouping = kAutomaticCatGrouping;
                }
            else
                SYSTHROW( ERR_VERSION );

            }

        //restore saved error handler
        TError::SetCatchBuf( pOldCBuf );

        //now delete containers that we just read in
        if( pTempMemNotes )
            {
            pTempMemNotes->flush( TXArray::kDelete );
            delete pTempMemNotes;
            }

        if( pTempMemViews )
            {
            pTempMemViews->flush( TXArray::kDelete );
            delete pTempMemViews;
            }

        if( pTempGroupCatList )
            {
            pTempGroupCatList->flush( TXArray::kNoDelete );
            delete pTempGroupCatList;
            }

        if( pTempNotes )
            {
            pTempNotes->flush( TXArray::kDelete );
            delete pTempNotes;
            }

        if( pTempCats )
            {
            pTempCats->flush( TXArray::kDelete );
            delete pTempCats;
            }

        if( pTempAlarms )
            {
            pTempAlarms->flush( TXArray::kDelete );
            delete pTempAlarms;
            }

        if( pTempSysState )
            delete pTempSysState;

        }
    else
        rc= ERR_OBJSTRM;

    if( !is.good() )    //read failed?
        rc=  ERR_OBJSTRM;

    is.close();
    }
#endif

//file is readable; give it its rightful name
if( rc == 0 )
    {
    //rename (possibly) existing file (give it .~xx extension )

    //figure out backup file name
    fnsplit( aFName, bakDrive, bakDir, bakName, bakExt );
    fnmerge( bakPath, bakDrive, bakDir, bakName, TOEM::backupExt );

    //get rid of existing backup (will fail soft if not exist)
    unlink( bakPath );
    //rename current file to backup name (will fail soft if not exist)
    rename( aFName, bakPath );

    //rename temp file to fname
    // this op should always work!
    if( 0 != rename( tempFile, aFName ) )
        rc= ERR_OBJSTRM;
    }
else
    {
    unlink( tempFile );        //something went wrong, erase temp file
    }

TCursor::Busy( FALSE );

return !rc;     //rc == 0 iff ok; return == TRUE if ok
}

//*****************************************************************************
// read containers from object stream
//*****************************************************************************
BOOL TSysState::ReadFile( char *fName, BOOL /*checkWorkTime*/ )
{
int version;
BOOL rc= 0;
CATCHBUF cBuf;
BOOL bOpenError= FALSE;
PTSysState pSysState= NULL;

TCursor::Busy( TRUE );

LPCATCHBUF pOldCBuf= TError::SetCatchBuf( cBuf );
rc= Catch( cBuf );
if( rc != 0 )
    {
    TError::ProcessSysCatch();      //show error msg

    //we had some sort of nasty error reading a file; so the global containers
    // are in an uncertain state.  To avoid problems, a complete new (empty) set
    // of containers is allocated.
    FlushSystem();

    //delete filter and selected noteset
    delete pFilter;
    delete pSelNotes;
    }
else
    {
    ifpstream is( fName );

    if( is.good() )
        {
        //NB sets all global container pointers to NULL
        DeleteContainers();

        //check file header string
        char buf[ FILE_HEADER_LEN ];
        is.freadString( buf, FILE_HEADER_LEN );

        if( strcmp( buf, FILE_HEADER ))
            {   //not one of our files!
            is.close();
            SYSTHROW( ERR_HEADER );
            }

        is >> version;

        if( version == 7 )      //introduced category grouping;
            {                   //also, notes can have worktime or elapsed time
            int i;

            is >> pCategories;

            #if SAFE_READ
            if( !pCategories->HealthCheck() )
               SYSTHROW( ERR_HEALTH );
            #endif

            is >> pAllNotes;

            #if SAFE_READ
            if( !pAllNotes->HealthCheck() )
                SYSTHROW( ERR_HEALTH );
            #endif

            is >> pMemNotes;

            is >> pAlarms;

            is >> pSysState;
            is >> pMemViews;
            is >> GroupCatListP;
            is >> i;
            CatGrouping = (TCatGrouping) i;
            }
        else if( version == 6 )     //introduced memorized views
            {
            is >> pCategories;

            #if SAFE_READ
            if( !pCategories->HealthCheck() )
               SYSTHROW( ERR_HEALTH );
            #endif

            is >> pAllNotes;

            #if SAFE_READ
            if( !pAllNotes->HealthCheck() )
                SYSTHROW( ERR_HEALTH );
            #endif

            is >> pMemNotes;

            is >> pAlarms;

            is >> pSysState;
            is >> pMemViews;

            GroupCatListP = new TCatList;
            CatGrouping = kAutomaticCatGrouping;
            }
        else if( version == 5 )     //htp version 1.00
            {
            is >> pCategories;

            #if SAFE_READ
            if( !pCategories->HealthCheck() )
               SYSTHROW( ERR_HEALTH );
            #endif

            is >> pAllNotes;

            #if SAFE_READ
            if( !pAllNotes->HealthCheck() )
                SYSTHROW( ERR_HEALTH );
            #endif

            is >> pMemNotes;

            is >> pAlarms;

            is >> pSysState;

            pMemViews= new TMemViewSet();
            GroupCatListP = new TCatList;
            CatGrouping = kAutomaticCatGrouping;
            }
        else if( version == 4 )
            {
            is >> pCategories;

            #if SAFE_READ
            if( !pCategories->HealthCheck() )
               SYSTHROW( ERR_HEALTH );
            #endif

            is >> pAllNotes;

            #if SAFE_READ
            if( !pAllNotes->HealthCheck() )
                SYSTHROW( ERR_HEALTH );
            #endif

            is >> pMemNotes;

            is >> pAlarms;
            pMemViews= new TMemViewSet();
            GroupCatListP = new TCatList;
            CatGrouping = kAutomaticCatGrouping;
            }
        else
            {
            SYSTHROW( ERR_VERSION );
            }

        is.close();

        //check that we successfully created containers
        if( pCategories == NULL )
            SYSTHROW( ERR_NULLPTR );

        if( pAllNotes == NULL )
            SYSTHROW( ERR_NULLPTR );

        if( pMemNotes == NULL )
            SYSTHROW( ERR_NULLPTR );

        if( pMemViews == NULL )
            SYSTHROW( ERR_NULLPTR );

        if( GroupCatListP == NULL )
            SYSTHROW( ERR_NULLPTR );

        if( pAlarms == NULL )
            SYSTHROW( ERR_NULLPTR );

        }
     else
        {
        rc= -1;     //file open error
        bOpenError= TRUE;
        }

     } //protected exception block (file read)

//restore previous error buffer
TError::SetCatchBuf( pOldCBuf );

//by the time we get here, we'll have a new set of containers
//either from the file, or, if there were errors, an empty
//set from the exception handler
// However, we will not have a global filter object

if( !bOpenError )
    {
    if( pSysState != NULL )     //we got a new system state (version >= 5)
        {
        if( pFilter )
            delete pFilter;

        pFilter= pSysState->its_pFilter;
        pSysState->its_pFilter= NULL;      //avoid pSysState filter deletion
        }
    else
        pFilter= new TFilter();        //older files

    pSelNotes= new TNoteSet( *(pFilter) );

    // now update the view
    // this includes setting a new system time and timescale from file
    if( pSysState )
        {
        TPimFrame::pFrame->UpdateView( pSysState->its_timeScale,
                                       pSysState->its_selTime,
                                       pSysState->its_activeView );
        }

    bSystemDirty= FALSE;
    }


if( pSysState )
    delete pSysState;

TCursor::Busy( FALSE );

return !rc;     //rc == 0 if no errors; result == TRUE if no errors
}

//*****************************************************************************
// MergeFile
//
//     Merge another file into this one
//*****************************************************************************
BOOL TSysState::MergeFile( char *fName )
{
int version;
CATCHBUF cBuf;
BOOL rc= 0;
PTSysState pSysState;

TCursor::Busy( TRUE );

// because the read method for categories requires that each
// category be in pCategories, in order for notes
// to be read in properly, the categories read in from the file
// have to be in pCategories when "is >> pNotes" is
// called. Since we don't want to mix old categories and new,
// we save the value of pCategories, set it to be
// the categories read in from the file, read in the notes,
// and then reset the original value of pCategories,
// before merging the new categories in

PTCatTree pOldCategories = pCategories;
pCategories     = NULL;

PTNoteSet    pTempNotes    = NULL;
PTAlarmSet   pTempAlarms   = NULL;
PTMemNoteSet pTempMemNotes = NULL;
PTMemViewSet pTempMemViews = NULL;
PTCatList    pTempGroupCatList = NULL;

LPCATCHBUF pOldCBuf= TError::SetCatchBuf( cBuf );
rc= Catch( cBuf );
if( rc != 0 )
    {
    TError::ProcessSysCatch();

    // throw away the new stuff
    if( pTempGroupCatList )
        pTempGroupCatList->flush( TXArray::kNoDelete );
    delete pTempGroupCatList;

    if( pCategories )
        pCategories->flush( TXArray::kDelete );
    delete pCategories;

    if( pTempNotes )
        pTempNotes->flush( TXArray::kDelete );
    delete pTempNotes;

    if( pTempAlarms )
        pTempAlarms->flush( TXArray::kDelete );
    delete pTempAlarms;

    if( pTempMemNotes )
        pTempMemNotes->flush( TXArray::kDelete );
    delete pTempMemNotes;

    if( pTempMemViews )
        pTempMemViews->flush( TXArray::kDelete );
    delete pTempMemViews;

    }
else
	{
    ifpstream is( fName );

    if( is.good() ) 
        {
        BOOL tempCatGrouping;

        //check file header string
        char buf[ FILE_HEADER_LEN ];
        is.freadString( buf, FILE_HEADER_LEN );

        if( strcmp( buf, FILE_HEADER ))
            {   //not one of our files!
            is.close();
            SYSTHROW( ERR_HEADER );
            }
    
		is >> version;

        // version 7 introduces category grouping
        // also, notes in worktime or elapsed time
        if( version == 7 )
        	{
            // read categories into global list
            is >> pCategories;

            #if SAFE_READ
            if( !pCategories->HealthCheck() )
                SYSTHROW( ERR_HEALTH );
            #endif

	        // read notes into new list
            is >> pTempNotes;

            #if SAFE_READ
            if( !pTempNotes->HealthCheck() )
                SYSTHROW( ERR_HEALTH );
            #endif

            // read in memorized notes
            is >> pTempMemNotes;

            #if SAFE_READ
            if( !pTempMemNotes->HealthCheck() )
                SYSTHROW( ERR_HEALTH );
            #endif


            // read in alarms and
            is >> pTempAlarms;

            // ignore system state
            is >> pSysState;
            delete pSysState;

            // read in memorized views
            is >> pTempMemViews;

            #if SAFE_READ
            if( !pTempMemViews->HealthCheck() )
                SYSTHROW( ERR_HEALTH );
            #endif

            // read in category groups
            is >> pTempGroupCatList;

            #if SAFE_READ
            if( !pTempGroupCatList->HealthCheck() )
                SYSTHROW( ERR_HEALTH );
            #endif

            is >> tempCatGrouping;
        	}
        // version 6 introduces memorized views
        // and merges memorized notes and views
        else if( version == 6 )
        	{
            // read categories into global list
            is >> pCategories;

            #if SAFE_READ
            if( !pCategories->HealthCheck() )
                SYSTHROW( ERR_HEALTH );
            #endif

	        // read notes into new list
            is >> pTempNotes;

            #if SAFE_READ
            if( !pTempNotes->HealthCheck() )
                SYSTHROW( ERR_HEALTH );
            #endif

            // read in memorized notes
            is >> pTempMemNotes;

            // read in alarms and
            is >> pTempAlarms;

            // ignore system state
            is >> pSysState;
            delete pSysState;

            // read in memorized views
            is >> pTempMemViews;

            pTempGroupCatList = new TCatList;
            tempCatGrouping = kAutomaticCatGrouping;
        	}
        else if( version == 5 )     //htp version 1.00
        	{
            // read categories into global list
            is >> pCategories;

            #if SAFE_READ
            if( !pCategories->HealthCheck() )
                SYSTHROW( ERR_HEALTH );
            #endif

	        // read notes into new list
            is >> pTempNotes;

            #if SAFE_READ
            if( !pTempNotes->HealthCheck() )
                SYSTHROW( ERR_HEALTH );
            #endif

            // read in memorized notes and dispose of them right away
            is >> pTempMemNotes;

            // read in alarms and
            is >> pTempAlarms;

            // ignore system state
            is >> pSysState;
            delete pSysState;

            // set up some temporary view
            pTempMemViews = new TMemViewSet;
            pTempGroupCatList = new TCatList;
            tempCatGrouping = kAutomaticCatGrouping;
        	}
        else if( version == 4 )
        	{
            // read categories into global list
            is >> pCategories;

            #if SAFE_READ
            if( !pCategories->HealthCheck() )
                SYSTHROW( ERR_HEALTH );
            #endif

	        // read notes into new list
            is >> pTempNotes;

            #if SAFE_READ
            if( !pTempNotes->HealthCheck() )
                SYSTHROW( ERR_HEALTH );
            #endif

            // read in memorized notes
            is >> pTempMemNotes;

            // read in alarms
            is >> pTempAlarms;

            // set up some temporary view
            pTempMemViews = new TMemViewSet;
            pTempGroupCatList = new TCatList;
            tempCatGrouping = kAutomaticCatGrouping;
        	}
        else        //unknown version
        	{
            SYSTHROW( ERR_VERSION );
			}

        is.close();

        //sanity checks
        if( pCategories == NULL )
            SYSTHROW( ERR_NULLPTR );

        if( pTempNotes == NULL )
            SYSTHROW( ERR_NULLPTR );

        if( pTempMemNotes == NULL )
            SYSTHROW( ERR_NULLPTR );

        if( pTempMemViews == NULL )
            SYSTHROW( ERR_NULLPTR );

        if( pTempGroupCatList == NULL )
            SYSTHROW( ERR_NULLPTR );

        //check now,  BEFORE reswitching pointers!
        #if SAFE_READ
        if( !pCategories->HealthCheck() )
            SYSTHROW( ERR_HEALTH );

        if( !pTempNotes->HealthCheck() )
            SYSTHROW( ERR_HEALTH );

        if( !pTempMemNotes->HealthCheck() )
            SYSTHROW( ERR_HEALTH );

        if( !pTempMemViews->HealthCheck() )
            SYSTHROW( ERR_HEALTH );

        if( !pTempGroupCatList->HealthCheck() )
            SYSTHROW( ERR_HEALTH );
        #endif
        }
    else
        {
        rc= -1;         //file open failed
        }
    } //end protected block

TError::SetCatchBuf( pOldCBuf );

// now that the notes have been successfully read in, the value
// of pCategories can be returned to its original value
PTCatTree pNewCategories = pCategories;
pCategories     = pOldCategories;

if( rc == 0 )       //attempt the merge
    {

    // create a temporary category tree, merge the old cats and then
    // the new cats into it, and see how many cats it has total. If
    // this total is over 1000, then the merged combination would have
    // too many categories
    PTCatTree tempTree = new TCatTree( (PTCat) NULL );
    tempTree->Merge( NULL, *pCategories );
    tempTree->Merge( NULL, *pNewCategories );
    BOOL tooManyCats = !tempTree->RoomForMore( 0, TRUE, FALSE, 
      "Merging this file would put you over the 3000 folder limit." );
    tempTree->flush( TXArray::kDelete );
    delete tempTree;

    if (tooManyCats)
        {
        // throw away the new stuff
        pTempGroupCatList->flush( TXArray::kNoDelete );
        pNewCategories->flush( TXArray::kDelete );
        pTempNotes->flush( TXArray::kDelete );
        pTempAlarms->flush( TXArray::kDelete );
        pTempMemNotes->flush( TXArray::kDelete );
        pTempMemViews->flush( TXArray::kDelete );
        delete pNewCategories;
        delete pTempNotes;
        delete pTempAlarms;
        delete pTempMemNotes;
        delete pTempMemViews;
        delete pTempGroupCatList;

        }
    else
        {
        // now merge the new categories just read in from the file with
        // the old categories which are once again in pCategories
        // and switch the categories in the notes for the equivalent
        // newly added categories in pCategories

        pCategories->Merge( NULL, *pNewCategories );
        SwitchCats( pCategories, *pTempNotes );
        SwitchCats( pCategories, *pTempMemNotes );
        SwitchCats( pCategories, *pTempMemViews );

        // now get rid of pNewCategories since no note uses them any more
        pNewCategories->flush( TXArray::kDelete );
        delete pNewCategories;

        CHKBUG( ERR_GEN, pCategories->HealthCheck() );

        // copy notes into global list and throw away the container
        for (int i = 0; i < pTempNotes->getItemsInContainer(); i++)
            {
            RTNote note = (*pTempNotes)[i];

            // is this a non-duplicated note?
            if (pAllNotes->FindNote( note ) == NOOBJECT)
                {

                if (!pAllNotes->RoomForMore( 1, TRUE, FALSE,
                    "Some notes were not merged because you reached the 3000 note limit. Delete some notes and merge again." ))
                    {
                    for( int j = i; j < pTempNotes->getItemsInContainer(); j++)
                        {
                        RTNote note = (*pTempNotes)[j];
                        delete &note;
                        }
                    break;
                    }
                else
                    {
                    pAllNotes->add( note );
                    }
                }
            else
                {
                delete &note;
                }
            }
        delete pTempNotes;

        // copy alarms into the global list
        // is there a pointer to the original note??
        //FIXME who updates it?
        for (i = 0; i < pTempAlarms->getItemsInContainer(); i++)
            {
            RTAlarm alarm = (*pTempAlarms)[i];
            pAlarms->add( alarm );
            }
        delete pTempAlarms;

        // copy memorized notes into the global list
        for (i = 0; i < pTempMemNotes->getItemsInContainer(); i++)
            {
            RTMemorizedNote note = (RTMemorizedNote) (*pTempMemNotes)[i];
            if (pMemNotes->FindNote( note ) == NOOBJECT)
                {
                if (!pAllNotes->RoomForMore( 1, TRUE, FALSE,
                    "Some memorized notes were not merged because you reached the 1000 note limit. Delete some notes and merge again." ))
                    {
                    for( int j = i; j < pTempMemNotes->getItemsInContainer(); j++)
                        {
                        RTMemorizedNote note = (RTMemorizedNote) (*pTempMemNotes)[j];
                        delete &note;
                        }
                    break;
                    }
                else
                    {
                    pMemNotes->add( note );
                    }
                }
            else
                {
                delete &note;
                }
            }
        delete pTempMemNotes;

        // copy memorized view into the global list
        for (i = 0; i < pTempMemViews->getItemsInContainer(); i++)
            {
            RTMemorizedView view = (RTMemorizedView) (*pTempMemViews)[i];
            if (pMemViews->FindView( view ) == NOOBJECT)
                {
                pMemViews->add( view );
                }
            else
                {
                delete &view;
                }
            }
        delete pTempMemViews;

        delete pTempGroupCatList;
        }
    }

    #if SAFE_READ
    if( !pCategories->HealthCheck() )
        SYSTHROW( ERR_HEALTH );

    if( !pAllNotes->HealthCheck() )
        SYSTHROW( ERR_HEALTH );

    if( !pMemNotes->HealthCheck() )
        SYSTHROW( ERR_HEALTH );

    if( !pMemViews->HealthCheck() )
        SYSTHROW( ERR_HEALTH );

    if( !GroupCatListP->HealthCheck() )
        SYSTHROW( ERR_HEALTH );
    #endif

TCursor::Busy( FALSE );

bSystemDirty= TRUE;
return ! rc;    //rc== 0 if ok; return == TRUE if ok
}

//*****************************************************************************
// MarkSelectedCats
//
//    Mark all categories in pCatTree that are used in the notes
//*****************************************************************************
void TSysState::MarkSelectedCats( PTCatTree pCatTree, RTNoteSet notes )
{
    pCatTree->ClearMarks();

    int n= notes.getItemsInContainer();
    for (int i=0; i < n; i++) {

        RTNote note = (RTNote) notes[i];
        if( note != NOOBJECT ) {

            int numCats = note.GetCatList().getItemsInContainer();
            for (int j = 0; j < numCats; j++) {

                RTCat cat = (note.GetCatList())[j];
                if( (RObject) cat != NOOBJECT ) {

                    cat.Mark();
                }
            }
        }
    }
}

//*****************************************************************************
// SwitchCats
//
// Switch the categories in each of the notes from their current categories
// to the categories in the supplied list of categories. Used in merging
//*****************************************************************************
void TSysState::SwitchCats( PTCatTree pNewCats, RTNoteSet notes )
{
    for (int i = 0; i < notes.getItemsInContainer(); i++)
        {
        RTNote note = notes[i];
        (note.GetCatList()).SwitchCats( pNewCats );
        }
}

//*****************************************************************************
//memorized notes
//*****************************************************************************
void TSysState::SwitchCats( PTCatTree pNewCats, RTMemNoteSet memNotes )
{
for (int i = 0; i < memNotes.getItemsInContainer(); i++)
    {
    RTMemorizedNote note = (RTMemorizedNote) memNotes[i];
    note.SwitchCats( pNewCats );
    }
}

//*****************************************************************************
//memorized views
//*****************************************************************************
void TSysState::SwitchCats( PTCatTree pNewCats, RTMemViewSet memViews )
{
for (int i = 0; i < memViews.getItemsInContainer(); i++)
    {
    RTMemorizedView view = (RTMemorizedView) memViews[i];
    view.SwitchCats( pNewCats );
    }
}

//*****************************************************************************
// restore default category grouping
//*****************************************************************************
void TSysState::UseDefaultGroups( void )
{
    GroupCatListP->flush( TXArray::kNoDelete );
    CatGrouping = kAutomaticCatGrouping;
}


#if 0
    for (int i = 0; i < notes.getItemsInContainer(); i++) {

        RTNote note = notes[i];
        PTCatList newCatsP = new TCatList();

        for (int j = 0; j < note.GetCatList().getItemsInContainer(); j++) {

            TSafStr s;
            RTCat oldCat = (note.GetCatList())[j];
            oldCat.GetShortName( s );
            RTCat newCat = pNewCats->FindCatInTree( s );
            if (((RObject) newCat) != NOOBJECT) {

                newCatsP->add( newCat );

            }
            else {

                TMessageBox::Ok( TPimFrame::pFrame,
                    "Can't find new folder.",
                    "Merge Error" );
            }
        }


        // swap the new cat list for the old
        note.SetCatList( newCatsP );
    }

#endif
