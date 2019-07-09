#ifndef __workdlg__
#define __workdlg__

#ifndef __snoop__
#include "snoop.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif

_CLASSDEF( TTimeOnlyEdit )
_CLASSDEF( TWorkTimeD )

class _CLASSTYPE TWorkTimeD: public TOurDialog, TSnoop {

public:

    TWorkTimeD( PTWindowsObject AParent, int AResID );
    virtual ~TWorkTimeD( void );

private:

    void SetupWindow( void );
    void Handle9AM(      RTMessage msg ) = [ID_FIRST + IDD_WORK_DAY_STARTS_AT_9];
    void HandleDefault1( RTMessage msg ) = [ID_FIRST + IDD_CONVERSION_DEFAULT_1];
    void Ok( RTMessage msg ) = [ID_FIRST + IDOK];

    // controls for WorkStart
    PTBButton      its9AMButtonCP;
    PTTimeOnlyEdit itsStartTimeCP;

    // controls for HoursPerDay
    PTBButton     itsHoursButtonCP;
    PTEdit        itsHoursValueCP;

    // controls for DaysPerWeek
    PTGroupBox    itsDaysGroupCP;
    PTCheckBox    itsDayBoxes[7];

    struct {

        char      hoursPerDay[256];
        WORD      dayValues[7];


    } itsTransferBuffer;
};
#endif
