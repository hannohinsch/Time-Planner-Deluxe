#ifndef __dayviewd__
#define __dayviewd__

#ifndef __ourdlg__
#include "ourdlg.h"
#endif

_CLASSDEF( TDayviewD )

const int kNumButtons        = 9;

class _CLASSTYPE TDayviewD: public TOurDialog {

public:

    TDayviewD( PTWindowsObject AParent, int AResID );
    virtual ~TDayviewD( void );

private:

    void Ok( RTMessage msg ) = [ID_FIRST + IDOK];
    void SetDefault( RTMessage msg ) = [ID_FIRST + IDC_SET_DEFAULT];
    void UseDefault( void );
    void SetValue( void );

    PTGroupBox    itsGroupBoxP;
    PTRadioButton itsButtonsP[kNumButtons];
    PTButton      itsOkButtonP;
    PTButton      itsCancelButtonP;

    WORD          itsTransferBuffer[kNumButtons];

};

#endif

