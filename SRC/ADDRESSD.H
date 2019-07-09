#ifndef __addressd__
#define __addressd__

_CLASSDEF( TAddressDialog )
_CLASSDEF( TCat )
_CLASSDEF( TCatTree )

#ifndef __address__
#include "address.h"
#endif
#ifndef __cattree__
#include "cattree.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif
#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __snoop__
#include "snoop.h"
#endif

const int kNumEditFields            = 20;
const int kMaxLen                   = 256;

typedef enum {

    kNormalMode,
    kEmptySetMode

    } TAddressMode;

class _CLASSTYPE TAddressDialog: public TOurDialog, TSnoop {

    public:

        TAddressDialog( PTWindowsObject AParent, WORD AResID  );
        TAddressDialog( PTWindowsObject AParent, WORD AResID, RTCat cat  );
        void IAddressDialog( void );
        virtual ~TAddressDialog( void );

        static HANDLE GetWindowHandle( void ) { return itsWindowHandle; }

    private:
        void SetupWindow();

        //child window intercept
        void DefChildProc( RTMessage );

        // handle buttons
        void HandleAdd( RTMessage Msg )   = [ ID_FIRST + IDD_ADD_ADD_BUTTON ];
        void HandleDelete( RTMessage Msg )= [ ID_FIRST + IDD_ADD_DEL_BUTTON ];
        void HandleNext( RTMessage Msg )  = [ ID_FIRST + IDD_ADD_NEXT_BUTTON ];
        void HandleLast( RTMessage Msg )  = [ ID_FIRST + IDD_ADD_LAST_BUTTON ];
        void HandlePrint( RTMessage Msg ) = [ ID_FIRST + IDD_ADD_PRINT ];
        void HandleClpbrd( RTMessage Msg ) = [ ID_FIRST + IDD_ADD_CLPBRD ];
        void HandleNewNote( RTMessage Msg )= [ ID_FIRST + IDD_ADD_NEW_NOTE ];
        void HandleDial( RTMessage Msg )=   [ ID_FIRST + IDD_ADD_DIAL ];
        void HandleHome( RTMessage )=       [ ID_FIRST + IDD_ADD_ADDR_HOME ];
        void HandleWork( RTMessage )=       [ ID_FIRST + IDD_ADD_ADDR_BUS ];
        void HandleAlt( RTMessage )=        [ ID_FIRST + IDD_ADD_ADDR_ALT ];
        void HandleUnmarkAll( RTMessage )= [ ID_FIRST + IDD_ADD_UNMARK_ALL ];
        void HandleFindNext( RTMessage )=  [ ID_FIRST + IDD_ADD_FIND ];
        void HandleMarkMatches( RTMessage )= [ ID_FIRST + IDD_ADD_MARK_MATCHES ];

        void HandleMark( RTMessage  )      = [ ID_FIRST + IDD_ADD_MARK ];
        void HandleNextMarked( RTMessage  )= [ ID_FIRST + IDD_ADD_NEXT_MARKED ];

        void Ok( RTMessage Msg )          = [ ID_FIRST + IDOK ];

        void HandleSearchField( RTMessage Msg )= [ ID_FIRST + IDD_ADD_SEARCH_FIELD ];

        void HandleExport( RTMessage Msg )        = [ ID_FIRST + IDD_ADD_EXPORT ];
        void HandleImport( RTMessage Msg )        = [ ID_FIRST + IDD_ADD_IMPORT ];


        virtual void ShutDownWindow( void );
        virtual void ShutDownWindow( int returnValue );


        BOOL HasName( void );
        void ShowAddress( void );
        void SelectAddress( PTCat pCat );
        void UpdateCount( void );
        void HandleDataField( RTMessage Msg, int field );
        void HandleLetterButton( char letter );
        void SetName( PTCat pCat );
        void SetMode( TAddressMode mode );
        int  NumAddresses( void );
        PTCat NthAddress( int n );
        void TurnOnLetters( void );
        void ShowNoteView( void );

        PTCat        itsAddressP;
        PTCatTree    itsAddressesP;
        int          itsAddressIndex;
        TAddressMode itsMode;
        TWHA         itsWHAMode;
        BOOL         itsDataFieldsShowing;

        //stuff to save between invocations
        static HANDLE   itsWindowHandle;
        static char     itsSavedSearchText[ kMaxLen ];
        static int      itsSavedIndex;

        // interface items
        PTButton     itsAddButton;
        PTButton     itsDeleteButton;
        PTButton     itsOkButton;
        PTButton     itsNextButton;
        PTButton     itsPreviousButton;
        PTButton     itsImportButton;
        PTButton     itsExportButton;
        PTButton     itsPrintButton;
        PTButton     itsClpbrdButton;
        PTButton     itsNewNoteButton;
        PTButton     itsDialButton;
        PTButton     itsUnmarkAll;
        PTButton     itsFindFirst;
        PTButton     itsMarkMatching;
        PTButton     itsNextMarkedButton;
        PTButton     itsMarkMatchingButton;
        PTButton     itsUnmarkAllButton;
        PTButton     itsFindButton;
        PTButton     itsLetterButtons[26];

        PTStatic     itsCountField;

        PTCheckBox   itsHomeCP;
        PTCheckBox   itsWorkCP;
        PTCheckBox   itsAltCP;
        PTGroupBox   itsAddrGroupCP;
        PTCheckBox   itsMarkCP;

        PTEdit       itsDataFields[kNumEditFields];
        PTEdit       itsSearchFieldCP;

        struct {

            char dataFields[kNumEditFields][kMaxLen];
            char searchField[kMaxLen];

        } trnBuf;


};


#endif
