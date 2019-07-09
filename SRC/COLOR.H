// define a "color" for a note, which might actually be a pattern
// or something else (depends on characteristics of display)

#ifndef __color__
#define __color__

_CLASSDEF( TColor )

typedef enum
    {

    kRed,                                   //highest priority
    kRedGray,
    kGray,
    kBlueGray,
    kBlue,                                  //lowest priority

    kFirstColor= kRed,
    kLastColor= kBlue

    } TVirtColor;

class _CLASSTYPE TColor: Object, TStreamable
    {
    public:
        TColor( void );
		TColor( StreamableInit ) {};
        virtual ~TColor();

        classType isA( void ) const;
        char *nameOf( void ) const;
        hashValueType hashValue( void ) const;
        int isEqual(const Object &) const;
        void printOn(ostream &) const;
        static PTStreamable build();

        void Set( TVirtColor n );
        TVirtColor Get( void ) const;

        void write( Ropstream );
        Pvoid read( Ripstream );

        TColor &operator =( const TColor &other );

    protected:
        const Pchar streamableName() const { return "TColor"; };

    private:

        TVirtColor virtColor;
    };

    inline Ripstream operator >> (Ripstream is, RTColor cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTColor cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTColor cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTColor cl)
        { return os << (PTStreamable) cl; }

#endif
