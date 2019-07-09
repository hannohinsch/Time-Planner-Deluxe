#ifndef __rulegrid__
#define __rulegrid__

#include "mousgrid.h"

#ifndef __apptime__
#include "apptime.h"
#endif

_CLASSDEF( TRuleGrid )

class _CLASSTYPE TRuleGrid: public TMousGrid
{
public:
    TRuleGrid( PTWindowsObject AParent, WORD resID );
    TRuleGrid( PTWindowsObject AParent );
    TRuleGrid( PTWindowsObject AParent, WORD anID, int x, int y, int dx, int dy );
    virtual~TRuleGrid( void );

protected:
    virtual int  GridWidth( PTPaint, int /*col*/ ) { return 1; };
    virtual int  GridHeight( PTPaint, int /*row*/ ) { return 1; };

    virtual int  TopLineHeight( PTPaint ) { return 1; };
    virtual int  LeftLineWidth( PTPaint ) { return 1; };

    virtual void PaintTopLine( RTDrawInfo );
    virtual void PaintLeftLine( RTDrawInfo );
    virtual void PaintGridCol( RTDrawInfo );
    virtual void PaintGridRow( RTDrawInfo );

};

#endif
