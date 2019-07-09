#ifndef __duration__
#define __duration__

_CLASSDEF( TDuration )
_CLASSDEF( TWorkTime )

#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __classid.h__
#include "classid.h"
#endif

class _CLASSTYPE TDuration: Object, TStreamable {

    friend TTime;
    friend TWorkTime;

public:

    TDuration( void );
    TDuration( RTDuration old );
    TDuration( TTimeUnit unit, long numUnits );
    TDuration( RTTime start, RTTime end );
    TDuration( StreamableInit ) {};
    virtual ~TDuration( void );

    // set
    void Clear( void );
    void Set( TTimeUnit unit, long numUnits );
    void Set( RTTime start, RTTime end );
    void Normalize( TTimeUnit unit );
    void PlusEquals( TTimeUnit unit, long numUnits );
    void PlusEquals( RTDuration duration );

    // query
    BOOL      IsZero( void );
    long      GetNumUnits( TTimeUnit unit );

    // format
    void Format( TTimeUnit  unit,  BOOL mixed, char *buf );
    void Format( TTimeScale scale, BOOL mixed, char *buf );

    // operators
    RTDuration operator =(  RTDuration rhs );
    TDuration  operator -(  void );                 // unary minus
    BOOL       operator ==( RTDuration rhs ) const;
    BOOL       operator < ( RTDuration rhs ) const;

    BOOL       HealthCheck( void );

    // basic object functions
    virtual classType     isA( void )                const { return IDDuration; };
    virtual char         *nameOf( void )             const { return "TDuration"; };
    virtual hashValueType hashValue( void )          const { return 0; };
    virtual int           isEqual(const Object &obj) const { return( this == & obj); };
    virtual void          printOn(ostream &)         const { }
    virtual const         Pchar streamableName()     const { return "TDuration"; };
    virtual void          write( Ropstream );
    virtual Pvoid         read( Ripstream );
    static  PTStreamable  build( void )                    { return( new TDuration( streamableInit )); };

protected:

    long        itsMinutes;
};

	inline Ripstream operator >> (Ripstream is, RTDuration cl)
        { return is >> (RTStreamable) cl; }

	inline Ripstream operator >> (Ripstream is, RPTDuration cl)
        { return is >> (RPvoid) cl; }

	inline Ropstream operator << (Ropstream os, RTDuration cl)
        { return os << (RTStreamable) cl; }

	inline Ropstream operator << (Ropstream os, PTDuration cl)
        { return os << (PTStreamable) cl; }


#endif
