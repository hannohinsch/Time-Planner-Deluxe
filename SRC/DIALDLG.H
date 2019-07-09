#ifndef __dialdlg__
#define __dialdlg__

#ifndef __ourdlg__
#include "ourdlg.h"
#endif
#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __duration__
#include "duration.h"
#endif
#include <time.h>

_CLASSDEF( TCat )
_CLASSDEF( TDialDialog )

class _CLASSTYPE TDialDialog: public TOurDialog
{
public:

    TDialDialog( PTWindowsObject parent, PTCat );
    virtual ~TDialDialog( void );

    static void Hangup( PTWindowsObject );
    static PTDialDialog CurrentDialDialog;

    void UpdateTime( void );

private:

    void HandleSetup( RTMessage ) = [ID_FIRST + IDD_DIAL_SETUP];
    void HandleAccess( RTMessage )= [ID_FIRST + IDD_DIAL_ACCESS];
    void HandlePhone( RTMessage )=  [ID_FIRST + IDD_DIAL_PHONE];
    void HandleStart( RTMessage )=  [ID_FIRST + IDD_DIAL_START];
    void HandleCopy( RTMessage )= [ID_FIRST + IDD_DIAL_COPY];
    void HandleDial( RTMessage )= [ID_FIRST + IDD_DIAL_DIAL ];
    void Cancel( RTMessage )= [ID_FIRST + IDCANCEL];

    static int WriteString( int port, char *string );
    void SetupWindow( void );
    void StripAreaCode( char *string );
    void GetPhone( char *phone );
    void SetDefaultAccess( void );
    BOOL IsLocalNumber( const char *string );

    PTCat       itsCatP;
    PTEdit      itsPhoneStringCP;
    PTStatic    itsNameCP;
    PTStatic    itsStartTimeCP;
    PTStatic    itsDurationCP;
    PTButton    itsStartStopCP;
    PTButton    itsDialHangupCP;
    PTButton    itsCopyCP;
    PTButton    itsSetupCP;
    PTComboBox  itsPhoneCP;
    PTComboBox  itsAccessCP;

    static int         itsSendTicks;
    static int         itsDialTicks;
    BOOL        itsCallActive;
    BOOL        itsTimerOn;
    BOOL        itsTipWinState;
    TTime       itsStartTime;
    time_t      itsRawStartTime;
    time_t      itsRawEndTime;
    };

#endif
