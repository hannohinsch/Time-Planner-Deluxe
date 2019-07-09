#ifndef __timepick__
#define __timepick__

_CLASSDEF( TTimePicker );
_CLASSDEF( TMarkCtl )

#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __duration__
#include "duration.h"
#endif
#ifndef __datepick__
#include "datepick.h"
#endif
#ifndef __note__
#include "note.h"
#endif
#ifndef __ticker__
#include "ticker.h"
#endif
#ifndef __freq__
#include "freq.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif


class _CLASSTYPE TTimePicker: public TOurDialog {

public:

    TTimePicker( PTWindowsObject AParent, int AResID, char *prompt,
        PTTime startTimeP, PTTime endTimeP, PTFreq freqP,
        PTDuration durationP, TTimeUnit *unit, RTColor color );
    virtual ~TTimePicker( void );

private:

    void SetupWindow();

    // control handlers
    void HandleUnit( RTMessage Msg )       = [ ID_FIRST + IDD_TIMEPICKER_UNITS ];
    void HandleUseDuration( RTMessage Msg )= [ ID_FIRST + IDD_TIMEPICKER_USE_DURATION ];
    void HandleUseTimes( RTMessage Msg )   = [ ID_FIRST + IDD_TIMEPICKER_USE_TIMES ];
    void HandleUseMilestone( RTMessage Msg )=[ ID_FIRST + IDD_TIMEPICKER_MILESTONE ];
    void HandleTicker( RTMessage Msg )     = [ ID_FIRST + IDD_TIMEPICKER_TICKER ];
    void HandleCalendar( RTMessage Msg )   = [ ID_FIRST + IDD_MONTH_PICKER ];
    void HandleNoTime( RTMessage Msg )     = [ ID_FIRST + IDD_TIMEPICKER_NO_TIME ];
    void Ok( RTMessage Msg )                = [ ID_FIRST + IDOK ];
    void Cancel( RTMessage Msg )            = [ ID_FIRST + IDCANCEL ];
    void HandleNow( RTMessage Msg )        = [ ID_FIRST + IDD_TIMEPICKER_NOW ];

    // utilities
    void UpdateDialog( void );
    void ShowDateControls( TTickerMode mode );


    // real data
    PTTime           itsStartTimeP;
    PTTime           itsEndTimeP;
    PTDuration       itsDurationP;
    double           itsDelta;
    TTimeUnit       *itsTimeUnitP;
    PTFreq           itsFreqP;

    // controls (no transfers because of combobox bug in borland)
    PTDatePick       itsMonthPickerCP;
    PTTicker         itsTickerCP;
    PTComboBox       itsTimeUnitBoxCP;
    PTStatic         itsDurationTextCP;
    PTStatic         itsWorkTimeTextCP;
    PTStatic         itsStartTextCP;
    PTStatic         itsEndTextCP;
    PTGroupBox       itsRadioGroupCP;
    PTRadioButton    itsUseDurationCP;
    PTRadioButton    itsUseTimesCP;
    PTRadioButton    itsUseMilestoneCP;

    PTStatic         itsDurationTitleCP;
    PTStatic         itsWorkTimeTitleCP;
    PTStatic         itsStartTitleCP;
    PTStatic         itsEndTitleCP;
    PTBButton        itsNowButtonCP;
    PTBButton        itsNoTimeButtonCP;

    PTMarkCtl       itsFloatCP;
    PTMarkCtl       itsItemCP;
    PTMarkCtl       itsMilestoneCP;
    PTMarkCtl       itsTaskCP;

};

#endif
