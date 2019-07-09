#ifndef __note__
#define __note__

_CLASSDEF(TCatList)
_CLASSDEF(TCat)
_CLASSDEF(TNote)
_CLASSDEF(TFreq)

#ifndef __catlist__
#include "catlist.h"
#endif
#ifndef __error__
#include "error.h"
#endif
#ifndef __value__
#include "value.h"
#endif
#ifndef __color__
#include "color.h"
#endif
#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __ink__
#include "ink.h"
#endif
#ifndef __duration__
#include "duration.h"
#endif
#ifndef __timecomp__
#include "timecomp.h"
#endif
#ifndef __profile__
#include "profile.h"
#endif


//NB Order of definition matters! the notestyle affect the sort order
typedef enum {
                        // start     duration
    kItemStyle,         // no        no
    kFloatStyle,        // no        yes
    kMilestoneStyle,    // yes       no
    kTaskStyle          // yes       yes

    } TNoteStyle;


class _CLASSTYPE TNote: public Sortable, public TStreamable
{

public:

    TNote( void );
	TNote( StreamableInit );
    TNote( RTNote oldNote );
    virtual ~TNote( void );

	BOOL HealthCheck( void );

	classType isA( void ) const;
    char *nameOf( void ) const;
    hashValueType hashValue( void ) const;
    void printOn(ostream &) const;
    static PTStreamable build();
    int isEqual( const Object & ) const;//test for identity

    BOOL ExactlyMatches( RTNote note );    //compare fields individually

    // these two for sort on start date, etc.
    int Compare( RTNote other, TNoteOrder criterion, BOOL useTime = TRUE ) const ;
    int isLessThan( const Object & ) const;

    void Title( RTSafStr title );  // replaces title data item


    // data operators
    TTime      GetStart(    void ) const;
    TTime      GetEnd(      void ) const;
    TNoteStyle GetStyle(    void ) const;
    TValue     GetRate(     void ) const;
    TValue     GetValue(    void ) const;
    TDuration  GetDuration( void ) const;
    TWorkTime  GetWorkTime( void ) const;
    TTimeUnit  GetTimeUnit( void ) const;
    PTFreq     GetFreq(     void ) const;       // NOTE: returns pointer which may be NULL
    BOOL       IsWorkTime(  void ) const;
    RTCatList  GetCatList(  void ) const { return *itsCatListP; };
    BOOL       GetInkMode(  void ) const { return itsInkMode; };
    BOOL       IsANewNote(  void ) const { return itsANewNote; };
    BOOL       IsDone(      void ) const { return itsDone; };
    RTInk      GetInk(      void )       { return itsInk; };
    RTColor    GetColor(    void )       { return itsColor; };
    RTSafStr   GetText(     void )       { return itsText; };
    void       GetTimes( RTTimeComplex times ) { times = itsTimes; };
    void       SetCatList(  PTCatList );
    void       SetRate(     RTValue    rate );
    void       SetValue(    RTValue    value );
    void       SetTimeUnit( TTimeUnit  unit );
    void       SetFreq(     RTFreq freq );
    void       SetWorkTime(  BOOL workTime );
    void       SetDone( BOOL done )      { itsDone = done; };
    void       SetColor( RTColor color ) { itsColor = color; };
    void       MakeNoteOld( void )       { itsANewNote = FALSE; };
    void       SetTimes( RTTimeComplex times );

    void       SetInk( RTInk ink )       { itsInk = ink; }
    void       SetText( RTSafStr text );
    void       SetInkMode( BOOL inkMode );

    // setting time
    void       SetStartAndKeepDuration( RTTime start );
    void       SetDurationAndKeepStart( RTDuration duration );
    void       SetStartAndClearDuration( RTTime start );
    void       KeepStartAndClearDuration( void );
    void       SetStartAndEnd( RTTime start, RTTime end, TTimeUnit unit );
    void       SetFromRawData( RTTime start, RTDuration duration,
                   TTimeUnit unit, BOOL isWorkTime );

    BOOL       CheckRateFlag( void );
    void       ClearRateFlag( void );

    BOOL       HollowOnDate( RTTime date );

    void        UpdateUnitLength( void );
    BOOL        ModifyForChangedWorkTime( void );
    int         GetNoteID( void ) { return itsNoteID; }

protected:

    void write( Ropstream );
    Pvoid read( Ripstream );

    void RecalcDuration( RTTime start );
    void SetEnd( void );
    void SetAmount( void );
    void UpdateFieldsAfterTimeChange( void );


    TValue       itsValue;
    TValue       itsRate;
    TTimeComplex itsTimes;

    // (never set directly, here only for caching purposes)
    BOOL       itsRateFlag;
    PTFreq     itsFreqP;      // PASS AUF: could be NULL

    //data items
    PTCatList  itsCatListP;
    TColor     itsColor;
    TSafStr    itsText;
    BOOL       itsInkMode;
    TInk       itsInk;
    int        itsNoteID;       //unique id assigned at creation time
    BOOL       itsDone;
    BOOL       itsANewNote;     //true at creation; turned off in noteview

private:
    TNote &operator =( const TNote &other ) { ASSERT( ERR_ASSIGN, FALSE ); return *this; };
    const Pchar streamableName() const { return "TNote"; };

};

    inline Ripstream operator >> (Ripstream is, RTNote cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTNote cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTNote cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTNote cl)
        { return os << (PTStreamable) cl; }

#endif
