#ifndef __freqcal__
#define __freqcal__

_CLASSDEF( TFreqCal )

#ifndef __calctl__
#include "calctl.h"
#endif
#ifndef __freq__
#include "freq.h"
#endif

static const int kNumDays = 32;

class _CLASSTYPE TFreqCal: public TCalCtl {

public:

    TFreqCal( PTWindowsObject AParent, WORD resID, RTTime startDate );
    virtual ~TFreqCal( void );

    void BeginCellPaint( RTDrawInfo );
    int  CellWidth( PTPaint, int col );
    int  CellHeight( PTPaint, int row );
    void PaintCell( RTDrawInfo );
    void DrawDay( RTDrawInfo );

    void MousRtClick( RTMousInfo );

    void SetCellType( RTTime date, TFreq::TCellType cellType );
    TFreq::TCellType GetCellType( RTTime date );
    void ClearCells( void );

protected:

    LPSTR  GetClassName( void ) { return "FreqCal";}
    void GetWindowClass( WNDCLASS & AWndClass );

private:

    TFreq::TCellType itsCellTypes[kNumDays];

};

#endif
