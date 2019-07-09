#ifndef __memchk__
#define __memchk__

_CLASSDEF( TMemChk )

#include "safepool.h"
#include "error.h"

class _CLASSTYPE TMemChk
{
public:
    TMemChk( void ) { };
    virtual ~TMemChk() { };

    void Checkpoint()
        {
        count= SafetyPool::GetCount();
        accum= SafetyPool::GetAccum();
        };

    void Assert()
        {
        char buf[200];
        int  c= (int)  (SafetyPool::GetCount() - count );
        long a= (long) (SafetyPool::GetAccum() - accum );
        if( c )
            {
            wsprintf( buf, "count: %d; accum: %ld", c, a );
            MessageBox( NULL, buf, "Memory Leak!", 0 );
            }
        }

private:
    int count;
    long accum;
};

#endif
