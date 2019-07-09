#ifndef __xarray__
#define __xarray__

_CLASSDEF( TXArray )

class _CLASSTYPE TXArray: public TNSSortedCollection
{
    public:
        typedef enum
        {
            kSort,
            kNoSort,
            kDefSort
        } TSortStyle;

        typedef enum
        {
            kDelete,
            kNoDelete
        } TDeleteStyle;

        TXArray( int limit, int delta, TSortStyle sortStyle= kSort );
        virtual ~TXArray();

        int  GetCount( void ) const { return count; };
        BOOL HasMember( Object &, TSortStyle style= kDefSort );
        int  GetIndex( Object &, TSortStyle style= kDefSort );
        void Add( Object &, TSortStyle sortStyle= kDefSort );
        void Detach( Object &, TDeleteStyle dt= kNoDelete );
        void Flush( TDeleteStyle dt=kNoDelete );
        void Sort( void );

        //
        //lowercase, for compatibility w/ old classlib fns
        //
        int getItemsInContainer( void ) const { return count; };
        BOOL hasMember( Object &obj, TSortStyle style= kDefSort ) { return HasMember( obj, style ); };
        int  getIndex( Object &obj, TSortStyle style= kDefSort ){ return GetIndex( obj, style ); };
        void add( Object &obj, TSortStyle style= kDefSort ){ Add( obj, style ); };
        void detach( Object &obj, TDeleteStyle dt= kNoDelete ){ Detach( obj, dt ); };
        void flush( TDeleteStyle dt=kNoDelete ){ Flush( dt ); };
        void sort( void ) { Sort(); };

        Object & operator [] (int index);

    private:
        BOOL IsSorted( void );
        int compare( Pvoid key1, Pvoid key2 );  //overloaded from TNSSortCol

        //for qsort call...
        static TXArray *itsCompP;
        static int Comp( const void *, const void * );

        void freeItem( void *item );    //overloaded from TNSCollection

        TXArray();              //unused
        TXArray( TXArray &);    //unused
        operator = (TXArray);   //unused

        TSortStyle itsSortStyle;
};


#endif
