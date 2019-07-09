//*****************************************************************************
//(hmh) changed fns() to use naked Windows memory calls, not suballocation calls
// (makes memory leak tracing easier)
// added exception handling
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "error.h"

#include <stddef.h>
#include <stdlib.h>
#include "safepool.h"

//
// switch between Borland's subsegment allocation and naked Windows calls
// The naked calls allow better checking of how much memory is leaking,
// and more accurate reports from Windows calls that check memory usage.
// (but they're too slow for production use)
//
#define SUBSEG_ALLOC TRUE


typedef void (_FAR *pvf)();

pvf _new_handler;

int	SafetyPool::Size = DEFAULT_SAFETY_POOL_SIZE;
void _FAR *  SafetyPool::safetyPool = new char[DEFAULT_SAFETY_POOL_SIZE];

int   SafetyPool::Count= 0;
long  SafetyPool::AccumMem= 0;

//*****************************************************************************
// unused!
//*****************************************************************************
pvf set_new_handler(pvf p)
{
    pvf t = _new_handler;
    _new_handler = p;
    return t;
}

//*****************************************************************************
// allocator
//*****************************************************************************
static void _FAR *originalNew(size_t size)
{
    void _FAR * p= NULL;

    size = size ? size : 1;

#if SUBSEG_ALLOC

    p = malloc(size);
    SafetyPool::Count++;                    //track # of allocations

#else

    HANDLE h = GlobalAlloc( GMEM_MOVEABLE, size);


    if( h )
        {
        p= GlobalLock( h );
        SafetyPool::AccumMem+= GlobalSize( h );
        SafetyPool::Count++;                    //track # of allocations
        }
#endif

    return p;
}

//*****************************************************************************

//*****************************************************************************
void _FAR *operator new( size_t size )
{
    void _FAR *allocated = originalNew (size);

    if( allocated == NULL && !SafetyPool::IsExhausted())
        {
        SYSERR( ERR_LOWMEM );
        delete SafetyPool::safetyPool;
        SafetyPool::safetyPool = NULL;
        allocated = originalNew (size);
        }

    if( allocated == NULL )
        {
        SYSTHROW( ERR_NEW );
        }

    return allocated;
}

//*****************************************************************************

//*****************************************************************************
void operator delete( void _FAR *ptr )
{

if (ptr)
    {

#if SUBSEG_ALLOC

    free(ptr);
    SafetyPool::Count--;

#else

    HANDLE h= NULL;

    if( ptr != NULL )
        h= LOWORD( GlobalHandle( SELECTOROF( ptr ) ) );

    if( h )
        {
        SafetyPool::AccumMem -= GlobalSize( h );
        SafetyPool::Count--;

        GlobalUnlock( h );
        GlobalFree( h );
        };

#endif

    // try to restore the safety pool if it is exhausted
    if (SafetyPool::IsExhausted())
        SafetyPool::Allocate();
    }
else
    {
    SYSERR( ERR_GEN );    //ptr was null!
    }
}

//*****************************************************************************
//   This static member function deletes the existing safetypool and
//   reallocates one with the specified size.  It returns TRUE if this
//   is successful, otherwise FALSE.  Note that if this function returns
//   FALSE, the next call to check on the safety pool will indicate the
//   low memory condition.
//*****************************************************************************
int SafetyPool::Allocate(size_t size)
{
    if( safetyPool )
        delete safetyPool;       // delete existing safetyPool
	
    safetyPool = new char[size]; // reallocate safetyPool
    Size       = size;

    return !IsExhausted(); // return success/failure
}
