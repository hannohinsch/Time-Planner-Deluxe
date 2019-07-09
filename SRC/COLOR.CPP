//*****************************************************************************
// define virtual color object (used in note object;
// is really a "priority" value )
//*****************************************************************************

#include "standard.h"
#pragma hdrstop

#include "color.h"

#ifndef __classid__
#include "classid.h"
#endif
#ifndef __error__
#include "error.h"
#endif

//*****************************************************************************

//*****************************************************************************
TColor::TColor( void )
{
}

//*****************************************************************************

//*****************************************************************************
TColor::~TColor()
{
}

//*****************************************************************************

//*****************************************************************************
void TColor::Set( TVirtColor n )
{
virtColor= n;
}

//*****************************************************************************

//*****************************************************************************
TColor &TColor::operator =( const TColor &other )
{
virtColor= other.virtColor;
return *this;
}

//*****************************************************************************

//*****************************************************************************
TVirtColor TColor::Get( void ) const
{
return virtColor;
}


//*****************************************************************************
//
//*****************************************************************************
classType TColor::isA( void ) const
{
return IDColor;
}

char * TColor::nameOf( void ) const
{
return "TColor";
}

hashValueType TColor::hashValue( void ) const
{
return 0; //FIXME
}

int TColor::isEqual(const Object & obj) const
{
return( this == & obj );
}

void TColor::printOn(ostream far&) const
{
//later
}

//*****************************************************************************
// stream support
//*****************************************************************************
PTStreamable TColor::build( void )
{
return( new TColor( streamableInit ) );
}

TStreamableClass RegTColor( "TColor", TColor::build, __DELTA(TColor) );

//*****************************************************************************

//*****************************************************************************
Pvoid TColor::read( Ripstream is )
{
int version, temp;

is >> version;

if( version == 1 )
	{
	is >> temp;
	virtColor= (TVirtColor) temp;

    // make sure color is within range
    if ((virtColor < kFirstColor) || (kLastColor < virtColor))
        {
        virtColor = kGray;
        }

	}
else
	SYSERR( ERR_VER );

return this;
}

//*****************************************************************************

//*****************************************************************************
void TColor::write( Ropstream os )
{
os << (int) 1;      //version

os << (int) virtColor;
}

