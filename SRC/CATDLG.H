#ifndef __catdlg__
#define __catdlg__

_CLASSDEF(TCatBoxData)
_CLASSDEF(TCatBox)
_CLASSDEF(TCatList)
_CLASSDEF(TCatTree )
_CLASSDEF(TCat)

_CLASSDEF(TCatNew)
_CLASSDEF(TCatMgr)

#ifndef __msg__
#include "msg.h"
#endif
#ifndef __snoop__
#include "snoop.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif

const MAX_CAT_KEY_LEN=  1000;
const MAX_CAT_DESC_LEN= 1000;
const MAX_CAT_NAME_LEN= 100;
const MAX_CAT_BUFFER_LEN= 1000;             //MAX of above !!!

class _CLASSTYPE TCatNew : public TOurDialog {

    public:

        TCatNew( PTWindowsObject AParent, int AResID, PTCat pCat );
        void Ok( RTMessage Msg )= [ ID_FIRST + IDOK ];

    private:

        PTCat  itsCatP;
        PTEdit pNameBox;
        PTEdit pNicknameBox;
        PTEdit pKeywordBox;
        PTEdit pDescriptionBox;
    };


class _CLASSTYPE TCatMgr : public TOurDialog, TSnoop {

    public:

        TCatMgr( PTWindowsObject AParent, int AResID );
        TCatMgr( PTWindowsObject AParent, int AResID, RTCat cat );
        virtual ~TCatMgr();

        static PTCat AddCat( PTWindowsObject parent, PTCat pParentCat );

    private:
        void Ok( RTMessage Msg )= [ ID_FIRST + IDOK ];
        void CreateCat(  RTMessage Msg )= [ ID_FIRST + IDD_CATMGRNEW ];
        void CreateSubcat( RTMessage Msg )= [ ID_FIRST + IDD_CATMGRNEW_SUBCAT ];
        void NameCat( RTMessage Msg )= [ ID_FIRST + IDD_CATNAME ];
        void NicknameCat( RTMessage Msg )= [ ID_FIRST + IDD_SHORT_CATNAME ];
        void DeleteCat( RTMessage Msg )=  [ ID_FIRST + IDD_CATDELETE ];
        void SelectCat( RTCat cat );

        void ListHandler( RTMessage Msg ) = [ WM_FIRST + WM_CATSELCHG ];

        void Listen( TSnoopMsg, RObject );
        BOOL UpdateCatFields( void );
        void SetupWindow( void );
        BOOL ValidCatOperation( RTCat category, int operation );
        void ShowFields( RTCat cat );

        //transfer buffer for catdelete
        struct
            {
            char name[ MAX_CAT_NAME_LEN ];
            char nickname[ MAX_CAT_NAME_LEN ];
            char keywords[ MAX_CAT_KEY_LEN ];
            char description[ MAX_CAT_DESC_LEN ];
            PTCatBoxData pCatBoxData;
            } trnBuf;

        PTEdit pNameBox;
        PTEdit pNicknameBox;
        PTEdit pKeywordBox;
        PTEdit pDescriptionBox;
        PTCat  pCat;
        PTButton pSubcatButton;
        PTButton pDeleteButton;
        PTCat    itsStartCatP;
    };


#endif
