// also, changed malloc() to new() in freadString()
// removed DLL support to improve readability
// added exception handling (now throws errors instead of messageboxes/asserts)

#include "standard.h"
#pragma hdrstop

//for our exception handling....
#ifndef __error__
#include "error.h"
#endif

// Borland C++ - (C) Copyright 1992 by Borland International

/*------------------------------------------------------------*/
/* filename -       objstrm.cpp                              */
/*                                                            */
/* function(s)                                                */
/*                  Member function(s) of following classes:  */
/*                     TStreamable                            */
/*                     TStreamableClass                       */
/*                     TStreamableTypes                       */
/*                     TPWrittenObjects                       */
/*                     TPReadObjects                          */
/*                     pstream                                */
/*                     ipstream                               */
/*                     opstream                               */
/*                     iopstream                              */
/*                     fpbase                                 */
/*                     ifpstream                              */
/*                     ofpstream                              */
/*                     fpstream                               */
/*------------------------------------------------------------*/

#if !defined( __OBJSTRM_H )
#include "objstrm.h"            //use local copy (hmh)
#endif	// __OBJSTRM_H

#if !defined( __STRING_H )
#include <string.h>
#endif  // __STRING_H

#if !defined( __FSTREAM_H )
#include <fstream.h>
#endif  // __FSTREAM_H

#if !defined( __IO_H )
#include <io.h>
#endif  // __IO_H

#if !defined( __STAT_H )
#include <sys\Stat.h>
#endif  // __STAT_H

#if !defined( __FCNTL_H )
#include <fcntl.h>
#endif  // __FCNTL_H

#if !defined( __ASSERT_H )
#include <assert.h>
#endif  // __ASSERT_H

#if !defined(__ALLOC_H)
#include <alloc.h>
#endif // __ALLOC_H

#if !defined(__APPLICAT_H)
#include <applicat.h>
#endif

#if !defined(__WINDOBJ_H)
#include <windobj.h>
#endif


//////////////////////////////////////////////////////////////////////////
TStreamableClass::TStreamableClass( const char *n, BUILDER b, int d ) :
    name( n ),
    build( b ),
    delta( d )
{
    pstream::initTypes();

    pstream::types->registerType(this);
}



////////////// TStreamableTypes /////////////////////////////////////////////////////////////

//static!
TStreamableTypes *pstream::types;


Boolean TStreamableTypes::search( void *key, ccIndex& index )
{
    ccIndex l = 0;
    ccIndex h = count - 1;
    while( l <= h )
    {
        ccIndex i = (l +  h) >> 1;
        // i is our midpoint
        ccIndex c = compare( keyOf( items[i] ), key );
        if (c < 0)
            l = i + 1;
        else if (c > 0)
            h = i - 1;
        else
        {
            index = i;
            return True;
        }
    }
    index = l;
    return False;
}

const TStreamableClass *TStreamableTypes::lookup( const char *name )
{
    ccIndex loc;

    if (!search((void *)name, loc))
        return NULL;

    return (PTStreamableClass)at(loc);
}

Pvoid TStreamableTypes::keyOf(Pvoid d)
{
    return (void *)((PTStreamableClass)d)->name;
}

int TStreamableTypes::compare( void *d1, void *d2 )
{
    return strcmp( (char *)d1, (char *)d2 );
}

//////////////////////////////////////////////////////////////////////////////
TPWrittenObjects::TPWrittenObjects() :
    TNSSortedCollection( 1024, 1024 ), curId( 1 )
{
}

TPWrittenObjects::~TPWrittenObjects()
{
shouldDelete = True; //was False (hmh)

#if FALSE        //diagnostic
char report[100];
wsprintf( report, "%d objects written.", getCount() );
MessageBox( NULL, report, "Diagnostic Report", 0 );
#endif
}

void TPWrittenObjects::registerObject( const void *adr )
{
    TPWObj *o = new TPWObj( adr, curId++ );
    insert( o );
}

P_id_type TPWrittenObjects::find( const void *d )
{
    ccIndex loc;
    if( search( Pvoid( d ), loc ) )
        return ((TPWObj *)at( loc ))->ident;
    else
        return 0;
}

int TPWrittenObjects::compare( void *o1, void *o2 )
{
    if( o1 == o2 )
        return 0;
    else if( (void huge *)o1 < (void huge *)o2 )
        return -1;
    else
        return 1;
}

////////////////////////////////////////////////////////////////////////////
TPReadObjects::TPReadObjects()
    :TNSCollection( 1024, 1024 ), curId( 1 )
{
insert( 0 );        // prime it, so we won't use index 0
}

TPReadObjects::~TPReadObjects()
{
shouldDelete = False;
}

void TPReadObjects::registerObject( const void *adr )
{
    insert( Pvoid( adr ));
}

////////////////////////////////////////////////////////////////////////////
void _Cdecl pstream::error( StreamableError errorcondition )
{
  setstate( errorcondition );

  if(errorcondition == peNotRegistered)
    SYSTHROW( ERR_OBJSTRM );
  else
    SYSTHROW( ERR_OBJSTRM );

  clear(ios::badbit);
}

void _Cdecl pstream::error( StreamableError errorcondition, RCTStreamable )
{
  error( errorcondition );
}

void pstream::initTypes()
{
    if( types == NULL )
    {
        types = new TStreamableTypes;
    }
}

/////////////////////////////////////////////////////////////////////////////
ushort ipstream::readWord()
{
    ushort temp;
    bp->sgetn( (char *)&temp, sizeof( ushort ) );
    return temp;
}

void ipstream::freadBytes( void far *data, size_t sz )
{
    if (sz > 0)
    {
        char *buf = new char[sz+1];

        //FIXME why does this need a temporary buffer???? (hmh)

        bp->sgetn( (char *)buf, sz );
        _fstrncpy((char far *)data, buf, sz);

        delete buf;
    }
}

char *ipstream::readString()
{
    uchar len = readByte();

    char *buf = new char[len+1];

    if( buf != 0 )
    {
        readBytes( buf, len );
        buf[len] = EOS;
    }
    return buf;
}

char *ipstream::readString( char *buf, unsigned maxLen )
{
    if( buf == 0 )
        SYSTHROW( ERR_OBJSTRM );

    uchar len = readByte();

    if( len > maxLen-1 )
        len = maxLen;           // but see buf[len] = EOS;

    readBytes( buf, len );
    buf[len] = EOS;
    return buf;
}

char far *ipstream::freadString( char far *buf, unsigned maxLen )
{
    uchar len = readByte();

    if( len > maxLen-1 )
        len = maxLen;           // but see buf[len] = EOS;

    freadBytes( buf, len);
    buf[len] = EOS;
    return buf;
}

//added (hmh) for strings > 256 bytes
char far *ipstream::freadBigString()
{
    int len = readWord();

    char far *buf = (char far *) new char[ len + 1 ];

    freadBytes(buf, len);
    buf[len] = EOS;
    return buf;
}

char far *ipstream::freadString()
{
    uchar len = readByte();

//    char far *buf = (char far *)farmalloc(len+1);     //old (hmh)
    char far *buf = (char far *) new char[ len + 1 ];

    freadBytes(buf, len);
    buf[len] = EOS;
    return buf;
}

ipstream& operator >> ( ipstream& ps, signed char &ch )
{
    ch = ps.readByte();
    return ps;
}

ipstream& operator >> ( ipstream& ps, unsigned char &ch )
{
    ch = ps.readByte();
    return ps;
}

ipstream& operator >> ( ipstream& ps, signed short &sh )
{
    sh = ps.readWord();
    return ps;
}

ipstream& operator >> ( ipstream& ps, unsigned short &sh )
{
    sh = ps.readWord();
    return ps;
}

ipstream& operator >> ( ipstream& ps, signed int &i )
{
    i = ps.readWord();
    return ps;
}

ipstream& operator >> ( ipstream& ps, unsigned int &i )
{
    i = ps.readWord();
    return ps;
}

ipstream& operator >> ( ipstream& ps, signed long &l )
{
    ps.readBytes( &l, sizeof(l) );
    return ps;
}

ipstream& operator >> ( ipstream& ps, unsigned long &l )
{
    ps.readBytes( &l, sizeof(l) );
    return ps;
}

ipstream& operator >> ( ipstream& ps, float &l )
{
    ps.readBytes( &l, sizeof(l) );
    return ps;
}

ipstream& operator >> ( ipstream& ps, double &l )
{
    ps.readBytes( &l, sizeof(l) );
    return ps;
}

ipstream& operator >> ( ipstream& ps, long double &l )
{
    ps.readBytes( &l, sizeof(l) );
    return ps;
}

ipstream& operator >> ( ipstream& ps, TStreamable& t )
{
    const TStreamableClass *pc;

    if((pc = ps.readPrefix()) == NULL)
      ps.error(pstream::peInvalidType );
    ps.readData( pc, &t );
    ps.readSuffix();
    return ps;
}

ipstream& operator >> ( ipstream& ps, void *&t )
{
    char ch = ps.readByte();
    switch( ch )
        {
        case pstream::ptNull:
            t = 0;
            break;
        case pstream::ptIndexed:
            {
            P_id_type index = ps.readWord();
            t = Pvoid( ps.find( index ) );

            if( t == 0 )
                SYSTHROW( ERR_OBJSTRM );
            break;
            }
        case pstream::ptObject:
            {
            const TStreamableClass *pc;
	
	    if((pc = ps.readPrefix()) == NULL)
              ps.error( pstream::peInvalidType );

            t = ps.readData( pc, 0 );
            ps.readSuffix();
            break;
            }
        default:
            ps.error( pstream::peInvalidType );
            break;
        }
    return ps;
}

const TStreamableClass *ipstream::readPrefix()
{
    char ch = readByte();

    if( ch != '[' )
       SYSTHROW( ERR_OBJSTRM );

    char name[125];
    readString( name, sizeof name );
    return types->lookup( name );
}

void *ipstream::readData( const TStreamableClass *c, TStreamable *mem )
{
    if( mem == 0 )
        mem = c->build();

    registerObject( (char *)mem - c->delta );   // register the actual address
                                        // of the object, not the address
                                        // of the TStreamable sub-object
    return mem->read( *this );
}

void ipstream::readSuffix()
{
    char ch = readByte();

    if( ch != ']' )
        SYSTHROW( ERR_OBJSTRM );
}

/////////////////////////////////////////////////////////////////////////////

opstream& opstream::flush()
{
    bp->sync();
    return *this;
}

void opstream::writeByte( uchar ch)
{
if( EOF == bp->sputc( ch ) )
    {
    SYSTHROW( ERR_OBJSTRM );
    }
}

void opstream::writeBytes( PCvoid data, size_t sz)
{
if( EOF == bp->sputn( (char *)data, sz ) )
    {
    SYSTHROW( ERR_OBJSTRM );
    }
}

void opstream:: writeWord( ushort sh)
{
if( EOF == bp->sputn( (char *)&sh, sizeof( ushort ) ) )
    {
    SYSTHROW( ERR_OBJSTRM );
    }
}

void opstream::fwriteBytes( const void far *data, size_t sz )
{
    if (sz > 0)
    {
        char *buf = new char[sz+1];

        _fstrcpy(buf, (char far *)data);
        int rc= bp->sputn( (char *)buf, sz );

        delete buf;

        if( rc == EOF )
            {
            SYSTHROW( ERR_OBJSTRM );
            }
    }
}

void opstream::writeString( const char *str )
{
    int len = strlen( str );
    writeByte( (uchar)len );
    writeBytes( str, len );
}

//added to handle strings > 256 chars (hmh)
void opstream::fwriteBigString( const char far * str )
{
    WORD len = _fstrlen( str );
    writeWord( len );
    fwriteBytes(str, len);
}

void opstream::fwriteString( const char far * str )
{
    int len = _fstrlen( str );
    writeByte( (uchar)len );
    fwriteBytes(str, len);
}

opstream& operator << ( opstream& ps, signed char ch )
{
    ps.writeByte( ch );
    return ps;
}

opstream& operator << ( opstream& ps, unsigned char ch )
{
    ps.writeByte( ch );
    return ps;
}

opstream& operator << ( opstream& ps, signed short sh )
{
    ps.writeWord( sh );
    return ps;
}

opstream& operator << ( opstream& ps, unsigned short sh )
{
    ps.writeWord( sh );
    return ps;
}

opstream& operator << ( opstream& ps, signed int i )
{
    ps.writeWord( i );
    return ps;
}

opstream& operator << ( opstream& ps, unsigned int i )
{
    ps.writeWord( i );
    return ps;
}

opstream& operator << ( opstream& ps, signed long l )
{
    ps.writeBytes( &l, sizeof(l) );
    return ps;
}

opstream& operator << ( opstream& ps, unsigned long l )
{
    ps.writeBytes( &l, sizeof(l) );
    return ps;
}

opstream& operator << ( opstream& ps, float l )
{
    ps.writeBytes( &l, sizeof(l) );
    return ps;
}

opstream& operator << ( opstream& ps, double l )
{
    ps.writeBytes( &l, sizeof(l) );
    return ps;
}

opstream& operator << ( opstream& ps, long double l )
{
    ps.writeBytes( &l, sizeof(l) );
    return ps;
}

opstream& operator << ( opstream& ps, TStreamable& t )
{
    ps.writePrefix( t );
    ps.writeData( t );
    ps.writeSuffix( t );
    return ps;
}

opstream& operator << ( opstream& ps, TStreamable *t )
{
    P_id_type index;
    if( t == 0 )
        ps.writeByte( pstream::ptNull );
    else if( (index = ps.find( t )) != 0 )
        {
        ps.writeByte( pstream::ptIndexed );
        ps.writeWord( index );
        }
    else
        {
        ps.writeByte( pstream::ptObject );
        ps << *t;
        }
    return ps;
}

void opstream::writePrefix( const TStreamable& t )
{
    writeByte( '[' );
    writeString( t.streamableName() );
}

void opstream::writeData( TStreamable& t )
{
    if ( types->lookup( t.streamableName() ) == NULL )
        error( peNotRegistered, t );
    else
    {
        registerObject( &t );
        t.write( *this );
    }
}

////////////////////////////////////////////////////////////////////////////
fpbase::fpbase( const char *name, int omode, int prot )
{
    pstream::init( &buf );
    open( name, omode, prot );
}

void fpbase::open( const char *b, int m, int prot )
{
    if( buf.is_open() )
        clear(ios::failbit);        // fail - already open
    else if( buf.open(b, m, prot) )
    {
      clear(ios::goodbit);
    }
    else
        clear(ios::badbit);        // open failed
}

void fpbase::attach( int f )
{
    if( buf.is_open() )
        setstate(ios::failbit);
    else if( buf.attach(f) )
        clear(ios::goodbit);
    else
        clear(ios::badbit);
}

void fpbase::close()
{
    if( buf.close() )
        clear(ios::goodbit);
    else
        setstate(ios::failbit);
}

void fpbase::setbuf(char* b, int len)
{
    if( buf.setbuf(b, len) )
        clear(ios::goodbit);
    else
        setstate(ios::failbit);
}

////////////////////////////////////////////////////////////////////////////
ifpstream::ifpstream( const char* name, int omode, int prot ) :
        fpbase( name, omode | ios::in | ios::binary, prot )
{
}

void ifpstream::open( const char _FAR *name, int omode, int prot )
{
    fpbase::open( name, omode | ios::in | ios::binary, prot );
}

////////////////////////////////////////////////////////////////////////////
ofpstream::ofpstream( const char* name, int omode, int prot ) :
        fpbase( name, omode | ios::out | ios::binary, prot )
{
}

void ofpstream::open( const char _FAR *name, int omode, int prot )
{
    fpbase::open( name, omode | ios::out | ios::binary, prot );
}

