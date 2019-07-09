#ifndef __tmlinegr__
#define __tmlinegr__

_CLASSDEF( TTimeLineGrid )
_CLASSDEF( TNoteTable )

#ifndef __snoop__
#include "snoop.h"
#endif
#ifndef __draggrid__
#include "draggrid.h"
#endif
#ifndef __note__
#include "note.h"
#endif
#ifndef __dragwin__
#include "dragwin.h"
#endif

typedef enum { kGrowStart, kDrag, kGrowEnd } TDragMode;

//*****************************************************************************
// time grid, customized for timeline view
//*****************************************************************************
class _CLASSTYPE TTimeLineGrid: public TDragGrid, TSnoop {

    public:

    TTimeLineGrid( PTWindowsObject AParent );
    virtual ~TTimeLineGrid( void );

    typedef enum {

        kColorHeader,
        kNoteHeader,
        kTimeHeader,
        kColorTitle,
        kNoteTitle,
        kTimeTitle,
        kColor,
        kNote,
        kTime,
        kNotACell

        } TCellType;

    typedef enum {

        kHeaderRow,
        kTitleRow,
        kTimeRow,
        kNotARow

        } TRowType;

    typedef enum {

        kColorCol,
        kNoteCol,
        kTimeCol,
        kNotACol

        } TColType;

    private:

    int CellWidth( PTPaint, int col );
    int CellHeight( PTPaint, int row );
    int  GridWidth( PTPaint, int /*col*/ );
    int  GridHeight( PTPaint, int /*row*/ );

    void PaintCell( RTDrawInfo );
    void PaintColor( RTDrawInfo di );
    void PaintNote( RTDrawInfo di );
    void PaintTime( RTDrawInfo di );
    void PaintNoteHeader( RTDrawInfo di );
    void PaintTimeHeader( RTDrawInfo di );
    void PaintNoteTitle( RTDrawInfo di );
    void PaintTask( RTDrawInfo di, RTTime start, RTNote note );
    void PaintMilestone( RTDrawInfo di, RTTime start, RTNote note );

    void MousOver( RTMousInfo );
    void MousDblClick( RTMousInfo );
    void MousRtClick( RTMousInfo );

    BOOL IsCellSelectable( int row, int col );
    void GetDefaultFocusCell( int &row, int &col );
    void MousKey( TMousKey );
    void InvalidateNote( RTNote note );

    void Listen( TSnoopMsg, RObject );
    void Reset( void );
    void ResetNotesNoGrouping( void );
    void ResetNotesWithGrouping( void );

    void WMVScroll(RTMessage)= [WM_FIRST + WM_VSCROLL];
    void WMHScroll(RTMessage)= [WM_FIRST + WM_HSCROLL];

    BOOL   ValidDragCell( TDragMode mode, int row, int col );
    void   GetNoteRects( RTNote note, RTMousInfo mi, RECT &growStartR, RECT &dragR, RECT &growEndR, RECT &marqueeR );
    void   PointToTime( RTMousInfo mi, RTTime time );
    RTNote PointToNote( RTMousInfo mi );
    void   SetStart( RTTime start );
    void   SetEnd( RTTime end );
    void   SetBoth( RTMousInfo mi, RTTime start );
    void   ConstrainMarqueeRect( TDragMode mode, RECT &r );
    void   Release( void );
    void   ScrollStart(  RTMousInfo mi, int deltaRow, int deltaCol );
    void   ScrollEnd(    RTMousInfo mi, int deltaRow, int deltaCol );
    void   ScrollBoth(   RTMousInfo mi, int deltaRow, int deltaCol );

    int NumVisibleCells( void );

    void ScrollToNote(RTMessage Msg)       = [CM_FIRST + IDM_SCROLL_TO_NOTE];
    void MoveNoteHere(RTMessage Msg)       = [CM_FIRST + IDM_MOVE_NOTE_HERE];
    void ConvertToTask(RTMessage Msg)      = [CM_FIRST + IDM_CONVERT_TO_TASK];
    void ConvertToMilestone(RTMessage Msg) = [CM_FIRST + IDM_CONVERT_TO_MILESTONE];

    // from TimeGrid
    void  ResetTime( RTTime selTime, TTimeScale timeScale );
    TTime GetCellStart( int row, int col );
    TTime GetCellEnd(   int row, int col );
    void  GetCellTimes( int row, int col, RTTime start, RTTime end );
    int   LogicalRow( int row )  { return row - frozenRows; }
    int   LogicalCol( int col )  { return col - frozenCols; }
    TTime TimeOffset( RTTime start, int offset );
    void  FormatTime( RTTime time, char *buf );
    void  TimeToCell( RTTime time, int &row, int &col );
    BOOL  TimeToPoint( RTTime time, POINT &p );

    void  SetDefaultPrintRange( void );
    BOOL  HChgCellStartIndex( int n, char *buf, int size);
    BOOL  HChgCellEndIndex(   int n, char *buf, int size);

    BOOL   IsNoteRow( int row );
    RTNote GetNote( int row );
    BOOL   IsNoteInCol( RTNote note, int col );

    TCellType CellType( int row, int col );
    TRowType  RowType(  int row );
    TColType  ColType(  int col );

    virtual TTimeUnit GetPrecision( void );

    static int   itsTimeUnitsPerCell[4];
    TTime        itsFirstCell;
    TTime        itsLastCell;
    int          itsDragRow;
    TTime        itsStartTime;
    int          itsMenuRow;
    int          itsMenuCol;
    PTNoteTable  itsNoteTableP;
    int          itsRowCount;
};

#endif
