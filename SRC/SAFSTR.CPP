//*****************************************************************************
// safe string object (streamable, sortable)
// Uses Windows string functions to insure language independence
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include <stdarg.h>     //for printf handling

#include "safstr.h"

#ifndef __classid__
#include "classid.h"
#endif
#ifndef __error__
#include "error.h"
#endif


//enforce use of Windows string fns, not those in runtime library
#undef strlen
#undef strcpy
#undef strncpy
#undef strcmp
#undef stricmp
#undef strcat

//shared scratch buffer
char TSafStr::itsScratchBuffer[ 1024 ];
char TSafStr::itsNullString[1]= "\0";

//*****************************************************************************

//*****************************************************************************
TSafStr::TSafStr( void )
{
itsLength= 0;
itsStringP= NULL;
itsCheckSum= hashValue();
CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
TSafStr::TSafStr( const char *aPtr )
{
    if ( aPtr != NULL )
        {
        itsLength = lstrlen( aPtr ) + 1;
        itsStringP = new char[ itsLength ];
        lstrcpy( itsStringP, aPtr );
        }
    else
        {
        itsLength= 0;
        itsStringP= NULL;
        }

    itsCheckSum= hashValue();

    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
TSafStr::TSafStr( const TSafStr& sourceString )
{
    itsLength = sourceString.itsLength;

    if( itsLength > 0 )
        {
        itsStringP = new char[ itsLength ];
        lstrcpy( itsStringP, sourceString.itsStringP );
        }
    else
        itsStringP= NULL;

    itsCheckSum= hashValue();

    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************
// load string from resource file
//*****************************************************************************
TSafStr::TSafStr( const WORD id )
{
    HANDLE hInstance= GetApplicationObject()->hInstance;
    int n= LoadString( hInstance, id, itsScratchBuffer, sizeof( itsScratchBuffer ) );

    if( n > 0 )
        {
        itsLength= n;
        itsStringP= new char[ itsLength ];
        lstrcpy( itsStringP, itsScratchBuffer );
        }
    else
        {
        itsLength= 0;
        itsStringP= NULL;
        }

    itsCheckSum= hashValue();

    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
TSafStr::~TSafStr()
{
CHKBUG( ERR_GEN, HealthCheck() );

if( itsStringP )
    {
    delete itsStringP;
    itsStringP= NULL;
    }
}

//*****************************************************************************

//*****************************************************************************
BOOL TSafStr::HealthCheck( void ) const
{
BOOL rc= TRUE;

if( itsLength == 0 || itsStringP == NULL )
    {
    if( itsLength != 0 || itsStringP != NULL || itsCheckSum != 0 )
        rc= FALSE;
    }
else
    rc= (itsCheckSum == hashValue());

return rc;
}

//*****************************************************************************

//*****************************************************************************
void TSafStr::Get( char *p, int n ) const
{
if( itsLength > 0 )
    {
    if( n < itsLength )
        {
        lstrcpyn( p, itsStringP, n-1 );
        p[n-1]= '\0';
        }
    else
        lstrcpy( p, itsStringP );
    }
else
    {
    if( n > 0 )
        *p= '\0';
    }

CHKBUG( ERR_GEN, HealthCheck() );
}


//*****************************************************************************

//*****************************************************************************
void TSafStr::Set( const char *p )
{
    if ( p != NULL )
        {
        sizeType sourceLen= 1 + lstrlen( p );
        if( itsLength < sourceLen )
            {
            if( itsStringP )
                delete itsStringP;

            itsStringP = new char[ sourceLen ];
            }

        lstrcpy( itsStringP, p );
        itsLength = sourceLen;
        }
    else
        {
        if( itsStringP )
            delete itsStringP;

        itsStringP= NULL;
        itsLength= 0;
        }

    itsCheckSum= hashValue();

    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
void TSafStr::Set( const TSafStr & sourceString)
{
    if( sourceString.itsLength )
        {
        if( itsLength < sourceString.itsLength )
            {
            if( itsStringP )
                delete itsStringP;

            itsStringP = new char[ sourceString.itsLength ];
            }

        itsLength = sourceString.itsLength;
        lstrcpy( itsStringP, sourceString.itsStringP );
        }
    else
        {
        if( itsStringP )
            delete itsStringP;

        itsStringP= NULL;
        itsLength= 0;
        }

    itsCheckSum= hashValue();

    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************
//set from resource id
//*****************************************************************************
void TSafStr::Set( const WORD id )
{
    HANDLE hInstance= GetApplicationObject()->hInstance;
    int n= LoadString( hInstance, id, itsScratchBuffer, sizeof( itsScratchBuffer ) );

    if( itsStringP )
        delete itsStringP;

    if( n > 0 )
        {
        itsLength= n;
        itsStringP= new char[ itsLength ];
        lstrcpy( itsStringP, itsScratchBuffer );
        }
    else
        {
        itsLength= 0;
        itsStringP= NULL;
        }

    itsCheckSum= hashValue();

    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
TSafStr & TSafStr::operator =( const TSafStr &other )
{
    if( other.itsLength )
        {
        if( itsLength < other.itsLength )
            {
            if( itsStringP )
                delete itsStringP;

            itsStringP = new char[ other.itsLength ];
            }

        itsLength = other.itsLength;
        lstrcpy( itsStringP, other.itsStringP );
        }
    else
        {
        if( itsStringP )
            delete itsStringP;

        itsStringP= NULL;
        itsLength= 0;
        }

    itsCheckSum= hashValue();

    CHKBUG( ERR_GEN, HealthCheck() );

    return *this;
}


//*****************************************************************************

//*****************************************************************************
BOOL TSafStr::operator == (const TSafStr &a) const
{
return ( OurCmp ( itsStringP, a.itsStringP ) == 0 );
}

//*****************************************************************************

//*****************************************************************************
BOOL TSafStr::operator != (const TSafStr &a) const
{
return ( OurCmp ( itsStringP, a.itsStringP ) != 0 );
}

//*****************************************************************************

//*****************************************************************************
BOOL TSafStr::operator > (const TSafStr &a) const
{
return ( OurCmp ( itsStringP, a.itsStringP ) > 0);
}

//*****************************************************************************

//*****************************************************************************
BOOL TSafStr::operator >= (const TSafStr &a) const
{
return ( OurCmp ( itsStringP, a.itsStringP ) >= 0);
}

//*****************************************************************************

//*****************************************************************************
BOOL TSafStr::operator < (const TSafStr &a) const
{
return ( OurCmp ( itsStringP, a.itsStringP ) < 0);
}

//*****************************************************************************

//*****************************************************************************
BOOL TSafStr::operator == (const char *a) const
{
return ( OurCmp ( itsStringP, a ) == 0 );
}

//*****************************************************************************

//*****************************************************************************
BOOL TSafStr::operator != (const char *a) const
{
return( OurCmp ( itsStringP, a ) != 0 );
}

//*****************************************************************************

//*****************************************************************************
BOOL TSafStr::operator > (const char *a) const
{
return ( OurCmp ( itsStringP, a ) > 0);
}

//*****************************************************************************

//*****************************************************************************
BOOL TSafStr::operator >= (const char *a) const
{
return ( OurCmp ( itsStringP, a ) >= 0);
}

//*****************************************************************************

//*****************************************************************************
BOOL TSafStr::operator < (const char *a) const
{
return ( OurCmp ( itsStringP, a ) < 0 );
}

//*****************************************************************************

//*****************************************************************************
BOOL TSafStr::operator <= (const char *a) const
{
return ( OurCmp ( itsStringP, a ) <= 0 );
}

////////////// inherited Object fns //////////////
//*****************************************************************************

//*****************************************************************************
TSafStr::isEqual( const Object& other ) const
{
    RTSafStr testString= (TSafStr &) other;

    // note: 2 zero length strings are equal by definition
    return ( itsLength == testString.itsLength &&
             ( itsLength == 0 ||
               !lstrcmp( itsStringP, testString.itsStringP )));
}

//*****************************************************************************

//*****************************************************************************
int TSafStr::isLessThan( const Object& testString ) const
{
    return ( 0 < OurCmp ( itsStringP, ((TSafStr &)testString).itsStringP ) );
}

//*****************************************************************************

//*****************************************************************************
hashValueType TSafStr::hashValue() const
{
    hashValueType   value = hashValueType(0);
    for( int i = 0; i < itsLength; i++ )
        {
        value ^= itsStringP[i];
        value = _rotl( value, 1 );
        }
    return value;
}

//*****************************************************************************

//*****************************************************************************
void TSafStr::printOn( ostream& outputStream ) const
{
    outputStream << itsStringP;
}

//*****************************************************************************

//*****************************************************************************
classType TSafStr::isA() const
{
    return IDSafStr;
}

//*****************************************************************************

//*****************************************************************************
char * TSafStr::nameOf() const
{
    return "TSafStr";
}

/////////////////////// stream support //////////////////////////////

//*****************************************************************************

//*****************************************************************************
PTStreamable TSafStr::build( void )
{
return( new TSafStr( streamableInit ) );
}

TStreamableClass RegTSafStr( "TSafStr", TSafStr::build, __DELTA(TSafStr) );

//*****************************************************************************

//*****************************************************************************
Pvoid TSafStr::read( Ripstream is )
{
int version;

is >> version;

if( version == 4 )              //allowed length == 0 (no string chars)
	{
    is >> itsCheckSum;
    is >> itsLength;

    if( itsLength > 0 )
        itsStringP= is.freadBigString();
    else
        itsStringP= NULL;

    if( itsCheckSum != hashValue() )
        {
        SYSERR( ERR_CHK );
        itsCheckSum= hashValue();
        }
    }
else if( version == 3 )              //converted to BigStrings ( >256 chars )
    {                               //version htp 1.00
    is >> itsCheckSum;
    is >> itsLength;
    itsStringP= is.freadBigString();
    if( itsCheckSum != hashValue() )
        {
        SYSERR( ERR_CHK );
        itsCheckSum= hashValue();
        }

    //we now handle empty strings more efficiently -- so convert
    //if possible
    if( itsLength == 0 )
        {
        delete itsStringP;
        itsStringP= NULL;
        itsCheckSum= hashValue();
        }
    }
else
	SYSERR( ERR_VER );

CHKBUG( ERR_GEN, HealthCheck() );
return this;
}

//*****************************************************************************

//*****************************************************************************
void TSafStr::write( Ropstream os )
{

os << (int) 4;  //version

if( itsCheckSum != hashValue() )       //make sure we're writing valid object
    {
    SYSERR( ERR_GEN );
    itsCheckSum= hashValue();          //repair before write
    }

os << itsCheckSum;
os << itsLength;
if( itsLength > 0 )
    os.fwriteBigString( itsStringP );
}

//*****************************************************************************
// if the string was created using the StreamableInit constructor, no
// storage was allocated ( read() was expected to do it ). But, for some
// reason (i.e., stream doesn't contain this string ), we chose not to call
// read() for this string.  Instead, call StreamFix() to
// make a valid object out of itself.
//*****************************************************************************
void TSafStr::StreamFix()
{
itsLength= 0;
itsStringP= NULL;
itsCheckSum= hashValue();
CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
BOOL TSafStr::IsBlank( void ) const
{
    for (int i = 0; i < itsLength; i++) {

        char c= *( itsStringP + i);
        if ( c != '\0' && !isspace( c )) {

            return( FALSE );
        }
    }

    return( TRUE );
}



// -----------------------------------------------------------------
// Contains (NB case insensitive)
//
// Is lookFor somewhere in 'this'?
// -----------------------------------------------------------------
BOOL TSafStr::Contains( const char *lookFor ) const
{

CHKBUG( ERR_GEN, HealthCheck() );

    if( itsStringP == NULL )
        return FALSE;

    int  len   = lstrlen( lookFor );
    if( len > 0 )
        {
        char *p = (char*) itsStringP;

        while ( *p )
            {
            //FIXME not DBCS compatible!  There is no lstrncmpi(), so
            //we use the one from the runtime library (which won't work
            //with DBCS
            if (strnicmp( lookFor, p, len ) == 0)
                {
                return( TRUE );
                }
            p= AnsiNext( p );
            }
        }
    else
        {
        return( TRUE );   // lookFor is empty
        }

    return( FALSE );      // ran out
}

// -----------------------------------------------------------------
// Clear
// -----------------------------------------------------------------
void TSafStr::Clear( void )
{
    if( itsStringP )
        delete itsStringP;

    itsLength= 0;
    itsStringP= NULL;
    itsCheckSum= hashValue();

    CHKBUG( ERR_GEN, HealthCheck() );
}

// -----------------------------------------------------------------
// Cat (concatenate)
// -----------------------------------------------------------------
void TSafStr::Cat( const char c )
{
    if( itsLength > 0 )
        {
        // copy the old string into a new one which is one char longer
        itsLength++;
        char *s = new char[itsLength];
        lstrcpy( s, itsStringP );

        delete itsStringP;
        itsStringP = s;

        // add the character to the end
        itsStringP[itsLength - 2] = c;
        itsStringP[itsLength - 1] = '\0';
        }
    else    //null string
        {
        if( itsStringP )
            delete itsStringP;

        itsLength= 2;
        itsStringP= new char[ itsLength ];
        itsStringP[0]= c;
        itsStringP[1]= '\0';
        }

    itsCheckSum= hashValue();

    CHKBUG( ERR_GEN, HealthCheck() );
}

// -----------------------------------------------------------------
// Cat (concatenate)
// -----------------------------------------------------------------
void TSafStr::Cat( const char * arg )
{
    // append arg to current string
    int argLength= lstrlen( arg );

    if( itsLength > 0 )
        {
        char *s = new char[itsLength + argLength];
        lstrcpy( s, itsStringP );
        lstrcpy( s + itsLength - 1, arg );

        if( itsStringP )
            delete itsStringP;

        itsStringP = s;
        itsLength+= argLength;
        }
    else
        {
        char *s = new char[ argLength + 1 ];
        lstrcpy( s, arg );

        if( itsStringP )
            delete itsStringP;

        itsStringP = s;
        itsLength= argLength + 1;
        }

    itsCheckSum= hashValue();

    CHKBUG( ERR_GEN, HealthCheck() );

}

// -----------------------------------------------------------------
// Cat (concatenate)
// -----------------------------------------------------------------
void TSafStr::Cat( const TSafStr & arg )
{
    // append arg to current string
    if( arg.itsLength > 0 )
        {
        if( itsLength > 0 )
            {
            char *s = new char[ itsLength + arg.itsLength -1 ];
            lstrcpy( s, itsStringP );
            lstrcpy( s + itsLength - 1, arg.itsStringP );

            delete itsStringP;

            itsStringP = s;
            itsLength+= arg.itsLength - 1;
            }
        else  //current string is empty
            {
            char *s = new char[ arg.itsLength ];
            lstrcpy( s, arg.itsStringP );

            if( itsStringP )
                delete itsStringP;

            itsStringP = s;
            itsLength= arg.itsLength;
            }

        itsCheckSum= hashValue();
        }

    CHKBUG( ERR_GEN, HealthCheck() );
}

//*****************************************************************************
// Printf directly into safstr
//*****************************************************************************
int TSafStr::Printf( const char *fmt, ... )
{

//FIXME this is 16-bit dependent!
va_list ap;
va_start( ap, fmt );
int n= wvsprintf( itsScratchBuffer, fmt, ap );
va_end( ap );

//now copy buffer into this string
if( n > 0 )
    {
    if( n + 1 > itsLength )
        {
        if( itsStringP )
            delete itsStringP;

        itsStringP = new char[n + 1];
        }

    lstrcpy( itsStringP, itsScratchBuffer );
    itsLength= n + 1;        //for terminating zero
    }
else
    {
    if( itsStringP )
        delete itsStringP;

    itsLength= 0;
    itsStringP= NULL;
    }

itsCheckSum= hashValue();

CHKBUG( ERR_GEN, HealthCheck() );
return n;
}

//*****************************************************************************

//*****************************************************************************
void TSafStr::ToUpper( void )
{
if( itsStringP )
    {
    AnsiUpper( itsStringP );
    itsCheckSum= hashValue();
    }
}

//*****************************************************************************

//*****************************************************************************
void TSafStr::ToLower( void )
{
if( itsStringP )
    {
    AnsiLower( itsStringP );
    itsCheckSum= hashValue();
    }
}

//*****************************************************************************
// handles NULL ptrs as well as ptrs to strings
//*****************************************************************************
int TSafStr::OurCmp( const char *a, const char *b) const
{
    if( a == NULL )
        {
        if( b == NULL )
            return 0;   //equal
        else
            return -1;  // a < b
        }
    else
        {
        if( b == NULL )
            return 1;       // a > b
        else
            return lstrcmpi( a, b );
        }
}

//*****************************************************************************

//*****************************************************************************
const char * TSafStr::Addr( void )
{
CHKBUG( ERR_GEN, HealthCheck() );
return (itsStringP)? itsStringP: itsNullString;
};

//*****************************************************************************

//*****************************************************************************
sizeType TSafStr::Len( void ) const
{
CHKBUG( ERR_GEN, HealthCheck() );
return ((itsLength == 0) ? 0: itsLength - 1);
};

//*****************************************************************************

//*****************************************************************************
const char* TSafStr::operator const char *() const
{
CHKBUG( ERR_GEN, HealthCheck() );
return (itsStringP)? itsStringP: itsNullString;
}
