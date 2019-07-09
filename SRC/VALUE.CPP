//*****************************************************************************
// currency object
// note separate representation of whole and fraction
// FIXME some of these fns could use a rethinking; also,
// move formatting fns into here from TIntlFmt
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "value.h"

#ifndef __classid__
#include "classid.h"
#endif
#ifndef __error__
#include "error.h"
#endif

//
// NB an object with no value should act like 0 in operations,
//    but keep "no value" flag set so one can distinguish it
//    from real 0.
//

#define FRACTION_FACTOR 10000L
#define FRACTION_FACTOR_REAL 10000.0
#define FRACTION_DIVIDER 100  // set to FRACTION_FACTOR / 100
#define ROUNDING         50   // set to FRACTION_FACTOR / 200


// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
TValue::TValue( void )
{
    itsHasValue  = FALSE;
    itsWhole     = 0;
    itsFraction  = 0;
}

// -------------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------------
TValue::~TValue( void )
{
}

// -------------------------------------------------------------------------
// Set
// -------------------------------------------------------------------------
void TValue::Set( long w, long f )
{
    itsHasValue  = TRUE;
    itsWhole     = w;
    itsFraction  = f;
}

// -------------------------------------------------------------------------
// Clear
// -------------------------------------------------------------------------
void TValue::Clear( void )
{
    itsHasValue = FALSE;
    itsWhole    = 0;
    itsFraction = 0;
}

// -------------------------------------------------------------------------
// HasValue
// -------------------------------------------------------------------------
BOOL TValue::HasValue( void )
{
    return itsHasValue;
}

// -------------------------------------------------------------------------
// IsZero
// -------------------------------------------------------------------------
BOOL TValue::IsZero( void )
{
    return ( !itsHasValue || ((itsWhole + itsFraction) == 0) );
}

// -------------------------------------------------------------------------
// operator +
// -------------------------------------------------------------------------
TValue TValue::operator + ( const RTValue rhs )
{
    TValue temp= *this;

    if( itsHasValue )
        {
        if( rhs.itsHasValue )
            {
            temp.itsFraction+= rhs.itsFraction;

            while ( temp.itsFraction >= FRACTION_FACTOR )
                {
                temp.itsFraction -= FRACTION_FACTOR;
                temp.itsWhole += 1;
                }

            temp.itsWhole += rhs.itsWhole;
            }
        }
    else
        {
        temp= rhs;
        }

    return temp;
}

// -------------------------------------------------------------------------
// operator *
//
//  multiply by a long
// -------------------------------------------------------------------------
TValue TValue::operator * ( const long rhs )
{
    TValue total;
    double temp;

    if (!itsHasValue) {

       return( total );
    }

    temp = (itsWhole + itsFraction / FRACTION_FACTOR_REAL) * rhs;
    total.itsWhole    = temp;
    total.itsFraction = (temp - total.itsWhole) * FRACTION_FACTOR;
    total.itsHasValue = TRUE;

    return( total );
}


// -------------------------------------------------------------------------
// operator +=
//
//  add another value to this
// -------------------------------------------------------------------------
RTValue TValue::operator += ( const RTValue rhs )
{

    if(itsHasValue)
        {
        if( rhs.itsHasValue )
            {
            itsFraction+= rhs.itsFraction;

            while ( itsFraction >= FRACTION_FACTOR )
                {
                itsFraction -= FRACTION_FACTOR;
                itsWhole += 1;
                }

            itsWhole += rhs.itsWhole;
            }
        }
    else
        {
        *this= rhs;     //just take whole thing
        }

    return *this;
}

// -------------------------------------------------------------------------
// operator +=
//
//  add a long to this
// -------------------------------------------------------------------------
RTValue TValue::operator += ( const long rhs )
{
    itsWhole += rhs;
    itsHasValue = TRUE;
    return( *this );
}

// -------------------------------------------------------------------------
// operator /=
//
//  divide this value by a long
// -------------------------------------------------------------------------
RTValue TValue::operator /= ( const long rhs )
{

    if (itsHasValue && (rhs != 0)) {

        double temp;
        temp     = (itsWhole + itsFraction / FRACTION_FACTOR_REAL) / rhs;
        itsWhole    = temp;
        itsFraction = (temp - itsWhole) * (FRACTION_FACTOR * 10) + 5;
        itsFraction /= 10;
    }

    return( *this );

}


// ----------------------------------------------------------------------------
// operator =
//
//  Create a value from a long
// ----------------------------------------------------------------------------
RTValue TValue::operator =( const long rhs )
{
    itsHasValue = TRUE;
    itsWhole    = rhs;
    itsFraction = 0;

    return( *this );
}

// ----------------------------------------------------------------------------
// operator =
//
//  Create a value from a double
// ----------------------------------------------------------------------------
RTValue TValue::operator =( const double rhs )
{
    itsHasValue  = TRUE;
    itsWhole     = rhs;
    itsFraction  = (rhs - (double) itsWhole) * 10000;

    return( *this );
}

// ----------------------------------------------------------------------------
// operator =
//
//  Create a value from a value
// ----------------------------------------------------------------------------
RTValue TValue::operator = ( const RTValue rhs )
{
    itsHasValue = rhs.itsHasValue;
    itsWhole    = rhs.itsWhole;
    itsFraction = rhs.itsFraction;
    return( *this );
}

// ----------------------------------------------------------------------------
// operator ==
// ----------------------------------------------------------------------------
BOOL TValue::operator ==( const RTValue rhs )
{
    if (itsHasValue != rhs.itsHasValue)    return FALSE;
    if (!itsHasValue && !rhs.itsHasValue)  return TRUE;
    if (itsWhole != rhs.itsWhole)          return FALSE;
    if (itsFraction != rhs.itsFraction)    return FALSE;

    return TRUE;
}

// ----------------------------------------------------------------------------
// operator <
// ----------------------------------------------------------------------------
BOOL TValue::isLessThan( const Object &rhsObj ) const
{
    RTValue rhs= (RTValue) rhsObj;

    if (!rhs.itsHasValue)                   return FALSE;
    if (!itsHasValue)                       return TRUE;
    if (itsWhole < rhs.itsWhole)            return TRUE;
    if (itsWhole > rhs.itsWhole)            return FALSE;

    return( itsFraction < rhs.itsFraction );
}

// ----------------------------------------------------------------------------
// ToDouble
//
// convert to a double
// ----------------------------------------------------------------------------
double TValue::ToDouble( void )
{
    return itsWhole + itsFraction / FRACTION_FACTOR_REAL;
}

//*****************************************************************************
//
//                       BASIC OBJECT FUNCTIONS
//
//*****************************************************************************
classType TValue::isA( void ) const
{
return IDValue;
}

char * TValue::nameOf( void ) const
{
return "TValue";
}

hashValueType TValue::hashValue( void ) const
{
return 0; //FIXME
}

int TValue::isEqual(const Object & obj) const
{
return( this == & obj );
}

void TValue::printOn(ostream far&) const
{
//later
}

//*****************************************************************************
//
//                      stream support
//
//*****************************************************************************
PTStreamable TValue::build( void )
{
return( new TValue( streamableInit ) );
}

TStreamableClass RegTValue( "TValue", TValue::build, __DELTA(TValue) );

Pvoid TValue::read( Ripstream is )
{
int version;

is >> version;

if( version == 1 )
	{
    is >> itsHasValue;
    is >> itsWhole;
    is >> itsFraction;
	}
else
	SYSERR( ERR_VER );

return this;
}

void TValue::write( Ropstream os )
{

os << (int) 1;  //version

os << itsHasValue;
os << itsWhole;
os << itsFraction;
}

