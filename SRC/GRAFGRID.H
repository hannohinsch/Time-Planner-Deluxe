#ifndef __grafgrid__
#define __grafgrid__

_CLASSDEF( TNoteSet )
_CLASSDEF( TGrafGrid )
_CLASSDEF( TNote )

#ifndef __bounds__
#include "bounds.h"
#endif
#ifndef __snoop__
#include "snoop.h"
#endif
#ifndef __value__
#include "value.h"
#endif
#ifndef __rulegrid__
#include "rulegrid.h"
#endif
#ifndef __worktime__
#include "worktime.h"
#endif

typedef struct {

    TWorkTime  time;
    double     value;

    } TSlot, *PTSlot;


//*****************************************************************************
// grid object, customized for graph view
//*****************************************************************************
class TGrafGrid: public TRuleGrid, TSnoop {

    public:

    TGrafGrid( PTWindowsObject AParent );
    virtual ~TGrafGrid( void );

    protected:

    void Setup( void );

    void WMVScroll(RTMessage)= [WM_FIRST + WM_VSCROLL];
    void WMHScroll(RTMessage)= [WM_FIRST + WM_HSCROLL];

    int  CellWidth( PTPaint, int col );
    int  CellHeight( PTPaint, int row );
    void PaintCell( RTDrawInfo di );

    void MousDblClick( RTMousInfo );
    void MousRtClick( RTMousInfo );
    void MousOver( RTMousInfo );
    void ShowNoteList( int col );

    BOOL IsCellSelectable( int row, int col );
    void GetDefaultFocusCell( int &row, int &col );
    void MousKey( TMousKey );

    void Listen( TSnoopMsg, RObject );

    private:

    void  ResetTime( RTTime selTime );
    TTime GetCellStart( int row, int col );
    TTime GetCellEnd(   int row, int col );
    void  GetCellTimes( int row, int col, RTTime start, RTTime end );
    void  FormatTime( RTTime time, char *buf );

    void  SetDefaultPrintRange( void );
    BOOL  HChgCellStartIndex( int n, char *buf, int size);
    BOOL  HChgCellEndIndex(   int n, char *buf, int size);

    void SetupMilestone( RTNote note );
    void SetupTask( RTNote note );
    void SetupCell( int cell, RTTime start, RTTime end, long totalMins, double totalValue );

    int  GridWidth( PTPaint, int /*col*/ );
    int  GridHeight( PTPaint, int /*row*/ );

    void PaintGraph( RTDrawInfo );
    void PaintTitle( RTDrawInfo );
    void PaintValue( RTDrawInfo );

    TTime           itsFirstCell;
    TTime           itsLastCell;
    TDuration       itsMaxTime;
    double          itsMaxValue;
    TSlot           itsSlotTable[200];

    static int      itsTimeUnitsPerCell[4];

};


#endif



