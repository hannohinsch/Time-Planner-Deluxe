#ifndef __cattree__
#define __cattree__

_CLASSDEF( TCat )
_CLASSDEF( TCatTree )
_CLASSDEF( TSafStr )
_CLASSDEF( TXArray )

#ifndef __xarray__
#include "xarray.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// hierarchical category tree; each element (TCat) has a TCatTree member
/////////////////////////////////////////////////////////////////////////////
class _CLASSTYPE TCatTree: Object, TStreamable {

public:

    TCatTree( PTCat parentP );
    TCatTree( PTCatTree pCatTree, BOOL allSubcats = TRUE  );
    virtual ~TCatTree( void );

    // find a category in the tree
    RTCat FindCatInTopLevel( const char *name );
    RTCat FindCatInTree( RTSafStr name );
    BOOL  IsCatInTree( RTCat searchCat );
    BOOL  IsCatInTopLevel( RTCat searchCat );

    // array operations
    void  detach( RTCat cat );
    void  add( RTCat cat );
    int   getItemsInContainer( void );
    void  flush( TXArray::TDeleteStyle dt= TXArray::kNoDelete );
    int   NumCatsInTree( void );

    // operators
    RTCat     operator []( int index );
    int       find( RTCat );

    // others
    void  Merge( PTCat parentP, RTCatTree otherTree );
	BOOL  HealthCheck( void );
    BOOL  RoomForMore( int howManyMore,
        BOOL showError   = TRUE,
        BOOL showWarning = TRUE,
        char *altError   = NULL,
        char *altWarning = NULL );

    // marking categories
    void  ClearMarks( void );

    // object nonsense
    classType     isA( void ) const;
    char         *nameOf( void ) const;
    hashValueType hashValue( void ) const;
    int           isEqual(const Object &) const;
    void          printOn(ostream &) const;
    static        PTStreamable build();

    TCatTree( StreamableInit ) {};
    virtual const Pchar streamableName() const { return "TCatTree"; };
    virtual void write( Ropstream );
    virtual Pvoid read( Ripstream );
    void    FixupParents( PTCat parentP );

private:
    TCatTree &operator =( const TCatTree &other );

    PTXArray     pCatArray;
    PTCat        itsParentP;
    };


    inline Ripstream operator >> (Ripstream is, RTCatTree cl)
        { return is >> (RTStreamable) cl; }
    inline Ripstream operator >> (Ripstream is, RPTCatTree cl)
        { return is >> (RPvoid) cl; }
    inline Ropstream operator << (Ropstream os, RTCatTree cl)
        { return os << (RTStreamable) cl; }
    inline Ropstream operator << (Ropstream os, PTCatTree cl)
        { return os << (PTStreamable) cl; }

#endif
