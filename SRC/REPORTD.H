#ifndef __reportd__
#define __reportd__

_CLASSDEF( TReportD )

#ifndef __rptgrid__
#include "rptgrid.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif


class _CLASSTYPE TReportD: public TOurDialog
{
public:
    TReportD( PTWindowsObject parent, WORD id );
    virtual ~TReportD();

private:
    void SetupWindow( void );
    void Ok( RTMessage msg )         = [ID_FIRST + IDOK ];
    void SetDefault( RTMessage msg ) = [ID_FIRST + IDC_SET_DEFAULT];
    void UseDefault( RTMessage msg ) = [ID_FIRST + IDC_USE_DEFAULT];

    void SetValues( void  );

    PTSafStr        itsFieldNames[ kNumFields ];
    PTComboBox      itsFieldBoxCP[ kMaxReportCols ];

};

#endif
