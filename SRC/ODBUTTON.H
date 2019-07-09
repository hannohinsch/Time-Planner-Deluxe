#ifndef __odbutton__
#define __odbutton__

#ifndef __color__
#include "color.h"
#endif

_CLASSDEF( TODButton )
_CLASSDEF( TODBitMapButton )
_CLASSDEF( TODColorBitMapButton )
_CLASSDEF( TODTextButton )

// -------------------------------------------------
// Basic ODButton
// -------------------------------------------------
class _CLASSTYPE TODButton: public TButton
{

public:
    TODButton( PTWindowsObject aParent, int anId,
              int x, int y, int w, int h,
              PTModule aModule= NULL );

    TODButton( PTWindowsObject aParent, int anId,
              PTModule aModule= NULL );


    virtual ~TODButton( );

    void SetCheck( WORD checkFlag );
    WORD GetCheck( void );

protected:
    void ODADrawEntire( DRAWITEMSTRUCT &di );
    void ODASelect( DRAWITEMSTRUCT &di );
    void ODAFocus( DRAWITEMSTRUCT &di );

    virtual void DrawButton( DRAWITEMSTRUCT &di ) = 0;

    WORD itsCheckState;
};

// -------------------------------------------------
// TODBitMapButton
//
//  Draws a button with a bitmap
// -------------------------------------------------
class _CLASSTYPE TODBitMapButton: public TODButton
{

    public:

    TODBitMapButton( PTWindowsObject aParent, int anID,
              int x, int y, int w, int h,
              PTModule aModule= NULL );
    TODBitMapButton( PTWindowsObject aParent, int anID,
              PTModule aModule= NULL );
    virtual ~TODBitMapButton( );

    protected:

    void DrawButton( DRAWITEMSTRUCT &di );
    WORD itsID;
};

// -------------------------------------------------
// TODBitMapButton
//
//  Draws a button with a color bitmap
// -------------------------------------------------
class _CLASSTYPE TODColorBitMapButton: public TODBitMapButton
{

    public:

    TODColorBitMapButton( PTWindowsObject aParent, int anID, TVirtColor color,
              PTModule aModule= NULL );
    virtual ~TODColorBitMapButton( );

    protected:

    void DrawButton( DRAWITEMSTRUCT &di );

    private:

    TVirtColor itsColor;

};

// -------------------------------------------------
// TODTextButton
//
//  Draws a button with text
// -------------------------------------------------
class _CLASSTYPE TODTextButton: public TODButton
{

    public:

    TODTextButton( PTWindowsObject aParent, int anId,
        char *name, int x, int y, int w, int h,
        PTModule aModule= NULL );
    TODTextButton( PTWindowsObject aParent, int anId,
              PTModule aModule= NULL );
    virtual ~TODTextButton( );

    void ODASelect( DRAWITEMSTRUCT &di );

    protected:

    void DrawButton( DRAWITEMSTRUCT &di );

};

#endif
