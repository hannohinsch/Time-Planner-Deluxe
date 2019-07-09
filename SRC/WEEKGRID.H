#ifndef __weekgrid__
#define __weekgrid__

_CLASSDEF( TWeekGrid )
_CLASSDEF( TLongNote )

#ifndef __draggrid__
#include "draggrid.h"
#endif
#ifndef __snoop__
#include "snoop.h"
#endif
#ifndef __bounds__
#include "bounds.h"
#endif
#ifndef __entrytab__
#include "entrytab.h"
#endif
#ifndef __color__
#include "color.h"
#endif
#ifndef __note__
#include "note.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif

const int kMaxLongNotes = 15;

class _CLASSTYPE TWeekGrid: public TDragGrid, TSnoop {

    public:

    TWeekGrid( PTWindowsObject AParent );
    virtual ~TWeekGrid( void );

    void SetDefaultPrintRange( void );
    BOOL HChgCellStartIndex( int n, char *buf, int size);
    BOOL HChgCellEndIndex( int n, char *buf, int size);
    BOOL VChgCellStartIndex( int n, char *buf, int size);
    BOOL VChgCellEndIndex( int n, char *buf, int size);

    typedef enum {

        kTimeTitle,
        kStripeTitle,
        kNoteTitle,

        kLongStripeTime,
        kLongStripe,

        kLongNoteNames,
        kLongNote,

        kTime,
        kStripe,
        kNote,
        kNotACell

        } TCellType;

    typedef enum {

        kTitleRow,
        kLongStripeRow,
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

    int  CellWidth(  PTPaint, int col );
    int  CellHeight( PTPaint, int row );
    int  GridWidth(  PTPaint, int col );
    int  GridHeight( PTPaint, int row );

    void PaintCell( RTDrawInfo );
    void PaintTimeTitle( RTDrawInfo );
    void PaintNoteTitle( RTDrawInfo );
    void PaintTime( RTDrawInfo );
    void PaintStripe( RTDrawInfo );
    void PaintNote( RTDrawInfo );
    void PaintLongNames( RTDrawInfo );
    void PaintLongStripe( RTDrawInfo );
    void PaintLongNote( RTDrawInfo );
    void PaintLongHourNote( RTDrawInfo di, RTNote note, RECT &r );

    void PaintMilestone( RTDrawInfo di, RECT &r, RTColor color );
    void PaintTask(      RTDrawInfo di, RECT &r, RTColor color, int startPos, int stopPos );
    void PaintNoteTitle( RTDrawInfo di, RECT &r, PTNote pNote, int lastSlotFilled );

    void MousOver( RTMousInfo mi );
    void MousDblClick( RTMousInfo mi );
    void MousRtClick( RTMousInfo mi );
    BOOL MousRangeInit( RTMousInfo mi  );
    void MousRangeOver( RTMousInfo mi );
    void MousRangeRelease( RTMousInfo mi );

    void MousDragOver( RTMousInfo );
    void MousDragRelease( RTMousInfo );

    void GetDefaultFocusCell( int &row, int &col );
    void MousKey( TMousKey );

    void GotoDay( RTMousInfo mi );
    void ShowAllLongNotes( void );
    void ShowTodaysLongNotes( RTMousInfo mi );
    void ShowCellsNote( RTMousInfo mi );

    void ResetTime( void );
    void Listen( TSnoopMsg, RObject );

    void Setup( RTTime startTime );
    int  CellToEntry( int row, int col );
    void AssignEntry( RTNote note, RTBoundsPositions positions );
    void AssignLongEntry( RTNote note, RTBoundsPositions positions );

    void FormatColDate( int col, char *buf );

    void CellToTime(  int row, int col, RTTime time );
    void GetCellTimes( int row, int col, RTTime start, RTTime end );
    void TimeToCell( RTTime time, int &row, int &col );

    void WMVScroll(RTMessage)= [WM_FIRST + WM_VSCROLL];
    void WMHScroll(RTMessage)= [WM_FIRST + WM_HSCROLL];
    void WeekGridNewPageHook( void );

    TCellType CellType( int row, int col );
    TRowType  RowType(  int row );
    TColType  ColType(  int col );

    TLongNote &LongNoteInRow( int i );
    int        RowsForWeek( int dayIndex, BOOL forPrinter );

    PTNote           *itsTitlesP;
    TXArray          *itsLongNoteTableP;  //long notes, as slot array
    int               itsNumLongNotes;
    int               itsMinutesPerCell;
    int               itsLiveRows;
    int               itsNumDays;
    BOOL              itsDraggingDays;
    BOOL              itsSavePosition;          // save position on exit?
    int               itsFirstVisibleLongNote;

    TTime             itsFirstCell;
    TTime             itsLastCell;
    TTime             itsStartTime;
    int               itsNumTimes;

    PTEntryTable      itsEntryTableP;
    BOOL              itsDraggingLongNotes;

    // -------------------------------------------------------------------
    BOOL   ValidDragCell( TDragMode mode, int row, int col );
    void   SetStart( RTTime start );
    void   SetEnd( RTTime end );
    void   SetBoth( RTMousInfo mi, RTTime start );
    void   ConstrainBoth( RTMousInfo );
    void   ConstrainMarqueeRect( TDragMode mode, RECT &r );

    void   GetNoteRects( RTNote note, RTMousInfo mi, RECT &growStartR, RECT &dragR, RECT &growEndR, RECT &marqueeR );
    void   PointToTime( RTMousInfo mi, RTTime time );
    BOOL   TimeToPoint( RTTime time, POINT &p );
    RTNote PointToNote( RTMousInfo mi );
    void   Release( void );

    void   ScrollStart( RTMousInfo mi, int deltaRow, int deltaCol );
    void   ScrollEnd(   RTMousInfo mi, int deltaRow, int deltaCol );
    void   ScrollBoth(  RTMousInfo mi, int deltaRow, int deltaCol );

    // -------------------------------------------------------------------
    BOOL   HValidDragCell( TDragMode mode, int row, int col );
    void   HSetBoth( RTMousInfo mi, RTTime start  );

    void   HGetNoteRects( RTNote note, RTMousInfo mi, RECT &growStartR, RECT &dragR, RECT &growEndR, RECT &marqueeR );
    void   HPointToTime( RTMousInfo mi, RTTime time );
    RTNote HPointToNote( RTMousInfo mi );
    void   HRelease( void );

    void   HMousDragOver( RTMousInfo );
    void   HMousDragRelease( RTMousInfo );

    void   HScrollStart( RTMousInfo mi, int deltaRow, int deltaCol );
    void   HScrollEnd(   RTMousInfo mi, int deltaRow, int deltaCol );
    void   HScrollBoth(  RTMousInfo mi, int deltaRow, int deltaCol );

    // -------------------------------------------------------------------
    BOOL   VValidDragCell( TDragMode mode, int row, int col );
    void   VSetBoth( RTMousInfo mi, RTTime start );

    void   VGetNoteRects( RTNote note, RTMousInfo mi, RECT &growStartR, RECT &dragR, RECT &growEndR, RECT &marqueeR );
    void   VPointToTime( RTMousInfo mi, RTTime time );
    RTNote VPointToNote( RTMousInfo mi );
    void   VRelease( void );

    void   VMousDragOver( RTMousInfo );
    void   VMousDragRelease( RTMousInfo );

    void   VScrollStart( RTMousInfo mi, int deltaRow, int deltaCol );
    void   VScrollEnd(   RTMousInfo mi, int deltaRow, int deltaCol );
    void   VScrollBoth(  RTMousInfo mi, int deltaRow, int deltaCol );

};


class _CLASSTYPE TLongNote: public Sortable {

    public:

    TLongNote( void );
    virtual ~TLongNote( void );

    // object nonsense
    classType     isA( void ) const;
    char         *nameOf( void ) const;
    hashValueType hashValue( void ) const;
    int           isLessThan( const Object & ) const;
    int           isEqual(const Object &) const;
    void          printOn(ostream &) const;
    static        PTStreamable build();

    PTNote itsNoteP;
    int    itsRow;
    int    itsStartCol;
    int    itsEndCol;
    double itsStartPos;
    double itsEndPos;

    };

#endif
