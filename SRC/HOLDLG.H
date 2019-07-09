#ifndef __holdlg__
#define __holdlg__

_CLASSDEF( THolidayDialog )
_CLASSDEF( TMonthCalendarCtl )
_CLASSDEF( TXArray )

#ifndef __holiday__
#include "holiday.h"
#endif
#ifndef __holcal__
#include "holcal.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif

typedef char Str255[256];

const int kNumMonths = 6;

class _CLASSTYPE THolidayDialog: public TOurDialog {

    public:

    THolidayDialog( PTWindowsObject AParent, int AResID,
        RTTime startDate );
    virtual ~THolidayDialog( void );

	void HandleCalendar1(  RTMessage Msg ) = [ ID_FIRST + IDD_HOLIDAY_MONTH_1 ];
	void HandleCalendar2(  RTMessage Msg ) = [ ID_FIRST + IDD_HOLIDAY_MONTH_2 ];
	void HandleCalendar3(  RTMessage Msg ) = [ ID_FIRST + IDD_HOLIDAY_MONTH_3 ];
	void HandleCalendar4(  RTMessage Msg ) = [ ID_FIRST + IDD_HOLIDAY_MONTH_4 ];
	void HandleCalendar5(  RTMessage Msg ) = [ ID_FIRST + IDD_HOLIDAY_MONTH_5 ];
	void HandleCalendar6(  RTMessage Msg ) = [ ID_FIRST + IDD_HOLIDAY_MONTH_6 ];
    void HandleCountry(    RTMessage Msg ) = [ ID_FIRST + IDD_HOLIDAY_COUNTRY ];
    void HandleScroll(     RTMessage )     = [ ID_FIRST + IDD_HOLIDAY_SCROLL ];
    void Ok(               RTMessage )     = [ ID_FIRST + IDOK ];
    void Cancel(           RTMessage )     = [ ID_FIRST + IDCANCEL ];

	private:

    void SetupWindow();
    void ShowDates();

	void HandleCalendar( RTMessage Msg, int monthIndex );
    void MarkDate( RTTime date, THolCal::TCellType cellType );
    THolCal::TCellType GetMark( RTTime date );

    PTHoliday FindHoliday( RTTime date );
    void AddHoliday( void );
    void DeleteHoliday( void );

    PTHolCal        itsMonthCtls[kNumMonths];
    PTStatic        itsMonthNameCtls[kNumMonths];
    PTStatic        itsPromptCtl;
    PTScrollBar     itsScrollCtl;
    PTListBox       itsCountryCtl;
    TTime           itsSelectedDate;
    TTime           itsFirstVisibleDate;
    TTime           itsLastVisibleDate;


    PTXArray        itsHolidaysP;
    TCountry        itsOldCountry;

	struct {

	    Str255	monthName[kNumMonths];
		char	prompt[256];

	} itsTransferBuffer;

};


#endif
