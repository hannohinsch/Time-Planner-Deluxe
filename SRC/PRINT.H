#ifndef __print__
#define __print__

_CLASSDEF( TPrint )
_CLASSDEF( TPrtStatusDlg )

_CLASSDEF( TRawGrid )
_CLASSDEF( TAddrGrid )
_CLASSDEF( TPrintView )
_CLASSDEF( TPaint )
_CLASSDEF( TSafStr )
_CLASSDEF( TPrintMaster )

#ifndef __ourdlg__
#include "ourdlg.h"
#endif

class _CLASSTYPE TPrint
{
friend TPrtStatusDlg;
friend TPrintView;

public:
    TPrint( PTWindowsObject aParent );
    TPrint( void );
    virtual ~TPrint();

    static BOOL SetupPrinter( PTWindowsObject aParent );
    static void GetPrtPixels( RECT *r );                   //get size in pixels
    static void GetPageOrientation( RTSafStr );
    static void GetPrtName( RTSafStr );

    static HDC hPrintDC;            //shared by all instances of TPrint

protected:
    BOOL Print( PTPaint, WORD hPage, WORD vPage );
    virtual void PrintPage( PTPaint, WORD hPage, WORD vPage ) {};

    static void SetupDC( void );

    static PTWindowsObject pParent;

private:
    BOOL PrintSub( PTPaint, WORD hPage, WORD vPage );

    //callback abort fn
    static BOOL FAR PASCAL AbortProc( HDC hdcPrn, short nCode );

    //these variables are used while sending the job to Windows
    //! must be static, so that TPrtStatusDlg can access them!
    static BOOL bUserAbort;
    static HWND hStatusDlg;
    static PTPrtStatusDlg pStatusDlg;

    //shared by all TPrint objects
    static PRINTDLG pd;
    static BOOL bValidPD;
    static int objectCount;

    //printer/page characteristics
    static WORD vDPI;
    static WORD hDPI;

    static RECT rHeader;
    static RECT rBody;
    static RECT rFooter;

};

///////////////////////////////////////////////////////////////////////////////
//          PrintView
//////////////////////////////////////////////////////////////////////////////
class _CLASSTYPE TPrintView: public TPrint
{
public:
    TPrintView( PTWindowsObject aParent );
    virtual ~TPrintView();

    void PrepareToPrint( PTRawGrid gridP, WORD id );
    void PrintPage( PTPaint, WORD hPage, WORD vPage );

private:
    PTRawGrid itsGridP;

};

class _CLASSTYPE TPrtStatusDlg: public TOurDialog
{
public:
    TPrtStatusDlg(PTWindowsObject aParent, WORD anID);
    void SetMsg( char * );

private:
    void SetupWindow( void );
    void Cancel( RTMessage )= [ ID_FIRST + IDCANCEL ];

    PTStatic itsStatusMsg;
};

#endif
