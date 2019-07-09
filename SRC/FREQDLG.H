#ifndef __freqdlg__
#define __freqdlg__

_CLASSDEF( TFrequencyDialog );
_CLASSDEF( TMonthCalendarCtl );

#ifndef __Freq__
#include "freq.h"
#endif
#ifndef __freqcal__
#include "freqcal.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif

typedef char Str255[256];

const int kNumMonths = 6;

class _CLASSTYPE TFrequencyDialog: public TOurDialog {

    public:

    TFrequencyDialog( PTWindowsObject AParent, int AResID,
        PTFreq FreqP, PTTime noteStartP );
    virtual ~TFrequencyDialog( void );

	void HandleCalendar1(  RTMessage Msg ) = [ ID_FIRST + IDD_MONTH_1 ];
	void HandleCalendar2(  RTMessage Msg ) = [ ID_FIRST + IDD_MONTH_2 ];
	void HandleCalendar3(  RTMessage Msg ) = [ ID_FIRST + IDD_MONTH_3 ];
	void HandleCalendar4(  RTMessage Msg ) = [ ID_FIRST + IDD_MONTH_4 ];
	void HandleCalendar5(  RTMessage Msg ) = [ ID_FIRST + IDD_MONTH_5 ];
	void HandleCalendar6(  RTMessage Msg ) = [ ID_FIRST + IDD_MONTH_6 ];

    void SetStartDate(     RTMessage Msg)  = [ CM_FIRST + IDM_FIRSTDATE];
    void SetSecondDate(    RTMessage Msg)  = [ CM_FIRST + IDM_SECONDDATE];
    void SetEndDate(       RTMessage Msg)  = [ CM_FIRST + IDM_LASTDATE];
    void RemoveEndDate(    RTMessage Msg)  = [ CM_FIRST + IDM_REMOVE_LAST_DATE];
    void AddDate(          RTMessage Msg)  = [ CM_FIRST + IDM_ADDDATE];
    void ExcludeDate(      RTMessage Msg)  = [ CM_FIRST + IDM_EXCLUDEDATE];
    void RemoveAddedDate(  RTMessage Msg)  = [ CM_FIRST + IDM_DELETE_ADDED];
    void RestoreDate(      RTMessage Msg)  = [ CM_FIRST + IDM_RESTORE_EXCLUDED];

    void Ok(               RTMessage )     = [ ID_FIRST + IDOK ];
    void Cancel(           RTMessage )     = [ ID_FIRST + IDCANCEL ];

    void HandleScroll(     RTMessage )     = [ ID_FIRST + IDD_FREQ_SCROLL ];

    void SetupWindow();

	private:

	void HandleCalendar( int monthIndex );
    void ShowDates( void );
    void ShowPrompts( void );
    void MarkDate( RTTime date, TFreq::TCellType cellType );
    TFreq::TCellType GetMark( RTTime date );

    PTFreqCal       itsMonthCtls[kNumMonths];
    PTStatic        itsMonthNameCtls[kNumMonths];
    PTStatic        itsTextCtl;
    PTStatic        itsPromptCtl;
    PTScrollBar     itsScrollCtl;
    PTStatic        itsStartCtl;
    PTStatic        itsEndCtl;

    PTFreq          itsFreqP;
    TFreq           itsFreq;
    PTTime          itsNoteStartP;
    TTime           itsNoteStart;

    TTime           itsSecondDate;
    TTime           itsSelectedDate;

    TTime           itsFirstVisibleDate;
    TTime           itsLastVisibleDate;

	struct {

	    Str255	monthName[kNumMonths];
		char	frequency[256];
		char	prompt[256];
        char    start[256];
        char    end[256];

	} itsTransferBuffer;

};


#endif
