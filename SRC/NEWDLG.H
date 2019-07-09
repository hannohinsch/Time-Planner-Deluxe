#ifndef __newdlg__
#define __newdlg__

#ifndef __ourfiled__
#include "ourfiled.h"
#endif

_CLASSDEF( TNewDialog )


class _CLASSTYPE TNewDialog: public TOurFileDialog {

public:

    TNewDialog( PTWindowsObject AParent,
        LPSTR AFilePath, BOOL *shouldCopyP, PTModule AModule = NULL );
    virtual ~TNewDialog( void );

private:

    BOOL CanClose( void );
    void HandleEmpty( RTMessage Msg ) = [ ID_FIRST + IDD_NEW_DIALOG_EMPTY ];
    void HandleCopy( RTMessage Msg )  = [ ID_FIRST + IDD_NEW_DIALOG_COPY ];
    void Ok( RTMessage Msg )          = [ ID_FIRST + IDOK ];
    void SetDlgSize( BOOL show );
    void SetupWindow( void );

    BOOL           *itsShouldCopyP;
    int            itsFullWindowWidth;
    int            itsFullWindowHeight;
    PTGroupBox     itsMoreGroupCP;
    PTGroupBox     itsMainGroupCP;
    PTRadioButton  itsEmptyButtonCP;
    PTRadioButton  itsCopyButtonCP;

    WORD itsTransferBuffer[2];
};


#endif

