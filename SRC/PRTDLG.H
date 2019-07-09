#ifndef __prtdlg__
#define __prtdlg__

#ifndef __rc__
#include "rc.h"
#endif
#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif

_CLASSDEF( TPrtViewDlg )
_CLASSDEF( TRawGrid )


//*****************************************************************************

//*****************************************************************************
class _CLASSTYPE TPrtViewDlg: public TOurDialog
{
public:
    TPrtViewDlg( PTWindowsObject, WORD anID, PTRawGrid gridP );
    virtual ~TPrtViewDlg() { };

    static BOOL itsShowPage;
    static BOOL itsShowDate;

private:
    void SetupWindow( void );
    void Ok( RTMessage )= [ID_FIRST + IDOK];
    void SetupPrinter( RTMessage )= [ID_FIRST + IDD_PRTSETUP];

    void Redisplay();

    void WMHScroll( RTMessage ) = [ WM_FIRST + WM_HSCROLL ];
    void HandleMarkedOnly( RTMessage ) = [ ID_FIRST + IDD_PRTMARKEDONLY ];

    void HStartSpinHandler( RTMessage );
    void HEndSpinHandler( RTMessage );
    void VStartSpinHandler( RTMessage );
    void VEndSpinHandler( RTMessage );

    //linked grid
    PTRawGrid itsGridP;

    //control objects
    PTEdit pTitle;

    PTCheckBox  itsDateCP;
    PTCheckBox  itsPageCP;
    PTCheckBox  itsMarkCP;

    PTStatic pHStart;
    PTScrollBar pHStartSpin;
    PTStatic pHEnd;
    PTScrollBar pHEndSpin;

    PTStatic pVStart;
    PTScrollBar pVStartSpin;
    PTStatic pVEnd;
    PTScrollBar pVEndSpin;

    PTStatic pHorzPageNum;
    PTStatic pVertPageNum;
    PTStatic pOrientation;
    PTStatic pPrtName;

    BOOL    itsVSpinEnabled;
    BOOL    itsHSpinEnabled;
    BOOL    itsMarkEnabled;
    BOOL    itsMark;
    };

#endif
