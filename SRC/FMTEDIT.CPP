//*****************************************************************************
// pen-aware edit control
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "fmtedit.h"
#include "pen.h"
#include "ink.h"
#include "parse.h"

#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __fontmap__
#include "fontmap.h"
#endif
#ifndef __worktime__
#include "worktime.h"
#endif

//*****************************************************************************

//****************************************************************************
TFmtEdit::TFmtEdit( PTWindowsObject aParent, WORD anID ):
    TEdit( aParent, anID, 0 )
{
    itsInvalidMsgPosted = FALSE;
}

//*****************************************************************************

//*****************************************************************************
TFmtEdit::~TFmtEdit()
{
}

//*****************************************************************************
// utility bitmap setter, for constraint functions
//*****************************************************************************
void TFmtEdit::BitSet( BYTE map[], BYTE c )
{
    switch( c & (BYTE) 7 )     // 0-7
        {
        case 0:   map[ c/8 ] |= (BYTE) 1;   break;
        case 1:   map[ c/8 ] |= (BYTE) 2;   break;
        case 2:   map[ c/8 ] |= (BYTE) 4;   break;
        case 3:   map[ c/8 ] |= (BYTE) 8;   break;
        case 4:   map[ c/8 ] |= (BYTE) 16;  break;
        case 5:   map[ c/8 ] |= (BYTE) 32;  break;
        case 6:   map[ c/8 ] |= (BYTE) 64;  break;
        case 7:   map[ c/8 ] |= (BYTE) 128; break;
        default:                            break;
        }
}

//*****************************************************************************

//*****************************************************************************
void TFmtEdit::SetupWindow( void )
{
    TEdit::SetupWindow();

    //set better font
    SendMessage( HWindow, WM_SETFONT,
        TFontMap::GetScreenFont( kBoldFont ), (DWORD) 0 );

    if( TPen::bPenActive )
	    {
        SetConstraints();
	    }
}

//*****************************************************************************
// WMKillFocus
//
//  If the user has just typed something, then transfer it into the local
//  buffer
//*****************************************************************************
void TFmtEdit::WMKillFocus( RTMessage msg )
{
    if( !Validate() )
        return;

    DefWndProc( msg );
}



//*****************************************************************************
//  Validate
//
//  Reparse control text and ask user to correct it, as necessary
//*****************************************************************************
BOOL TFmtEdit::Validate( void )
{
    if( !ControlIsValid() || itsInvalidMsgPosted )
        {
        //don't give msg immediately, cause focus processing messes up callers
        // instead, post msg that we'll process later
        if( !itsInvalidMsgPosted )
            {
            PostMessage( HWindow, WM_EDIT_INVALID, 0, 0 );
            itsInvalidMsgPosted= TRUE;
            }

        return( FALSE );
        }
    else
        {
        // copy ctl's value and reformat ctl
        CopyFromControl();
        CopyToControl();
        return( TRUE );
        }
}

//*****************************************************************************

//*****************************************************************************
void TFmtEdit::ShowErrorMsg( void )
{
    TMessageBox::Ok( this->Parent,
        "Please check that this field\nis properly formatted.",
        "Unexpected Format" );

}

//*****************************************************************************

//*****************************************************************************
void TFmtEdit::WMInvalid( RTMessage )
{
    ShowErrorMsg();

    //select erroneous text
    SetFocus( HWindow );
    SetSelection( 0, 32767 );

    itsInvalidMsgPosted= FALSE;
}

//////////////////////////////// Time /////////////////////////////////////////

//*****************************************************************************
// Constructor
//*****************************************************************************
TTimeStatic::TTimeStatic( PTWindowsObject aParent, WORD anID,
    RTTime time)
    : TFmtEdit( aParent, anID )
{
    itsTime    = time;
    itsHoursOn = itsTime.HasTime();

    // no automatic transfer
    DisableTransfer();
}

//*****************************************************************************

//*****************************************************************************
TTimeStatic::~TTimeStatic()
{
}


//*****************************************************************************
// SetTime
//*****************************************************************************
void TTimeStatic::Set( RTTime time )
{
    itsTime = time;
    CheckHours();
    CopyToControl();
}

//*****************************************************************************
// Clear
//*****************************************************************************
void TTimeStatic::Clear( void )
{
    itsTime.Clear();
    CopyToControl();
}

//*****************************************************************************
// SetNow
//*****************************************************************************
void TTimeStatic::SetNow( void )
{
    itsTime.SetNow();
    CheckHours();
    CopyToControl();
}

//*****************************************************************************
// UsesHours
//*****************************************************************************
void TTimeStatic::UsesHours( BOOL hours )
{
    itsHoursOn = hours;
    CheckHours();
    CopyToControl();
}

//*****************************************************************************
// CheckHours
//
// Make sure the time either does or doesn't have a time, depending on
// itsHoursOn. Sets the time to 9am if it doesn't have a time and
// needs one.
//*****************************************************************************
void TTimeStatic::CheckHours( void )
{
    // if it has no date, it has no time
    if (!itsTime.HasDate()) {

        itsTime.Clear();
    }

    // if it has no time but needs one, then set to 9 am
    else if (itsHoursOn && !itsTime.HasTime()) {

        itsTime.SetHour( 9 );       //FIXME should be first work hour
    }

    // if it has a time but doesn't need one, clear the time
    else if ((!itsHoursOn) && itsTime.HasTime()) {

        itsTime.ClearTime();
    }
}

//*****************************************************************************
// CopyToControl
//
// Copy the time into the control
//*****************************************************************************
void TTimeStatic::CopyToControl( void )
{
    char buf[100];

    TIntlFmt::FormatDateAndTime( itsTime, kShortDate, buf );
    SetText( buf );
}

//*****************************************************************************
// HasDate
//*****************************************************************************
BOOL TTimeStatic::HasDate( void )
{
    return (itsTime.HasDate());
}

//*****************************************************************************
// HasTime
//*****************************************************************************
BOOL TTimeStatic::HasTime( void )
{
    return (itsTime.HasTime());
}

//*****************************************************************************
// Get Date
//*****************************************************************************
RTTime TTimeStatic::Get( void )
{
    return itsTime;
}


//*****************************************************************************
// CopyDate
//*****************************************************************************
void TTimeStatic::CopyDate( RTTime date )
{
    itsTime.CopyDate( date );
    CopyToControl();
}

//////////////////////////////// TimeOnly /////////////////////////////////////////

//*****************************************************************************
// Constructor
//*****************************************************************************
TTimeOnlyEdit::TTimeOnlyEdit( PTWindowsObject aParent, WORD anID,
    RTTime time, TTimeOnlyMode mode)

    : TFmtEdit( aParent, anID )
{
    itsTime = time;
    itsMode = mode;

    // no automatic transfer
    DisableTransfer();
}

//*****************************************************************************

//*****************************************************************************
TTimeOnlyEdit::~TTimeOnlyEdit()
{
}

//*****************************************************************************

//*****************************************************************************
void TTimeOnlyEdit::SetConstraints( void )
{
    RC rc;		//pen recognition context

    if( SendMessage( HWindow, WM_HEDITCTL, HE_GETRC, (LONG)((LPRC) &rc ) ))
      	{
	    rc.alc= ALC_NUMERIC | ALC_USEBITMAP | ALC_GESTURE;
	    BitSet( rc.rgbfAlc, ':' );
	    BitSet( rc.rgbfAlc, 'P' );
	    BitSet( rc.rgbfAlc, 'M' );
	    BitSet( rc.rgbfAlc, 'A' );
	    BitSet( rc.rgbfAlc, '-' );
	    SendMessage( HWindow, WM_HEDITCTL, HE_SETRC, (LONG)((LPRC) &rc ));
        }
}


//*****************************************************************************
// SetTime
//*****************************************************************************
void TTimeOnlyEdit::Set( RTTime time )
{
    itsTime.CopyTime( time );
    AdjustForMode();
    CopyToControl();
}

//*****************************************************************************
// Clear
//*****************************************************************************
void TTimeOnlyEdit::Clear( void )
{
    itsTime.Clear();
    AdjustForMode();
    CopyToControl();
}

//*****************************************************************************
// SetNow
//*****************************************************************************
void TTimeOnlyEdit::SetNow( void )
{
    itsTime.SetNow();
    itsTime.ClearDate();
    AdjustForMode();
    CopyToControl();
}



//*****************************************************************************
// CopyToControl
//
// Copy the time into the control
//*****************************************************************************
void TTimeOnlyEdit::CopyToControl( void )
{
    char buf[100];

    TIntlFmt::FormatTime( itsTime, FALSE, buf );
    SetText( buf );
}

//*****************************************************************************

//*****************************************************************************
void TTimeOnlyEdit::CopyFromControl( void )
{
    char     buf[100];
    TTime    time;

    GetText( buf, 100 );
    TParse::ChkTime( buf, time );

    if( time.HasTime() )
        {
        itsTime.CopyTime( time );
        AdjustForMode();
        }
    else
        {
        itsTime.Clear();
        AdjustForMode();
        }
}

//*****************************************************************************

//*****************************************************************************
BOOL TTimeOnlyEdit::ControlIsValid( void )
{
    char     buf[100];
    TTime    time;

    GetText( buf, 100 );

    if( buf[0] == '\0' && itsMode == kMustHaveTime )
        return FALSE;
    else if( buf[0] != '\0' && itsMode == kCantHaveTime )
        return FALSE;

    TParse::ChkTime( buf, time );

    if( time.HasTime() )
        {
        //FIXME comparison operators don't work on TTime objects that
        // have only a time ( no date ), so here's a kludge
        if( itsStartLimit.HasTime() )
            {
            TTime aTime, aStart;

            aTime.SetDate( 1, 1, 93 );
            aStart.SetDate( 1, 1, 93 );

            aTime.CopyTime( time );
            aStart.CopyTime( itsStartLimit );

            if( aStart > aTime )
                return FALSE;
            }

        if( itsEndLimit.HasTime() )
            {
            TTime aTime, aEnd;

            aTime.SetDate( 1, 1, 93 );
            aEnd.SetDate( 1, 1, 93 );

            aTime.CopyTime( time );
            aEnd.CopyTime( itsEndLimit );

            if( aEnd < aTime )
                return FALSE;
            }

        }

    return( time.HasTime() || buf[0] == '\0' );
}

//*****************************************************************************
// HasTime
//*****************************************************************************
BOOL TTimeOnlyEdit::HasTime( void )
{
    CopyFromControl();
    return (itsTime.HasTime());
}

//*****************************************************************************
// Get
//*****************************************************************************
RTTime TTimeOnlyEdit::Get( void )
{
    return itsTime;
}


//*****************************************************************************
// CopyTime
//*****************************************************************************
void TTimeOnlyEdit::CopyTime( RTTime date )
{
    itsTime.CopyTime( date );
    AdjustForMode();
    CopyToControl();
}

//*****************************************************************************
// SetMode
//*****************************************************************************
void TTimeOnlyEdit::SetMode( BOOL hasDate, BOOL isInHours )
{
    if (hasDate && isInHours)
        itsMode = kMustHaveTime;
    else if (hasDate && !isInHours)
        itsMode = kCantHaveTime;
    else if (!hasDate && isInHours)
        itsMode = kCanHaveTime;
    else
        itsMode = kCantHaveTime;

    AdjustForMode();
}

//*****************************************************************************
// AdjustForMode
//
//  Adjust the time based on the mode
//*****************************************************************************
void TTimeOnlyEdit::AdjustForMode( void )
{
    switch( itsMode )
        {
        case kMustHaveTime:

            if (!itsTime.HasTime())
                {
                //set to first hour of workday
                TTime time;
                time.SetNow();
                time.SetToMidnight();
                time.ForwardToWorkTime();
                itsTime.CopyTime( time );
                CopyToControl();
                }
            break;

        case kCanHaveTime:

            break;

        case kCantHaveTime:

            if (itsTime.HasTime())
                {
                itsTime.Clear();
                CopyToControl();
                }
            break;
        }
}

//*****************************************************************************

//*****************************************************************************
void TTimeOnlyEdit::ShowErrorMsg( void )
{
    if( itsStartLimit.HasTime() || itsEndLimit.HasTime() )
        {
        TSafStr msg;
        char start[ 100 ], end[ 100 ];

        TIntlFmt::FormatTime( itsStartLimit, TRUE, start );
        TIntlFmt::FormatTime( itsEndLimit, TRUE, end );
        msg.Printf(  "Please check that this field contains a\nproperly formatted time between %s and %s",
                      start,
                      end );

        TMessageBox::Ok( this->Parent, SAF2CP( msg ), "Unexpected Time" );
        }
    else
        {
        TMessageBox::Ok( this->Parent,
            "Please check that this field contains a\nproper time.",
            "Unexpected Time" );
        }

}

//*****************************************************************************

//*****************************************************************************
void TTimeOnlyEdit::SetRange( RTTime start, RTTime end )
{
itsStartLimit= start;
itsEndLimit= end;
}

//////////////////////////////// DateOnly /////////////////////////////////////////

//*****************************************************************************
// Constructor
//*****************************************************************************
TDateOnlyEdit::TDateOnlyEdit( PTWindowsObject aParent, WORD anID,
    RTTime time)

    : TFmtEdit( aParent, anID )
{
    itsTime = time;

    // no automatic transfer
    DisableTransfer();
}

//*****************************************************************************

//*****************************************************************************
TDateOnlyEdit::~TDateOnlyEdit()
{
}

//*****************************************************************************

//*****************************************************************************
void TDateOnlyEdit::SetConstraints( void )
{
    RC rc;		//pen recognition context

    if( SendMessage( HWindow, WM_HEDITCTL, HE_GETRC, (LONG)((LPRC) &rc ) ))
	    {
	    rc.alc= ALC_NUMERIC | ALC_USEBITMAP | ALC_GESTURE;
	    BitSet( rc.rgbfAlc, ':' );
	    BitSet( rc.rgbfAlc, 'P' );
	    BitSet( rc.rgbfAlc, 'M' );
	    BitSet( rc.rgbfAlc, 'A' );
	    BitSet( rc.rgbfAlc, '-' );
	    SendMessage( HWindow, WM_HEDITCTL, HE_SETRC, (LONG)((LPRC) &rc ));
	    }
}


//*****************************************************************************
// SetTime
//*****************************************************************************
void TDateOnlyEdit::Set( RTTime time )
{
    itsTime.CopyDate( time );
    CopyToControl();
}

//*****************************************************************************
// Clear
//*****************************************************************************
void TDateOnlyEdit::Clear( void )
{
    itsTime.Clear();
    CopyToControl();
}

//*****************************************************************************
// SetNow
//*****************************************************************************
void TDateOnlyEdit::SetNow( void )
{
    itsTime.SetNow();
    itsTime.ClearTime();
    CopyToControl();
}



//*****************************************************************************
// CopyToControl
//
// Copy the time into the control
//*****************************************************************************
void TDateOnlyEdit::CopyToControl( void )
{
    char buf[100];

    TIntlFmt::FormatDate( itsTime, kShortDate, buf );
    SetText( buf );
}

//*****************************************************************************

//*****************************************************************************
void TDateOnlyEdit::CopyFromControl( void )
{
    char     buf[100];
    TTime   time;

    GetText( buf, 100 );
    TParse::ChkDate( buf, time );
    if( time.HasDate() )
        {
        itsTime.CopyDate( time );
        }
    else
        {
        itsTime.Clear();
        }
}

//*****************************************************************************

//*****************************************************************************
BOOL TDateOnlyEdit::ControlIsValid( void )
{
    char     buf[100];
    TTime    time;

    GetText( buf, 100 );
    TParse::ChkDate( buf, time );
    return (time.HasDate() || (buf[0] == '\0'));
}

//*****************************************************************************
// HasDate
//*****************************************************************************
BOOL TDateOnlyEdit::HasDate( void )
{
    return (itsTime.HasDate());
}

//*****************************************************************************
// Get
//*****************************************************************************
RTTime TDateOnlyEdit::Get( void )
{
    return itsTime;
}


//*****************************************************************************
// CopyDate
//*****************************************************************************
void TDateOnlyEdit::CopyDate( RTTime date )
{
    itsTime.CopyDate( date );
    CopyToControl();
}


/////////////////////////////////// VALUE ////////////////////////////////////

//*****************************************************************************

//*****************************************************************************
TValueEdit::TValueEdit( PTWindowsObject aParent, WORD anID,
    RTValue value )

    :TFmtEdit( aParent, anID )
{
    itsValue = value;

    // no automatic transfer
    DisableTransfer();
}

//*****************************************************************************

//*****************************************************************************
TValueEdit::~TValueEdit()
{
}

//*****************************************************************************

//*****************************************************************************
void TValueEdit::SetConstraints( void )
{
    RC rc;		//pen recognition context

    if( SendMessage( HWindow, WM_HEDITCTL, HE_GETRC, (LONG)((LPRC) &rc ) ))
    	{
	    rc.alc= ALC_NUMERIC | ALC_MONETARY | ALC_USEBITMAP | ALC_GESTURE;
	    BitSet( rc.rgbfAlc, '.' );
	    SendMessage( HWindow, WM_HEDITCTL, HE_SETRC, (LONG)((LPRC) &rc ));
	    }
}


//*****************************************************************************
// SetValue
//
//*****************************************************************************
void TValueEdit::Set( RTValue value )
{
    itsValue = value;
    CopyToControl();
}

//*****************************************************************************
// ClearValue
//
//*****************************************************************************
void TValueEdit::Clear( void )
{
    itsValue.Clear();
    CopyToControl();
}

//*****************************************************************************

//*****************************************************************************
BOOL TValueEdit::ControlIsValid( void )
{
    char     buf[100];
    TValue   value;

    GetText( buf, 100 );
    TParse::ChkValue( buf, value );
    return ( value.HasValue() || (buf[0] == '\0'));
}

//*****************************************************************************
// HasValue
//
//*****************************************************************************
BOOL TValueEdit::HasValue( void )
{
    CopyFromControl();
    return( itsValue.HasValue());
}

//*****************************************************************************
// GetValue
//
//*****************************************************************************
RTValue TValueEdit::Get( void )
{
    CopyFromControl();
    return( itsValue );
}

//*****************************************************************************
// CopyToControl
//
// Copy the value into the control
//*****************************************************************************
void TValueEdit::CopyToControl( void )
{
    TSafStr buf;

    if( itsValue.HasValue() )
        TIntlFmt::FmtDollar( itsValue, buf, TRUE );

    SetText( (char *) (const char *) buf );
}

//*****************************************************************************

//*****************************************************************************
void TValueEdit::CopyFromControl( void )
{
    char     buf[100];
    TValue   value;

    GetText( buf, 100 );

    TParse::ChkValue( buf, value );

    if( value.HasValue() )
        {
        itsValue = value;                   //store locally
        }
    else
        {
        itsValue.Clear();
        }
}



//*****************************************************************************
//////////////////////////////// Duration /////////////////////////////////////////
//*****************************************************************************

//*****************************************************************************
// Constructor
//*****************************************************************************
TDurationStatic::TDurationStatic( PTWindowsObject aParent, WORD anID,
    RTDuration duration, TTimeUnit unit )

    : TFmtEdit( aParent, anID )
{
    itsDuration   = duration;
    itsTimeUnit   = unit;

    // no automatic transfer
    DisableTransfer();
}

//*****************************************************************************

//*****************************************************************************
TDurationStatic::~TDurationStatic()
{
}

//*****************************************************************************
// Set
//*****************************************************************************
void TDurationStatic::Set( RTDuration duration )
{
    itsDuration = duration;
    CopyToControl();
}

//*****************************************************************************
// Set
//*****************************************************************************
void TDurationStatic::Set( TTimeUnit unit, long numUnits)
{
    itsDuration.Set( unit, numUnits );
    CopyToControl();
}

//*****************************************************************************
// Clear
//*****************************************************************************
void TDurationStatic::Clear( void )
{
    itsDuration.Clear();
    CopyToControl();
}

//*****************************************************************************
// SetTime
//*****************************************************************************
void TDurationStatic::Set( RTTime start, RTTime end )
{
    itsDuration.Set( start, end );
    CopyToControl();
}



//*****************************************************************************
// CopyToControl
//
// Copy the Duration into the control
//*****************************************************************************
void TDurationStatic::CopyToControl( void )
{
    char buf[100];

    itsDuration.Format( itsTimeUnit, TRUE, buf );
    SetText( buf );
}


//*****************************************************************************
// GetDuration
//*****************************************************************************
RTDuration TDurationStatic::Get( void )
{
    return( itsDuration );
}

//*****************************************************************************
// GetNumUnits
//*****************************************************************************
long TDurationStatic::GetNumUnits( TTimeUnit unit )
{
    return( itsDuration.GetNumUnits( unit ) );
}

//*****************************************************************************
// SetScale
//*****************************************************************************
void TDurationStatic::SetTimeUnit( TTimeUnit unit )
{
    itsTimeUnit = unit;
}

//*****************************************************************************
// IsZero
//*****************************************************************************
BOOL TDurationStatic::IsZero( void )
{
    return( itsDuration.IsZero());
}

