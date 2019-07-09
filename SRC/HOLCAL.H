#ifndef __holcal__
#define __holcal__

_CLASSDEF( THolCal )

#ifndef __calctl__
#include "calctl.h"
#endif
#ifndef __holiday__
#include "holiday.h"
#endif

static const int kNumDays = 32;

#define MOUSE_MESSAGE 50

class _CLASSTYPE THolCal: public TCalCtl {

public:

    typedef enum {

        kNormalCell,
        kHolidayCell,
        kFixedHolidayCell,
        kNotACell

    } TCellType;

    THolCal( PTWindowsObject AParent, WORD resID, RTTime startDate );
    virtual ~THolCal( void );

    void BeginCellPaint( RTDrawInfo );
    int  CellWidth( PTPaint, int col );
    int  CellHeight( PTPaint, int row );
    void PaintCell( RTDrawInfo );
    void DrawDay( RTDrawInfo );

    void SetCellType( RTTime date, TCellType cellType );
    TCellType GetCellType( RTTime date );
    void ClearCells( void );

    virtual LPSTR  GetClassName( void ) { return "HolCal";}
    virtual void GetWindowClass( WNDCLASS & AWndClass );

    void MousOver( RTMousInfo mi );
    void GetMouseDate( TCellType &cellType, RTTime day );

    void MousRtClick( RTMousInfo );

	private:

    TCellType itsCellTypes[kNumDays];

    int       itsMouseDay;

};

#endif
