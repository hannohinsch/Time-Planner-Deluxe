#ifndef __cattable__
#define __cattable__

_CLASSDEF( TCatTable )
_CLASSDEF( TCatList )
_CLASSDEF( TCat )

#ifndef __keyedlst__
#include "keyedlst.h"
#endif

class _CLASSTYPE TCatTable: public TKeyedList {

    public:

    TCatTable( PTCatList catListP, Pvoid data[] );
    virtual ~TCatTable( void );


    protected:

    virtual void AddSubcats( PTCat catP,
        Pvoid catDataP, PTCatList );
};

#endif