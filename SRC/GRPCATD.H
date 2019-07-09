#ifndef __grpcatd__
#define __grpcatd__

_CLASSDEF( TGroupCatDialog )
_CLASSDEF( TCatBox )
_CLASSDEF( TCatBoxData )
_CLASSDEF( TCatList )
_CLASSDEF( TFilter )

#ifndef __rc__
#include "rc.h"
#endif
#ifndef __msg__
#include "msg.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif


class _CLASSTYPE TGroupCatDialog: public TOurDialog {

public:

    TGroupCatDialog( PTWindowsObject AParent );
    virtual ~TGroupCatDialog( void );
    void SetupWindow( void );

    void HandleAutomatic( RTMessage )  = [ID_FIRST + IDD_GROUPCAT_AUTOMATIC];
    void HandleNoGrouping( RTMessage ) = [ID_FIRST + IDD_GROUPCAT_NO_GROUPING];
    void HandleSelected( RTMessage )   = [ID_FIRST + IDD_GROUPCAT_SELECTED];
    void HandleCatBox( RTMessage )     = [WM_FIRST + WM_CATSELCHG];
	void Ok( RTMessage msg )           = [ID_FIRST + IDOK];

private:

    PTCatBoxData  itsCatBoxDataP;
    PTCatBox      itsCatBoxCP;

    PTCheckBox    itsSubtotalCP;

    PTGroupBox    itsRadioGroup;
    PTRadioButton itsAutomaticCP;
    PTRadioButton itsNoGroupingCP;
    PTRadioButton itsSelectedCP;

    WORD          itsTransferBuffer[3];
};



#endif
