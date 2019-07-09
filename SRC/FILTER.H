#ifndef __filter__
#define __filter__

_CLASSDEF( TFilter )
_CLASSDEF( TFilterDlg )
_CLASSDEF( TCatTable )

#ifndef __noteview__
#include "noteview.h"       //needed for edit box lengths
#endif
#ifndef __cat__
#include "cat.h"
#endif
#ifndef __catlist__
#include "catlist.h"         //needed for matchstyle, cat types
#endif
#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __keyedlst__
#include "keyedlst.h"
#endif

typedef enum
{

    kExcUndated,
    kExcMilestone,
    kExcFloat,
    kExcTask,
    kExcRed,
    kExcRedGray,
    kExcGray,
    kExcBlueGray,
    kExcBlue,
    kExcDone,
    kExcUndone,
    kExcAlarm,
    kExcNoAlarm,
    kExcFreq,
    kExcNoFreq,

    kFirstExclusion= kExcUndated,
    kLastExclusion= kExcNoFreq

} TExclusion;

typedef enum
     {
     kAnyDateFilter,
     kBeforeFilter,
     kAfterFilter,
     kDayFilter,
     kWeekFilter,
     kMonthFilter,
     kNoteDatesFilter
     } TDateEffect;


//filter object, used to create noteset
class _CLASSTYPE TFilter: Object, TStreamable {
    friend TFilterDlg;
    friend TPimFrame;         //has interactive filter, needs access

    public:
        TFilter( RTFilter filter);
        TFilter( PTCatList pCategories = NULL,
            BOOL allSelected           = FALSE,
            TMatchStyle matchStyle     = kCatMatchAny,
            PTSafStr pSearchText       = NULL,
            BOOL aExcludeDatelessNotes = FALSE);

        virtual ~TFilter();

        BOOL Match( RTNote note );
        int  NumCats( void );
        BOOL IsActive( void );
        BOOL IsAllSelected( void ) { return itsAllSelected; };
        void SelectAll( void );
        void SelectNone( void );

        void Exclude( TExclusion, BOOL );
        BOOL IsExcluded( TExclusion );

        void IncludeUncat( BOOL b ) { itsIncludeUncat= b; };
        BOOL IsUncatIncluded( void ) { return itsIncludeUncat; };
        PTCatList GetSelCats( void ) { return itsSelCatsP; };

        void SetFilterDates( TDateEffect effect, RTTime time );
        void SetFilterDates( RTTime time );
        void SetFilterDates( RTTime start, RTTime end );

        void UpdateFloatingDates( void );
        void SwitchCats( PTCatTree );

        void PrepareCatTable( void );
        void DestroyCatTable( void );

        classType isA( void ) const;
        char *nameOf( void ) const;
        hashValueType hashValue( void ) const;
        int isEqual(const Object &) const;
        void printOn(ostream &) const;
        static PTStreamable build();

    private:

        TFilter( StreamableInit s );
        void write( Ropstream );
        Pvoid read( Ripstream );

        const Pchar streamableName() const { return "TFilter"; };
        TFilter &operator =( const TFilter &other );

        BOOL DatesMatch( RTNote note );
        BOOL NoteEndsBefore( RTNote note, RTTime endTime );

        PTCatList   itsSelCatsP;
        BOOL        itsAllSelected;
        TMatchStyle itsMatchStyle;
        TTime       itsStart;
        TTime       itsEnd;
        TSafStr     itsFindText;
        TDateEffect itsDateEffect;

        BOOL        itsIncludeUncat;
        BOOL        itsExclusions[kLastExclusion + 1];

        BOOL MatchAnyCat( RTCatList );
        BOOL MatchEveryCat( RTCatList );

        BOOL        itsCatTableSetUp;
        int         itsCatTableSize;
        PTCatList   **itsCatTableData;
        PTCatTable  itsCatTableP;

};

    inline Ripstream operator >> (Ripstream is, RTFilter cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTFilter cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTFilter cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTFilter cl)
        { return os << (PTStreamable) cl; }

#endif

