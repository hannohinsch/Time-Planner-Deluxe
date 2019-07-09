#ifndef __daygrid__
#define __daygrid__

_CLASSDEF( TDayGrid )

#ifndef __draggrid__
#include "draggrid.h"
#endif
#ifndef __snoop__
#include "snoop.h"
#endif
#ifndef __color__
#include "color.h"
#endif
#ifndef __entrytab__
#include "entrytab.h"
#endif
#ifndef __bounds__
#include "bounds.h"
#endif
#ifndef __note__
#include "note.h"
#endif

static const int kMaxLongDayNotes = 6;

class _CLASSTYPE TDayGrid: public TDragGrid, TSnoop {

public:

    TDayGrid( PTWindowsObject AParent );
    virtual ~TDayGrid( void );

    void SetDefaultPrintRange( void );
    BOOL VChgCellStartIndex( int n, char *buf, int size);
    BOOL VChgCellEndIndex( int n, char *buf, int size);

    typedef struct {

        PTNote noteP;
        int    row;

    } TLongNote;

    typedef enum {

        kTimeTitle,
        kStripeTitle,
        kNoteTitle,
        kLongNoteNames,
        kLongNote,
        kLongNoteStripe,
        kTime,
        kStripe,
        kNote,
        kNotACell

        } TCellType;

    typedef enum {

        kTitleRow,
        kLongNoteRow,
        kTimeRow,
        kNotARow

        } TRowType;

    typedef enum {

        kTimeCol,
        kStripeCol,
        kNoteCol,
        kNotACol

        } TColType;

private:

    typedef struct {

        PTNote noteP;
        int    entry;

        } TTitle, *PTTitle;

    int  CellWidth(  PTPaint, int col );
    int  CellHeight( PTPaint, int row );
    int  GridWidth(  PTPaint, int col );
    int  GridHeight( PTPaint, int row );

    void PaintCell( RTDrawInfo );
    void PaintTime( RTDrawInfo );
    void PaintDrag( RTDrawInfo );
    void PaintTitle( RTDrawInfo );
    void PaintNote( RTDrawInfo );
    void PaintLongNames( RTDrawInfo );
    void PaintLongNote( RTDrawInfo );

    void PaintMilestone( RTDrawInfo di, RECT &r, RTColor color );
    void PaintTask(      RTDrawInfo di, RECT &r, RTColor color, int startPos, int stopPos );
    void PaintNoteTitle( RTDrawInfo di, RECT &r, int entry );

    void MousOver( RTMousInfo mi );
    void MousDblClick( RTMousInfo mi );
    void MousRtClick( RTMousInfo mi );
    BOOL MousRangeInit( RTMousInfo );
    void MousRangeOver( RTMousInfo );
    void MousRangeRelease( RTMousInfo mi );

    BOOL IsCellSelectable( int row, int col );
    void GetDefaultFocusCell( int &row, int &col );
    void MousKey( TMousKey );

    void ResetTime( void );
    void ResetNotes( void );
    void Listen( TSnoopMsg, RObject );

    void WMVScroll(RTMessage)= [WM_FIRST + WM_VSCROLL];

    void Setup( RTTime startTime );
    int  CellToEntry( int row, int col, BOOL canFail = FALSE );
    void AssignEntry( RTNote note, RTBoundsPositions positions );
    void AssignLongEntry( RTNote note, RTBoundsPositions positions );

    void CellToTime( int row, int col, RTTime time );
    void GetCellTimes( int row, int col, RTTime start, RTTime end );
    void TimeToCell( RTTime time, int &row, int &col );
    BOOL TimeToPoint( RTTime time, POINT &p );

    void ShowNote( RTMousInfo mi  );
    void ShowLongNote( RTMousInfo mi  );

    TCellType CellType( int row, int col );
    TRowType  RowType(  int row );
    TColType  ColType(  int col );

    TTime             itsStartTime;
    TTime             itsFirstCell;
    TTime             itsLastCell;
    int               itsNumTimes;
    int               itsMinutesPerCell;
    PTEntryTable      itsEntryTableP;
    TTitle           *itsTitlesP;
    TLongNote         itsLongNotes[kMaxLongDayNotes];  //long notes, as slot array
    int               itsNumLongNotes;
    PTNoteSet         itsLongNoteSetP;           //long notes, as set

    BOOL   IsAllowedToDrag( RTNote note, char *message );

    BOOL   ValidDragCell( TDragMode mode, int row, int col );
    void   SetStart( RTTime start );
    void   SetEnd( RTTime end );
    void   SetBoth( RTMousInfo mi, RTTime start );
    void   ConstrainMarqueeRect( TDragMode mode, RECT &r );
    void   ConstrainToWorkTime( RTTime start, RTTime end );

    void   GetNoteRects( RTNote note, RTMousInfo mi, RECT &growStartR, RECT &dragR, RECT &growEndR, RECT &marqueeR );
    void   PointToTime( RTMousInfo mi, RTTime time );
    RTNote PointToNote( RTMousInfo mi );
    void   Release( void );

    void   ScrollStart( RTMousInfo mi, int deltaRow, int deltaCol );
    void   ScrollEnd(   RTMousInfo mi, int deltaRow, int deltaCol );
    void   ScrollBoth(  RTMousInfo mi, int deltaRow, int deltaCol );
    };


#endif
