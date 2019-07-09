#ifndef __notetab__
#define __notetab__

_CLASSDEF( TNoteRow )
_CLASSDEF( TNoteTotal )
_CLASSDEF( TNoteTable )
_CLASSDEF( TCat )
_CLASSDEF( TSafStr )
_CLASSDEF( TColor )
_CLASSDEF( TDrawInfo )
_CLASSDEF( TAppTime )
_CLASSDEF( TWorkTime )
_CLASSDEF( TNoteSet )
_CLASSDEF( TXArray )
_CLASSDEF( TCatTable )

#ifndef __profile__
#include "profile.h"
#endif
#ifndef __value__
#include "value.h"
#endif
#ifndef __worktime__
#include "worktime.h"
#endif
#ifndef __note__
#include "note.h"
#endif
#ifndef __xarray__
#include "xarray.h"
#endif

typedef enum {

    kNoteTableHeader,
    kNoteTableNote,
    kNoteTableTotal

    } TRowType;

class _CLASSTYPE TNoteTotal: public Object
{
    public:

    TNoteTotal( int level );
    virtual ~TNoteTotal();

    void Sum( RTNote );
    void SetSample( TNoteOrder order, PTNote noteP );
    void SetCat( PTCat catP );
    void Report( RTDrawInfo di );

    TValue      itsValue;
    TWorkTime   itsWorkTime;
    TDuration   itsDuration;
    TTime       itsFirstTime;
    TTime       itsLastTime;
    int         itsLevel;

    private:

    TNoteOrder  itsNoteOrder;
    PTCat       itsCatP;
    PTNote      itsSampleNoteP;

    TNoteTotal( void ){};          //unused
    void ReportElement( int level,
        RTSafStr   text,   BOOL &useText,
        TColor     &color, BOOL &useColor,
        TNoteStyle &style, BOOL &useStyle );

    //object paraphenalia (needed cause it's put into XArray)
    classType isA( void ) const { return IDRptTotal; };
    char *nameOf( void ) const { return "TNoteTotal"; };
    hashValueType hashValue( void ) const { return 0; };
    int isEqual(const Object &other) const { return this == &other;} ;
    void printOn(ostream &) const {};

};

class _CLASSTYPE TNoteRow: public Object
{
    public:

    TNoteRow( TRowType rowType, char *header, PTNote noteP, PTNoteTotal subtotalP )
        {
        itsRowType   = rowType;
        itsNoteP     = noteP;
        itsSubtotalP = subtotalP;
        strcpy( itsHeader, header );
        }

    virtual ~TNoteRow()
        {
        if (itsSubtotalP)
            delete itsSubtotalP;
        };

    TRowType      itsRowType;
    PTNote        itsNoteP;
    PTNoteTotal   itsSubtotalP;
    char          itsHeader[256];

    private:

    //object paraphenalia (needed cause it's put into XArray)
    TNoteRow( void ){};          //unused
    classType isA( void ) const { return 0; };
    char *nameOf( void ) const { return "TNoteRow"; };
    hashValueType hashValue( void ) const { return 0; };
    int isEqual(const Object &other) const { return this == &other;} ;
    void printOn(ostream &) const {};
};

class _CLASSTYPE TNoteTable: public TXArray
{
    public:

    TNoteTable( void );
    ~TNoteTable( void );

    int AddWithGrouping( BOOL useSubtotal );

    int Add( char *header );
    int Add( PTNote noteP );
    int Add( PTNoteTotal totalP );
    int Add( PTCatList catListP, BOOL useSubtotal );
    int Add( PTNoteSet noteSetP, PTCat catP,
        BOOL useHeader, BOOL useSubtotal );

};

#endif
