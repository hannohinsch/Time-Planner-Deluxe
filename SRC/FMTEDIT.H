#ifndef __fmtedit__
#define __fmtedit__

_CLASSDEF( TFmtEdit )
_CLASSDEF( TTimeStatic )
_CLASSDEF( TDurationStatic )
_CLASSDEF( TValueEdit )

#ifndef __pen__
#include "pen.h"
#endif
#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __duration__
#include "duration.h"
#endif
#ifndef __msg__
#include "msg.h"
#endif
#ifndef __value__
#include "value.h"
#endif
#ifndef __safstr__
#include "safstr.h"
#endif


// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// FmtEdit
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
class _CLASSTYPE TFmtEdit: public TEdit
{
public:

    TFmtEdit( PTWindowsObject aParent, WORD anID );
    virtual ~TFmtEdit();

    virtual BOOL Validate( void );

protected:

    void    BitSet( BYTE map[], BYTE c );

    virtual BOOL ControlIsValid( void ) { return TRUE; };

    virtual void ShowErrorMsg( void );
    virtual void SetupWindow( void );
    virtual void WMKillFocus( RTMessage )=[ WM_FIRST + WM_KILLFOCUS ];
    virtual void WMInvalid( RTMessage )= [ WM_FIRST + WM_EDIT_INVALID ];

    virtual void CopyToControl( void )   {};
    virtual void CopyFromControl( void ) {};

    virtual void SetConstraints( void ) {};

private:

    BOOL    itsInvalidMsgPosted;
};


// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// TimeOnlyEdit
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
typedef enum {
     kMustHaveTime,
     kCanHaveTime,
     kCantHaveTime
     } TTimeOnlyMode;

class _CLASSTYPE TTimeOnlyEdit: public TFmtEdit
{
    public:

    TTimeOnlyEdit( PTWindowsObject aParent, WORD anID,
        RTTime time, TTimeOnlyMode mode );
	virtual ~TTimeOnlyEdit();

    void   Set( RTTime time );
    void   Clear( void );
    void   SetNow( void );
    BOOL   HasTime( void );
    RTTime Get( void ) ;
    void   CopyTime( RTTime date );
    void   SetMode( BOOL hasDate, BOOL isInHours );
    void   SetRange( RTTime start, RTTime end );

private:

    void   AdjustForMode( void );
    BOOL   ControlIsValid( void );
    void   CopyToControl( void );
    void   CopyFromControl( void );
    void   SetConstraints( void );
    void   ShowErrorMsg( void );

    TTime          itsTime;
    TTimeOnlyMode  itsMode;
    TTime          itsStartLimit;
    TTime          itsEndLimit;

};

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// DateOnlyEdit
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
class _CLASSTYPE TDateOnlyEdit: public TFmtEdit
{
    public:

    TDateOnlyEdit( PTWindowsObject aParent, WORD anID, RTTime time );
	virtual ~TDateOnlyEdit();

    void   Set( RTTime time );
    void   Clear( void );
    void   SetNow( void );
    BOOL   HasDate( void );
    RTTime Get( void ) ;
    void   CopyDate( RTTime date );

private:

    BOOL   ControlIsValid( void );

    void   CopyToControl( void );
    void   CopyFromControl( void );
    void   SetConstraints( void );

    TTime itsTime;

};

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// TimeStatic
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
class _CLASSTYPE TTimeStatic: public TFmtEdit
{
    public:

    TTimeStatic( PTWindowsObject aParent, WORD anID, RTTime time );
	virtual ~TTimeStatic();

    void  Set( RTTime time );
    void  Clear( void );
    void  SetNow( void );
    void  UsesHours( BOOL hours );
    BOOL  HasDate( void );
    BOOL  HasTime( void );
    RTTime Get( void ) ;
    void  CopyDate( RTTime date );

private:

    void CopyToControl( void );
    void CheckHours( void );

    TTime itsTime;
    BOOL  itsHoursOn;

};

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// ValueEdit
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
class _CLASSTYPE TValueEdit: public TFmtEdit
{

public:

    TValueEdit( PTWindowsObject aParent, WORD anID, RTValue value );
	virtual ~TValueEdit();

    void Set( RTValue value );
    BOOL HasValue( void );
    RTValue Get( void );
    void Clear( void );


private:

    BOOL ControlIsValid( void );
    void SetConstraints( void );
    void CopyToControl( void );
    void CopyFromControl( void );

    TValue itsValue;
};


// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// DurationStatic
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
class _CLASSTYPE TDurationStatic: public TFmtEdit
{
public:

    TDurationStatic( PTWindowsObject aParent, WORD anID,
        RTDuration duration, TTimeUnit unit );
	virtual ~TDurationStatic();

    void Set( RTDuration duration );
    void Set( TTimeUnit unit, long numUnits );
    void Set( RTTime start, RTTime end );
    void SetTimeUnit( TTimeUnit unit );
    BOOL IsZero( void );
    void Clear( void );

    RTDuration Get( void );
    long GetNumUnits( TTimeUnit unit );

private:

    void CopyToControl( void );

    TDuration   itsDuration;
    TTimeUnit   itsTimeUnit;
    BOOL        itsWorkTime;


};


#endif
