#include "standard.h"
#pragma hdrstop

void Communicate( void );

#define WRITECOMM( STRING ) WriteComm( port, STRING, strlen( STRING ))
void Communicate( void )
{
    DCB   dcb;
    int   port, err, count;


    port   = OpenComm( "COM1", 2000, 2000 );
    err    = GetCommState( port, &dcb );
    dcb.BaudRate = 2400;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    err    = SetCommState( &dcb );
    err    = GetCommState( port, &dcb );
    count  = WRITECOMM( "ATQ0V1E1S0=0\r" );
    do
        {
            char    buf[2000];
            COMSTAT stat;

            count = ReadComm( port, buf, 2000 );
            err   = GetCommError( port, &stat );
        } while ((count == 0) && (err == 0));

    long ticks = GetTickCount();
    while (ticks + 3 * 60 > GetTickCount());

    count  = WRITECOMM( "ATDT 3661404\r" );
    err    = CloseComm( port );
}