//////////////////////////////////////////////////////////////////////////////
//
//  adds gridlines ("rules") to a grid object
//
//////////////////////////////////////////////////////////////////////////////
#include "standard.h"
#pragma hdrstop

#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __mousgrid__
#include "mousgrid.h"
#endif
#ifndef __paint__
#include "paint.h"
#endif
#ifndef __profile__
#include "profile.h"
#endif
#ifndef __rulegrid__
#include "rulegrid.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif

//*****************************************************************************
//
//*****************************************************************************
TRuleGrid::TRuleGrid ( PTWindowsObject AParent, WORD resID ): TMousGrid( AParent, resID )
{
}

//*****************************************************************************
//
//*****************************************************************************
TRuleGrid::TRuleGrid ( PTWindowsObject AParent, WORD anID, int x, int y, int dx, int dy ):
     TMousGrid( AParent, anID, x, y, dx, dy )
{
}

//*****************************************************************************
// all the zeros are filled in after creation.....
//*****************************************************************************
TRuleGrid::TRuleGrid ( PTWindowsObject AParent ):
     TMousGrid( AParent, 0, 0, 0, 0, 0 )
{
}

//*****************************************************************************

//*****************************************************************************
TRuleGrid::~TRuleGrid( void )
{
}

//*****************************************************************************

//*****************************************************************************
void TRuleGrid::PaintGridCol( RTDrawInfo di  )
{
di.pPainter->GridLine( di, di.cellRect );
}


//*****************************************************************************

//*****************************************************************************
void TRuleGrid::PaintGridRow( RTDrawInfo di  )
{
di.pPainter->GridLine( di, di.cellRect );
}

//*****************************************************************************

//*****************************************************************************
void TRuleGrid::PaintTopLine( RTDrawInfo di )
{
di.pPainter->GridLine( di, di.cellRect );
}

//*****************************************************************************

//*****************************************************************************
void TRuleGrid::PaintLeftLine( RTDrawInfo di )
{
di.pPainter->GridLine( di, di.cellRect );
}

