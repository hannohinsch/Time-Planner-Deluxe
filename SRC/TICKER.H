#ifndef __ticker__
#define __ticker__

_CLASSDEF( TTicker )

#ifndef __mousgrid__
#include "mousgrid.h"
#endif
#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __note__
#include "note.h"
#endif

typedef enum {

    kDurationOnly,
    kMilestoneOnly,
    kStartEnd

    } TTickerMode;

typedef enum {

    kGrowLeft,
    kGrowRight,
    kDrag

    } TTickerScrollMode;

class _CLASSTYPE TTicker: public TMousGrid {

    public:

    TTicker( PTWindowsObject AParent, int resId,
        RTTime startTime, RTDuration duration, PTTimeUnit unitP );
    virtual ~TTicker( void );

    // painting
    virtual void  PaintCell(     RTDrawInfo );
    virtual void  PaintTopLine(  RTDrawInfo );
    virtual void  PaintLeftLine( RTDrawInfo );

    // sizes
    virtual int   TopLineHeight(  PTPaint )         { return 1; };
    virtual int   LeftLineWidth(  PTPaint )         { return 1; };
    virtual int   CellWidth( PTPaint, int col )     { return GetColWidth( col ); };
    virtual int   CellHeight( PTPaint, int row )    { return GetRowHeight( row ); };
    virtual int   GetColWidth( int col );
    virtual int   GetRowHeight( int col );

    // mousing
    virtual void  MousOver( RTMousInfo mi );
    virtual BOOL  MousRangeInit( RTMousInfo );
    virtual void  MousRangeOver( RTMousInfo );
    virtual void  MousRangeRelease( RTMousInfo info );
    virtual BOOL  IsCellSelectable( int row, int col );
            void  GetDefaultFocusCell( int &row, int &col );

    virtual LPSTR GetClassName() { return "Ticker"; };

    virtual BOOL  RangeInitDuration( RTMousInfo );
    virtual void  RangeOverDuration( RTMousInfo );
    virtual void  RangeReleaseDuration( RTMousInfo );

    virtual BOOL  RangeInitMilestone( RTMousInfo );
    virtual void  RangeOverMilestone( RTMousInfo );
    virtual void  RangeReleaseMilestone( RTMousInfo );

    virtual BOOL  RangeInitStartEnd( RTMousInfo );
    virtual void  RangeOverStartEnd( RTMousInfo );
    virtual void  RangeReleaseStartEnd( RTMousInfo );

    void SetTimeUnit( TTimeUnit unit );
    void SetStart( RTTime time );
    void SwitchModes( TTickerMode mode );
    void GetTimes( RTTime start, RTTime end, RTDuration duration );

    private:

    void ColToTime( int col, RTTime time );
    void TimeToCol( RTTime time, int &col );
    void NormalizeCells( void );
    void NormalizeStartTime( void );
    void NormalizeCellsStartEnd( void );
    void CalculateSelection( BOOL gotoSelection );

    void PaintRow0( RTDrawInfo di );
    void PaintRow1( RTDrawInfo di );
    void PaintTick( RTDrawInfo di, BOOL longTick );

    BOOL Row1TextDurationNeeded( RTDrawInfo di, char *text );
    BOOL Row1TextTimeNeeded( RTDrawInfo di, char *text );
    void Row1Text( RTDrawInfo di );
    void Row1TextDuration( RTDrawInfo di );
    void Row1TextTime( RTDrawInfo di );
    void Row1Color( RTDrawInfo di );
    void Row1ColorTime( RTDrawInfo di );
    BOOL Label( RTDrawInfo di, char *buf );
    BOOL LabelDuration( RTDrawInfo di, char *buf );
    BOOL LabelTime( RTDrawInfo di, char *buf );
    void CalculateDuration( void );
    void CalculateStart( void );
    void PaintArrow( RTDrawInfo di, BOOL leftSide );

    virtual void WMHScroll(RTMessage Msg)= [WM_FIRST + WM_HSCROLL];

    TTime             itsFirstCell;
    TTime             itsStartTime;
    TDuration         itsDuration;
    PTTimeUnit        itsTimeUnitP;
    TTickerMode       itsMode;
    TTickerScrollMode itsScrollMode;
    int               itsDragOffset;

};


#endif
