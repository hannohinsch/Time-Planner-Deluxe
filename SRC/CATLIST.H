#ifndef __catlist__
#define __catlist__

_CLASSDEF( TCat)
_CLASSDEF( TCatList )
_CLASSDEF( TCatTree )

#ifndef __address__
#include "address.h"
#endif
#ifndef __snoop__
#include "snoop.h"
#endif
#ifndef __xarray__
#include "xarray.h"
#endif

typedef enum{ kAddressInit } TAddressInit;
typedef enum{ kCatMatchAll, kCatMatchAny } TMatchStyle;

// category array (flat, not tree-structured); member of notes and views.
// It is assumed throughout this class, especially by Listen(), that the
// cats in a catlist are not owned by the catlist!
class _CLASSTYPE TCatList: Object, TSnoop, TStreamable
{

public:

    // constructors
    TCatList( void );
    TCatList( const char *text );
    TCatList( RTCatList catList );
    TCatList( RTCatTree catTree, BOOL doOnlyTopLevel= FALSE );
    TCatList( RTCatTree catTree, TAddressInit );
    TCatList( StreamableInit ) {};
    virtual ~TCatList( void );

    // miscellaneous
    BOOL      IsCatInList( RTCat );
    BOOL      IsCatOrChildInList( RTCatTree catTree );
    BOOL      IsCatOrChildInList( RTCat cat );
    void      Union( PTCatList newCatList );
    BOOL      Match( PTCatList pCatList, TMatchStyle matchStyle );
    void      FmtCatNames( char *pBuf, int n, char sep );
	BOOL      HealthCheck( void );
    void      AddVisibleCats( PTCatTree catTreeP );
    void      SwitchCats( PTCatTree catTreeP );

    // array operators
    void      detach( RTCat cat );
    void      add( RTCat cat );
    int       getItemsInContainer( void ) { return pCatArray->getItemsInContainer(); };
    void      flush( TXArray::TDeleteStyle dt= TXArray::kNoDelete );
    int       find( const RTCat cat );
    RTCat     operator []( int index );

    // object nonsense
    classType     isA( void ) const;
    char         *nameOf( void ) const;
    hashValueType hashValue( void ) const;
    int           isEqual(const Object &) const;
    void          printOn(ostream &) const;
    static        PTStreamable build();

    void write( Ropstream );
    Pvoid read( Ripstream );

protected:

    void  Listen( TSnoopMsg, RObject );
    const Pchar streamableName() const { return "TCatList"; };

private:

    TCatList &operator =( const TCatList &other );

    void AddRecurse( RTCatTree catTree, BOOL doOnlyTopLevel );
    void KeyRecurse( RTCat cat, const char *text);

    PTXArray pCatArray;
};

    inline Ripstream operator >> (Ripstream is, RTCatList cl)
        { return is >> (RTStreamable) cl; }
    inline Ripstream operator >> (Ripstream is, RPTCatList cl)
        { return is >> (RPvoid) cl; }
    inline Ropstream operator << (Ropstream os, RTCatList cl)
        { return os << (RTStreamable) cl; }
    inline Ropstream operator << (Ropstream os, PTCatList cl)
        { return os << (PTStreamable) cl; }

#endif
