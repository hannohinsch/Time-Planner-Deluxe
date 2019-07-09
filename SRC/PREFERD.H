#ifndef __preferd__
#define __preferd__

#ifndef __ourdlg__
#include "ourdlg.h"
#endif

_CLASSDEF( TPreferD )

class _CLASSTYPE TPreferD: public TOurDialog
{
public:
    TPreferD( PTWindowsObject aParent, WORD anID );
    virtual ~TPreferD();

private:
    void SetupWindow( );
    void Ok( RTMessage );

    PTGroupBox      itsDspGroupCP;
    PTRadioButton   itsStdButtonCP;
    PTRadioButton   itsLCDButtonCP;

    PTCheckBox      itsAutoLoadCP;
    PTCheckBox      itsShowTipsCP;

    PTGroupBox      itsChkGroupCP;
    PTRadioButton   itsChkNoneCP;
    PTRadioButton   itsChkSelectCP;
    PTRadioButton   itsChkAllCP;

};

#endif
