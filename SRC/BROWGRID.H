#ifndef __browgrid__
#define __browgrid__

_CLASSDEF( TBrowGrid )
_CLASSDEF( TBrowGridData )
_CLASSDEF( TCatList )
_CLASSDEF( TNoteSet )
_CLASSDEF( TCat )
_CLASSDEF( TCatTree )
_CLASSDEF( TXArray )
_CLASSDEF( TNoteTable )

#ifndef __rulegrid__
#include "rulegrid.h"
#endif
#ifndef __snoop__
#include "snoop.h"
#endif
#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __note__
#include "note.h"
#endif

class _CLASSTYPE TBrowGridData: Sortable {

    public:

    PTCat      catP;
    char       nickname[256];
    TSafStr    longName;

    TBrowGridData( void );
    virtual ~TBrowGridData( void );

    int operator < ( const TBrowGridData &other ) const;
    int operator == ( const TBrowGridData &other ) const;

    classType isA() const;
    char *nameOf() const;
    hashValueType hashValue() const;
    int isEqual( const Object  &other ) const;
    int isLessThan( const Object &other ) const;
    void printOn( ostream & ) const;

    };

//*****************************************************************************
// grid object, customized for browser view
//*****************************************************************************
class _CLASSTYPE TBrowGrid: public TRuleGrid, TSnoop {

    public:

    TBrowGrid( PTWindowsObject AParent );
    virtual ~TBrowGrid( void );

    typedef enum {

        kHeaderStyle,
        kHeaderName,
        kHeaderCat,
        kTitleStyle,
        kTitleName,
        kTitleCat,
        kNoteStyle,
        kNoteName,
        kNoteCat,
        kNotACell

        } TCellType;

    typedef enum {

		kHeaderRow,
		kTitleRow,
		kNoteRow,
        kNotARow

        } TRowType;

    typedef enum {

		kStyleCol,
        kNameCol,
        kCatCol,
        kNotACol

        } TColType;

    protected:

    void SetupWindow( void );
    void Listen( TSnoopMsg, RObject );
    void Reset( void );

    int CellWidth( PTPaint, int col );
    int CellHeight( PTPaint, int row );
    void PaintCell( RTDrawInfo );

    BOOL IsCellSelectable( int row, int col );
    void GetDefaultFocusCell( int &row, int &col );
    void MousKey( TMousKey );

    void MousClick( RTMousInfo );
    void MousRtClick( RTMousInfo );
    void MousDblClick( RTMousInfo );
    void MousOver( RTMousInfo );

    void WMVScroll(RTMessage)= [WM_FIRST + WM_VSCROLL];
    void WMHScroll(RTMessage)= [WM_FIRST + WM_HSCROLL];



    void SetDefaultPrintRange( void );

    private:

    int  GridWidth( PTPaint, int /*col*/ );
    int  GridHeight( PTPaint, int /*row*/ );

    void PaintHeaderName( RTDrawInfo di );
    void PaintHeaderCat( RTDrawInfo di );
    void PaintNoteStyle( RTDrawInfo di );
    void PaintNoteName( RTDrawInfo di );
    void PaintNoteCat( RTDrawInfo di );
    void PaintTitleName( RTDrawInfo di );

    void ResetNotesNoGrouping( void );
    void ResetNotesWithGrouping( void );
    BOOL   IsNoteRow( int row );
    RTNote GetNote( int row );
    void   InvalidateNote( RTNote );

    TCellType CellType( int row, int col );
    TRowType  RowType(  int row );
    TColType  ColType(  int col );

    PTCatList     itsCatListP;
    int           itsNumCats;
    int           itsRowCount;
    int           itsMaxCatDepth;
    PTXArray      itsCatTableP;
    PTNoteTable   itsNoteTableP;
    TInk          itsInk;
    };


#endif
