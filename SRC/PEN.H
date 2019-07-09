#ifndef __pen__
#define __pen__

_CLASSDEF( TPen )

#include "penwin.h"

//typedefs for pen DLL functions
typedef void ( FAR PASCAL *PTRegisterPenApp) (WORD, BOOL);
typedef HPENDATA ( FAR PASCAL *PTDuplicatePenData) ( HPENDATA, WORD );
typedef HPENDATA ( FAR PASCAL *PTCompactPenData ) ( HPENDATA, WORD );
typedef void ( FAR PASCAL *PTDrawPenData) ( HDC, LPRECT, HPENDATA );
typedef BOOL ( FAR PASCAL *PTMetricScalePenData) (HPENDATA, WORD);
typedef BOOL ( FAR PASCAL *PTSetRecogHook) (WORD, WORD, HWND );
typedef BOOL ( FAR PASCAL *PTGetPenDataInfo) ( HPENDATA, LPPENDATAHEADER, LPPENINFO, DWORD );
typedef BOOL ( FAR PASCAL *PTOffsetPenData) ( HPENDATA, int, int );
typedef BOOL ( FAR PASCAL *PTResizePenData) (HPENDATA, LPRECT);

class _CLASSTYPE TPen {

public:
    TPen( void );
    virtual~TPen( void );

    static BOOL bPenActive;
    static HANDLE hPenDLL;

	static PTRegisterPenApp lpfnRegisterPenApp;
	static PTDuplicatePenData lpfnDuplicatePenData;
	static PTCompactPenData lpfnCompactPenData;
	static PTDrawPenData lpfnDrawPenData;
	static PTMetricScalePenData lpfnMetricScalePenData;
	static PTSetRecogHook lpfnSetRecogHook;
    static PTGetPenDataInfo lpfnGetPenDataInfo;
    static PTOffsetPenData lpfnOffsetPenData;
    static PTResizePenData lpfnResizePenData;
    };

#endif
