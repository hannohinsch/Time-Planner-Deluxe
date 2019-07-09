#ifndef __noteset__
#define __noteset__

_CLASSDEF( TNote )
_CLASSDEF( TFilter )
_CLASSDEF( TNoteSet )
_CLASSDEF( TPimFrame )
_CLASSDEF( TXArray )

#ifndef __cat__
#include "cat.h"
#endif
#ifndef __error__
#include "error.h"
#endif
#ifndef __xarray__
#include "xarray.h"
#endif

class _CLASSTYPE TNoteSet: Object, TStreamable {

    public:
        TNoteSet( void );
        TNoteSet( RTFilter filter, PTNoteSet noteUniverse = NULL );
        TNoteSet( RTCat cat, PTNoteSet noteUniverse );
        virtual ~TNoteSet( void );

        classType isA( void ) const;
        char *nameOf( void ) const;
        hashValueType hashValue( void ) const;
        int isEqual(const Object &) const;
        void printOn(ostream &) const;
        static PTStreamable build();

        PTNote CreateNote( PTWindowsObject aParent );
        PTNote CopyNote( RTNote oldNote );
        void Sort( void );
        void detach( RTNote note );
        void add( RTNote note );
        BOOL HasNote( RTNote note );
        int getItemsInContainer( void ) { return pNoteArray->getItemsInContainer(); };
        void flush( TXArray::TDeleteStyle dt= TXArray::kNoDelete );

        RTNote operator []( int index );
		BOOL HealthCheck( void );

        BOOL  RoomForMore( int howManyMore,
            BOOL showError   = TRUE,
            BOOL showWarning = TRUE,
            char *altError   = NULL,
            char *altWarning = NULL );
        RTNote FindNote( RTNote note );

        int     ModifyForChangedWorkTime( void );
        void    UpdateUnitLength( void );

        static BOOL CancelBecauseOfTimeConflict( RTNote note, RTTime newStart, RTTime newEnd );

    protected:
        const Pchar streamableName() const { return "TNoteSet"; };
        TNoteSet( StreamableInit ) {};
        void write( Ropstream );
        Pvoid read( Ripstream );

    private:
        TNoteSet &operator =( const TNoteSet &other ) { ASSERT( ERR_ASSIGN, FALSE ); return *this; };

        PTXArray pNoteArray;
    };

    inline Ripstream operator >> (Ripstream is, RTNoteSet cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTNoteSet cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTNoteSet cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTNoteSet cl)
        { return os << (PTStreamable) cl; }


#endif

