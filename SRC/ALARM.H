#ifndef __alarm__
#define __alarm__

_CLASSDEF( TAlarm )
_CLASSDEF( TAlarmDialog )

#ifndef __note__
#include "note.h"
#endif
#ifndef __freq__
#include "freq.h"
#endif

class _CLASSTYPE TAlarm: public Object, TStreamable {

    friend TAlarmDialog;

public:

    // constructors
    TAlarm( PTNote noteP, RTTime baseTime, PTFreq freqP );
    TAlarm( RTAlarm alarm );
    virtual ~TAlarm( void );

    void    SetBaseTime( RTTime time, PTFreq freqP );
    const RTTime  GetAlarmTime( void ) { return itsAlarmTime; };

    RTAlarm operator =( RTAlarm alarm );
    BOOL    Match( PTNote noteP );
    PTNote  AlarmRinging( RTTime time );
    PTNote  GetNote( void ) { return itsNoteP; };
    void    Snooze( void );
    void    CancelAlarm( void );
    BOOL    IsDone( void );

    // basic object methods
    classType isA( void ) const             { return IDAlarm; };
    char *nameOf( void ) const              { return "TAlarm"; };
    hashValueType hashValue( void ) const   { return 0; };
    int isEqual(const Object &obj) const    { return( this == & obj); };
    void printOn(ostream &) const           { }

    // streamable methods
    static PTStreamable build( void )       { return( new TAlarm( streamableInit )); };
    void write( Ropstream );
    Pvoid read( Ripstream );
    TAlarm( StreamableInit )             {};
    const Pchar streamableName() const { return "TAlarm"; };

    protected:

    void SetUnit( TTimeUnit unit );
    void SetOffset( int offset );
    void SetAlarmTime( void );

    PTNote     itsNoteP;
    TTime      itsBaseTime;
    TTime      itsAlarmTime;
    int        itsOffset;
    TTimeUnit  itsUnit;
    PTFreq     itsFreqP;
    BOOL       itsDone;
};

	inline Ripstream operator >> (Ripstream is, RTAlarm cl)
        { return is >> (RTStreamable) cl; }

	inline Ripstream operator >> (Ripstream is, RPTAlarm cl)
        { return is >> (RPvoid) cl; }

	inline Ropstream operator << (Ropstream os, RTAlarm cl)
        { return os << (RTStreamable) cl; }

	inline Ropstream operator << (Ropstream os, PTAlarm cl)
        { return os << (PTStreamable) cl; }


#endif
