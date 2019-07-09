#ifndef __addrbook_
#define __addrbook__

_CLASSDEF( TAddrGrid )
_CLASSDEF( TCatList )

#ifndef __rawgrid__
#include "rawgrid.h"
#endif

class _CLASSTYPE TAddrGrid: public TRawGrid
{
public:
    TAddrGrid( PTCatList );
    virtual ~TAddrGrid();

    void PrintPrep( BOOL markedOnly );

protected:
    int  CellWidth( PTPaint, int col );
    int  CellHeight( PTPaint, int row );

    void PaintCell( RTDrawInfo di );

private:
    PTCatList itsCatListP;
    PTCatList itsMasterListP;
};



#endif
