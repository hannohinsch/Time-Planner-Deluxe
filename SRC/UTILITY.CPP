//*****************************************************************************

//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "utility.h"

const double kTolerance = 0.000001;

// ------------------------------------------------------------------------
// IsBlank
//
// is the string empty or filled only with whitespace?
// ------------------------------------------------------------------------
BOOL IsBlank( char *s )
{
    int length = strlen( s );
    for (int i = 0; i < length; i++) {

        if (!isspace( s[i] )) {

            return( FALSE );
        }
    }

    return( TRUE );
}


// ------------------------------------------------------------------------
// FormatDouble
//
// round the double to 2 decimal places and then dump it into the buffer
// ------------------------------------------------------------------------
void FormatDouble( double value, char *buf )
{
    //round and format
    long valueTimes1000 = value * 1000;
    long valueTimes100  = (valueTimes1000 + 5) / 10;
    wsprintf( buf, "%.3ld", valueTimes100 );

    // .00
    int length = strlen( buf );
    if (buf[length - 1] == '0') {

        // .00: drop last two digits
        if (buf[length - 2] == '0') {

            buf[length - 2] = '\0';
        }

        // .x0: drop last digit
        else {

            buf[length - 1] = buf[length - 2];
            buf[length - 2] = '.';
        }
    }

    // .xx: put in decimals
    else {

        buf[length + 1] = '\0';
        buf[length    ] = buf[length - 1];
        buf[length - 1] = buf[length - 2];
        buf[length - 2] = '.';
    }
}


// ------------------------------------------------------------------------
// SplitDouble
//
// split a double into a whole number and the remaining decimal.
// (ie. 1.23 would return 1 and .23.)
// ------------------------------------------------------------------------
void SplitDouble( double d, long &value, double &remainder )
{
    if (d > 0) {

        value     = d + kTolerance;
        remainder = (d - value);
        if (remainder < kTolerance) {

            remainder = 0;
        }
    }
    else {

        value     = d - kTolerance;
        remainder = (d - value);
        if (remainder > -kTolerance) {

            remainder = 0;
        }
    }
}

// ------------------------------------------------------------------------
// DoubleToInt
//
// Convert a double to an integer
// ------------------------------------------------------------------------
int DoubleToInt( double d )
{
    if (d >= 0) {

        return d + kTolerance;
    }
    else {

        return d - kTolerance;
    }
}

// ------------------------------------------------------------------------
// DoubleToLong
//
// Convert a double to a long
// ------------------------------------------------------------------------
long DoubleToLong( double d )
{
    if (d >= 0) {

        return d + kTolerance;
    }
    else {

        return d - kTolerance;
    }
}

//*****************************************************************************

//*****************************************************************************
BOOL IsWin31( void )
{
DWORD  ver= GetVersion();
BYTE major= LOBYTE( LOWORD( ver ));
BYTE minor= HIBYTE( LOWORD( ver ));

return !( major < 3 || (major == 3 && minor < 10 ) );
}

