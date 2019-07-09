#ifndef __durfmtd__
#define __durfmtd__

#ifndef __snoop__
#include "snoop.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif

_CLASSDEF( TDurFmtD )

const int kNumTabButtons = 3;

class _CLASSTYPE TDurFmtD: public TOurDialog, TSnoop {

public:

    TDurFmtD( PTWindowsObject AParent, int AResID );
    virtual ~TDurFmtD( void );

private:

    void Ok( RTMessage msg ) = [ID_FIRST + IDOK];

    PTRadioButton itsButtonsP[kNumTabButtons];
    PTRadioButton itsMixButtonP;
    PTRadioButton itsDecimalButtonP;

    struct {

        WORD      buttonValues[kNumTabButtons];
        WORD      mixValues[2];
        WORD      dayValues[7];


    } itsTransferBuffer;
};

#endif
