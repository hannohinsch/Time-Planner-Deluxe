#ifndef __filterd__
#define __filterd__

_CLASSDEF( TCatBox )
_CLASSDEF( TFilterDlg)
_CLASSDEF( TFilter )
_CLASSDEF( TDatePick )

#ifndef __noteview__
#include "noteview.h"       //needed for edit box lengths
#endif
#ifndef __catlist__
#include "catlist.h"         //needed for matchstyle
#endif
#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif

const MAX_SEARCH_LEN= 100;

//dialog object, used to create filter object
class _CLASSTYPE TFilterDlg : public TOurDialog {
    public:
        TFilterDlg( PTWindowsObject AParent, int AResID, PTFilter pFilterArg );
        virtual ~TFilterDlg();

    private:

        void SetupWindow( void );

        void Ok( RTMessage Msg )=                   [ ID_FIRST + IDOK ];
        void OnClearAll( RTMessage Msg ) =          [ ID_FIRST + IDC_FILTER_CLEAR_ALL ];

        PTEdit          itsFindTextCP;
        PTRadioButton   itsAnyRadioCP;
        PTRadioButton   itsEveryRadioCP;

        PTRadioButton   itsAnyDateCP;
        PTRadioButton   itsBeforeCP;
        PTRadioButton   itsAfterCP;
        PTRadioButton   itsDayCP;
        PTRadioButton   itsWeekCP;
        PTRadioButton   itsMonthCP;
        PTDatePick      itsDateCP;

        PTCheckBox      itsUndatedCP;
        PTCheckBox      itsMilestoneCP;
        PTCheckBox      itsFloatCP;
        PTCheckBox      itsTaskCP;

        PTCheckBox      itsRedCP;
        PTCheckBox      itsRedGrayCP;
        PTCheckBox      itsGrayCP;
        PTCheckBox      itsBlueGrayCP;
        PTCheckBox      itsBlueCP;

        PTCheckBox      itsDoneCP;
        PTCheckBox      itsUndoneCP;
        PTCheckBox      itsAlarmCP;
        PTCheckBox      itsNoAlarmCP;
        PTCheckBox      itsFreqCP;
        PTCheckBox      itsNoFreqCP;

        PTFilter        itsFilterP;
    };

#endif
