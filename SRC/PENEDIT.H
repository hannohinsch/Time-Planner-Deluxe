#ifndef __penedit__
#define __penedit__

_CLASSDEF( TPenEdit )

#ifndef __pen__
#include "pen.h"
#endif
#ifndef __safstr__
#include "safstr.h"
#endif

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// PenEdit
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
class _CLASSTYPE TPenEdit: public TEdit
{
public:
	TPenEdit( PTWindowsObject aParent, int anID, PTInk anInk= NULL, BOOL aBInkMode= FALSE );
	virtual ~TPenEdit();

    void SetupWindow( void );

	void ForceUpdate( void );

    void Set( RTSafStr safStr );
    RTSafStr GetString( void );
    void Clear( void );

    void SetInkMode( void );
	void SetTextMode( void );
	BOOL IsInkMode( void ) { return bInkMode; };
	void Recognize( void );


private:

    void WMKillFocus( RTMessage )=[ WM_FIRST + WM_KILLFOCUS ];
    void WMSetFocus( RTMessage )=[ WM_FIRST + WM_SETFOCUS ];
    void WMKeyDown( RTMessage )= [ WM_FIRST + WM_KEYDOWN ];

    void DefNotificationProc( RTMessage Msg );

	TSafStr     itsText;
    BOOL        bInkMode;
    PTInk       pInk;
};

#endif
