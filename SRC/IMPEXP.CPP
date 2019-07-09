//*****************************************************************************
// import/export file read/write routines
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "frame.h"
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __intlfmt__
#include "intlfmt.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __parse__
#include "parse.h"
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
#ifndef __note__
#include "note.h"
#endif
#ifndef __csv__
#include "csv.h"
#endif
#ifndef  __cursor__
#include "cursor.h"
#endif
#ifndef  __fieldmap__
#include "fieldmap.h"
#endif


// -----------------------------------------------------------------------
// Export
// -----------------------------------------------------------------------
BOOL TPimFrame::ExportFile( char *fName, BOOL headlinesOnly )
{
    TCursor::Busy( TRUE );

    // open the output file
    ofstream os( fName, ios::trunc );


    // loop through the notes writing them out
    int n= TSysState::pSelNotes->getItemsInContainer();
    for (int i = 0; i < n; i++) {

        RTNote note= ( * TSysState::pSelNotes ) [ i ];
	if( note != NOOBJECT ) {

            ExportNote( os, note, headlinesOnly );
        }
    }

    // close the file
    os.close();
    TCursor::Busy( FALSE );
    return( os.good() );

}

// -----------------------------------------------------------------------
// ExportNote
//
//     write the note's data out as "comma separated values"
// -----------------------------------------------------------------------
int TPimFrame::ExportNote( ofstream &os, RTNote note, BOOL headlinesOnly )
{
    TCSV    csv;
    char    buf[256]; 
    TSafStr valStr;

    int numFields = TFieldMap::gFieldMapP->itsNumExportDataKeys;
    for (int i = 0; i < numFields; i++) {

        BOOL eol = (i == numFields - 1);
        switch( TFieldMap::gFieldMapP->itsExportDataKeys[i] )
            {
            case kColorDataKey:

                csv.WriteInt( os, (int) note.GetColor().Get(), eol );
                break;

            case kTextDataKey:

                if (headlinesOnly)
                    {
                    TSafStr title;
                    note.Title( title );
                    csv.WriteString( os, title, eol );
                    }
                else
                    {
                    csv.WriteString( os, note.GetText(), eol );
                    }
                break;

            case kUnitDataKey:

                csv.WriteInt( os, (int) note.GetTimeUnit(), eol );
                break;

            case kStartDateDataKey:

                TIntlFmt::SetStandardFormats();
                TIntlFmt::FormatDate( note.GetStart(), kShortDate, buf );
                csv.WriteString( os, buf, eol );
                TIntlFmt::ResetFormats();
                break;

            case kStartTimeDataKey:

                TIntlFmt::SetStandardFormats();
                TIntlFmt::FormatTime( note.GetStart(), TRUE, buf, FALSE );
                csv.WriteString( os, buf, eol );
                TIntlFmt::ResetFormats();
                break;

            case kDurationDataKey:

                long minutes = note.GetDuration().GetNumUnits( k1Min );
                wsprintf( buf, "%ld", minutes );
                csv.WriteString( os, buf, eol );
                break;

            case kValueDataKey:

                valStr.Clear();
                if( note.GetValue().HasValue() )
                    TIntlFmt::FmtDollar( note.GetValue(), valStr, FALSE ) ;
                csv.WriteString( os, valStr, eol );
                break;

            case kRateDataKey:

                valStr.Clear();
                if( note.GetRate().HasValue() )
                    TIntlFmt::FmtDollar( note.GetRate(), valStr, FALSE ) ;
                csv.WriteString( os, valStr, eol );
                break;
            }
        }

    return( kCSVOk );
}


// -----------------------------------------------------------------------
// Import
// -----------------------------------------------------------------------
BOOL TPimFrame::ImportFile( char *fName )
{
    TCSV csv;

    TCursor::Busy( TRUE );

    // open the input file
    ifstream is( fName, ios::in );


    // loop through the notes reading them in
    char c;
    int  count = 0;
    while (is.get( c )) {

        is.putback( c );
        TCSVError err = (TCSVError) ImportNote( is, &csv );
		if ((err == kCSVErrorCancel) || (err == kCSVOverflow)) {

            break;
        }
        else if (err == kCSVOk) {

            count++;
        }
    }

    char s[50];
	wsprintf( s, "%d records read.", count );
    TMessageBox::Ok( this, s, "Import" );

    // close the file
    is.close();

    TCursor::Busy( FALSE );
    return( is.good() );

}

// -----------------------------------------------------------------------
// ImportNote
// -----------------------------------------------------------------------
#define CHECK_ERROR( ERROR )             \
                                         \
    switch( ERROR ) {                    \
                                         \
        case kCSVOk:                     \
                                         \
            break;                       \
                                         \
        case kCSVErrorCancel:            \
                                         \
            delete pNote;                \
            return( kCSVErrorCancel  );  \
                                         \
        case kCSVErrorSkip:              \
                                         \
            delete pNote;                \
            return( kCSVErrorSkip  );    \
    }

#define CHECK_EOL_ERROR( LASTFIELD, IS, EOL )         \
    if (LASTFIELD) {                                  \
        CHECK_ERROR( csvP->ShouldBeEOL( IS, EOL ) );  \
    }                                                 \
    else {                                            \
        CHECK_ERROR( csvP->ShouldntBeEOL( IS, EOL )); \
    }

//*****************************************************************************

//*****************************************************************************
int TPimFrame::ImportNote( ifstream &is, PTCSV csvP )
{
    TSafStr    s;
    PTNote     pNote;
    int        x;
    TTime      date;
    TTime      time;
    TValue     value;

    if (!TSysState::pAllNotes->RoomForMore( 1, TRUE, FALSE ))
        {
        return kCSVOverflow;
        }

    if (!TSysState::pCategories->RoomForMore( 1, TRUE, FALSE ))
        {
        return kCSVOverflow;
        }

    // initialize note
    pNote = new TNote();
    pNote->MakeNoteOld();

    // make the note's category "imported," creating "imported" if needed
    RTCat importCat = TSysState::pCategories->FindCatInTopLevel( "imported" );
    if ((RObject) importCat == NOOBJECT)
        {
        PTCat pCat = new TCat;
        pCat->Rename( "imported" );
        TSysState::pCategories->add( *pCat );
        pNote->GetCatList().add( *pCat );
        Notify( kCatNew, NOOBJECT );
        }
    else
        {
        (pNote->GetCatList()).add( importCat );
        }

    BOOL lastField;
    BOOL eol;
    TDuration duration;
    TTime     start;
    TTimeUnit unit       = k1Day;
    BOOL      isWorkTime = FALSE;
    for (int i = 0; i < TFieldMap::gFieldMapP->itsNumImportDataKeys; i++)
        {
        lastField = (i == TFieldMap::gFieldMapP->itsNumImportDataKeys - 1);
        eol = FALSE;
         switch( TFieldMap::gFieldMapP->itsImportDataKeys[i] )
            {
            case kColorDataKey:

                CHECK_ERROR( csvP->ReadEnum( is, kFirstColor, kLastColor, x, eol ))
                CHECK_EOL_ERROR( lastField, is, eol );
                (pNote->GetColor()).Set( (TVirtColor) x);
                break;

            case kTextDataKey:

                CHECK_ERROR( csvP->ReadString( is, s, eol ))
                CHECK_EOL_ERROR( lastField, is, eol );
                pNote->SetText( s );
                break;

            case kUnitDataKey:

                CHECK_ERROR( csvP->ReadEnum( is, k1Min, k1Week, x, eol ))
                CHECK_EOL_ERROR( lastField, is, eol );
                unit = (TTimeUnit) x;
                break;

            case kStartDateDataKey:

                CHECK_ERROR( csvP->ReadDate( is, date, eol ))
                CHECK_EOL_ERROR( lastField, is, eol );
                start.CopyDate( date );
                break;

            case kStartTimeDataKey:

                CHECK_ERROR( csvP->ReadTime( is, time, eol ))
                CHECK_EOL_ERROR( lastField, is, eol );
                start.CopyTime( time );
                break;

            case kDurationDataKey:

                CHECK_ERROR( csvP->ReadDuration( is, duration, eol ))
                CHECK_EOL_ERROR( lastField, is, eol );
                break;

            case kValueDataKey:

                CHECK_ERROR( csvP->ReadValue( is, value, eol ))
                CHECK_EOL_ERROR( lastField, is, eol );
                pNote->SetValue( value );
                break;

            case kRateDataKey:

                CHECK_ERROR( csvP->ReadValue( is, value, eol ))
                CHECK_EOL_ERROR( lastField, is, eol );
                pNote->SetRate( value );
                break;

            case kIgnoreDataKey:

                CHECK_ERROR( csvP->ReadString( is, s, eol ))
                CHECK_EOL_ERROR( lastField, is, eol );
                break;

            }
        }

    // fix up the date if necessary
    if (start.HasTime() && !start.HasDate()) {

       TTime now;
       now.SetNow();
       start.CopyDate( now );
    }

    // fix up the units if necessary
    if (start.HasTime() && (unit >= k1Day)) {

        unit = k1Hour;
    }
    else if ((!start.HasTime()) && (unit < k1Day)) {

        unit = k1Day;
    }

    // set the note's times all at once
    TTimeComplex times;
    times.SetFromRawData( start, duration,
        unit, isWorkTime );
    pNote->SetTimes( times );

    // there's an error if we're at the eol, but we're
    // not out of fields yet
    CHECK_EOL_ERROR( lastField, is, eol );

    //check it
    ASSERT( ERR_GEN, pNote->HealthCheck() );

    // stuff note into global note list
    TSysState::pAllNotes->add( *pNote );
    TSysState::bSystemDirty = TRUE;
    return( kCSVOk );
}

