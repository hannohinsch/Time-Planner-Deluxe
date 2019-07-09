#ifndef __setmond__
#define __setmond__

_CLASSDEF( TSetMonthD )

#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif



class _CLASSTYPE TSetMonthD: public TOurDialog
{
    public:

    TSetMonthD( PTWindowsObject aParent, PTTime dateP );
    virtual ~TSetMonthD();

    private:

    void Jan(    RTMessage ) = [ ID_FIRST + IDD_CALMON_JAN ];
    void Feb(    RTMessage ) = [ ID_FIRST + IDD_CALMON_FEB ];
    void Mar(    RTMessage ) = [ ID_FIRST + IDD_CALMON_MAR ];
    void Apr(    RTMessage ) = [ ID_FIRST + IDD_CALMON_APR ];
    void May(    RTMessage ) = [ ID_FIRST + IDD_CALMON_MAY ];
    void Jun(    RTMessage ) = [ ID_FIRST + IDD_CALMON_JUN ];
    void Jul(    RTMessage ) = [ ID_FIRST + IDD_CALMON_JUL ];
    void Aug(    RTMessage ) = [ ID_FIRST + IDD_CALMON_AUG ];
    void Sep(    RTMessage ) = [ ID_FIRST + IDD_CALMON_SEP ];
    void Oct(    RTMessage ) = [ ID_FIRST + IDD_CALMON_OCT ];
    void Nov(    RTMessage ) = [ ID_FIRST + IDD_CALMON_NOV ];
    void Dec(    RTMessage ) = [ ID_FIRST + IDD_CALMON_DEC ];

    PTTime   itsDateP;
};

#endif
