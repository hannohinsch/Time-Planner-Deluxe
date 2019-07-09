//*****************************************************************************
// Calendar routines adapted from article in PC Tech Journal, Dec. 85
// (with some additions)
// modified to look like stuff in Computer Language, Dec 1990
// w/ corrections published 2(?) months later
//*****************************************************************************

#include <math.h>

#include "basecal.h"

//*****************************************************************************
// Return day number of first day of the Nth week that starts on Dth day
//*****************************************************************************
long nthweek ( int yr, int wkn, int dow )
{
long z;
int d;

z=daynumber(1,1,yr);
d=weekday(z);
/* calculate daynumber of first 'dow' in year */
if (dow >= d)
   z+= (dow-d);
else
   z+= 7 - (d-dow);
z+= 7 * (wkn-1);	/* daynumber for start of Nth week */

return(z);
}

//*****************************************************************************
// Daynumber of last day of month
//*****************************************************************************
long ldom( int yr, int mon)
{
long d;

if(mon == 12)
  d=daynumber( 31, 12, yr );
else
  d=daynumber( 1, (mon+1), yr ) - 1;

return(d);
}

//*****************************************************************************
// Return day of week given day number
// Sunday == 1
//*****************************************************************************
int weekday( long date)
{
if (date > 0)
    return (int) (1 + (date+1) % 7 );
else
    return (int) (7 + (date-5) % 7 );
}

//*****************************************************************************
// Julian day number (from  Jan 1, 4713 B.C. )
//*****************************************************************************
long daynumber( int day, int month, int year)
{
int a,b;
long daynum;

//correct for B.C.
if( year<0 )
    year++;

if (month < 3)
  {
  month+=12;
  year--;
  }

//this could be sped up!
daynum= (long)(365.25*year) + (long)(30.6001*(month+1)) + day - 723244L;

//Gregorian reforms
if( daynum >= -145068L )
  {
  a= year/100;
  b= ((2 - a) + a/4);
  daynum+= b;
  }

return( daynum );
} /* daynumber */

//*****************************************************************************
// given date, finds corresponding year, month, day (note pointer arguments)
//*****************************************************************************
void caldate( long date, int *day, int *month, int *year)
{
long a,b,c,d,e,z,alpha;

z= date + 2444239L;

//Gregorian reform
if (date < -145078L)
    a= z;
else
    {
    alpha= (long) (( z-1867216.25)/36524.25);
    a= z + 1 + alpha - (alpha/4);
    }

b = a + 1524;
c = (long) ((b-122.1)/365.25);
d = (long) (365.25*c);
e = (long) ((b-d)/30.6001);

*day = (int) (b - d - (long)(30.6001*e));

*month= (int) ((e <= 13 ) ? e - 1 : e - 13);

*year= (int) ((*month > 2 ) ? c - 4716 : c - 4715 );

if( *year < 1 )
    *year--;
}

