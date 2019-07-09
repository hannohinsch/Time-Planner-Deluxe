#ifndef __keyedlst__
#define __keyedlst__

_CLASSDEF( TKeyedList )

#define KEYNOTFOUND -1

typedef int (*CompareFunc)( const void*, const void* );

class _CLASSTYPE TKeyedList
{
    public:

    TKeyedList( BOOL ownsData, CompareFunc func = Compare );
    ~TKeyedList( void );

    virtual void  Add( long key, Pvoid data );
    virtual void  Detach( long key );
    virtual void  Sort( void );
    virtual Pvoid LinearSearch( long key );
    virtual Pvoid BinarySearch( long key );
    virtual void  SetData( long key, Pvoid dataP );

    static void Test( void );

    protected:

    static int Compare( const void*, const void* );

    private:

    typedef struct {

        long       Key;
        Pvoid      DataP;
        } TKeyedPair, *PTKeyedPair;

    BOOL              itsOwnsData;
    BOOL              itsSorted;
    unsigned int      itsNumEntries;
    unsigned int      itsMaxEntries;
    PTKeyedPair       itsDataP;
    CompareFunc       itsCompareFunc;

    int LinearFind( long key );
    int BinaryFind( long key );
    BOOL HealthCheck( void );
};


#endif