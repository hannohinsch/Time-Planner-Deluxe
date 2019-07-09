#ifndef __ink__
#define __ink__

_CLASSDEF( TInk )

class _CLASSTYPE TInk: Object, TStreamable
    {

    public:
        TInk( void );
		TInk( RTInk );
		virtual ~TInk( void );
		TInk( StreamableInit ) {};

        classType isA( void ) const;
        char *nameOf( void ) const;
        hashValueType hashValue( void ) const;
        int isEqual(const Object &) const;
        void printOn(ostream &) const;
        static PTStreamable build();

        BOOL HasInk( void );
        void Compress( void );
        void Decompress( void );
        void Destroy( void );
        void Show( HDC hdc, RECT *pRect );
        void Measure( RECT *pRect );
        void ShowFirstLine( HDC hdc, RECT *pRect );
        void Recognize( void );
        HANDLE GetInkHandle( void );
        void SetInkHandle( HANDLE hCtlInk );

        // creates new ink handle: doesn't just copy old one
        RTInk operator =( RTInk other );

        void write( Ropstream );
        Pvoid read( Ripstream );

    private:
        virtual const Pchar streamableName() const { return "TInk"; };
        HANDLE hInk;
        BOOL bCompressed;
    };

    inline Ripstream operator >> (Ripstream is, RTInk cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTInk cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTInk cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTInk cl)
        { return os << (PTStreamable) cl; }

#endif
