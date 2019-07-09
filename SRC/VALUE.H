#ifndef __value__
#define __value__

_CLASSDEF( TValue )
_CLASSDEF( TIntlFmt )

#include "error.h"

// define a currency class nnn.n


class _CLASSTYPE TValue: Sortable, TStreamable
    {

    public:

        TValue( void );
        TValue( StreamableInit ) {};
        static PTStreamable build();
        virtual ~TValue();

        void Set( long w, long f );
        void Get( long &w, long &f ) { w= itsWhole; f=itsFraction; };

        void Clear( void );

        // queries
        BOOL HasValue( void );
        BOOL IsZero( void );

        // mathematical operators
        TValue operator   + ( const RTValue rhs );
        TValue operator   * ( const long rhs );

        // in place operators
        RTValue operator += ( const RTValue rhs);
        RTValue operator += ( const long rhs);
        RTValue operator /= ( const long rhs );

        // assignments
        RTValue operator  = ( const long rhs );
        RTValue operator  = ( const double rhs );
        RTValue operator  = ( const RTValue rhs );

        // mathematical comparisons
        BOOL   operator  == ( const RTValue rhs );
        BOOL   isLessThan( const Object & ) const;

        double ToDouble( void );

        void write( Ropstream );
        Pvoid read( Ripstream );

    protected:


        // object doodads
        classType isA( void ) const;
        char *nameOf( void ) const;
        hashValueType hashValue( void ) const;
        int isEqual(const Object &) const;
        void printOn(ostream &) const;


    private:

        const Pchar streamableName() const { return "TValue"; };

        BOOL itsHasValue;
        long itsWhole;
        long itsFraction;
};

    inline Ripstream operator >> (Ripstream is, RTValue cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTValue cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTValue cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTValue cl)
        { return os << (PTStreamable) cl; }


#endif
