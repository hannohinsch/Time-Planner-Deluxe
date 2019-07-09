#ifndef __guessdlg__
#define __guessdlg__

_CLASSDEF( TMemorizedDialog )

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


class _CLASSTYPE TGuessDialog: public TOurDialog {

    public:

        TGuessDialog( PTWindowsObject AParent, int AResID,
            PTCatList guessList, PTCatList realList );
        virtual ~TGuessDialog( void );

        void Ok( RTMessage msg )                = [ID_FIRST + IDOK ];

	private:

		PTCatList       itsGuessListP;
		PTCatList       itsRealListP;
        TSelector       *itsSelector;
        TButton         *itsOkButton;
        TButton         *itsCancelButton;
        TSelectorData   *itsSelectorData;
};



#endif
