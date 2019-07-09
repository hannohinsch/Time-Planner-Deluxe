#ifndef __alarmd__
#define __alarmd__

_CLASSDEF( TAlarmDialog )
_CLASSDEF( TAlarmRingsDialog )

#ifndef __alarm__
#include "alarm.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif

class _CLASSTYPE TAlarmDialog: public TOurDialog {

public:

    TAlarmDialog( PTWindowsObject AParent, PTAlarm alarmP );
    virtual ~TAlarmDialog( void );

private:

    void SetupWindow( void );

    void HandleUnit(    RTMessage msg )= [ID_FIRST + IDD_ALARM_UNITS];
    void WMHScroll(     RTMessage Msg) = [ WM_FIRST + WM_HSCROLL   ];

    PTStatic      itsTextP;
    PTScrollBar   itsScrollBarP;
    PTComboBox    itsUnitP;

    PTAlarm       itsAlarmP;
};

class _CLASSTYPE TAlarmRingsDialog: public TOurDialog {

    public:

    TAlarmRingsDialog( PTWindowsObject AParent, PTAlarm alarmP );
    virtual ~TAlarmRingsDialog( void );

    private:

    void SetupWindow( void );

    void HandleShowNote( RTMessage msg )= [ID_FIRST + IDD_ALARM_SHOW_NOTE];
    void Ok(             RTMessage msg )= [ID_FIRST + IDOK];
    void Cancel(         RTMessage msg )= [ID_FIRST + IDCANCEL];

    PTStatic      itsNoteTextCtl;
    PTStatic      itsNoteTimeCtl;
    PTStatic      itsTimeNowCtl;

    PTButton      itsSnoozeCtl;
    PTButton      itsShowNoteCtl;

    PTAlarm       itsAlarmP;
};

#endif

