#ifndef __dialsetd__
#define __dialsetd__

#ifndef __ourdlg__
#include "ourdlg.h"
#endif

_CLASSDEF( TDialSetupDialog )


class _CLASSTYPE TDialSetupDialog: public TOurDialog
    {
    public:

    TDialSetupDialog( PTWindowsObject parent );
    virtual ~TDialSetupDialog( void );

    private:

    void Ok( RTMessage )          = [ID_FIRST + IDOK];
    void SetupWindow( void );

    PTEdit        itsLocalPrefixCP;
    PTEdit        itsLocalSuffixCP;
    PTEdit        itsLongDistancePrefixCP;
    PTEdit        itsLongDistanceSuffixCP;
    PTEdit        itsInternationalPrefixCP;
    PTEdit        itsInternationalSuffixCP;
    PTEdit        itsAlternatePrefixCP;
    PTEdit        itsAlternateSuffixCP;
    PTEdit        itsLocalAreaCodeCP;
    PTComboBox    itsBaudCP;
    PTComboBox    itsPortCP;
    PTGroupBox    itsGroupCP;
    PTRadioButton itsPulseCP;
    PTRadioButton itsToneCP;

    };

#endif
