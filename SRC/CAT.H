#ifndef __cat__
#define __cat__

_CLASSDEF( TBrowGridData )
_CLASSDEF( TCat)
_CLASSDEF( TCatList )
_CLASSDEF( TCatTree )
_CLASSDEF( TAddress )

#ifndef __address__
#include "address.h"
#endif
#ifndef __error__
#include "error.h"
#endif
#ifndef __safstr__
#include "safstr.h"
#endif

//CAREFUL: these values are written out to file, so don't rearrange!
typedef enum {

    kUserCatType            = 0,
    kUncategorizedCatType   = 1,
    kObsolete               = 2,            //was kDoneCatType
    kAddressCatType         = 3,
    kMasterAddressCatType   = 4,
    kLastCatType            = 4

    } TCatType;

class _CLASSTYPE TCat: Sortable, TStreamable {

    friend TCatList;
    friend TCatTree;

public:

    // constructors
    TCat( const TCatType t= kUserCatType );
    TCat( StreamableInit );
    TCat( RTCat cat, BOOL allSubcats = TRUE );
    TCat( const TCatType catType, const char *name );
    virtual ~TCat();

    // access methods
    void      GetDescription( char *description, int maxLen = 255 );
    void      GetNickname( char *nickname, BOOL useShortName, int maxLen = 255  ) const;
    void      GetKeywords( char *keywords, int maxLen = 255  );
    void      GetShortName( RTSafStr name, int maxCharsPerLevel = 0 ) const;
    void      GetFullName( RTSafStr name, BOOL useNickname, char separator = ':', int maxCharsPerLevel = 0 ) const;
    int       GetDepth( void );
    PTCat     GetParent( void );
    TCatType  GetCatType( void );
    BOOL      HasSubCats( void );
    PTCatTree GetSubCats( void ) { return itsSubCatsP; }
    BOOL      IsSubCat( RTCat cat );
    BOOL      IsAddress( void )  { return itsAddressP != NULL; }
    BOOL      IsShowingSubCats( void );
    void      SetDescription( const char *description );
    void      SetNickname( const char *nickname );
    void      SetKeywords( const char *keywords );
    void      AddSubCat( RTCat cat );
    void      ToggleVisibleSubCats( void );
    void      SetVisibleSubCats( BOOL visible );

    // other methods
    void      Mark( void );
    void      Rename( const char *newName );
    BOOL      KeyMatch( const char *pText );
	BOOL      HealthCheck( void );

    // operators
    BOOL      operator < ( RTCat other ) { return isLessThan( other ); };
    BOOL      operator == ( RTCat other ) { return isEqual( other ); };

    // address methods
    void      GetField( TAddressField field, char *buf, int len, TWHA wha= kWorkAddress );
    void      SetField( TAddressField field, char *buf, TWHA wha= kWorkAddress );
    void      GetField( TAddressField field, RTSafStr str, TWHA wha= kWorkAddress );
    void      FormatAddr( RTSafStr str, BOOL name, BOOL address, BOOL extra, TWHA wha= kWorkAddress );
    void      SetAddrMark( BOOL b );
    BOOL      GetAddrMark( void );
    BOOL      Matches( const char *match );

    TCSVError ExportAddress( ofstream &os, BOOL fieldNames= FALSE );
    TCSVError ImportAddress( ifstream &os, PTCSV csvP );

    // object nonsense
    classType     isA( void ) const;
    char         *nameOf( void ) const;
    hashValueType hashValue( void ) const;
    int           isLessThan( const Object & ) const;
    int           isEqual(const Object &) const;
    void          printOn(ostream &) const;
    static        PTStreamable build();

protected:

    virtual const Pchar streamableName() const { return "TCat"; };
    virtual void write( Ropstream );
    virtual Pvoid read( Ripstream );

private:

    TCat     &operator =( const TCat &other );

    TSafStr   itsDescription;
    TSafStr   itsKeywords;
    TSafStr   itsShortName;
    TSafStr   itsNickname;
    TCatType  itsCatType;
    BOOL      itsShowingSubs;
    BOOL      itsMarked;
    PTCat     itsParentP;
    PTCatTree itsSubCatsP;

    PTAddress itsAddressP;  // CAREFUL: itsAddressP is allowed to be NULL

    };

    inline Ripstream operator >> (Ripstream is, RTCat cl)
        { return is >> (RTStreamable) cl; }
    inline Ripstream operator >> (Ripstream is, RPTCat cl)
        { return is >> (RPvoid) cl; }
    inline Ropstream operator << (Ropstream os, RTCat cl)
        { return os << (RTStreamable) cl; }
    inline Ropstream operator << (Ropstream os, PTCat cl)
        { return os << (PTStreamable) cl; }

#endif
