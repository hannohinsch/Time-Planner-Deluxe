#ifndef __sortd__
#define __sortd__

_CLASSDEF( TSortD )

#ifndef __rc__
#include "rc.h"
#endif
#ifndef __profile__
#include "profile.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif

const int kNumGroups         = 3;
const int kNumButtonsInGroup = 5;

class _CLASSTYPE TSortD: public TOurDialog {

public:

    TSortD( PTWindowsObject AParent, int resId );
    virtual ~TSortD( void );

private:

    void Ok( RTMessage msg );
    void HandleGroup( RTMessage, int group, TNoteOrder item );
    void SetDefault( RTMessage msg ) = [ID_FIRST + IDC_SET_DEFAULT];
    void SetValues( void );

    void HandleColor1( RTMessage msg )    = [ID_FIRST + IDD_ORDER_COLOR_1]       { HandleGroup( msg, 0, kNoteOrder_Color ); }
    void HandleDate1( RTMessage msg  )     = [ID_FIRST + IDD_ORDER_DATE_1]       { HandleGroup( msg, 0, kNoteOrder_Date ); }
    void HandleDuration1( RTMessage msg  ) = [ID_FIRST + IDD_ORDER_DURATION_1]   { HandleGroup( msg,0, kNoteOrder_Duration ); }
    void HandleType1( RTMessage msg  )     = [ID_FIRST + IDD_ORDER_TYPE_1]       { HandleGroup( msg,0, kNoteOrder_Type ); }
    void HandleAlpha1( RTMessage msg  )     = [ID_FIRST + IDD_ORDER_ALPHA_1]     { HandleGroup( msg,0, kNoteOrder_Alpha ); }

    void HandleColor2( RTMessage msg  )    = [ID_FIRST + IDD_ORDER_COLOR_2]      { HandleGroup( msg,1, kNoteOrder_Color ); }
    void HandleDate2( RTMessage msg  )     = [ID_FIRST + IDD_ORDER_DATE_2]       { HandleGroup( msg,1, kNoteOrder_Date ); }
    void HandleDuration2( RTMessage msg  ) = [ID_FIRST + IDD_ORDER_DURATION_2]   { HandleGroup( msg,1, kNoteOrder_Duration ); }
    void HandleType2( RTMessage msg  )     = [ID_FIRST + IDD_ORDER_TYPE_2]       { HandleGroup( msg,1, kNoteOrder_Type ); }
    void HandleAlpha2( RTMessage msg  )    = [ID_FIRST + IDD_ORDER_ALPHA_2]      { HandleGroup( msg,1, kNoteOrder_Alpha ); }

    void HandleColor3( RTMessage msg  )    = [ID_FIRST + IDD_ORDER_COLOR_3]      { HandleGroup( msg,2, kNoteOrder_Color ); }
    void HandleDate3( RTMessage msg  )     = [ID_FIRST + IDD_ORDER_DATE_3]       { HandleGroup( msg,2, kNoteOrder_Date ); }
    void HandleDuration3( RTMessage msg  ) = [ID_FIRST + IDD_ORDER_DURATION_3]   { HandleGroup( msg,2, kNoteOrder_Duration ); }
    void HandleType3( RTMessage msg  )     = [ID_FIRST + IDD_ORDER_TYPE_3]       { HandleGroup( msg,2, kNoteOrder_Type ); }
    void HandleAlpha3( RTMessage msg  )    = [ID_FIRST + IDD_ORDER_ALPHA_3]      { HandleGroup( msg,2, kNoteOrder_Alpha ); }

    PTGroupBox    itsGroupBoxes[kNumGroups];
    PTRadioButton itsButtons[kNumGroups][kNumButtonsInGroup];

    PTButton      itsOkButtonP;
    PTButton      itsCancelButtonP;

    WORD          itsTransferBuffer[kNumGroups][kNumButtonsInGroup];

};


#endif

