//*****************************************************************************
// pen initialization stuff: get addresses of dll functions
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "pen.h"

HANDLE TPen::hPenDLL= (HANDLE) NULL;
BOOL TPen::bPenActive= FALSE;

PTRegisterPenApp   TPen:: lpfnRegisterPenApp= NULL;
PTDuplicatePenData TPen:: lpfnDuplicatePenData= NULL;
PTCompactPenData   TPen:: lpfnCompactPenData= NULL;
PTDrawPenData	   TPen:: lpfnDrawPenData= NULL;
PTMetricScalePenData TPen:: lpfnMetricScalePenData= NULL;
PTSetRecogHook		TPen:: lpfnSetRecogHook= NULL;
PTGetPenDataInfo    TPen:: lpfnGetPenDataInfo= NULL;
PTOffsetPenData     TPen:: lpfnOffsetPenData= NULL;
PTResizePenData     TPen:: lpfnResizePenData= NULL;

//*****************************************************************************
//pen window registration
//*****************************************************************************
TPen::TPen( void )
{
BOOL error= FALSE;

bPenActive= FALSE;

hPenDLL= GetSystemMetrics( SM_PENWINDOWS );
if( hPenDLL != NULL )
    {
	if( NULL == (lpfnRegisterPenApp= (PTRegisterPenApp) GetProcAddress( hPenDLL, "RegisterPenApp") ))
        error= TRUE;

	if( NULL == (lpfnDuplicatePenData= (PTDuplicatePenData) GetProcAddress( hPenDLL, "DuplicatePenData" ) ))
        error= TRUE;

	if( NULL == (lpfnCompactPenData= (PTCompactPenData) GetProcAddress( hPenDLL, "CompactPenData" ) ))
        error= TRUE;

	if( NULL == (lpfnDrawPenData= (PTDrawPenData) GetProcAddress( hPenDLL, "DrawPenData" ) ))
        error= TRUE;

	if( NULL == (lpfnMetricScalePenData= (PTMetricScalePenData) GetProcAddress( hPenDLL, "MetricScalePenData" ) ))
        error= TRUE;

	if( NULL == (lpfnSetRecogHook= (PTSetRecogHook) GetProcAddress( hPenDLL, "SetRecogHook" ) ))
        error= TRUE;

    if( NULL == (lpfnGetPenDataInfo= (PTGetPenDataInfo) GetProcAddress( hPenDLL, "GetPenDataInfo" ) ))
        error= TRUE;

    if( NULL == (lpfnOffsetPenData= (PTOffsetPenData) GetProcAddress( hPenDLL, "OffsetPenData" ) ))
        error= TRUE;

    if( NULL == (lpfnResizePenData= (PTResizePenData) GetProcAddress( hPenDLL, "ResizePenData" ) ))
        error= TRUE;

	if( lpfnRegisterPenApp != NULL )
        (*lpfnRegisterPenApp)( RPA_DEFAULT, TRUE );


    if( !error )
        bPenActive= TRUE;
    }

}

//*****************************************************************************

//*****************************************************************************
TPen::~TPen( void )
{
if( bPenActive )
    (*lpfnRegisterPenApp)( RPA_DEFAULT, FALSE );
}

