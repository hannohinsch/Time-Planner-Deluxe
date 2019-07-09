#ifndef __tipwin__
#define __tipwin

_CLASSDEF( TTipWin )

class _CLASSTYPE TTipWin: public TDialog
{
public:
    TTipWin( PTWindowsObject aParent, WORD anID );
    virtual ~TTipWin();

    void Update( void );
    BOOL MakeActive( BOOL b );

private:
    LPSTR  GetClassName( void ) { return "tipwin";}

    PTStatic  itsTextCP;
    BOOL      itsActive;
};

#endif
