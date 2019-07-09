#ifndef __exportd__
#define __exportd__

#ifndef __ourfiled__
#include "ourfiled.h"
#endif

_CLASSDEF( TFieldMap )
_CLASSDEF( TExportDialog )
_CLASSDEF( TImportDialog )
_CLASSDEF( TExportAddressDialog )
_CLASSDEF( TImportAddressDialog )

class _CLASSTYPE TExportDialog: public TOurFileDialog {

public:

    TExportDialog( PTWindowsObject AParent,
        LPSTR AFilePath, BOOL *headlinesOnlyP );
    virtual ~TExportDialog( void );

private:

    void Ok( RTMessage Msg )  = [ ID_FIRST + IDOK ];
    void HandleFieldMap( RTMessage Msg )  = [ ID_FIRST + IDD_EXPORT_FIELD_ORDER ];
    void SetupWindow( void );

    PTRadioButton itsFullTextButtonP;
    PTRadioButton itsHeadlinesButtonP;
    BOOL       *itsHeadlinesOnlyP;
};

class _CLASSTYPE TExportAddressDialog: public TOurFileDialog {

public:

    TExportAddressDialog( PTWindowsObject AParent,
        LPSTR AFilePath, BOOL *markedNotesOnlyP );
    virtual ~TExportAddressDialog( void );
    void Ok( RTMessage Msg )  = [ ID_FIRST + IDOK ];

private:

    void HandleFieldMap( RTMessage Msg )  = [ ID_FIRST + IDD_EXPORT_FIELD_ORDER ];
    void SetupWindow( void );

    TCheckBox *itsMarkedAddrOnlyCP;
    BOOL      *itsMarkedAddrOnlyP;
};

class _CLASSTYPE TImportDialog: public TOurFileDialog {

    public:

	TImportDialog( PTWindowsObject AParent, LPSTR AFilePath );

};

class _CLASSTYPE TImportAddressDialog: public TOurFileDialog {

    public:

	TImportAddressDialog( PTWindowsObject AParent, LPSTR AFilePath );
};

#endif
