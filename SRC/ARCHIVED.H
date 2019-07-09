#ifndef __archived__
#define __archived__

#ifndef __ourfiled__
#include "ourfiled.h"
#endif

_CLASSDEF( TArchiveD )

class _CLASSTYPE TArchiveD: public TOurFileDialog {

public:

    TArchiveD( PTWindowsObject AParent,
        LPSTR AFilePath, BOOL *shouldDeleteP, PTModule AModule = NULL );
    virtual ~TArchiveD( void );

private:

    void Ok( RTMessage Msg )  = [ ID_FIRST + IDOK ];

    PTCheckBox itsDeleteButtonP;
    BOOL       *itsShouldDeleteP;
};


#endif

