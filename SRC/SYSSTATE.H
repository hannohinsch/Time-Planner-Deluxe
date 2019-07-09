#ifndef __sysstate__
#define __sysstate__

_CLASSDEF( TPimFrame )
_CLASSDEF( TFilter )
_CLASSDEF( TSysState )
_CLASSDEF( TNoteSet )
_CLASSDEF( TAlarmSet )
_CLASSDEF( TCatList )
_CLASSDEF( TCatTree )
_CLASSDEF( TMemNoteSet )
_CLASSDEF( TMemViewSet )

#include "frame.h"
#include "apptime.h"

typedef enum {

    kAutomaticCatGrouping  = 0,
    kNoCatGrouping         = 1,
    kSelectedCatGrouping   = 2
    } TCatGrouping;

class _CLASSTYPE TSysState: Object, TStreamable
{

public:
    TSysState( void );
    virtual ~TSysState();

    classType isA( void ) const;
    char *nameOf( void ) const;
    hashValueType hashValue( void ) const;
    int isEqual(const Object &) const;
    void printOn(ostream &) const;
    static PTStreamable build();

    static BOOL WriteContainers(
        char *fName, PTCatTree pCatTree,
        PTNoteSet pNotes, PTMemNoteSet pMemNotes,
        PTMemViewSet pMemViews, PTAlarmSet pAlarms,
        PTCatList pGroupCatList );
    static BOOL WriteFile( char *fName );
    static BOOL ReadFile( char *fName, BOOL checkWorkTime );
    static BOOL MergeFile( char *fName );
    static BOOL ArchiveFile( char *fName );

    static void UseDefaultGroups( void );

    static void FlushSystem( void );
    static void MarkSelectedCats( PTCatTree pCatTree, RTNoteSet notes );
    static void SwitchCats( PTCatTree pNewCats, RTNoteSet notes );
    static void SwitchCats( PTCatTree pNewCats, RTMemViewSet );
    static void SwitchCats( PTCatTree pNewCats, RTMemNoteSet );

    static void CreateContainers( void );
    static void DeleteContainers( void );

    //
    //the public containers and state variables. Note that they're static.
    //
    static PTCatTree    pCategories;
    static PTNoteSet    pAllNotes;
    static PTMemNoteSet pMemNotes;
    static PTMemViewSet pMemViews;
    static PTAlarmSet   pAlarms;
    static PTFilter     pFilter;
    static PTNoteSet    pSelNotes;
    static int          nextNoteID;
    static BOOL         bSystemDirty;
    static TTime        selTime;
    static TTimeScale   timeScale;
    static int          currentNoteIndex;
    static BOOL         bShowFullText;     
    static BOOL         bShowBars;         
    static TActiveView  activeView;
    static TCatGrouping CatGrouping;
    static PTCatList    GroupCatListP;

protected:

    TSysState( StreamableInit );
    void write( Ropstream );
    Pvoid read( Ripstream );

private:

    const Pchar streamableName() const { return "TSysState"; };

    //these values are written to and read from every file.  They
    //are not the global values! (But the file reader typically
    //sets the global values from these values ).

    PTFilter        its_pFilter;
    TTime           its_selTime;
    TTimeScale      its_timeScale;
    int             its_currentNoteIndex;
    BOOL            its_bShowFullText;
    TActiveView     its_activeView;

};

    inline Ripstream operator >> (Ripstream is, RTSysState cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTSysState cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTSysState cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTSysState cl)
        { return os << (PTStreamable) cl; }


#endif
