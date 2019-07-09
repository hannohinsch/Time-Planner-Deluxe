#ifndef __paint__
#define __paint__

_CLASSDEF( TDrawInfo )                                                  
_CLASSDEF( TPaint )
_CLASSDEF( TPaint3D )
_CLASSDEF( TPaintPrt )

#ifndef __fontmap__
#include "fontmap.h"
#endif
#ifndef __note__
#include "note.h"
#endif
#ifndef __color__
#include "color.h"
#endif

const kSingleLine= DT_SINGLELINE | DT_NOPREFIX | DT_EXTERNALLEADING | DT_BOTTOM;
const kMultiLine=  DT_WORDBREAK  | DT_NOPREFIX | DT_EXTERNALLEADING;

typedef enum {

    kNoHilite,
    kFrameHilite,
    kFocusHilite,
    kRangeHilite,
    kInvertHilite

} THilite;


class _CLASSTYPE TPaint {

	public:
        TPaint( void );
        TPaint( HDC );
        virtual ~TPaint();

        virtual void Text( RTDrawInfo di,
                    const char *buf,
                    THilite hilite= kNoHilite,
                    WORD style= kSingleLine,
                    TFont font= kNormalFont,
                    RECT *drawRect= NULL
             );

        virtual void TextLR( RTDrawInfo di,
                    const char *lBuf,
                    const char *rBuf,
                    THilite hilite= kNoHilite,
                    WORD style= kSingleLine,
                    TFont font= kNormalFont,
                    RECT *drawRect= NULL
             );

        virtual void DrawMark( RTDrawInfo di, RTNote note );
        static  void DrawScreenMark( HDC hdc, RECT &r, TColor color, TNoteStyle style );
        virtual void GridLine( RTDrawInfo di, RECT &rect );

        virtual void FillRectangle( RTDrawInfo di, RECT &rect, TLogBrush );
        virtual void FillRectangle( RTDrawInfo di, RECT &rect, HBRUSH );
        virtual void FillRectangle( RTDrawInfo di, RECT &rect, TColor color );
        virtual void FrameRectangle( RTDrawInfo di, RECT &rect, TColor color );
        virtual void FillDiamond( RTDrawInfo di, RECT &rect, TColor color );
        virtual void FolderTab( RTDrawInfo di, BOOL placeHolder, RTSafStr text );

        virtual void Hilite( RTDrawInfo di, THilite hilite );

        virtual int CalcHeight( const char *buf, int width, TFont font, int defHeight=19 );
        virtual int CalcWidth( const char *buf, TFont font );

        virtual BOOL IsPrintPainter( void ) { return FALSE; };

protected:
        virtual HFONT SelectFont( HDC hdc, TFont font );
        virtual HDC GimmeDC( void );
        virtual void TrashDC( HDC );

        virtual void TextSub( RTDrawInfo di,
                    RECT &r,
                    const char *lBuf, int nLeft,
                    const char *rBuf, int nRight,
                    THilite hilite,
                    WORD style
             );

        HDC   itsOwnDC;
        TFont itsLastFont;

private:
    void OurDrawText( HDC hdc, char *buf, int n, RECT *r, WORD style );
    BOOL CharBreaker( HDC hdc, char * &p, WORD &lineLength, WORD maxWidth );
    BOOL LineBreaker( HDC hdc, char * &p, WORD &lineLength, WORD maxWidth );
    void Realize( RTDrawInfo, HBRUSH );
};


class _CLASSTYPE TPaint3D: public TPaint
{
protected:
    virtual void TextSub( RTDrawInfo di,
                RECT &r,
                const char *lBuf, int nLeft,
                const char *rBuf, int nRight,
                THilite hilite,
                WORD style
         );

};

class _CLASSTYPE TPaintPrt: public TPaint
{
public:
    virtual void DrawMark( RTDrawInfo di, RTNote note );
    virtual void GridLine( RTDrawInfo di, RECT &rect );

    virtual void FillRectangle( RTDrawInfo di, RECT &rect, TLogBrush brush );
    virtual void FillRectangle( RTDrawInfo di, RECT &rect, TColor color );
    virtual void FrameRectangle( RTDrawInfo di, RECT &rect, TColor color );
    virtual void FillDiamond( RTDrawInfo di, RECT &rect, TColor color );
    virtual void FolderTab( RTDrawInfo di, BOOL placeHolder, RTSafStr text );

    virtual int CalcHeight( const char *buf, int width, TFont font, int defHeight=19 );
    virtual int CalcWidth( const char *buf, TFont font );
    virtual void Hilite( RTDrawInfo di, THilite hilite );

    virtual BOOL IsPrintPainter( void ) { return TRUE; };

protected:
    virtual HFONT SelectFont( HDC hdc, TFont font );
    virtual HDC GimmeDC( void );
    virtual void TrashDC( HDC );

    virtual void TextSub( RTDrawInfo di,
                RECT &r,
                const char *lBuf, int nLeft,
                const char *rBuf, int nRight,
                THilite hilite,
                WORD style
         );

};


#endif
