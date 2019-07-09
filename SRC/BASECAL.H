#ifndef __basecal__
#define __basecal__

// calendar routines

long daynumber(int day, int month, int year );
void caldate( long date, int *day, int *month, int *year);
int weekday( long date );
long nthweek( int year, int wknum, int dow);
long ldom(int year, int month );

#endif