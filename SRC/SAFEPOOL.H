// (hmh) added allocation count
//
// ObjectWindows - (C) Copyright 1992 by Borland International

#if !defined(__SAFEPOOL_H)
#define __SAFEPOOL_H

#if !defined(__OWLDEFS_H)
#include <owldefs.h>
#endif

#pragma option -Vo-
#if     defined(__BCOPT__) && !defined(_ALLOW_po)
#pragma option -po-
#endif

const int DEFAULT_SAFETY_POOL_SIZE = 32000;         //was 8192 (hmh)

_CLASSDEF(SafetyPool)

class _EXPORT SafetyPool
{
public:
    static  int		    Allocate(size_t size
				    = DEFAULT_SAFETY_POOL_SIZE);
    static  int		    IsExhausted()
			      // returns true if safetyPool is exhausted
			      { return safetyPool == NULL; };
    static  int         GetCount( void ) { return Count; };     //(hmh)
    static  long        GetAccum( void ) { return AccumMem; };  //hmh
    static  int         Size;
    static  Pvoid 	    safetyPool;

    static  int         Count;                  //(hmh)
    static  long        AccumMem;               //(hmh)
};

#pragma option -Vo.
#if     defined(__BCOPT__) && !defined(_ALLOW_po)
#pragma option -po.
#endif

#endif // __SAFEPOOL_H
