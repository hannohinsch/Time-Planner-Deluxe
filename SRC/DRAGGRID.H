#ifndef __draggrid__
#define __draggrid__

_CLASSDEF( TDragGrid )

#ifndef __rulegrid__
#include "rulegrid.h"
#endif
#ifndef __cursor__
#include "cursor.h"
#endif
#ifndef __dragwin__
#include "dragwin.h"
#endif
#ifndef __worktime__
#include "worktime.h"
#endif

#define AT_LEAST( X, Y ) (max( X, Y ))
#define AT_MOST( X, Y )  (min( X, Y ))

class _CLASSTYPE TDragGrid: public TRuleGrid {

    public:

    typedef enum {

        kGrowStart,
        kDrag,
        kGrowEnd
        } TDragMode;

    typedef enum {

        kHorizontal,
        kVertical,
        kBoth
        } TScrollDirection;

    // constructors
    TDragGrid( PTWindowsObject AParent, TScrollDirection direction,
        int deltaCol );
    virtual ~TDragGrid( void );

    protected:

    virtual BOOL CursorForDrag( TCursorStyle &style, RTMousInfo mi );

    // initialization
    virtual BOOL   MousDragInit( RTMousInfo mi );
    virtual void   MousDragOver( RTMousInfo mi );
    virtual void   MousDragRelease( RTMousInfo );

    void SetScrollDirection( TScrollDirection direction ) { itsScrollDirection = direction; }

    virtual BOOL IsAllowedToDrag( RTNote note, char *message );
    virtual void OpenDragWindow( BOOL hasMessage );
    virtual void MoveDragWindow( POINT p );
    virtual void PlaceDragWindow( void );
    virtual void CloseDragWindow( void );

    virtual int  FrozenHeight( PTPaint painter );
    virtual int  FrozenWidth( PTPaint painter );

    virtual TTimeUnit GetPrecision( void );

    PTDragWin        itsDragWinP;
    TCursorStyle     itsGrowStartCursor;
    TCursorStyle     itsGrowEndCursor;
    TCursorStyle     itsDragCursor;
    PTNote           itsNoteP;
    RECT             itsMarqueeRect;
    TDuration        itsOffset;
    TTimeComplex     itsNewTimes;
    TTimeUnit        itsPrecision;

    private:

    // dragging operations
    virtual void   DragActive( RTMousInfo mi );
    virtual void   ScrollTime( RTMousInfo mi, int deltaRow, int deltaCol );

    // actual dragging
    virtual void   DragStart(     RTMousInfo mi );
    virtual void   DragEnd(       RTMousInfo mi );
    virtual void   DragBoth(      RTMousInfo mi );

    virtual BOOL   ValidDragCell( TDragMode mode, int row, int col ) = 0;
    virtual void   GetNoteRects( RTNote note, RTMousInfo mi, RECT &growStartR, RECT &dragR, RECT &growEndR, RECT &marqueeRect ) = 0;
    virtual void   PointToTime( RTMousInfo mi, RTTime time ) = 0;
    virtual RTNote PointToNote( RTMousInfo mi ) = 0;
    virtual void   SetStart( RTTime start ) = 0;
    virtual void   SetEnd( RTTime end ) = 0;
    virtual void   SetBoth( RTMousInfo mi, RTTime start ) = 0;
    virtual void   Release( void ) = 0;
    virtual void   ConstrainMarqueeRect( TDragMode mode, RECT &r ) = 0;

    // autoscrolling dragging
    virtual void   ScrollStart(   RTMousInfo mi, int deltaRow, int deltaCol ) = 0;
    virtual void   ScrollEnd(     RTMousInfo mi, int deltaRow, int deltaCol ) = 0;
    virtual void   ScrollBoth(    RTMousInfo mi, int deltaRow, int deltaCol ) = 0;

    // miscellany
    virtual void    DrawMarquee(  RECT &r, BOOL on );

    TDragMode        itsMode;
    POINT            itsAnchorPoint;

    RECT             itsDragWinR;
    POINT            itsLastPoint;
    TScrollDirection itsScrollDirection;
    int              itsDeltaCol;
    BOOL             itsAllowedToDrag;
    BOOL             itsMarqueeVisible;
    TWorkTime        itsWorkTime;

};

#endif
