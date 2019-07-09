#ifndef __notelisd__
#define __notelisd__

_CLASSDEF( TNoteListDialog )
_CLASSDEF( TNoteListGrid )

#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __rulegrid__
#include "rulegrid.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif


const int kCanceledNotelist = 32700;

class _CLASSTYPE TNoteListDialog: public TOurDialog {

    public:

		TNoteListDialog( PTWindowsObject AParent, int AResID, TNoteSet &noteSet, char *prompt );
        virtual ~TNoteListDialog( void );


	private:

        void HandlePopup( RTMessage msg ) = [ID_FIRST + IDD_NOTELIST_SELECTOR ];
		void Ok( RTMessage msg )          = [ID_FIRST + IDOK];
		void Cancel( RTMessage msg )      = [ID_FIRST + IDCANCEL];
        void SetupWindow( void );


        int             itsNumItems;
		TNoteListGrid   *itsSelector;
        PTStatic        itsPromptP;
        TButton         *itsOkButton;
        TButton         *itsCancelButton;

        char            *itsPromptString;

};

class _CLASSTYPE TNoteListGrid: public TRuleGrid {

public:

    TNoteListGrid( PTWindowsObject AParent, WORD resID,
        BOOL live, PTNoteSet noteSetP );
    virtual ~TNoteListGrid( void );

    int  GetSelection( void );

protected:

    int  CellWidth( PTPaint, int col );
    int  CellHeight( PTPaint, int );
    void PaintCell( RTDrawInfo );

    void MousClick( RTMousInfo );
    void MousDblClick( RTMousInfo );

    void GetWindowClass( WNDCLASS & AWndClass );
    virtual LPSTR  GetClassName( void ) { return "NoteListGrid";}

private:

        PTNoteSet   itsNoteSetP;
        int         itsSelection;
        BOOL        itsLive;
};

#endif
