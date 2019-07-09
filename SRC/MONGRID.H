#ifndef __mongrid__
#define __mongrid__

_CLASSDEF( TMonGrid )

#ifndef __rulegrid__
#include "rulegrid.h"
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
#ifndef __msg__
#include "msg.h"
#endif

class _CLASSTYPE TMonGrid: public TRuleGrid, TSnoop {

    public:

    TMonGrid( PTWindowsObject AParent );
    virtual ~TMonGrid( void );

    void SetDefaultPrintRange( void );

    typedef enum {

        kMonthTitle,
		kDayTitle,
        kDay,
        kNotACell

        } TCellType;

    typedef enum {

		kMonthTitleRow,
		kDayTitleRow,
		kTimeRow,
        kNotARow

        } TRowType;

    typedef enum {

		kTimeCol,
        kNotACol

        } TColType;

    private:

    int  CellWidth(  PTPaint, int col );
    int  CellHeight( PTPaint, int row );
    int  GridWidth(  PTPaint, int col );
    int  GridHeight( PTPaint, int row );


    void PaintGridCol( RTDrawInfo di  );
    void PaintCell( RTDrawInfo );
    void PaintMonthTitle( RTDrawInfo );
    void PaintDayTitle( RTDrawInfo );
	void PaintNote( RTDrawInfo );

    void PaintMilestone( RTDrawInfo di, RECT &r, RTColor color, BOOL fullDay );
    void PaintMilestone( RTDrawInfo di, RECT &r, RTColor color, TSafStr &title );
    void PaintTask(      RTDrawInfo di, RECT &r, RTColor color, int startPos, int stopPos, BOOL hollow );
    void PaintTask(      RTDrawInfo di, RECT &r, RTColor color, TSafStr &title, BOOL hollow );

    void MousOver( RTMousInfo mi );
    void MousDblClick( RTMousInfo mi );
    void MousRtClick( RTMousInfo );
    void HandleZoom( RTMessage )  = [CM_FIRST + IDM_MONTH_ZOOM ];
    void HandleNotes( RTMessage ) = [CM_FIRST + IDM_MONTH_NOTES ];

    BOOL IsCellSelectable( int row, int col );
    void GetDefaultFocusCell( int &row, int &col );
    void MousKey( TMousKey );

    void ResetTime( void );
    void ResetNotes( void );
    void Listen( TSnoopMsg, RObject );

    void Setup( RTTime startTime  );
    int  CellToEntry( int row, int col );

    void CellToTime( int row, int col, RTTime time );
    void GetCellTimes( int row, int col, RTTime start, RTTime end );

    TMonGrid::TCellType CellType( int row, int col );
    TRowType  RowType(  int row );
    TColType  ColType(  int col );

    void HeadlineChanged( RTMessage ) = [WM_FIRST + WM_HEADLINE_CHG];

    TTime             itsStartTime;
    TTime             itsFirstCell;
    TTime             itsLastCell;
    TTime             itsTargetMonth;
    int               itsNumTimes;
    PTEntryTable      itsEntryTableP;
    int               itsMenuRow;
    int               itsMenuCol;

    };


#endif
