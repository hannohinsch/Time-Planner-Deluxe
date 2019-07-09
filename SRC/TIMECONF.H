#ifndef __timeconf__
#define __timeconf__

_CLASSDEF( TNoteListGrid )
_CLASSDEF( TTimeConflictDialog )

#ifndef __notelisd__
#include "notelisd.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif


class _CLASSTYPE TTimeConflictDialog: public TOurDialog {

    public:

		TTimeConflictDialog( PTWindowsObject AParent, int AResID, TNoteSet &noteSet );
        virtual ~TTimeConflictDialog( void );

	private:

		PTNoteListGrid   itsSelectorP;

};


#endif
