#ifndef __catbox__
#define __catbox__

_CLASSDEF( TCat )
_CLASSDEF( TCatList )
_CLASSDEF( TCatTree)

_CLASSDEF( TCatBox )
_CLASSDEF( TVisiCat )
_CLASSDEF( TCatBoxData )
_CLASSDEF( TXArray )

#ifndef __rc__
#include "rc.h"
#endif
#ifndef __mousgrid__
#include "mousgrid.h"
#endif
#ifndef __snoop__
#include "snoop.h"
#endif
#ifndef __msg__
#include "msg.h"
#endif

class _CLASSTYPE TCatBox: public TMousGrid, TSnoop {

public:
    TCatBox( PTWindowsObject AParent, WORD resID );
    TCatBox( PTWindowsObject AParent, WORD resID, int x, int y,
        int dx, int dy );
    virtual ~TCatBox( void );

    LPSTR GetClassName() { return "CatBox"; };
    void GetWindowClass( WNDCLASS & AWndClass );

    WORD Transfer(Pvoid DataPtr, WORD TransferFlag);

    void SelectAll( void );
    void SelectNone( void );

protected:
    int CellWidth( PTPaint, int col );
    int CellHeight( PTPaint, int row );
    void PaintCell( RTDrawInfo );

    void MousClick( RTMousInfo );
    void MousOver( RTMousInfo );
    void MousRtClick( RTMousInfo );
    void GetDefaultFocusCell( int &row, int &col );
    void PaintFocus( HDC, RECT & );

    void Listen( TSnoopMsg, RObject );

private:
    void WMLocalColor( RTMessage ) = [ WM_FIRST + WM_LOCALCOLOR ];

    void HandleCategory( RTMessage ) = [CM_FIRST + IDM_CATBOX_CATEGORY];
    void HandleAddress( RTMessage )  = [CM_FIRST + IDM_CATBOX_ADDRESS];
    void HandleNewNote( RTMessage )  = [CM_FIRST + IDM_CATBOX_NEW_NOTE];
    void HandleNewFolder( RTMessage )  = [CM_FIRST + IDM_CATBOX_NEW_FOLDER];
    void HandleNewSubfolder( RTMessage )  = [CM_FIRST + IDM_CATBOX_NEW_SUBFOLDER];
    void HandleDelete( RTMessage )  = [CM_FIRST + IDM_CATBOX_DELETE];
    void HandleDial( RTMessage )  = [CM_FIRST + IDM_CATBOX_DIAL];
    void SelectCat( int row, int col );

    RTVisiCat RowItem( int );

    void PaintName( RTDrawInfo );
    void PaintPlusMinus( RTDrawInfo );

    PTCatBoxData pData;
    HANDLE       itsMessageWindow;
    int          itsMenuRow;
    int          itsMenuCol;
    };

///////////////////////////////////////////////////////////////////////////////
// one of these per on-screen cat name in TCatBoxData
///////////////////////////////////////////////////////////////////////////////
class _CLASSTYPE TVisiCat: public Object
    {
    friend TCatBoxData;
    friend TCatBox;

    public:
        TVisiCat( RTCat cat, int aLevel, BOOL select );
        virtual ~TVisiCat( void );

        classType isA( void ) const;
        char *nameOf( void ) const;
        hashValueType hashValue( void ) const;
        int isEqual(const Object &) const;
        void printOn(ostream &) const;

    private:
        PTCat pCat;
        BOOL bSelected;
        BOOL bHiddenSels;
        int level;
    };

///////////////////////////////////////////////////////////////////////////////
// transfer object
///////////////////////////////////////////////////////////////////////////////
class _CLASSTYPE TCatBoxData
    {
    friend TCatBox;

    public:
        static void AppInit();
        static void AppCleanup();

        TCatBoxData( BOOL hasSystemCats = FALSE,
            BOOL hasAddressCats = TRUE  );
        virtual ~TCatBoxData();

        void  SetSingleSelect( BOOL b);       //constructor default is multi-select

        void  Prepare( PTCatList );
        void  PrepareNone( void );
        void  PrepareAll( void );
        void  Store( PTCatList );

        void  Select( RTCatList catList );
        void  Select( RTCat cat );
        void  SelectAll( void );
        void  SelectFirst( void );            //select first item in set
        RTCat GetSelCat( void );        //for single select only -- return selection
        RTCat GetPrevCat( RTCat cat );
        int   GetSelCount(void);

        void  SelectUncat( BOOL select );

    private:

        void  BuildVisiList( BOOL bSelect );
        void  AddSystemCats( void );

        void      ClearSelections( void );
        PTVisiCat FindVisiCat( PTCat catP );
        PTVisiCat FindParentVisiCat( PTCat catP );

        PTVisiCat FirstVisiCat( void );
        PTVisiCat NextVisiCat( void );
        PTVisiCat LastVisiCat( void );

        static PTCat   unCatP;

        PTXArray  pVisiCatList;
        PTCatList pHiddenSelections;
        BOOL      bSingleSelect;
        BOOL      itsHasSystemCats;
        BOOL      itsHasAddressCats;
        int       itsCurrentVisiCat;
    };

#endif
