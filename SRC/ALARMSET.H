#ifndef __alarmset__
#define __alarmset__

_CLASSDEF(TAlarmSet)

#ifndef __alarm__
#include "alarm.h"
#endif
#ifndef __xarray__
#include "xarray.h"
#endif


class _CLASSTYPE TAlarmSet: TStreamable {

    public:

        TAlarmSet( void );
        virtual ~TAlarmSet( void );

        void detach( RTAlarm Alarm );
        void add( RTAlarm Alarm );
        int  getItemsInContainer( void ) { return pAlarmArray->getItemsInContainer(); };
        void flush( TXArray::TDeleteStyle dt= TXArray::kNoDelete );

        RTAlarm operator []( int index );
        BOOL AlarmInList( PTAlarm alarmP );

        PTAlarm AlarmForNote( RTNote note );
        void    CheckAlarms( void );

        // streamable functions
        static PTStreamable build( void )          { return( new TAlarmSet( streamableInit )); };
        const Pchar streamableName() const { return "TAlarmSet"; };
        TAlarmSet( StreamableInit )                {};
        void write( Ropstream );
        Pvoid read( Ripstream );

    private:
        TAlarmSet &operator =( const TAlarmSet &other );

        PTXArray pAlarmArray;
    };

    inline Ripstream operator >> (Ripstream is, RTAlarmSet cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTAlarmSet cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTAlarmSet cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTAlarmSet cl)
        { return os << (PTStreamable) cl; }


#endif

