#ifndef __bounds__
#define __bounds__

_CLASSDEF( TBounds )

#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif

typedef struct {

    int    startCell;
    int    endCell;
    double startPos;
    double endPos;

    } TBoundsPositions;

typedef TBoundsPositions &RTBoundsPositions;

class _CLASSTYPE TBounds {

    public:

    TBounds( RTTime start, RTTime end );
    virtual ~TBounds( void );

    BOOL NoteOverlaps( RTNote note );
    BOOL NoteOverlaps( RTNote note, int numCells, RTBoundsPositions positions );
    BOOL NoteOverlapsAgain( RTNote note, BOOL &overlaps );
    BOOL NoteOverlapsAgain( RTNote note, int numCells, RTBoundsPositions positions, BOOL &overlaps );

    private:

    BOOL TimeIsWithin( RTTime time );
    BOOL RangeOverlaps( TTime start, TTime end );

    void Position( TTime time, int numCells,
        RTBoundsPositions position  );
    void DoublePosition( TTime start, TTime end, int numCells,
        RTBoundsPositions positions );

    TTime itsStart;
    TTime itsEnd;
};

#endif
