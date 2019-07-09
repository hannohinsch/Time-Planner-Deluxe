#ifndef __utility__
#define __utility__

extern const double kTolerance;

BOOL IsBlank( char *s );
void FormatDouble( double value, char *buf );
void SplitDouble( double d, long &value, double &remainder );
int  DoubleToInt( double d );
long DoubleToLong( double d );
BOOL IsWin31( void );
#endif
