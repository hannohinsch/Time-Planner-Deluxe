#ifndef __memnote__
#define __memnote__

_CLASSDEF( TMemorizedNote )
_CLASSDEF( TMemNoteSet )
_CLASSDEF( TNote )
_CLASSDEF( TMemorizedDialog )

#ifndef __classid__
#include "classid.h"
#endif
#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __xarray__
#include "xarray.h"
#endif
#ifndef __note__
#include "note.h"
#endif

class _CLASSTYPE TMemorizedNote: public Sortable, TStreamable {

  friend TMemorizedDialog;
  friend TMemNoteSet;

  public:

    TMemorizedNote( void );
    TMemorizedNote( PTNote pNote, const char *name );
    virtual ~TMemorizedNote( void );

    void SwitchCats( PTCatTree cats ) { (itsNoteP->GetCatList()).SwitchCats( cats ); };
    int isEqual( const Object &other ) const;
    int isLessThan( const Object &other ) const;

    // Object nonsense
    classType isA() const { return IDMemNote; };
    char _FAR *nameOf() const { return( "TMemorizedNote" ); };
    hashValueType hashValue() const { return 0; };
    void printOn( ostream _FAR & ) const { };

    static PTStreamable build();
    BOOL   HealthCheck( void );

private:
    PTNote    itsNoteP;
    TSafStr   itsName;

    TMemorizedNote( StreamableInit s): itsName( s ) {};
    void write( Ropstream );
    Pvoid read( Ripstream );

    const Pchar streamableName() const { return "TMemorizedNote"; };

};

//*****************************************************************************

//*****************************************************************************
class _CLASSTYPE TMemNoteSet: public TXArray, TStreamable
{
    public:
        TMemNoteSet( void ):
                TXArray( 10, 10 ) {};
        virtual ~TMemNoteSet() {};

        RTMemorizedNote FindNote( RTMemorizedNote );

        static PTStreamable build();
		BOOL HealthCheck( void );

        int     ModifyForChangedWorkTime( void );
        void    UpdateUnitLength( void );

    private:
        TMemNoteSet( StreamableInit ): TXArray(10, 10) {};
        void write( Ropstream );
        Pvoid read( Ripstream );

        const Pchar streamableName() const { return "TMemNoteSet"; };

};

    inline Ripstream operator >> (Ripstream is, RTMemNoteSet cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTMemNoteSet cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTMemNoteSet cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTMemNoteSet cl)
        { return os << (PTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RTMemorizedNote cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTMemorizedNote cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTMemorizedNote cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTMemorizedNote cl)
        { return os << (PTStreamable) cl; }


#endif
