#ifndef __fontmap__
#define __fontmap__

_CLASSDEF( TFontMap )

typedef enum {

    kSmallFont,
    kSmallStrikeFont,

    kNormalFont,
    kBoldFont,
    kItalicFont,
    kStrikeFont,

    kBigFont,
    kBigBoldFont,
    kBigStrikeFont

} TFont;

class _CLASSTYPE TFontMap
{
public:
        TFontMap();
        virtual ~TFontMap();

        static WORD GetFontHeight( HDC hdc, TFont font );

        static HANDLE GetScreenFont( TFont font );
        static HANDLE GetPrintFont( TFont font );

private:
        void CreateFonts();
        void DeleteFonts();

        BOOL itsErrorFlag;

        static HANDLE hSmallScrFont;        //screen fonts
        static HANDLE hSmallStrikeScrFont;

        static HANDLE hNormalScrFont;
        static HANDLE hBoldScrFont;
        static HANDLE hStrikeScrFont;
        static HANDLE hItalicScrFont;

        static HANDLE hBigScrFont;
        static HANDLE hBigBoldScrFont;
        static HANDLE hBigStrikeScrFont;

        static HANDLE hSmallPrtFont;        //printer fonts
        static HANDLE hSmallStrikePrtFont;

        static HANDLE hNormalPrtFont;
        static HANDLE hBoldPrtFont;
        static HANDLE hStrikePrtFont;
        static HANDLE hItalicPrtFont;

        static HANDLE hBigPrtFont;
        static HANDLE hBigBoldPrtFont;
        static HANDLE hBigStrikePrtFont;

};

#endif
