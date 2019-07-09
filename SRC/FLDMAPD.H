#ifndef __fldmapd__
#define __fldmapd__

_CLASSDEF( TExportMapDialog )
_CLASSDEF( TExportAddressMapDialog )

_CLASSDEF( TImportMapDialog )
_CLASSDEF( TImportAddressMapDialog )

#ifndef __ourdlg__
#include "ourdlg.h"
#endif
#ifndef __fieldmap__
#include "fieldmap.h"
#endif

typedef char Str32[32];

extern Str32 addressFieldNames[];
extern Str32 noteFieldNames[];

class _CLASSTYPE TExportMapDialog: public TOurDialog
{

    public:

    TExportMapDialog( PTWindowsObject AParent, WORD id= IDD_EXPORT_MAP );
    virtual ~TExportMapDialog( void ) {};

    protected:

    void SetupWindow( void );
    virtual void SetupStrings( void );
    virtual int StringToKey( char *buf );
    virtual void KeyToString( int key, char *buf );

    void HandleSource(  RTMessage ) = [ID_FIRST + IDD_EXPORT_MAP_SOURCE];
    void HandleTarget(  RTMessage ) = [ID_FIRST + IDD_EXPORT_MAP_TARGET];
    void HandleAdd(     RTMessage ) = [ID_FIRST + IDD_EXPORT_MAP_ADD];
    void HandleRemove(  RTMessage ) = [ID_FIRST + IDD_EXPORT_MAP_REMOVE];
    void HandleClear(   RTMessage ) = [ID_FIRST + IDD_EXPORT_MAP_CLEAR];
    void HandleDefault( RTMessage ) = [ID_FIRST + IDD_EXPORT_MAP_DEFAULT];
    void Ok(            RTMessage ) = [ID_FIRST + IDOK];

    PTListBox     itsSourceCP;
    PTListBox     itsTargetCP;
    PTButton      itsAddCP;
    PTButton      itsRemoveCP;
    PTButton      itsClearCP;
    PTButton      itsDefaultCP;

    Str32         *itsKeyStrings;
    int           itsNumKeyStrings;
    int           *itsKeys;
    int           *itsNumKeysP;
};

class _CLASSTYPE TExportAddressMapDialog: public TExportMapDialog
{
    public:

    TExportAddressMapDialog( PTWindowsObject AParent );
    virtual ~TExportAddressMapDialog( void ) {};

    private:

    virtual void SetupStrings( void );
};


class _CLASSTYPE TImportMapDialog: public TOurDialog
{
    public:

    TImportMapDialog( PTWindowsObject AParent,
        char *fileName, WORD id= IDD_IMPORT_MAP );
    virtual ~TImportMapDialog( void ) {};

    protected:

    virtual void SetupWindow( void );

    void SetTarget( int sourceItem, int destItem );
    void HandleTarget( RTMessage msg )=[ID_FIRST + IDD_IMPORT_MAP_TARGET];
    void HandleAdd(     RTMessage ) = [ID_FIRST + IDD_IMPORT_MAP_ADD];
    void HandleRemove(  RTMessage ) = [ID_FIRST + IDD_IMPORT_MAP_REMOVE];
    void Ok(            RTMessage ) = [ID_FIRST + IDOK];

    virtual void SetupStrings( void );

    PTListBox     itsSourceCP;
    PTListBox     itsTargetCP;
    PTButton      itsAddCP;
    PTButton      itsRemoveCP;

    Str32         *itsKeyStrings;
    int           itsNumKeyStrings;
    int           *itsKeys;
    int           *itsNumKeysP;
    int           itsIgnoreKey;

    int           itsMatchKeys[100];
    char          *itsFileName;

};

class _CLASSTYPE TImportAddressMapDialog: public TImportMapDialog
{
    public:

    TImportAddressMapDialog( PTWindowsObject AParent,
        char *fileName );
    virtual ~TImportAddressMapDialog( void ) {};

    private:

    virtual void SetupStrings( void );
};
#endif

