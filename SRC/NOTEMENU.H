#ifndef __notemenu__
#define __notemenu__

#ifndef __snoop_
#include "snoop.h"
#endif

_CLASSDEF( TNoteMenu )
_CLASSDEF( TNote )

class _CLASSTYPE TNoteMenu: public TWindow, TSnoop
{
public:
    //THIS IS THE MAIN FUNCTION!
    static void PopUp( PTWindowsObject, PTNote, int xPos, int yPos);

private:
    TNoteMenu( PTWindowsObject, PTNote );
    virtual ~TNoteMenu();

    void DoneCmd( RTMessage )= [CM_FIRST + IDM_POPDONE ];
    void UndoneCmd( RTMessage )= [CM_FIRST + IDM_POPUNDONE];
    void CopyCmd( RTMessage )= [CM_FIRST + IDM_POPCOPY];
    void DeleteCmd( RTMessage )= [CM_FIRST + IDM_POPDELETE];
    void RedCmd( RTMessage )= [CM_FIRST + IDM_POPRED];
    void RedGrayCmd( RTMessage )= [CM_FIRST + IDM_POPREDGRAY];
    void GrayCmd( RTMessage )= [CM_FIRST + IDM_POPGRAY];
    void BlueGrayCmd( RTMessage )= [CM_FIRST + IDM_POPBLUEGRAY];
    void BlueCmd( RTMessage )= [CM_FIRST + IDM_POPBLUE];
    void ViewCmd( RTMessage) = [CM_FIRST + IDM_NOTE_VIEW];
    PTNote itsNoteP;
};

#endif
