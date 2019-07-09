#ifndef __selector__
#define __selector__

_CLASSDEF( TSelector )

#ifndef __rulegrid__
#include "rulegrid.h"
#endif

typedef struct {

    PTSafStr                   pText;
    long                       value;
    BOOL                       selected;

    } TSelectorData;

class _CLASSTYPE TSelector: public TRuleGrid {

	public:

	    TSelector( PTWindowsObject AParent, WORD resID, int numItems,
            TSelectorData *data);
	    ~TSelector( void );

        void SetData( int numItems, TSelectorData *data );

        void SetTextStyle( WORD style );
        LPSTR  GetClassName( void ) { return "Selector";}

    protected:

        int             itsNumItems;
        TSelectorData   *itsData;
        int             itsCellWidth;
        int             itsCellHeight;
        WORD            itsTextStyle;

        void MousClick( RTMousInfo );

        int  CellWidth( PTPaint, int col );
        int  CellHeight( PTPaint, int col );
        void PaintCell( RTDrawInfo );

        void WMSize( RTMessage );
};

class _CLASSTYPE TSingleSelector: public TSelector {

	public:

	    TSingleSelector( PTWindowsObject AParent, WORD resID, int numItems,
            TSelectorData *data);
	    ~TSingleSelector( void );

        void SetData( int numItems, TSelectorData *data );
        void SetSelection( int selection );
        long GetSelection( void );

    private:

        void MakeSelection( void );
        void MousClick( RTMousInfo );
        void MousDblClick( RTMousInfo );

        int             itsSelection;
};

#endif

