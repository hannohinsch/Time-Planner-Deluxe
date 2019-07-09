/***************************************************************************
 *                                                                         *
 *  MODULE     : rptgrid.cpp
 *               implement main grid of report view
 *
 *
 ***************************************************************************/
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "rptgrid.h"

#ifndef __alarmset__
#include "alarmset.h"
#endif
#ifndef __frame__
#include "frame.h"
#endif
#ifndef __note__
#include "note.h"
#endif
#ifndef __noteview__
#include "noteview.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __paint__
#include "paint.h"
#endif
#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __pen__
#include "pen.h"
#endif
#ifndef __profile__
#include "profile.h"
#endif
#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __cat__
#include "cat.h"
#endif
#ifndef __catlist__
#include "catlist.h"
#endif
#ifndef __cattree__
#include "cattree.h"
#endif
#ifndef  __cursor__
#include "cursor.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __worktime__
#include "worktime.h"
#endif
#ifndef __notemenu__
#include "notemenu.h"
#endif
#ifndef __print__
#include "print.h"
#endif
#ifndef __freq__
#include "freq.h"
#endif
#ifndef __oem__
#include "oem.h"
#endif
#ifndef __parse__
#include "parse.h"
#endif
#ifndef __utility__
#include "utility.h"
#endif
#ifndef __filter__
#include "filter.h"
#endif
#ifndef __notetab__
#include "notetab.h"
#endif

const kTopLeading= 0;       //idea that didn't work
const kBotLeading= 0;

TReportField    TReportGrid::itsColumnFields[ kMaxReportCols ];
int             TReportGrid::itsBreakLevel;
PTSafStr        TReportGrid::itsFieldTitles[ kNumFields ];

//NB must be same order as field ids
WORD TReportGrid::itsFieldWidths[ kNumFields ]=
    {
    19,         //mark
    153,        //text
    143,        //start
    143,        //end
    101,        //elapsed
    101,        //worktime
    103,        //freq
    85,         //rate
    95,         //amount
    143,        //alarm
    81,         //time units
    0           //dont use
    };

static const int kTitleRow = 0;

//============================================================================
// The TReportGrid class functions (override virtual fns in TRuleGrid abstract class)
//============================================================================

//*****************************************************************************
// create new table grid window
//
//*****************************************************************************
TReportGrid::TReportGrid( PTWindowsObject AParent ): TRuleGrid( AParent )
{
TCursor::Busy( TRUE );

scrPainter= new TPaint;
prtPainter= new TPaintPrt;

itsFieldTitles[ kStartField ]= new KSTR( "Start" );
itsFieldTitles[ kEndField ]= new KSTR( "End" );
itsFieldTitles[ kElapsedField ]= new KSTR( "Elapsed time" );
itsFieldTitles[ kWorktimeField ]= new KSTR( "Work time" );
itsFieldTitles[ kFreqField ]= new KSTR( "Freq" );
itsFieldTitles[ kRateField ]= new KSTR( "Rate" );
itsFieldTitles[ kAmtField ]= new KSTR( "Amount" );
itsFieldTitles[ kAlarmField ]= new KSTR( "Alarm time" );
itsFieldTitles[ kMarkField ]= new KSTR( "" );
itsFieldTitles[ kTextField ]= new KSTR( "Note" );
itsFieldTitles[ kDontUseField ]= new KSTR( "" );
itsFieldTitles[ kTimeUnitField ]= new KSTR( "Time units" );

itsNoteTableP = new TNoteTable;

SetColFreeze( 2 );
SetRowFreeze( 1 );

Reset();

KSTR titleText( "Report" );
SetPrtTitle( titleText );

SetDefaultPrintRange();
}

//*****************************************************************************

//*****************************************************************************
TReportGrid::~TReportGrid( void )
{
for( int i=kFirstField; i <= kLastField; i++ )
    {
    delete itsFieldTitles[ i ];
    }

if( itsNoteTableP )
    {
    itsNoteTableP->Flush( TXArray::kDelete );
    delete itsNoteTableP;
    }

delete scrPainter;
delete prtPainter;
}

//*****************************************************************************

//*****************************************************************************
void TReportGrid::SetupWindow( void )
{
TRuleGrid::SetupWindow();

int r= TSysState::currentNoteIndex;
r= max( 0, min( r, TSysState::pSelNotes->getItemsInContainer() ) );
GotoRC( r, -1 );

TCursor::Busy( FALSE );
}

//*****************************************************************************
// get profile info
//*****************************************************************************
void TReportGrid::AppInit( void )
{
ReadProfile();
}

//*****************************************************************************

//*****************************************************************************
void TReportGrid::ReadProfile( void )
{
char buf[ 100 ];

itsBreakLevel = GetPrivateProfileInt( TOEM::profileSection,
        "RptBreaks", 0, TOEM::profileName );

GetPrivateProfileString( TOEM::profileSection,
        "RptFields", "2,3,4,5,7,8", buf, 100, TOEM::profileName );

//first two columns are constant
itsColumnFields[ 0 ]= kMarkField;
itsColumnFields[ 1 ]= kTextField;

char *p= strtok( buf, "," );
for( int i= 2; i < kMaxReportCols; i++ )
    {
    if( !IsBlank( p ) )
        {
        TReportField f= (TReportField) TParse::MkNum( p );
        if( f >= kFirstField && f <= kLastField )
            itsColumnFields[ i ]= f;
        else
            itsColumnFields[ i ]= kDontUseField;

        p= strtok( NULL, "," );
        }
    else
        itsColumnFields[ i ]= kDontUseField;
    }
}

//*****************************************************************************
// SaveProfile
//
//  write profile info when user selects MakeDefault
//  in Report Options Dialog
//*****************************************************************************
void TReportGrid::SaveProfile( void )
{
char buf[ 100 ];

wsprintf( buf, "%d", itsBreakLevel );
WritePrivateProfileString( TOEM::profileSection,
    "RptBreaks", buf, TOEM::profileName );

char *p= buf;
//first two columns are constant -- don't save
for( int i= 2; i < kMaxReportCols; i++ )
    {
    int f= (int) itsColumnFields[ i ];
    int n= wsprintf( p, "%d,", f );
    p+= n;      //advance to the null we just wrote
    }

WritePrivateProfileString( TOEM::profileSection,
    "RptFields", buf, TOEM::profileName );
}

//*****************************************************************************

//*****************************************************************************
void TReportGrid::SetBreakLevel( int n )
{
if (n != itsBreakLevel)
    {
    itsBreakLevel= n;
    TPimFrame::pFrame->ResetView();
    }
}

//*****************************************************************************

//*****************************************************************************
int TReportGrid::GetBreakLevel( void )
{
return itsBreakLevel;
}

//*****************************************************************************

//*****************************************************************************
void TReportGrid::SetColumnField( int col, TReportField f )
{
itsColumnFields[ col ]= f;
}

//*****************************************************************************

//*****************************************************************************
TReportField TReportGrid::GetColumnField( int col )
{
return itsColumnFields[ col ];
}

//*****************************************************************************

//*****************************************************************************
void TReportGrid::Listen( TSnoopMsg aMsg, RObject )
{
switch( aMsg )
    {
    case kNoteSetChg:
    case kCatNew:
    case kCatDel:

        Reset();
        break;
    default:
        break;
    }
}

//*****************************************************************************

//*****************************************************************************
int TReportGrid::ActiveColCnt( void )
{
int n;

//subtract however many trailing "dont use" columns we have
for( n= kMaxReportCols; n > 0 && kDontUseField == itsColumnFields[ n-1 ]; n-- );

return n;
}

//*****************************************************************************
// Reset
//*****************************************************************************
void TReportGrid::Reset( void )
{
    if (TSysState::CatGrouping == kNoCatGrouping)
        ResetNoGrouping();
    else
        ResetWithGrouping();
}

//*****************************************************************************
// ResetNoGrouping
//*****************************************************************************
void TReportGrid::ResetNoGrouping( void )
{
PTNoteTotal grandP = new TNoteTotal( 0 );
PTNoteTotal sub1P  = NULL;
PTNoteTotal sub2P  = NULL;
PTNoteTotal sub3P  = NULL;

//columns are easy
SetColCnt( ActiveColCnt() );

//for rows, we need to figure out how many "subtotal" rows we need, and
//prepare a TNoteTotal object for each one.  Also, itsNoteTableP
//is set to map grid rows to notes and subtotal rows.

//first, get rid of old subtotal info
itsNoteTableP->Flush( TXArray::kDelete );

int noteCount= (TSysState::pSelNotes)->getItemsInContainer();

// 1 row for the header
itsRowCount= itsNoteTableP->Add( "" );

//subtotal objects
switch( itsBreakLevel )
    {
    case 3:
            sub3P= new TNoteTotal( 3 );

    case 2:
            sub2P= new TNoteTotal( 2 );

    case 1:
            sub1P= new TNoteTotal( 1 );

            break;              //note fall thru cases 3, 2, 1

    case 0:
    default:
            break;
    }

// add up all the times and values
PTNote lastNoteP= NULL;
for( int i= 0; i < noteCount; i++ )
    {
    RTNote note= (*(TSysState::pSelNotes))[ i ];

    //if we have a ready subtotal...
    if( lastNoteP != NULL )
        {
        int dumpLevel= 0;       //default: don't dump


        if( itsBreakLevel > 0 && 0 != note.Compare( *lastNoteP, TProfile::NoteOrder[0], FALSE ))
            dumpLevel= 1;
        else if( itsBreakLevel > 1 && 0 != note.Compare( *lastNoteP, TProfile::NoteOrder[1], FALSE ))
            dumpLevel= 2;
        else if( itsBreakLevel > 2 && 0 != note.Compare( *lastNoteP, TProfile::NoteOrder[2], FALSE ))
            dumpLevel= 3;


        switch( itsBreakLevel )     //NB cases fall through!
            {
            case 3:
                if( dumpLevel == 3 || dumpLevel == 2 || dumpLevel == 1 )
                    {
                    // register the last note as typical for this subtotal
                    sub3P->SetSample( TProfile::NoteOrder[2], lastNoteP );

                    // remember index of current total object (and make it < 0 )
                    itsRowCount += itsNoteTableP->Add( sub3P );

                    //prepare new object
                    sub3P= new TNoteTotal( 3 );
                    }

            case 2:
                if( dumpLevel == 2 || dumpLevel == 1 )
                    {
                    // register the last note as typical for this subtotal
                    sub2P->SetSample( TProfile::NoteOrder[1], lastNoteP );

                    // remember index of current total object (and make it < 0 )
                    itsRowCount += itsNoteTableP->Add( sub2P );

                    //prepare new object
                    sub2P= new TNoteTotal( 2 );
                    }

            case 1:
                if( dumpLevel == 1 )
                    {
                    // register the last note as typical for this subtotal
                    sub1P->SetSample( TProfile::NoteOrder[1], lastNoteP );

                    // remember index of current total object (and make it < 0 )
                    itsRowCount += itsNoteTableP->Add( sub1P );

                    //prepare new object
                    sub1P= new TNoteTotal( 1 );
                    }

            case 0:
            default:
                break;
            }
        }

    //add this note to row index, and its values to total
    itsRowCount += itsNoteTableP->Add( &note );

    switch( itsBreakLevel )
        {
        case 3:
            sub3P->Sum( note );
        case 2:
            sub2P->Sum( note );
        case 1:
            sub1P->Sum( note );

        case 0:
        default:
            grandP->Sum( note );
            break;
        }

    //remember for comparison
    lastNoteP= &note;
    }

//add last subtotal rows to index array
switch( itsBreakLevel )
    {
    case 3:
        // register the last note as typical for this subtotal
        sub3P->SetSample( TProfile::NoteOrder[2], lastNoteP );
        itsRowCount += itsNoteTableP->Add( sub3P );

    case 2:

        // register the last note as typical for this subtotal
        sub2P->SetSample( TProfile::NoteOrder[2], lastNoteP );
        itsRowCount += itsNoteTableP->Add( sub2P );

    case 1:

        // register the last note as typical for this subtotal
        sub1P->SetSample( TProfile::NoteOrder[2], lastNoteP );
        itsRowCount += itsNoteTableP->Add( sub1P );

    case 0:
    default:
        // add grand total to indices
        itsRowCount += itsNoteTableP->Add( grandP );
        break;
    }

ASSERT( ERR_GEN, itsRowCount <= (5 + noteCount*( itsBreakLevel + 1 )) );

SetRowCnt( itsRowCount );

InvalidateFocus();
InvalidateCell(-1,-1);
}

//*****************************************************************************
// ResetWithGrouping
//*****************************************************************************
void TReportGrid::ResetWithGrouping( void )
{
//first, get rid of old info
itsNoteTableP->Flush( TXArray::kDelete );

// put in a dummy header row for the column headings
itsRowCount = itsNoteTableP->Add( "" );

// fill itsNoteTableP with group notes
itsRowCount += itsNoteTableP->AddWithGrouping( itsBreakLevel > 0 );

// calculate the grand total from the selected notes
PTNoteTotal  grandP = new TNoteTotal( 0 );
for (int i = 0; i < TSysState::pSelNotes->getItemsInContainer(); i++)
    {
    RTNote note = (*TSysState::pSelNotes)[i];
    grandP->Sum( note );
    }

// add the grand total to the notetable
itsRowCount += itsNoteTableP->Add( grandP );

SetColCnt( ActiveColCnt() );
SetRowCnt( itsRowCount );

InvalidateFocus();
InvalidateCell(-1,-1);
}


//*****************************************************************************

//*****************************************************************************
int  TReportGrid::CellWidth( PTPaint pPainter, int col )
{
int w;

if( pPainter->IsPrintPainter() )
    {
    //prt columns are same size as scrn columns, except note column gets
    // all the extra space.
    if( itsColumnFields[ col ] == kTextField )
        {
        RECT r;
        TPrint::GetPrtPixels( &r );

        //available
        int wRest= r.right - r.left;

        //subtract fixed field and gridline sizes
        wRest -= leftMargin;
        wRest -= LeftLineWidth( pPainter );
        for( int i= 0; i <= kMaxReportCols; i++ )
            {
            TReportField f= itsColumnFields[ i ];
            if( f == kTextField )
                {
                //don't subtract title size
                wRest -= GridWidth( pPainter, i );
                }
            else
                wRest -= COLWIDTH( pPainter, i );

            }

        // leftovers are for title
        // (minimum print width is 1.5 * min screen width)
        int minTitle= itsFieldWidths[ kTextField ] + (itsFieldWidths[ kTextField ]/2);

        //previous bug: compared int to WORD, resulted in unsigned
        //comparison, therefore -13 > 151!
        //now compares int to int
        w= max( minTitle, wRest );
        }
    else if( itsColumnFields[ col ] == kElapsedField ||
             itsColumnFields[ col ] == kWorktimeField )
        {
        //size according to time fmt specs
        int fw= itsFieldWidths[ itsColumnFields[ col] ];

        if( TProfile::baseTimeFmtUnits == kTimeFmt_Hours )
            w= fw;
        else
            w= fw + fw/2;
        }
    else
        w= itsFieldWidths[ itsColumnFields[ col] ];
    }
else  //not to the printer
    {
    if( itsColumnFields[ col ] == kElapsedField ||
        itsColumnFields[ col ] == kWorktimeField )
        {
        //size according to time fmt specs
        int fw= itsFieldWidths[ itsColumnFields[ col] ];

        if( TProfile::baseTimeFmtUnits == kTimeFmt_Hours )
            w= fw;
        else
            w= fw + fw/2;
        }
    else
        w= itsFieldWidths[ itsColumnFields[ col] ];

    }

return w;
}

//*****************************************************************************

//*****************************************************************************
int TReportGrid::CellHeight( PTPaint pPainter, int row )
{
//FIXME get std height from TGDIMap (?)
int defHeight= 19;  //TRuleGrid::CellHeight( row );
int h;  //height


if( TSysState::bShowFullText && IsNoteRow( row ))
    {
    RTNote note = GetNote( row );

    if( TPen::bPenActive && note.GetInk().HasInk() && (0 == note.GetText().Len()) )
        {
        RECT r;
        note.GetInk().Measure( &r );
        h= max( defHeight, r.bottom - r.top );
        }
    else
        {
        int width= CellWidth( pPainter, 1 );       //note column

        h= pPainter->CalcHeight( note.GetText().Addr(), width, kNormalFont );

        h+= kTopLeading + kBotLeading;

        //limit height of any one note to 90% of window height
        RECT pageSize;
        if( pPainter->IsPrintPainter() )
            TPrint::GetPrtPixels( &pageSize );
        else
            GetClientRect( HWindow, &pageSize );

        h= min( h, (int) (0.9 * (pageSize.bottom - pageSize.top)) );

        h= max( defHeight, h );
        }
    }
else if ((row > 0) &&
          row < itsRowCount &&
          ((RTNoteRow) (*itsNoteTableP)[row]).itsRowType == kNoteTableHeader)
    h = 2 * defHeight;
else
    h= defHeight;      //default

return h;
}

//*****************************************************************************
// IsNoteRow
//*****************************************************************************
BOOL TReportGrid::IsNoteRow( int row )
{
if( row > 0 && row < itsRowCount )
    {
    RTNoteRow noteRow = (RTNoteRow) (*itsNoteTableP)[row];
    return noteRow.itsRowType == kNoteTableNote;
    }
else
    return FALSE;
}

//*****************************************************************************

//*****************************************************************************
int  TReportGrid::GridWidth( PTPaint , int col )
{
int w;

if( col == 1 )
    w= 2;
else
    w= 1;

return w;
}

//*****************************************************************************

//*****************************************************************************
int  TReportGrid::GridHeight( PTPaint , int row )
{
int h;

if( (row == 0 )              ||
    (row >= itsRowCount - 2) ||
    (((RTNoteRow) (*itsNoteTableP)[row + 1]).itsRowType == kNoteTableHeader))
    h= 2;
else
    h= 1;

return h;
}

//*****************************************************************************
// PaintCell
//*****************************************************************************
void  TReportGrid::PaintCell( RTDrawInfo di )
{
    if (di.row == kTitleRow) {

        PaintTitle( di );
    }
    //note row
    else if ((di.row >= 0) && (di.row < itsRowCount)) {

        RTNoteRow noteRow = (RTNoteRow) (*itsNoteTableP)[di.row];
        switch( noteRow.itsRowType )
            {
            case kNoteTableNote:

                PaintNote( di );
                break;

            case kNoteTableTotal:

                PaintSummary( di );
                break;

            case kNoteTableHeader:

                PaintHeader( di );
                break;
        }
    }
    else {

        SYSERR( ERR_GEN );
        itsNoteTableP;      //for debug
    }

}



//*****************************************************************************
// PaintTitle
//*****************************************************************************
void TReportGrid::PaintTitle( RTDrawInfo di )
{
    di.pPainter->Text( di, (*itsFieldTitles[ itsColumnFields[ di.col ]]),
             kNoHilite, kSingleLine, kBoldFont );
}

//*****************************************************************************
// PaintNote
//*****************************************************************************
void TReportGrid::PaintNote( RTDrawInfo di )
{
    char buf[200], buf2[75], buf3[75];

    RTNote note = GetNote( di.row );

    THilite hilite= kNoHilite;

    //show data field
    switch( itsColumnFields[ di.col ] )
        {

        case kTextField:
            TSafStr title;

            note.Title( title );
            title.Get( buf, sizeof( buf ) );
            if( !buf[0] && note.GetInk().HasInk() )
                note.GetInk().Show( di.hdc, &di.cellRect );
		    else
                {
                TFont font= (note.IsDone()) ? kStrikeFont : kNormalFont;

                RECT r = di.cellRect;

                //added leading
                r.top    += kTopLeading;
                r.bottom -= kBotLeading;

                if( TSysState::bShowFullText )
                    di.pPainter->Text( di, (char *) note.GetText().Addr(), kNoHilite, kMultiLine, font, &r );
                else
                    di.pPainter->Text( di, buf, kNoHilite, kSingleLine | DT_LEFT, font, &r );
                }
            break;

        case kAmtField:

            if (note.GetValue().HasValue()) {

                TSafStr str;
                TIntlFmt::FmtDollar( note.GetValue(), str, FALSE );
                di.pPainter->Text( di, str, hilite, kSingleLine | DT_RIGHT );
            }
            break;

        case kStartField:
            {
            TIntlFmt::FormatDate( note.GetStart(), kShortDate, buf );
            TIntlFmt::FormatTime( note.GetStart(), FALSE, buf2 );

            di.pPainter->TextLR( di, buf, buf2, hilite );
            }
            break;

        case kEndField:
            {
            TIntlFmt::FormatDate( note.GetEnd(), kShortDate, buf );
            TIntlFmt::FormatTime( note.GetEnd(), FALSE, buf2 );

            di.pPainter->TextLR( di, buf, buf2, hilite );
            }
            break;

        case kWorktimeField:

            if ((note.GetStyle() == kTaskStyle) ||
                (note.GetStyle() == kFloatStyle))
                {

                // worktime in note units
                TWorkTime workTime = note.GetWorkTime();
                workTime.Format( note.GetTimeUnit(),
                    TProfile::bMixTimeFmtUnits, buf );

                // worktime duration in display units
                workTime.Format( 
                    (TTimeScale) TProfile::baseTimeFmtUnits,
                    TProfile::bMixTimeFmtUnits, buf2 );

                //don't show original if they're both the same
                if( !strcmp( buf, buf2 ) )
                    buf3[0]= '\0';
                else
                    wsprintf( buf3, "(%s)", buf );

                di.pPainter->TextLR( di, buf3, buf2, hilite );
                }

            break;


        case kAlarmField:

            PTAlarm alarmP= TSysState::pAlarms->AlarmForNote( note );

            if( alarmP != NULL )
                {
                TIntlFmt::FormatDate( alarmP->GetAlarmTime(), kShortDate, buf );
                TIntlFmt::FormatTime( alarmP->GetAlarmTime(), FALSE, buf2 );

                di.pPainter->TextLR( di, buf, buf2, hilite );
                }
            break;

        case kElapsedField:

            switch (note.GetStyle())
                {

                case kTaskStyle:
                case kFloatStyle:

                    // elapsed duration in note units
                    note.GetDuration().Format(
                        note.GetTimeUnit(), TProfile::bMixTimeFmtUnits, buf );

                    // elapsed duration in preferred units
                    note.GetDuration().Format(
                        (TTimeScale) TProfile::baseTimeFmtUnits,
                        TProfile::bMixTimeFmtUnits, buf2 );

                    //don't show original if they're both the same
                    if( !strcmp( buf, buf2 ) )
                        buf3[0]= '\0';
                    else
                        wsprintf( buf3, "(%s)", buf );

                    di.pPainter->TextLR( di, buf3, buf2, hilite );
                    break;

                default:
                    break;
                }

            break;

        case kDontUseField:
            break;

        case kMarkField:
            di.pPainter->DrawMark( di, note );
            break;

        case kFreqField:

            PTFreq freqP = note.GetFreq();
            if (freqP != NULL) {

                freqP->GetFreqText( buf );
                di.pPainter->Text( di, buf, hilite, kSingleLine | DT_LEFT );
            }
            break;

        case kRateField:

            if (note.GetRate().HasValue()) {

                TSafStr str;
                TIntlFmt::FmtDollar( note.GetRate(), str, FALSE ) ;
                switch( note.GetTimeUnit())
                    {
                    case k1Week:  str.Cat( "/wk" );    break;
                    case k1Day:   str.Cat( "/day" );   break;
                    default:      str.Cat( "/hr" );    break;
                    }
                di.pPainter->Text( di, str, hilite, kSingleLine | DT_RIGHT );
            }
            break;

        case kTimeUnitField:
            {
            TSafStr str;
            TTime::GetUnitName( note.GetTimeUnit(), str ) ;
            di.pPainter->Text( di, str, hilite, kSingleLine | DT_RIGHT );
            }
            break;

        default:

            KSTR errText( "<???>" );
            di.pPainter->Text( di, errText, hilite, kSingleLine | DT_RIGHT );
            break;

    }
}

//*****************************************************************************

//*****************************************************************************
void TReportGrid::PaintHeader( RTDrawInfo di )
{
    TSafStr buf;

    // ONLY in the fixed note column, not all note
    // columns
    if( di.col == 1)
        {
        RTNoteRow noteRow = (RTNoteRow) (*itsNoteTableP)[di.row];
        buf.Set( noteRow.itsHeader );
        di.pPainter->FolderTab( di, FALSE, buf );
        }
    else
        {
        di.pPainter->FolderTab( di, TRUE, buf );
        }
}

//*****************************************************************************
// PaintSummary
//*****************************************************************************
void TReportGrid::PaintSummary( RTDrawInfo di )
{

    char buf[200], buf2[75];

    RTNoteRow noteRow = (RTNoteRow) (*itsNoteTableP)[di.row];
    RTNoteTotal sub= *noteRow.itsSubtotalP;

    switch( itsColumnFields[ di.col ] ) {

        case kTextField:
            KSTR sumText( "Grand total" );


            if( sub.itsLevel == 0 )
                di.pPainter->Text( di, sumText, kNoHilite, kSingleLine | DT_LEFT, kItalicFont );
            else
                {
                sub.Report( di );
                }

            break;

        case kAmtField:
            if( sub.itsValue.HasValue() )
                {
                TSafStr str;
                TIntlFmt::FmtDollar( sub.itsValue, str, FALSE );
                di.pPainter->Text( di, str, kNoHilite, kSingleLine | DT_RIGHT, kItalicFont );
                }
            break;

        case kWorktimeField:

            sub.itsWorkTime.Format( (TTimeScale) TProfile::baseTimeFmtUnits,
                TProfile::bMixTimeFmtUnits, buf );
            di.pPainter->Text( di, buf, kNoHilite, kSingleLine | DT_RIGHT, kItalicFont );
            break;

        case kStartField:
            {
            TIntlFmt::FormatDate( sub.itsFirstTime, kShortDate, buf );
            TIntlFmt::FormatTime( sub.itsFirstTime, FALSE, buf2 );


            di.pPainter->TextLR( di, buf, buf2, kNoHilite, kSingleLine, kItalicFont );
            }
            break;

        case kEndField:
            {
            TIntlFmt::FormatDate( sub.itsLastTime, kShortDate, buf );
            TIntlFmt::FormatTime( sub.itsLastTime, FALSE, buf2 );

            di.pPainter->TextLR( di, buf, buf2, kNoHilite, kSingleLine, kItalicFont );
            }
            break;

        case kElapsedField:
            //I'm not sure what should be in the elapsed field -- the
            // column sum, or the difference between last and first times

            //column sum
            sub.itsDuration.Format( (TTimeScale) TProfile::baseTimeFmtUnits,
                TProfile::bMixTimeFmtUnits, buf );
            di.pPainter->Text( di, buf, kNoHilite, kSingleLine | DT_RIGHT, kItalicFont );

            #if 0
            // elapsed duration, from first to last
            if( sub.itsFirstTime.HasDate() && sub.itsLastTime.HasDate() )
                {
                TDuration dur( sub.itsFirstTime, sub.itsLastTime );

                dur.Format(
                    (TTimeScale) TProfile::baseTimeFmtUnits,
                    TProfile::bMixTimeFmtUnits, buf );

                di.pPainter->Text( di, buf, kNoHilite, kSingleLine | DT_RIGHT, kItalicFont );
                }
            #endif

            break;

        default:
            break;
        }

}

//*****************************************************************************

//*****************************************************************************
void  TReportGrid::MousDblClick( RTMousInfo mi  )
{
if( mi.style != kCellPoint )
    return;

int row= mi.row;
int col= mi.col;

// note the assumption that row 0 is frozen, and that all following
// rows correspond to notes, one row per note

if( col == 1 && IsNoteRow( row ))
    {
    RTNote note = GetNote( row );
    GetApplication()->ExecDialog(new TNoteView(
        TPimFrame::pFrame, IDD_NOTE, &note ));
    InvalidateNote( note );
    }
}


//*****************************************************************************

//*****************************************************************************
void TReportGrid::MousRtClick( RTMousInfo mi )
{

if ( mi.col == 1 && IsNoteRow( mi.row ))
    {
    // popup local note menu (change color, delete, etc. )
    RTNote note = GetNote( mi.row );
    TNoteMenu::PopUp( this, &note, mi.xy.x, mi.xy.y );
    InvalidateNote( note );
    }
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void TReportGrid::InvalidateNote( RTNote note )
{
    // invalidate all rows with this note in them
    for (int i = 0; i < itsRowCount; i++)
        if (IsNoteRow( i ) && note == GetNote( i ))
            InvalidateCell( i, -1 );
}


//*****************************************************************************
// GetNote
//
//  Get the note corresponding to the row
//*****************************************************************************
RTNote TReportGrid::GetNote( int row )
{
if( row > 0 && row < itsRowCount )
    {
    RTNoteRow noteRow = (RTNoteRow) (*itsNoteTableP)[row];
    return *noteRow.itsNoteP;
    }
else
    return (RTNote) NOOBJECT;
}

//*****************************************************************************

//*****************************************************************************
void TReportGrid::MousOver( RTMousInfo mi )
{
if( mi.col == 1 && IsNoteRow( mi.row ))
    TCursor::Set( kNoteSelCursor );
else
    TCursor::Set( kArrowCursor );
}


// -------------------------------------------------------------------------
//
//          Scrolling
//
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

//---------------------------------------------------------------------------
// WMVScroll
//
// Keeps track of the selected note
//---------------------------------------------------------------------------
void TReportGrid::WMVScroll( RTMessage Msg)
{
    TBaseGrid::WMVScroll( Msg );

    // save state
    //FIXME this won't work now that we have subtotals!
    TSysState::currentNoteIndex = vScrollPos;
}

//*****************************************************************************
//set focus to first visible note title
//*****************************************************************************
void TReportGrid::GetDefaultFocusCell( int &row, int &col )
{
row= frozenRows + vScrollPos;                //focus on first visible row!
col= 1;
}

//*****************************************************************************
// only note titles are selectable
//*****************************************************************************
BOOL TReportGrid::IsCellSelectable( int row, int col )
{
return( col == 1 && IsNoteRow( row ));
}


