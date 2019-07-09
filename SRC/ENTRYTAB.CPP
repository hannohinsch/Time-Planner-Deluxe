//*****************************************************************************
// entry tables are used by calendar views; one "entry" per cell
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "entrytab.h"

#ifndef __utility__
#include "utility.h"
#endif

//*****************************************************************************
// Constructor
//*****************************************************************************
void TEntryTable::TEntryTable( int entries, int slotsPerEntry )
{
    itsSlotsPerEntry = slotsPerEntry;
    itsNumEntries    = entries;

    // initialize entries
    itsEntriesPP     = (PTSlot*) new PTSlot[entries];
    for (int i = 0; i < entries; i++) {

        itsEntriesPP[i] = NULL;
    }
}

//*****************************************************************************
// Destructor
//*****************************************************************************
void TEntryTable::~TEntryTable( void )
{
    for (int i = 0; i < itsNumEntries; i++) {

        if (itsEntriesPP[i] != NULL) {

            delete itsEntriesPP[i];
        }
    }

    delete itsEntriesPP;
}

//*****************************************************************************
// SetEntry
//
// stuff the note into the entry table based on its positions
//*****************************************************************************
void TEntryTable::SetEntry( PTNote pNote, TBoundsPositions positions, int scaleFactor )
{
    // can we find a slot?
    int slot;
    int startEntry = positions.startCell;
    int endEntry   = positions.endCell;
    if ((startEntry < endEntry) && (positions.endPos == 0)) {

        endEntry--;
        positions.endCell--;
        positions.endPos = 1;
    }
    if (!AssignSlot( startEntry, endEntry, slot )) {

        // marker to make sure "more..." comes up at bottom of cell
        for (register int i = startEntry; i <= endEntry; i++) {

            FillEntry( i, itsSlotsPerEntry - 1, pNote, 0, 0 );
        }
        return;
    }


    if (startEntry == endEntry) {

        // since assigning 0 to both start and stop is supposed
        // to mean "more," to various drawing functions, make sure
        // that stop is at least 1
        // FIXME: use -1, -1 to mean more, not 0, 0
        FillEntry( startEntry, slot, pNote,
            DoubleToInt( positions.startPos * scaleFactor),
            max( 1, DoubleToInt( positions.endPos   * scaleFactor)) );
    }

    else {

        // start hour
        FillEntry( positions.startCell, slot, pNote,
            DoubleToInt( positions.startPos * scaleFactor),
            scaleFactor );

        // middle hours
        for (register int i = positions.startCell + 1; i < positions.endCell; i++) {

            FillEntry( i, slot, pNote, 0, scaleFactor );
        }

        // stop hour
        // since assigning 0 to both start and stop is supposed
        // to mean "more," to various drawing functions, make sure
        // that stop is at least 1
        FillEntry( positions.endCell, slot, pNote,
            0, max( 1, DoubleToInt( positions.endPos * scaleFactor )));
    }
}

//*****************************************************************************
// FillEntry
//
// Stuff the values into the entry, creating a new entry if necessary
//*****************************************************************************
void TEntryTable::FillEntry( int entry, int slot, PTNote pNote, int startPos, int stopPos )
{
    if (itsEntriesPP[entry] == NULL) {

        itsEntriesPP[entry] = new TSlot[itsSlotsPerEntry];
        for (int i = 0; i < itsSlotsPerEntry; i++) {

            itsEntriesPP[entry][i].pNote    = NULL;
            itsEntriesPP[entry][i].startPos = 0;
            itsEntriesPP[entry][i].stopPos  = 0;
        }
    }

    itsEntriesPP[entry][slot].pNote    = pNote;
    itsEntriesPP[entry][slot].startPos = startPos;
    itsEntriesPP[entry][slot].stopPos  = stopPos;
}

//*****************************************************************************
// AssignSlot
//
// Find a slot in the entry table that is currently empty for every entry
// between the startCell and endCell
//*****************************************************************************
BOOL TEntryTable::AssignSlot( int startEntry, int endEntry, int &assignedSlot)
{
    for (int slot = 0; slot < itsSlotsPerEntry; slot++) {

        BOOL validSlot = TRUE;
        for (int entry = startEntry; entry <= endEntry; entry++) {

            if ((itsEntriesPP[entry] != NULL) &&
                (itsEntriesPP[entry][slot].pNote != NULL)) {

                validSlot = FALSE;
                break;
            }
        }

        if (validSlot) {

            assignedSlot = slot;
            return TRUE;
        }
    }

    // failed
    assignedSlot = 0;
    return FALSE;

}

//*****************************************************************************
// GetEntry
//
// Get information from a slot of the specified entry. If first is TRUE,
// then start from the first slot, otherwise keep an internal counter
// and return the slots in order. The function returns TRUE if method
// is returning valid information.
//*****************************************************************************
BOOL TEntryTable::GetEntry( int entry, int slot,
    PTNote *pNote, long &startPos, long &stopPos )
{
    if (itsEntriesPP[entry] == NULL) goto NoMoreEntries;
    if (slot >= itsSlotsPerEntry)    goto NoMoreEntries;

    if (itsEntriesPP[entry][slot].pNote != NULL) {

        *pNote   = itsEntriesPP[entry][slot].pNote;
        startPos = itsEntriesPP[entry][slot].startPos;
        stopPos  = itsEntriesPP[entry][slot].stopPos;
        return TRUE;
    }

    NoMoreEntries:

    *pNote   = NULL;
    startPos = 0;
    stopPos  = 0;
    return FALSE;
}

//*****************************************************************************
// GetNotesNear
//
// Get the note at, above and below the entry in the given slot
//*****************************************************************************
void TEntryTable::GetNotesNear( int entry, int slot,
    PTNote *noteAtP, PTNote *noteAboveP, PTNote *noteBelowP )
{
    long startPos, stopPos;

    *noteAtP    = NULL;
    *noteAboveP = NULL;
    *noteBelowP = NULL;

    if ((0 <= entry) && (entry < itsNumEntries)) {

        GetEntry( entry, slot, noteAtP, startPos, stopPos );
    }

    entry--;
    if ((0 <= entry) && (entry < itsNumEntries)) {

        GetEntry( entry, slot, noteBelowP, startPos, stopPos );
    }

    entry += 2;
    if ((0 <= entry) && (entry < itsNumEntries)) {

        GetEntry( entry, slot, noteAboveP, startPos, stopPos );
    }
}


//*****************************************************************************
// GetSlot
//
// Get the slot for the note in the entry
//*****************************************************************************
int TEntryTable::GetSlot( PTNote pNote, int entry )
{
    if (itsEntriesPP[entry] == NULL) {

        return -1;
    }


    for (int slot = 0; slot < itsSlotsPerEntry; slot++) {

        if (itsEntriesPP[entry][slot].pNote == pNote) {

            return slot;
        }
    }

    return -1;
}

//*****************************************************************************
// Clear
//
// Clear out the entries
//*****************************************************************************
void TEntryTable::Clear( void )
{
    for (int i = 0; i < itsNumEntries; i++) {

        if (itsEntriesPP[i] != NULL) {

            for (int j = 0; j < itsSlotsPerEntry; j++) {

                itsEntriesPP[i][j].pNote = NULL;
            }
        }
    }
}

//*****************************************************************************
// return TRUE iff there's at least one note in one slot in this entry
//*****************************************************************************
BOOL TEntryTable::HasNoteInEntry( int entry )
{
    if (itsEntriesPP[entry] == NULL)
        return FALSE;

    for( int slot= 0; slot < itsSlotsPerEntry; slot++ )
        {
        if (itsEntriesPP[entry][slot].pNote != NULL)
            return TRUE;
        }

     return FALSE;
}
