//*****************************************************************************
// category objects: TCat
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "cat.h"

#ifndef __address__
#include "address.h"
#endif
#ifndef __catlist__
#include "catlist.h"
#endif
#ifndef __cattree__
#include "cattree.h"
#endif
#ifndef __classid__
#include "classid.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __error__
#include "error.h"
#endif
#ifndef __xarray__
#include "xarray.h"
#endif

//**************************************************************************
//
// Constructors
//
//**************************************************************************

//--------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------
TCat::TCat( const TCatType t )
{
    itsSubCatsP    = new TCatTree( this );
    itsShowingSubs = FALSE;
    itsMarked      = FALSE;
    itsCatType     = t;
    itsParentP     = NULL;

    if ( t == kAddressCatType ) {

        itsAddressP = new TAddress();
    }
    else {

        itsAddressP = NULL;
    }


}

//*****************************************************************************
// Constructor
//*****************************************************************************
TCat::TCat( const TCatType catType, const char *name )
{
    itsSubCatsP    = new TCatTree( this );
    itsShowingSubs = FALSE;
    itsMarked      = FALSE;
    itsCatType     = catType;
    itsParentP     = NULL;
    itsShortName.Set( name );

    if ( catType == kAddressCatType ) {

        itsAddressP = new TAddress();
    }
    else {

        itsAddressP = NULL;
    }

}


//--------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------
void TCat::TCat( StreamableInit s ):
	itsDescription( s ),
	itsKeywords( s ),
	itsNickname( s ),
	itsShortName( s )
{
    itsMarked    = FALSE;
    itsCatType   = kUserCatType;
};

//--------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------
void TCat::TCat( RTCat cat, BOOL allSubcats )
{
    itsDescription = cat.itsDescription;
    itsShortName   = cat.itsShortName;
    itsNickname    = cat.itsNickname;
    itsKeywords    = cat.itsKeywords;
    itsMarked      = cat.itsMarked;
    itsCatType     = cat.itsCatType;
    itsShowingSubs = cat.itsShowingSubs;
    itsSubCatsP    = new TCatTree( cat.itsSubCatsP, allSubcats );
    itsParentP     = NULL;

    if (cat.itsAddressP != NULL) {

        itsAddressP = new TAddress( *cat.itsAddressP );
    }
    else {

        itsAddressP = NULL;
    }
};

//--------------------------------------------------------------------------
// Destructor
//--------------------------------------------------------------------------
TCat::~TCat()
{
delete itsSubCatsP;

if( itsAddressP )
    delete itsAddressP;
}

//**************************************************************************
//
// Access Methods
//
//**************************************************************************

//--------------------------------------------------------------------------
// GetCatType
//--------------------------------------------------------------------------
TCatType TCat::GetCatType( void )
{
    return itsCatType;
}

//--------------------------------------------------------------------------
// GetDescription
//--------------------------------------------------------------------------
void TCat::GetDescription( char *description, int maxLen  )
{
    itsDescription.Get( description, maxLen );
}

//--------------------------------------------------------------------------
// GetKeywords
//--------------------------------------------------------------------------
void TCat::GetKeywords( char *keywords, int maxLen  )
{
    itsKeywords.Get( keywords, maxLen );
}

//--------------------------------------------------------------------------
// GetNickname
//  this is the abbreviation specified in the cat mgr dialog
//--------------------------------------------------------------------------
void TCat::GetNickname( char *nickname, BOOL useShortName, int maxLen  ) const
{
    if ((itsNickname.Len() == 0) && (useShortName)) {

        itsShortName.Get( nickname, maxLen );
    }
    else {

        itsNickname.Get( nickname, maxLen );
    }
}

//--------------------------------------------------------------------------
// GetShortName
// get the regular old name (no parent names)
//--------------------------------------------------------------------------
void TCat::GetShortName( RTSafStr name, int maxCharsPerLevel ) const
{
    if ((maxCharsPerLevel > 0) && (maxCharsPerLevel < 256)) {

        char string[256];

        itsShortName.Get( string, 255 );
        string[maxCharsPerLevel] = '\0';
        name.Set( string );
    }
    else {

        name.Set( itsShortName );
    }
}

//--------------------------------------------------------------------------
// GetFullName
// fully qualified name (all levels)
//--------------------------------------------------------------------------
void TCat::GetFullName( RTSafStr name, BOOL useNickname, char separator, int maxCharsPerLevel ) const
{
    if (itsParentP == NULL) {

        if (useNickname) {

            char nickname[256];
            GetNickname( nickname, TRUE, maxCharsPerLevel );
            name.Set( nickname );
        }
        else {

            GetShortName( name, maxCharsPerLevel );
        }
    }
    else {

        TSafStr rootName;
        char    string[256];

        itsParentP->GetFullName( rootName, useNickname, separator, maxCharsPerLevel );
        if (useNickname) {

            char nickname[256];
            GetNickname( nickname, TRUE, maxCharsPerLevel );
            name.Set( nickname );
        }
        else {

            GetShortName( name, maxCharsPerLevel );
        }
        wsprintf( string, "%s%c%s", rootName.Addr(), separator, name.Addr());
        name.Set( string );
    }
}

//--------------------------------------------------------------------------
// GetParent
//--------------------------------------------------------------------------
PTCat TCat::GetParent( void )
{
    return itsParentP;
}

//--------------------------------------------------------------------------
// GetDepth
//--------------------------------------------------------------------------
int TCat::GetDepth( void )
{
    if (itsParentP == NULL) {

        return 1;
    }
    else {

        return itsParentP->GetDepth() + 1;
    }
}

//--------------------------------------------------------------------------
// HasSubCats
//--------------------------------------------------------------------------
BOOL TCat::HasSubCats( void )
{
    return itsSubCatsP->getItemsInContainer() > 0;
}

//--------------------------------------------------------------------------
// IsShowingCats
//--------------------------------------------------------------------------
BOOL TCat::IsShowingSubCats( void )
{
    return itsShowingSubs;
}

//--------------------------------------------------------------------------
// IsSubCat
//--------------------------------------------------------------------------
BOOL TCat::IsSubCat( RTCat cat )
{
    if (cat == *this) {

        return TRUE;
    }
    else {

        return itsSubCatsP->IsCatInTree( cat );
    }
}

//--------------------------------------------------------------------------
// ToggleVisibleSubCats
//--------------------------------------------------------------------------
void TCat::ToggleVisibleSubCats( void )
{
    itsShowingSubs = !itsShowingSubs;
}

//--------------------------------------------------------------------------
// SetVisibleSubCats
//--------------------------------------------------------------------------
void TCat::SetVisibleSubCats( BOOL visible )
{
    itsShowingSubs = visible;
}


//--------------------------------------------------------------------------
// SetDescription
//--------------------------------------------------------------------------
void TCat::SetDescription( const char *description )
{
    itsDescription.Set( description );
}

//--------------------------------------------------------------------------
// SetKeywords
//--------------------------------------------------------------------------
void TCat::SetKeywords( const char *keywords)
{
    itsKeywords.Set( keywords );
}

//--------------------------------------------------------------------------
// SetNickname
//--------------------------------------------------------------------------
void TCat::SetNickname( const char *nickname )
{
    itsNickname.Set( nickname );
}

//--------------------------------------------------------------------------
// Rename
//
// Make sure to keep the parent's cat tree in alphabetical order
//--------------------------------------------------------------------------
void TCat::Rename( const char *newName )
{
    if (itsParentP == NULL) {

        itsShortName.Set( newName );
    }
    else {

        PTCat parentP = itsParentP;  // detaching clears out this field

        parentP->itsSubCatsP->detach( *this );
        itsShortName.Set( newName );
        parentP->itsSubCatsP->add( *this );
    }
}



//--------------------------------------------------------------------------
//check for keyword match in text
//FIXME ?? make recursive ??
//FIXME this is truly the wrong way to do it!!! (speed)
//--------------------------------------------------------------------------
BOOL TCat::KeyMatch( const char *pText )
{
BOOL found=FALSE;
PTSafStr keyCopy;
char *pKey;

keyCopy= new TSafStr( itsKeywords );

// if the cat is an address then append the first
// and last names to the key
if (itsAddressP)
    {
    TSafStr temp;

    itsAddressP->GetField( kFirstNameField, temp );
    keyCopy->Cat( " " );
    keyCopy->Cat( temp );

    itsAddressP->GetField( kLastNameField, temp );
    keyCopy->Cat( " " );
    keyCopy->Cat( temp );
    }

// otherwise append the name of the cat to the key
else
    {
    keyCopy->Cat( " " );
    keyCopy->Cat( itsShortName );
    }

//make a copy we can use for strtok
char *copy2= new char[ keyCopy->Len() + 10 ];
strcpy( copy2, keyCopy->Addr() );

// get the first token from the key
pKey= strtok( copy2, " " );

//check each keyword; walk thru string
while( pKey != NULL )
    {
    int len= strlen( pKey );
    const char *p= pText;

    while( *p && !found )
        {
        if( 0 == strnicmp( pKey, p, len ) )
            found= TRUE;
        p++;
        }

    pKey= strtok( NULL, " ");
    }

delete copy2;
delete keyCopy;
return found;
}

//--------------------------------------------------------------------------
// Mark
//
//     Mark a category and its parents in a cat tree
//--------------------------------------------------------------------------
void TCat::Mark( void )
{
    itsMarked = TRUE;
    if (itsParentP != NULL) {

        itsParentP->Mark();
    }
}

//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
BOOL TCat::HealthCheck( void )
{
BOOL ok= TRUE;

ok &= itsShortName.HealthCheck();
ok &= itsDescription.HealthCheck();
ok &= itsNickname.HealthCheck();
ok &= itsKeywords.HealthCheck();
ok &= itsSubCatsP->HealthCheck();

// check that the cat is in the list it should be
if (itsParentP != NULL) {

    ok &= itsParentP->itsSubCatsP->IsCatInTopLevel( *this );
}

return ok;
}

//--------------------------------------------------------------------------
// AddSubCat
//--------------------------------------------------------------------------
void TCat::AddSubCat( RTCat cat )
{
    itsSubCatsP->add( cat );
}

//**************************************************************************
//
// Address methods
//
//**************************************************************************
//--------------------------------------------------------------------------
// GetField
//--------------------------------------------------------------------------
void TCat::GetField( TAddressField field, char *buf, int len, TWHA wha )
{
    ASSERT( ERR_GEN, itsAddressP != NULL );
    itsAddressP->GetField( field, buf, len, wha );
}

//--------------------------------------------------------------------------
// GetField
//--------------------------------------------------------------------------
void TCat::GetField( TAddressField field, RTSafStr str, TWHA wha )
{
    ASSERT( ERR_GEN, itsAddressP != NULL );
    itsAddressP->GetField( field, str, wha );
}

//*****************************************************************************

//*****************************************************************************
void TCat::FormatAddr( RTSafStr str, BOOL name, BOOL address, BOOL extra, TWHA wha )
{
    ASSERT( ERR_GEN, itsAddressP != NULL );
    itsAddressP->FormatAddr( str, name, address, extra, wha );
}

//--------------------------------------------------------------------------
// SetField
//--------------------------------------------------------------------------
void TCat::SetField( TAddressField field, char *buf, TWHA wha )

{
    ASSERT( ERR_GEN, itsAddressP != NULL );
    itsAddressP->SetField( field, buf, wha );
}

//--------------------------------------------------------------------------
// Matches
//--------------------------------------------------------------------------
BOOL TCat::Matches( const char *match )
{
    ASSERT( ERR_GEN, itsAddressP != NULL );
    return( itsAddressP->Matches( match ));
}

//*****************************************************************************

//*****************************************************************************
BOOL TCat::GetAddrMark( void )
{
return itsAddressP->GetMark();
}

//*****************************************************************************

//*****************************************************************************
void TCat::SetAddrMark( BOOL b )
{
itsAddressP->SetMark( b );
}

//--------------------------------------------------------------------------
// ExportAddress
//--------------------------------------------------------------------------
TCSVError TCat::ExportAddress( ofstream &o, BOOL fieldNames )
{
    ASSERT( ERR_GEN, itsAddressP != NULL );
    return( itsAddressP->Export( o, fieldNames ));
}

//--------------------------------------------------------------------------
// ImportAddress
//--------------------------------------------------------------------------
TCSVError TCat::ImportAddress( ifstream &i, PTCSV csvP )
{
    ASSERT( ERR_GEN, itsAddressP != NULL );
    return( itsAddressP->Import( i, csvP ));
}

//**************************************************************************
//
// basic object fns
//
//**************************************************************************
classType TCat::isA( void ) const           { return IDCat;           }
char * TCat::nameOf( void ) const           { return "TCat";          }
hashValueType TCat::hashValue( void ) const { return 0;               }
int TCat::isEqual(const Object & obj) const { return( this == & obj );}
void TCat::printOn(ostream far&) const      {                         }

//--------------------------------------------------------------------------
// isLessThan
//
//--------------------------------------------------------------------------
int TCat::isLessThan( const Object &rhs ) const
{

    RTCat cat= ((RTCat)rhs);

    //handle the common case where the two cats have the same parent
    if( itsParentP == cat.itsParentP )
        {
        return itsShortName < cat.itsShortName;
        }

    const   kMaxDepth= 25;      //if trees are deeper, too bad!
    TCat    *(p0[kMaxDepth]);     //parents of this cat
    TCat    *(p1[kMaxDepth]);     //parents of argument cat

    //build parent array of this cat
    PTCat p= (PTCat) this;
    int i=0;
    while( p != NULL && i < kMaxDepth )
        {
        p0[i++]= p;
        p= p->itsParentP;
        }

    //build parent array of argument cat
    p= &cat;
    int j=0;
    while( p != NULL && j < kMaxDepth )
        {
        p1[j++]= p;
        p= p->itsParentP;
        }


    //compare from root, alphabetize at first place trees differ
    i--;    //index of root cat
    j--;
    while( i > 0 && j > 0 && p0[i] == p1[j] )
        {
        i--;
        j--;
        }

    //if the current indices still point to the same cat, it means one of
    //the two arrays is now empty, and that the one that is not empty
    //represents a subcategory of the other
    if( p0[i] == p1[j] )
        {
        return ( i == 0 );
        }

    //indices point to first cat that's not a common ancestor
    return p0[i]->itsShortName < p1[j]->itsShortName;
}

//**************************************************************************
//
// I/O
//
//**************************************************************************

//--------------------------------------------------------------------------
// build
//--------------------------------------------------------------------------
PTStreamable TCat::build( void )
{
return( new TCat( streamableInit ) );
}

TStreamableClass RegTCat( "TCat", TCat::build, __DELTA(TCat) );

//--------------------------------------------------------------------------
// read
//
// itsParentP is modified by calling TCatTree::FixupParents after all
// of the categories are read in
//--------------------------------------------------------------------------
Pvoid TCat::read( Ripstream is )
{
int version;

is >> version;

if (version == 8)              // direct subcats, address writes
    {
    itsShortName.read( is );
    itsNickname.read( is );
    itsKeywords.read( is );
    itsDescription.read( is );

    itsSubCatsP= new TCatTree( streamableInit );
    itsSubCatsP->read( is );

    is >> itsShowingSubs;

    itsAddressP= NULL;
    BOOL hasAddr;
    is >> hasAddr;
    if( hasAddr )
        {
        itsAddressP= new TAddress( streamableInit );
        itsAddressP->read( is );
        }

    int i;
    is >> i;
    itsCatType= (TCatType) i;

    itsMarked  = FALSE;
    itsParentP = NULL;
    }
else if (version == 7)              // catType
    {
    itsShortName.read( is );
    itsNickname.read( is );
    itsKeywords.read( is );
    itsDescription.read( is );

    is >> itsSubCatsP;
    is >> itsShowingSubs;
    is >> itsAddressP;

    int i;
    is >> i;
    itsCatType= (TCatType) i;

    itsMarked  = FALSE;
    itsParentP = NULL;
    }
else if (version == 6)              // direct writes
    {
    itsShortName.read( is );
    itsNickname.read( is );
    itsKeywords.read( is );
    itsDescription.read( is );

    is >> itsSubCatsP;
    is >> itsShowingSubs;
    is >> itsAddressP;

    itsCatType = kUserCatType;        //every cat in old files was a UserCatType
    if( itsShortName == "<addresses>" )
        itsCatType= kMasterAddressCatType;

    if( itsAddressP != NULL )
        itsCatType= kAddressCatType;

    itsMarked  = FALSE;
    itsParentP = NULL;
    }
else if (version == 5)              // added nickname  (version 1.00)
    {
    is >> itsShortName;
    is >> itsNickname;
    is >> itsKeywords;
    is >> itsDescription;

    is >> itsSubCatsP;
    is >> itsShowingSubs;
    is >> itsAddressP;

    itsCatType = kUserCatType;        //every cat in old files was a UserCatType
    if( itsShortName == "<addresses>" )
        itsCatType= kMasterAddressCatType;

    if( itsAddressP != NULL )
        itsCatType= kAddressCatType;

    itsMarked  = FALSE;
    itsParentP = NULL;
    }
else
	SYSERR( ERR_VER );

return this;
}

//--------------------------------------------------------------------------
// write
//--------------------------------------------------------------------------
void TCat::write( Ropstream os )
{
os << (int) 8;      //version 8: direct writes of address, subcats

itsShortName.write( os );
itsNickname.write( os );
itsKeywords.write( os );
itsDescription.write( os );

itsSubCatsP->write( os );

os << itsShowingSubs;

if( itsAddressP )
    {
    os << TRUE;
    itsAddressP->write( os );
    }
else
    {
    os << FALSE;
    }

os << (int) itsCatType;
}

