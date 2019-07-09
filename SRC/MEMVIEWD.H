#ifndef __memviewd__
#define __memviewd__

_CLASSDEF( TMemorizedViewDialog )
_CLASSDEF( TSelector )

#ifndef __paint__
#include "paint.h"
#endif
#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif
#ifndef __selector__
#include "selector.h"
#endif



class _CLASSTYPE TMemorizedViewDialog: public TOurDialog {

    public:

        TMemorizedViewDialog(
            PTWindowsObject AParent, int AResID );
        virtual ~TMemorizedViewDialog( void );

		void HandlePopup( RTMessage msg ) = [ID_FIRST + IDD_MEMORIZED_SELECTOR ];
		void HandleDelete( RTMessage msg )= [ID_FIRST + IDD_MEMORIZED_DELETE ];
		void Ok( RTMessage msg )          = [ID_FIRST + IDOK];

	private:

        void SetupWindow( void );
        void CreateSelectorData( void );
        void DestroySelectorData( void );

        int             itsNumItems;
		TSingleSelector *itsSelector;
        TButton         *itsOkButton;
        TButton         *itsCancelButton;
        TButton         *itsDeleteButton;
        TSelectorData   *itsSelectorData;
        TNote           **itsNotePP;

};



#endif
