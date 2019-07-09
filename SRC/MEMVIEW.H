#ifndef __memview__
#define __memview__

_CLASSDEF( TMemorizedView )
_CLASSDEF( TMemViewSet )
_CLASSDEF( TMemorizedViewDialog )

#ifndef __classid__
#include "classid.h"
#endif
#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __xarray__
#include "xarray.h"
#endif
#ifndef __frame__
#include "frame.h"
#endif
#ifndef __rptgrid__
#include "rptgrid.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif

class _CLASSTYPE TMemorizedView: public Sortable, TStreamable {

  public:

    TMemorizedView( void );
    TMemorizedView( const char *name );
    virtual ~TMemorizedView( void );

    int isEqual( const Object &other ) const;
    int isLessThan( const Object &other ) const;

    void GetName( RTSafStr name ) { name = itsName; };
    void Select( void );
    void SwitchCats( PTCatTree );

    // Object nonsense
    classType isA() const { return IDMemView; };
    char _FAR *nameOf() const { return( "TMemorizedView" ); };
    hashValueType hashValue() const { return 0; };
    void printOn( ostream _FAR & ) const { };

    static PTStreamable build();
    BOOL HealthCheck( void );

private:

    TActiveView         itsView;
    TTimeScale          itsTimeScale;
    TTime               itsTime;
    TSafStr             itsName;
    PTFilter            itsFilterP;
    int                 itsBreakLevel;
    TReportField        itsColumnFields[kMaxReportCols];
    TDayviewIncrements  itsDayviewIncrement;
    TNoteOrder          itsNoteOrders[kNumNoteOrders];
    TCatGrouping        itsCatGrouping;
    PTCatList           itsGroupCatListP;

    TMemorizedView( StreamableInit s): itsName( s ) {};
    void write( Ropstream );
    Pvoid read( Ripstream );

    const Pchar streamableName() const { return "TMemorizedView"; };

};

class _CLASSTYPE TMemViewSet: public TXArray, TStreamable
{
    public:
        TMemViewSet( void ):
                TXArray( 10, 10 ) {};
        virtual ~TMemViewSet() {};


        RTMemorizedView FindView( RTMemorizedView );

        static PTStreamable build();
		BOOL HealthCheck( void );

    private:
        TMemViewSet( StreamableInit ): TXArray(10, 10) {};
        void write( Ropstream );
        Pvoid read( Ripstream );

        const Pchar streamableName() const { return "TMemViewSet"; };

};

    inline Ripstream operator >> (Ripstream is, RTMemViewSet cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTMemViewSet cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTMemViewSet cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTMemViewSet cl)
        { return os << (PTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RTMemorizedView cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTMemorizedView cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTMemorizedView cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTMemorizedView cl)
        { return os << (PTStreamable) cl; }


#endif
