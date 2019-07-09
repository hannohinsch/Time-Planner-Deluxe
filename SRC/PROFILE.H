#ifndef __profile__
#define __profile__

_CLASSDEF( TProfile )

#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __gdimap__
#include "gdimap.h"
#endif

typedef enum {

    kDayview_5_Minutes    = 0,
    kDayview_6_Minutes    = 1,
    kDayview_10_Minutes   = 2,
    kDayview_12_Minutes   = 3,
    kDayview_15_Minutes   = 4,
    kDayview_20_Minutes   = 5,
    kDayview_30_Minutes   = 6,
    kDayview_60_Minutes   = 7,
    kDayview_120_Minutes  = 8

    } TDayviewIncrements;

typedef enum {

    kTimeConflict_Never   = 0,
    kTimeConflict_Cat     = 1,
    kTimeConflict_Always  = 2

    } TTimeConflict;

typedef enum {

    kTimeFmt_Hours    = 0,
    kTimeFmt_Days     = 1,
    kTimeFmt_Weeks    = 2

    } TTimeFmtUnits;

typedef enum {

    kNoteOrder_Color    = 0,
    kNoteOrder_Date,
    kNoteOrder_Duration,
    kNoteOrder_Type,
    kNoteOrder_Alpha

    } TNoteOrder;

const int kNumNoteOrders = 3;

class _CLASSTYPE TProfile {

    public:

    TProfile( void );
    virtual ~TProfile( void );

    static void StringToDouble( char *s, double &d );
    static void DoubleToString( double d, char *s );
    static void CalculateHours( void );
    static void GetNoteOrderName( TNoteOrder, RTSafStr );

    static void RestoreSort( void );
    static void RestoreInterval( void );

    // profile information read in at start up and
    // written out at shutdown
    static TDayviewIncrements dayviewIncrement;
    static TTimeConflict      TimeConflict;
    static TTimeFmtUnits      baseTimeFmtUnits;
    static BOOL               bMixTimeFmtUnits;
    static TNoteOrder         NoteOrder[kNumNoteOrders];

    static double             HoursPerDay;
    static double             DaysPerWeek;
    static double             HoursPerWeek;
    static int                WorkDayStartMinute;
    static int                WorkDayEndMinute;
    static BOOL               IsWorkDay[7];

    static TSafStr            MRUFile1;     //most recently used files
	static TSafStr			  MRUFile2;
	static TSafStr			  MRUFile3;
	static TSafStr			  MRUFile4;

    static TGDISet            colorSet;

    static int                debugLevel;
    static BOOL               bAutoLoad;
    static BOOL               bShowTips;

    static int                BaudRate;
    static int                CommPort;
    static BOOL               bTone;
    static char               LocalPrefix[25];
    static char               LocalSuffix[25];
    static char               LongDistancePrefix[25];
    static char               LongDistanceSuffix[25];
    static char               InternationalPrefix[25];
    static char               InternationalSuffix[25];
    static char               AlternatePrefix[25];
    static char               AlternateSuffix[25];
    static char               LocalAreaCode[25];

};


#endif


