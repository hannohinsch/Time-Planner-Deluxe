#ifndef __entrytab__
#define __entrytab__

_CLASSDEF( TEntryTable )

#ifndef __note__
#include "note.h"
#endif
#ifndef __bounds__
#include "bounds.h"
#endif


class _CLASSTYPE TEntryTable {

    public:

    TEntryTable( int entries, int slotsPerEntry );
    virtual ~TEntryTable( void );

    void SetEntry( PTNote pNote, TBoundsPositions positions, int scaleFactor );
    BOOL GetEntry( int entry, int slot,
        PTNote *pNote, long &startPos, long &stopPos );
    BOOL HasNoteInEntry( int entry );
    int  GetSlot( PTNote pNote, int entry );
    void GetNotesNear( int entry, int slot,
        PTNote *noteAtP, PTNote *noteAboveP, PTNote *noteBelowP );
    void Clear( void );

    private:

    typedef struct {

        PTNote pNote;
        long   startPos;
        long   stopPos;

    } TSlot, *PTSlot;

    void FillEntry( int entry, int slot, PTNote pNote, int startPos, int stopPos );
    BOOL AssignSlot( int startEntry, int endEntry, int &assignedSlot );

    int     itsSlotsPerEntry;
    int     itsNumEntries;
    PTSlot *itsEntriesPP;
};

#endif
