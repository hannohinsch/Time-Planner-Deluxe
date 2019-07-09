//*****************************************************************************
//  noteview
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "noteview.h"
#include "note.h"
#include "penedit.h"
#include "catdlg.h"

#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __parse__
#include "parse.h"
#endif
#ifndef __oem__
#include "oem.h"
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
#ifndef __catbox__
#include "catbox.h"
#endif
#ifndef __fmtedit__
#include "fmtedit.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __pen__
#include "pen.h"
#endif
#ifndef __freqdlg__
#include "freqdlg.h"
#endif
#ifndef __memnote__
#include "memnote.h"
#endif
#ifndef __gdimap__
#include "gdimap.h"
#endif
#ifndef __fontmap__
#include "fontmap.h"
#endif
#ifndef __utility__
#include "utility.h"
#endif
#ifndef __timeconf__
#include "timeconf.h"
#endif
#ifndef __odbutton__
#include "odbutton.h"
#endif
#ifndef __alarmd__
#include "alarmd.h"
#endif
#ifndef __alarmset__
#include "alarmset.h"
#endif
#ifndef __markctl__
#include "markctl.h"
#endif
#ifndef  __cursor__
#include "cursor.h"
#endif
#ifndef  __addressd__
#include "addressd.h"
#endif
#ifndef  __ourinpd__
#include "ourinpd.h"
#endif
#ifndef  __guessdlg__
#include "guessdlg.h"
#endif
#ifndef  __filter__
#include "filter.h"
#endif
#ifndef  __worktime__
#include "worktime.h"
#endif
#ifndef  __intlfmt__
#include "intlfmt.h"
#endif

#define SHOW_OR_HIDE_TIME                               \
    ShowWindow( itsStartTimeCP->HWindow,                \
        (itsTimeUnit <= k1Hour) ? SW_SHOW : SW_HIDE );  \
    ShowWindow( itsStartTimeTitleCP->HWindow,           \
        (itsTimeUnit <= k1Hour) ? SW_SHOW : SW_HIDE );  

PTNoteView TNoteView::CurrentNoteViewP = NULL;


// ------------------------------------------------------------------------------------------------------------------------------------------
// --------------                                    ----------------------------------------------------------------------------------------                                                                      ------------------
// --------------  Constructor/Destructor Functions  ----------------------------------------------------------------------------------------                                                                      ------------------
// --------------                                    ----------------------------------------------------------------------------------------                                                                      ------------------
// ------------------------------------------------------------------------------------------------------------------------------------------

//*****************************************************************************
// Constructor
//*****************************************************************************
TNoteView::TNoteView(PTWindowsObject AParent, WORD AResID, PTNote pANote ) :
		   TOurDialog( AParent, AResID )
{

    itsHelpTopic= _HelpNoteDialog;

    //set busy cursor; turned off in setupwindow()
    TCursor::Busy( TRUE );

    // initialize instance variables
    itsNoteP     = pANote;
    itsDirty     = FALSE;
    itsInkMode   = itsNoteP->GetInkMode() && TPen::bPenActive;
    itsTimeUnit  = itsNoteP->GetTimeUnit();
    itsCatListP  = new TCatList( itsNoteP->GetCatList() );
    itsStyle     = itsNoteP->GetStyle();
    itsInk       = itsNoteP->GetInk();
    itsColor     = itsNoteP->GetColor();

    // copy the note's alarm into the noteview
    PTAlarm notesAlarmP = TSysState::pAlarms->AlarmForNote( *itsNoteP );
    itsAlarmP = (notesAlarmP == NULL) ? NULL : new TAlarm( *notesAlarmP );

    // initialize the on-screen items
    InitializeNoteText();
    InitializeButtons();
    InitializeSmartControls();
    InitializeCatBox();
    InitializeMisc();

}


// ------------------------------------------------------------------------------------------------------------------------------------------
// --------------                                    ----------------------------------------------------------------------------------------                                                                      ------------------
// --------------      Initialization Functions      ----------------------------------------------------------------------------------------                                                                      ------------------
// --------------                                    ----------------------------------------------------------------------------------------                                                                      ------------------
// ------------------------------------------------------------------------------------------------------------------------------------------


//*****************************************************************************
// InitializeButtons
//
// create the buttons and check boxes (all of which are transferless)
//*****************************************************************************
void TNoteView::InitializeButtons( void )
{
    // buttons that are referred to elsewhere
    itsRecognizeCP     = new TButton( this, IDD_NOTERECOG );
    itsInkToggleCP     = new TCheckBox( this, IDD_NOTEINK, NULL );
    itsClearCP         = new TButton( this, IDD_NOTECLEAR );

    itsDoneToggleCP    = new TCheckBox( this, IDD_NOTEDONE, NULL );

    //note shape
    itsMarkCP= new TMarkCtl( this, IDD_NOTEMARK );
    itsMarkCP->Set( itsColor, itsStyle );

}

//*****************************************************************************
// InitializeSmartControls
//
// Initialize the controls that are based on TPenEdit and are "smart"
// because they contain their data themselves. Each of these items
// disables itself in its constructor, so we needn't do it here.
//*****************************************************************************
void TNoteView::InitializeSmartControls( void )
{
    TTime start( itsNoteP->GetStart());
    TTimeOnlyMode mode;

    if (start.HasDate())
        {
        mode = start.HasTime() ? kMustHaveTime : kCantHaveTime;
        }
    else
        {
        mode = kCanHaveTime;
        }
         
    itsStartDateCP = new TDateOnlyEdit(     this, IDD_NOTE_START_DATE,    start );
    itsStartTimeCP = new TTimeOnlyEdit(     this, IDD_NOTE_START_TIME,    start, mode );
    itsEndCP       = new TTimeStatic(     this, IDD_NOTEEND,      itsNoteP->GetEnd() );
    itsDurationCP  = new TDurationStatic( this, IDD_NOTEDURATION, itsNoteP->GetDuration(), itsNoteP->GetTimeUnit() );
    itsRateCP      = new TValueEdit(    this, IDD_NOTERATE,     itsNoteP->GetRate() );
    itsValueCP     = new TValueEdit(    this, IDD_NOTEVALUE,    itsNoteP->GetValue() );
    itsStartTimeTitleCP = new TStatic( this, IDD_NOTE_START_TIME_TITLE, 256 );


}

//*****************************************************************************
// InitializeNoteText
//
// Initialize the note text. Note has an unlimited length
//*****************************************************************************
void TNoteView::InitializeNoteText( void )
{
    itsTextCP = new TPenEdit( this, IDD_NOTEEDIT, &itsInk, itsInkMode );

}


//*****************************************************************************
// InitializeCatBox
//
// Initialize the category box
//*****************************************************************************
void TNoteView::InitializeCatBox( void )
{

    // initialize the cat box's data
    pCatBoxData = new TCatBoxData;
    pCatBoxData->Prepare( itsCatListP );

    // initialize the cat box itself
    itsCatBoxCP = new TCatBox( this, IDD_NOTELIST );

    // initialize the title
    itsCatTitleCP = new TBStatic( this, IDD_NOTECATTITLE, 50 );
}

//*****************************************************************************
// InitializeMisc
//
// Initialize the other controls
//*****************************************************************************
void TNoteView::InitializeMisc( void )
{
    // address and category buttons
    itsAddressCP = new TODTextButton( this, IDD_NOTE_ADDRESS );
    itsCategoryCP = new TODTextButton( this, IDD_NOTECATMGR );

    // duration scrollbar
    itsDurationScrollCP = new TScrollBar( this, IDD_NOTEDURATION_SCROLL );

    // duration units box
    itsDurationBoxCP= new TComboBox( this, IDD_NOTEUNITS, MAX_UNIT_LEN );

    // color buttons
    new TODColorBitMapButton( this, IDD_NOTERED, kRed );
    new TODColorBitMapButton( this, IDD_NOTEREDGRAY, kRedGray );
    new TODColorBitMapButton( this, IDD_NOTEGRAY, kGray );
    new TODColorBitMapButton( this, IDD_NOTEBLUEGRAY, kBlueGray );
    new TODColorBitMapButton( this, IDD_NOTEBLUE, kBlue );

    // frequency static
    itsFrequencyCP = new TStatic( this, IDD_FREQTEXT, MAX_FREQ_LEN );
    itsFrequencyTitleCP= new TStatic( this, IDD_FREQSTATIC, MAX_FREQ_LEN );

    // initialize the frequency from the note, but if the note has a freq
    // then make the noteview's version just a copy of it
    itsFreqP = itsNoteP->GetFreq();
    if (itsFreqP != NULL) {

        itsFreqP = new TFreq( *itsFreqP );
    }

    itsAlarmIconCP      = new TBButton( this, IDD_ALARM_ICON );
    itsRateUnitsCP      = new TStatic( this, IDD_RATEUNITS, 100 );
    itsMoreBtnCP        = new TButton( this, IDD_NOTEMORE );
    itsMoreGroupCP      = new TGroupBox( this, IDD_MOREBOX );
    itsRateTitleCP      = new TStatic( this, IDD_RATE_TITLE, 100 );
    itsValueTitleCP     = new TStatic( this, IDD_AMOUNT_TITLE, 100 );

    itsWorkElapsedCP    = new TComboBox( this, IDD_NOTEWORKELAPSED, MAX_UNIT_LEN );
}


// ------------------------------------------------------------------------------------------------------------------------------------------
// --------------                                    ----------------------------------------------------------------------------------------                                                                      ------------------
// --------------  Inherited Dialog/Setup Functions  ----------------------------------------------------------------------------------------                                                                      ------------------
// --------------                                    ----------------------------------------------------------------------------------------                                                                      ------------------
// ------------------------------------------------------------------------------------------------------------------------------------------


//*****************************************************************************
// SetupWindow
//*****************************************************************************
void TNoteView::SetupWindow()
{
    // inherited
    TOurDialog::SetupWindow();

    SetupInkControls();
    SetupTimeDurationControls();
    SetupMiscControls();

    //setup catbox here
    // note that we pass address of pointer!
    itsCatBoxCP->Transfer( &pCatBoxData, TF_SETDATA );

    // show address button
    if (TOEM::bAddressBook)
        {
        ShowWindow( itsAddressCP->HWindow, TRUE );
        ShowWindow( itsCategoryCP->HWindow, TRUE );
        HFONT hFont= TFontMap::GetScreenFont( kNormalFont );
        SendMessage( itsAddressCP->HWindow, WM_SETFONT,
            hFont, 0 );
        SendMessage( itsCategoryCP->HWindow, WM_SETFONT,
            hFont, 0 );

        }
    else
        {
        RECT    addR, catR;

        // resize the category button
        GetWindowRect( itsAddressCP->HWindow, &addR );
        GetWindowRect( itsCategoryCP->HWindow, &catR );
        SetWindowPos( itsCategoryCP->HWindow, 0, 0, 0,
            addR.right - catR.left,
            catR.bottom - catR.top,
            SWP_NOMOVE | SWP_NOZORDER );
        ShowWindow( itsCategoryCP->HWindow, TRUE );
        }

    //cover up controls in moregroup
    SetDlgSize( FALSE );

    CurrentNoteViewP = this;

    //Set selection of text edit control to -1,-1 (no selection)
    //For reasons I don't pretend to understand, simply calling
    // itsTextCP->SetSelection( -1, -1 ) here does not work.
    // Instead, reset the focus to NULL, then let the
    // setfocus handler in itsTextCP take care of the SetSelection() call
    // (dialog proc will reset focus to first control)
    // Mysterious.
    SetFocus( NULL );

    // turn off hourglass (was turned on in constructor)
    TCursor::Busy( FALSE );

}


//*****************************************************************************
// SetupInkControls
//
// Setup the various ink related controls now that they are actually
// created and about to go on the screen
//*****************************************************************************
void TNoteView::SetupInkControls()
{
    //set ink toggle and buttons
    if( TPen::bPenActive ) {

        if( itsInkMode ) {

            itsInkToggleCP->Check();
        }
    }
    else {
        itsInkToggleCP->Show( SW_HIDE );
        itsRecognizeCP->Show( SW_HIDE );
        itsClearCP->Show( SW_HIDE );
    }
}

//*****************************************************************************
// SetupTimeDurationControls
//
// Setup the various time/duration related controls now that they are actually
// created and about to go on the screen
//*****************************************************************************
void TNoteView::SetupTimeDurationControls()
{
    KSTR elapsed( "Elapsed Time" );
    KSTR work( "Work Time" );

    itsWorkElapsedCP->AddString( SAF2CP( elapsed ));
    itsWorkElapsedCP->AddString( SAF2CP( work ));
    itsWorkElapsedCP->SetSelIndex( itsNoteP->IsWorkTime() ? 1 : 0 );

    // initialize the units box (manually because of Borland combo box bug)
    TSafStr s;
    TTime::GetUnitName( k1Min, s );
    itsDurationBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k5Min, s );
    itsDurationBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k6Min, s );
    itsDurationBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k10Min, s );
    itsDurationBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k12Min, s );
    itsDurationBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k15Min, s );
    itsDurationBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k20Min, s );
    itsDurationBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k30Min, s );
    itsDurationBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k1Hour, s );
    itsDurationBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k1Day, s );
    itsDurationBoxCP->AddString( SAF2CP( s ) );

    TTime::GetUnitName( k1Week, s );
    itsDurationBoxCP->AddString( SAF2CP( s ) );

    // figure out what which time units to use
    itsDurationBoxCP->SetSelIndex( itsNoteP->GetTimeUnit() );

    // reinitialize the penedit doodads, now that the controls are created
    itsTextCP->Set( itsNoteP->GetText());

    itsStartDateCP->Set(  itsNoteP->GetStart() );
    itsStartTimeCP->Set(  itsNoteP->GetStart() );
    itsEndCP->Set(        itsNoteP->GetEnd() );
    itsRateCP->Set(       itsNoteP->GetRate() );
    itsValueCP->Set(      itsNoteP->GetValue() );

    // since the duration is always elapsed time,
    // convert to work time if necessary
    if (itsNoteP->IsWorkTime()) {

         TWorkTime workTime = itsNoteP->GetWorkTime();
         long days = workTime.GetNumUnits( itsTimeUnit );
         itsDurationCP->Set( itsTimeUnit, days );
    }
    else {

         itsDurationCP->Set(   itsNoteP->GetDuration() );
    }

    // tell the start and end times that they use or don't use hours
    if (itsStartTimeCP->HasTime())
        {
        itsStartTimeCP->SetMode( itsStartDateCP->HasDate(),
            itsTimeUnit <= k1Hour);
        if( itsNoteP->IsWorkTime() )
            {
            TTime start, end;
            start.SetHour( TProfile::WorkDayStartMinute / 60 );
            start.SetMinute( TProfile::WorkDayStartMinute % 60 );
            end.SetHour( TProfile::WorkDayEndMinute / 60 );
            end.SetMinute( TProfile::WorkDayEndMinute % 60 );
            itsStartTimeCP->SetRange( start, end );
            }
        }
    SHOW_OR_HIDE_TIME
    itsEndCP->UsesHours(   itsTimeUnit <= k1Hour );

    UpdateRateUnits();     //based on itsTimeUnits

    // setup the duration scroll bar
    itsDurationScrollCP->SetRange( 0, 10000 );
    itsDurationScrollCP->SetPosition( 0 );
    UpdateScrollPosition();

    // set note mark
    itsMarkCP->Set( itsColor, itsStyle );
}

//*****************************************************************************
// SetupMiscControls
//
// Setup the other controls
//*****************************************************************************
void TNoteView::SetupMiscControls( void )
{
    char buf[100];

    // set a larger font in main text window and cat title window
    SendMessage( itsTextCP->HWindow, WM_SETFONT,
        (itsNoteP->IsDone()) ? TFontMap::GetScreenFont( kBigStrikeFont ):
                           TFontMap::GetScreenFont( kBigFont ),
        (DWORD) 0 );

    SendMessage( itsCatTitleCP->HWindow, WM_SETFONT,
        TFontMap::GetScreenFont( kBigFont ), (DWORD) 0 );

    // initialize the cat title window
    wsprintf( buf, "%d assigned", itsCatListP->getItemsInContainer() );
    itsCatTitleCP->SetCaption( buf );

    // 'done' check box
    itsMarkedDone = itsNoteP->IsDone();
    itsDoneToggleCP->SetCheck( (itsMarkedDone)? BF_CHECKED: BF_UNCHECKED );

    // alarm text
    itsAlarmIconCP->Show( (itsAlarmP == NULL) ? SW_HIDE : SW_SHOW );

    // initialize freq text and related fields
    UpdateFrequency();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////// validation and dialog close ////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
// Destructor
//*****************************************************************************
TNoteView::~TNoteView()
{

delete pCatBoxData;
delete itsCatListP;

if (itsAlarmP != NULL)
    {
    delete itsAlarmP;
    }

if (itsFreqP != NULL)
    delete itsFreqP;
}


//*****************************************************************************
// IsOkToSave
//  called by Ok() and MemorizeNote()
//*****************************************************************************
BOOL TNoteView::IsOkToSave( void )
{
    if( !itsValueCP->Validate() )
        return FALSE;

    if( !itsRateCP->Validate() )
        return FALSE;

    if( !itsStartDateCP->Validate() )
        return FALSE;

    if( !itsStartTimeCP->Validate() )
        return FALSE;

    if( 0 == itsCatListP->getItemsInContainer() )
        {
        if( IDCANCEL == TMessageBox::OkAndCancel( this,
            "Because this note has not been assigned to any folders, it will automatically be put into the <unfiled> folder.",
            "No Folders Assigned" ))
                {
                return FALSE;
                }
        }

    TTime start = itsStartDateCP->Get();
    start.CopyTime( itsStartTimeCP->Get() );
    TTime end   = itsEndCP->Get();
    if (TNoteSet::CancelBecauseOfTimeConflict( *itsNoteP, start, end )) {

        return FALSE;
    }


    // if there is no frequency, or it's kOnce, then just return
    if ((itsFreqP == NULL) || !itsFreqP->IsRealFreq()) {

        return TRUE;
    }


    // first check: must be in hours if there's a frequency
    if (!itsStartTimeCP->HasTime()) {

        TMessageBox::Ok( this,
            "Notes with frequencies cannot be in days or weeks.",
            "Frequency Restriction" );
        return FALSE;
    }


    // second check: must start and end on one day if there's a freq
    if (!start.IsSameDay( end )) {

        TMessageBox::Ok( this,
            "Notes with frequencies must start and end on the same day.",
            "Frequency Restriction" );
        return FALSE;
    }

    // third check: can't change start day without invalidating freq
    TTime freqStart;
    itsFreqP->GetStart( freqStart );
    if (!start.IsSameDay( freqStart )) {

        TMessageBox::Ok( this,
            "Since you changed the note's start day, you must also modify the frequency before closing the note.",
            "Frequency Restriction" );
        return FALSE;
    }

    return TRUE;
}



//*****************************************************************************
// CheckDirtyFlags
//
// Check the PenEdit fields to see if the user typed something
// Updates itsDirty flag
//*****************************************************************************
void TNoteView::CheckDirtyFlags( void )
{
    itsDirty |= itsTextCP->IsModified();
    itsDirty |= itsDurationCP->IsModified();

    itsDirty |= itsStartDateCP->IsModified();
    itsDirty |= itsStartTimeCP->IsModified();
    itsDirty |= itsEndCP->IsModified();

    itsDirty |= itsValueCP->IsModified();
    itsDirty |= itsRateCP->IsModified();
}

//*****************************************************************************
// is the noteview (not the note itself) in worktime?
//*****************************************************************************
BOOL TNoteView::IsWorkTime( void )
{
return itsWorkElapsedCP->GetSelIndex() == 1;
}

//*****************************************************************************
// ShutDownWindow
//
//  Overrides TDialog's ShutDownWindow in order to test if we're allowed
//  to close this dialog yet. Will be called when the cancel button, or
//  the windows close box is clicked, or the <esc> key is pressed.
//
//  If this is a "new note" (ie. it was created just before the noteview
//  dialog was put up) then before closing the dialog, delete itsNoteP
//*****************************************************************************
void TNoteView::ShutDownWindow( void )
{
    BOOL shutDown = FALSE;

    CheckDirtyFlags();

    // if there's no note at all then just shut down
    if (itsNoteP == NULL) {

        shutDown = TRUE;
    }

    // if the note's not dirty, but it was a new note, then destroy it
    else if (!itsDirty) {

        // destroy the note if it's new
        if (itsNoteP->IsANewNote()) {

            DestroyNote( FALSE );
        }

        shutDown = TRUE;
    }

    // if the note was dirty, then cancel changes
    // only if the user agrees ( ...and destroy if it was new note )
    else if( IDOK == TMessageBox::OkAndCancel( this,
            "This note has changed.\n\nLose changes?",
            "Confirm Cancel" )) {

        // destroy the note if it's new
        if (itsNoteP->IsANewNote()) {

            DestroyNote( FALSE );
        }

        shutDown = TRUE;
    }

    if (shutDown) {

        // let the address mgr know that we're
        // finished. Can't do this in dispose
        // because dispose wouldn't get called
        // until after the dispose for the address
        // mgr dialog underneath
        CurrentNoteViewP = NULL;

        TDialog::ShutDownWindow();

        ShowAddressDialog();
    }
}

void TNoteView::ShutDownWindow( int returnValue )
{
    TDialog::ShutDownWindow( returnValue );
    ShowAddressDialog();
}

//*****************************************************************************
// ShowAddressDialog
//
//  Select the address dialog if there is one
//*****************************************************************************
void TNoteView::ShowAddressDialog( void )
{
    if (TAddressDialog::GetWindowHandle())
        {
        // enable the address dialog, disable the frame
        // make the address dialog active
        EnableWindow(
            TAddressDialog::GetWindowHandle(),TRUE );
        EnableWindow(
            TPimFrame::pFrame->HWindow, FALSE );
        SetActiveWindow(
            TAddressDialog::GetWindowHandle() );
        }
    else
        {
        // enable the frame (it might have been
        // disabled when closing an address dialog)
        EnableWindow(
            TPimFrame::pFrame->HWindow, TRUE );
        }
}

//*****************************************************************************
// DestroyNote
//
//  Pull itsNoteP out of the global list and destroy it.
//*****************************************************************************
void TNoteView::DestroyNote( BOOL notify )
{
    // remove the alarm for the note
    DestroyNotesAlarm( itsNoteP );

    // this order is important: detach, notify, delete
    TSysState::pAllNotes->detach( *itsNoteP );
    if (notify) {

        Notify( kNoteChg, NOOBJECT );
    }

    delete itsNoteP;
    itsNoteP= NULL;

}

// ------------------------------------------------------------------------------------------------------------------------------------------
// --------------                                    ----------------------------------------------------------------------------------------                                                                      ------------------
// --------------      Buffer To Note Functions      ----------------------------------------------------------------------------------------                                                                      ------------------
// --------------                                    ----------------------------------------------------------------------------------------                                                                      ------------------
// ------------------------------------------------------------------------------------------------------------------------------------------


//*****************************************************************************
// set values into note object
//
// recalculate the endText before calling this method: it just transfers
// whatever is already there into the note's end time
//*****************************************************************************
void TNoteView::BufferToNote( PTNote noteP )
{
    SmartControlsToNote( noteP );
    MiscControlsToNote( noteP );
    AlarmToNote( noteP );

    // no matter what happened, there's no need to see the rate message
    // next time we open a view on this note
    noteP->ClearRateFlag();
}


//*****************************************************************************
// SmartControlsToNote
//
// Copy the information found in the "smart" controls (those that contain
// their own data because they derive from TPenEdit) back into the note
//*****************************************************************************
void TNoteView::SmartControlsToNote( PTNote noteP )
{
    TTime start( itsStartDateCP->Get() );
    start.CopyTime( itsStartTimeCP->Get() );

    TDuration duration;

    #if 0
    TTime end( itsEndCP->Get() );
    if (IsWorkTime() && end.HasDate() ) {

        duration.Set( start, end );
    }
    else {

        duration = itsDurationCP->Get();
    }
    #else       //now pass in worktime duration directly
        duration = itsDurationCP->Get();
    #endif

    noteP->SetFromRawData( start, duration,
        itsTimeUnit, IsWorkTime() );


    if (IsWorkTime() && start.HasDate() ) {

       BOOL changed = !start.IsWorkDay();
       changed     |= start.HasTime() && !start.IsWorkTime();
       if (changed) {

           TMessageBox::Ok( this, "Note's times will be moved forward so that they fall during work time.",
               "Changing Times" );
       }
    }

    // we shouldn't save Value yet because the time units may
    // have changed, and we haven't saved them just yet (and
    // they affect the value calculation)
//     noteP->SetValue(    itsValueCP->Get());

    noteP->SetRate(     itsRateCP->Get());

}


//*****************************************************************************
// MiscControlsToNote
//
// Put other controls' information back into real note
//*****************************************************************************
void TNoteView::MiscControlsToNote( PTNote noteP )
{
    // update category list....
    pCatBoxData->Store( &noteP->GetCatList() );

    // save the units
    noteP->SetTimeUnit( itsTimeUnit );

    //the Value is in a nominally "smart" control, but we can't
    //save until after the time units have been saved, else
    //it'll be improperly calculated
    noteP->SetValue(    itsValueCP->Get());

    // set the note's text
    //FIXME as a side-effect, GetString() makes sure the pInk pointer
    // in fact points to the current ink.  This is terrible.  The
    // penEdit object should have a local copy of its ink, as it
    // has of all its other data!      //hmh
    noteP->SetText( itsTextCP->GetString() );

    // save the ink and ink mode
    itsInkMode = itsTextCP->IsInkMode();    //special handling
    noteP->SetInkMode( itsInkMode );
    noteP->SetInk( itsInk );

    // save the color
    noteP->SetColor( itsColor );

    // check the done cat
    noteP->SetDone( itsMarkedDone );

    // save the frequency
    if (itsFreqP != NULL) {

        // if the start time is not the same as the freq's start time,
        // then update the freq's start time
        TTime start = itsStartDateCP->Get();
        start.CopyTime( itsStartTimeCP->Get() );
        TTime freqStart;
        itsFreqP->GetStart( freqStart );
        if (start != freqStart && itsFreqP->IsRealFreq() ) {

            // since we don't allow the start and freqStart to be on
            // different days in IsOkToSave, they shouldn't be different here...
            ASSERT_STR( "MiscControlsToNote: frequency error",
                start.IsSameDay( freqStart ));

            itsFreqP->SetTime( start );
        }

        // save the frequency into the note
        noteP->SetFreq( *itsFreqP );
    }

    // style
    UpdateNoteStyle();
}

//*****************************************************************************
// AlarmToNote
//
// copy the noteview's alarm into the note
//*****************************************************************************
void TNoteView::AlarmToNote( PTNote noteP )
{
    // destroy the note's old alarm
    DestroyNotesAlarm( noteP );

    if ((noteP->GetStyle() == kItemStyle) || (noteP->GetStyle() == kFloatStyle)) {
        return;
    }

    if (itsAlarmP != NULL) {

        // the start time of the note might have changed
        // since the note was created, so update the noteview's alarm time
        // BTW, the note's freq and dates should be consistent with one
        // another by the time we get here (checked in IsOkToSave() )
        TTime start( itsStartDateCP->Get() );
        start.CopyTime( itsStartTimeCP->Get() );

        itsAlarmP->SetBaseTime( start, itsFreqP );

        //add to global set
        PTAlarm newAlarmP = new TAlarm( *itsAlarmP );
        TSysState::pAlarms->add( *newAlarmP );
    }
}


//*****************************************************************************
// DestroyNotesAlarm
//
// Throw away the alarm in the note
//*****************************************************************************
void TNoteView::DestroyNotesAlarm( PTNote noteP )
{
    PTAlarm notesAlarmP = TSysState::pAlarms->AlarmForNote( *noteP );

    if (notesAlarmP != NULL) {

        TSysState::pAlarms->detach( *notesAlarmP );
        delete notesAlarmP;
    }
}


// ------------------------------------------------------------------------------------------------------------------------------------------
// --------------                                    ----------------------------------------------------------------------------------------                                                                      ------------------
// --------------         Button Handlers            ----------------------------------------------------------------------------------------                                                                      ------------------
// --------------                                    ----------------------------------------------------------------------------------------                                                                      ------------------
// ------------------------------------------------------------------------------------------------------------------------------------------

//*****************************************************************************
// OK
//*****************************************************************************
void TNoteView::Ok( RTMessage  )
{
    if (IsOkToSave()) {

        // don't just call TDialog::Ok because it will call CanClose().
        // Shutting down the window before transferring
        // the data from the buffer to the note makes
        // sure that KILLFOCUS is sent to whatever
        // has the focus and ensures that the values
        // will be updated even after typing in a field
        // and then pressing the enter button.
        // Since the KILLFOCUS handlers all transfer
        // data from the windows controls into the
        // objects, we don't care that the windows
        // controls have been destroyed when we use
        // the data in the objects.
        TDialog::ShutDownWindow( IDOK );

	    BufferToNote( itsNoteP );

        itsNoteP->MakeNoteOld(); //no longer a "new" note

        ASSERT( ERR_GEN, itsNoteP->HealthCheck());

        // let the address mgr know that we're
        // finished. Can't do this in dispose
        // because dispose wouldn't get called
        // until after the dispose for the address
        // mgr dialog underneath
        CurrentNoteViewP = NULL;

        Notify( kNoteChg, NOOBJECT );
    }
}

//*****************************************************************************

//*****************************************************************************
void TNoteView::NoteMore( RTMessage )
{
    SetDlgSize( TRUE );
    //don't allow a second press
    EnableWindow( itsMoreBtnCP->HWindow, FALSE );
}


//*****************************************************************************
// user pressed Guess button: parse text, fill in note fields
// NB now restricted to guess only category matches (using cat keywords)
//*****************************************************************************
void TNoteView::Guess( RTMessage )
{
PTCatList   pGuessList;
char buf[255];

RTSafStr safStr = itsTextCP->GetString();

//create catlist based on keywords
pGuessList = new TCatList( (char *) (const char *) safStr );

// do guess dialog w/ parsed values
if( IDOK == GetApplication()->ExecDialog(
    new TGuessDialog( this, IDD_CHKGUESS, pGuessList, itsCatListP ) ) )
    {
    //update the listbox
    pCatBoxData->Prepare( itsCatListP );
    InvalidateRect( itsCatBoxCP->HWindow, NULL, TRUE );

    wsprintf( buf, "%d assigned", itsCatListP->getItemsInContainer() );
    itsCatTitleCP->SetCaption( buf );

    itsDirty = TRUE;
    }

delete pGuessList;
}

//*****************************************************************************
// toggle between text and ink mode
//*****************************************************************************
void TNoteView::NoteInk( RTMessage )
{
// NB GetCheck() returns state of button after click has already changed it
if( BF_CHECKED == itsInkToggleCP->GetCheck() )
    itsTextCP->SetInkMode();
else
    itsTextCP->SetTextMode();

}

//*****************************************************************************
// ink recognition
//*****************************************************************************
void TNoteView::NoteRecog( RTMessage )
{
itsInkToggleCP->Uncheck();
itsTextCP->Recognize();
itsDirty = TRUE;
}

//*****************************************************************************
// clear the ink and text in a note (leave fields as is)
//*****************************************************************************
void TNoteView::NoteClear( RTMessage )
{
itsTextCP->Clear();
itsDirty = TRUE;
}

//*****************************************************************************
// CatMgr
//
//  Opens the cat mgr from the short category button
//*****************************************************************************
void TNoteView::CatMgr( RTMessage )
{
GetApplication()->ExecDialog(new TCatMgr( this, IDD_CATMGR ));
}

//*****************************************************************************
// open address mgr dialog
//*****************************************************************************
void TNoteView::NoteAddress( RTMessage )
{
// if the address window is already open then just
// pop it to the top
HANDLE window = TAddressDialog::GetWindowHandle();
if (window != NULL)
    {
    SetWindowPos( window, HWND_TOP, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
    }

// otherwise open the address window
else
    {
    GetApplication()->ExecDialog(new TAddressDialog(
        TPimFrame::pFrame, IDD_ADDRESS ));
    }
}

//*****************************************************************************
// AlarmButton
//
// Modify the alarm for the note
//*****************************************************************************
void TNoteView::AlarmButton( RTMessage )
{
    // check for a start time
    if (!itsStartDateCP->HasDate()) {

        TMessageBox::Ok( this,
            "Set a start time before activating an alarm.",
            "Missing start time" );
        return;
    }

    // create a temporary alarm based on the noteview's alarm
    // (if the note has no alarm, the noteview might also have none)
    TTime start( itsStartDateCP->Get() );
    start.CopyTime( itsStartTimeCP->Get() );

    TAlarm alarm( itsNoteP, start, itsFreqP );
    if (itsAlarmP != NULL) {

        alarm = *itsAlarmP;
    }


    switch(GetApplication()->ExecDialog(new TAlarmDialog( this, &alarm ))) {

        case IDCANCEL:    // delete the noteview's alarm

            itsAlarmIconCP->Show( SW_HIDE );
            if (itsAlarmP != NULL) {

                delete itsAlarmP;
                itsAlarmP = NULL;
            }
            break;

        case IDOK:       // copy the temporary alarm into the noteview

            itsAlarmIconCP->Show( SW_SHOW );
            if (itsAlarmP == NULL) {

                itsAlarmP = new TAlarm( alarm );
            }
            else {

                *itsAlarmP = alarm;
            }
            break;
    }

    itsDirty= TRUE;
}

//*****************************************************************************
// Template
//
// Memorize this note (create template)
//*****************************************************************************
void TNoteView::NoteTemplate( RTMessage )
{
    TSafStr name;
    BOOL wasSaved= FALSE;

    //make sure note is logically consistent
    if( !IsOkToSave() )
        return;

    // get a name for this memorized note
    TOurInputDialog *aDialog = new TOurInputDialog( this, "Memorizing Note Template",
        "Enter name for note template:", name, _HelpTemplate );
    if (GetApplication()->ExecDialog( aDialog ) != IDOK)
        return;

    if (name.IsBlank() )
        {
        TMessageBox::Ok( this,
            "Blank names are not allowed.",
            "Bad Template Name" );
        return;
        }

    // create a new note and update it from the controls
    PTNote newNoteP = new TNote();
    BufferToNote( newNoteP );

    // create a MemorizedNote and initialize it with the new note and name
    TMemorizedNote *newMemP = new TMemorizedNote( newNoteP, name );

    //see if we already have note w/ that name
    RTMemorizedNote oldMem = TSysState::pMemNotes->FindNote( *newMemP );

    // if no other memorized note with this name exists,
    if (oldMem == NOOBJECT)
        {
        // ...add it to the memorized list
        TSysState::pMemNotes->add( *newMemP );
        wasSaved= TRUE;
        }
    else  // another one with this name DOES exist: ask for more instructions
        {
        // note should be replaced?
        if( IDOK == TMessageBox::OkAndCancel( this,
            "There is already a note template with this name.\n\nReplace it?",
            "Replace Memorized Note" ))
            {
            // YES, replace old note with new
            TSysState::pMemNotes->detach( oldMem, TXArray::kDelete );
            TSysState::pMemNotes->add( *newMemP );
            wasSaved= TRUE;
            }
        else
            {
            // NO, delete new note and memorized note
            delete newMemP;
            }
        }

    // delete the temporary note (memorized note creates its own copy)
    delete newNoteP;

    if( wasSaved )      //shut down noteview
        {
        // don't just call TDialog::Ok because it will call CanClose().
        // Shutting down the window before transferring
        // the data from the buffer to the note makes
        // sure that KILLFOCUS is sent to whatever
        // has the focus and ensures that the values
        // will be updated even after typing in a field
        // and then pressing the enter button.
        // Since the KILLFOCUS handlers all transfer
        // data from the windows controls into the
        // objects, we don't care that the windows
        // controls have been destroyed when we use
        // the data in the objects.
        TOurDialog::ShutDownWindow( IDOK );

        // let the address mgr know that we're
        // finished. Can't do this in dispose
        // because dispose wouldn't get called
        // until after the dispose for the address
        // mgr dialog underneath
        CurrentNoteViewP = NULL;
        }

}

//*****************************************************************************
// delete this note altogether
//*****************************************************************************
void TNoteView::NoteDelete( RTMessage Msg )
{
if( IDOK == TMessageBox::OkAndCancel( this,
    "Deleting this note will remove it from all views and folders!",
    "Confirm Note Deletion" )) {

        DestroyNote( TRUE );
        TDialog::Cancel( Msg );
    }
}


// -------------------------------------------------
//  Start Time Picker
//
//	Handle the StartTime button by putting up the
//	TimePicker dialog box
// -------------------------------------------------
void TNoteView::StartTimeButton( RTMessage )
{
#if 0   //obsolete
	TTimePicker     	*aDialog;
	TTime				startTime;
	TTime				endTime;
    TDuration           duration;
    TTimeUnit           unit = (TTimeUnit) itsDurationBoxCP->GetSelIndex();

	// put up the time picker dialog
    startTime = itsStartDateCP->Get();
    startTime.CopyTime( itsStartTimeCP->Get() );
    endTime   = itsEndCP->Get();

    // the timepicker doesn't know how to deal with
    // worktime, so make sure the duration is the
    // elapsed time
    if (startTime.HasDate()) {

        duration.Set( startTime, endTime );
    }
    else {

        duration  = itsDurationCP->Get();
    }
	aDialog = new TTimePicker( this, IDD_TIMEPICKER, "Start time",
        &startTime, &endTime, itsFreqP, &duration, &unit, itsColor );
	if( IDOK == GetApplicationObject()->ExecDialog( aDialog ) )
	{
        // the timepicker doesn't handle worktime well
        // calculate work time from the start/end
        if (startTime.HasDate() && itsNoteP->IsWorkTime()) {

            TWorkTime worktime( startTime, endTime );
            long units = worktime.GetNumUnits( itsTimeUnit );
            duration.Set( itsTimeUnit, units );
        }

        if (IsWorkTime() &&
           (!(startTime.IsWorkDay() && startTime.IsWorkTime()))) {

            char oldTimeStr[50], newTimeStr[50], s[256];
            TTime newTime( startTime );
            newTime.ForwardToWorkTime();

            TIntlFmt::FormatDateAndTime( startTime, kShortDate, oldTimeStr, FALSE );
            TIntlFmt::FormatDateAndTime( newTime, kShortDate, newTimeStr, FALSE );
            wsprintf( s, "The starting time, %s, is not a work time and will be moved to %s.",
                oldTimeStr, newTimeStr );
            TMessageBox::Ok( this, s, "Not A Work Time" );

            startTime = newTime;
        }

        itsStartDateCP->Set( startTime );
        itsStartTimeCP->Set( startTime );
        itsStartTimeCP->SetMode( startTime.HasDate(),
            unit <= k1Hour );

        itsEndCP->UsesHours( endTime.HasTime() );
        itsEndCP->Set( endTime );
        itsDurationCP->SetTimeUnit( unit );
        itsDurationCP->Set( duration );
        itsDurationBoxCP->SetSelIndex( unit );
        itsTimeUnit = unit;
        UpdateScrollPosition();
        UpdateDuration();
        itsDirty = TRUE;
    }

#endif
}

//*****************************************************************************
// start timestamp button
//*****************************************************************************
void TNoteView:: StartNow( RTMessage  )
{
    TTime startTime;
    startTime.SetNow();

    if (IsWorkTime() &&
        (!(startTime.IsWorkDay() && startTime.IsWorkTime()))) {

        char oldTimeStr[50], newTimeStr[50], s[256];
        TTime newTime( startTime );
        newTime.ForwardToWorkTime();

        TIntlFmt::FormatDateAndTime( startTime, kShortDate, oldTimeStr, FALSE );
        TIntlFmt::FormatDateAndTime( newTime, kShortDate, newTimeStr, FALSE );
        wsprintf( s, "The current time, %s, is not a work time and will be moved to %s.",
            oldTimeStr, newTimeStr );
        TMessageBox::Ok( this, s, "Not A Work Time" );

        startTime = newTime;
    }

    itsStartDateCP->Set( startTime );
    itsStartTimeCP->Set( startTime );

    UpdateEndTime();
    UpdateNoteStyle();
    itsDirty = TRUE;
}

//*****************************************************************************
// end timestamp button
//*****************************************************************************
void TNoteView::EndNow( RTMessage )
{
    TTime start, end;

    if (!itsStartDateCP->HasDate())
        return;

    start = itsStartDateCP->Get();
    start.CopyTime( itsStartTimeCP->Get() );
    if (!start.HasDate())
        return;

    //end should not be before start
    TTime now;
    now.SetNow();
    if( start > now )
        return;

    // calculate the duration backwards from now
    // rounding up by time unit
    if( IsWorkTime() )
        {
        TWorkTime w( start, now );
        long length= w.GetNumUnits( itsTimeUnit );
        if( length == 0 )
            length= 1;
        itsDurationCP->Set( itsTimeUnit, length );
        }
    else
        {
        TDuration d( start, now );
        long length= d.GetNumUnits( itsTimeUnit );
        if( length == 0 )
            length= 1;
        itsDurationCP->Set( itsTimeUnit, length );
        }

    // recalculate the end time
    UpdateEndTime();

    UpdateValue();
    UpdateScrollPosition();
    UpdateNoteStyle();
    itsDirty = TRUE;
}

// -------------------------------------------------
//	WorkElapsed
//
//	Convert the duration from work to elapsed and
//  vice versa
// -------------------------------------------------
void TNoteView::WorkElapsed( RTMessage msg )
{
    if( msg.LP.Hi == CBN_SELCHANGE )
        {
        if (itsWorkElapsedCP->GetSelIndex() == 1)
            {
            //set limits on time control
            TTime start, end;
            start.SetHour( TProfile::WorkDayStartMinute / 60 );
            start.SetMinute( TProfile::WorkDayStartMinute % 60 );
            end.SetHour( TProfile::WorkDayEndMinute / 60 );
            end.SetMinute( TProfile::WorkDayEndMinute % 60 );
            itsStartTimeCP->SetRange( start, end );

            // move the start date forward
            start= itsStartDateCP->Get();
            start.CopyTime( itsStartTimeCP->Get() );
            if (start.HasDate())
                {
                start.ForwardToWorkTime();
                itsStartDateCP->Set( start );
                itsStartTimeCP->Set( start );
                }
            }
        else    //elapsed time
            {
            //clear time ranges
            TTime start,end;
            itsStartTimeCP->SetRange( start, end );
            }

        // now update the end time
        UpdateEndTime();
        }
}


// -------------------------------------------------
//	FrequencyButton
//
//	Handle the frequency button by putting up the
//	frequency dialog box
// -------------------------------------------------
void TNoteView::FrequencyButton( RTMessage )
{
	// check to see if there's a date
	if (!itsStartDateCP->HasDate()) {

        TMessageBox::Ok( this,
            "Please assign a date and time first.",
            "Note Can't Have Frequency" );
		return;
	}

	// check to see if there's a time
	if (!itsStartTimeCP->HasTime()) {

        TMessageBox::Ok( this,
            "Please assign a time first.",
            "Note Can't Have Frequency" );
		return;
	}

	// if there's an end time make sure it's on the same day as the start
	if (itsEndCP->HasTime()) {

        if (!itsStartDateCP->Get().IsSameDay( itsEndCP->Get())) {

            TMessageBox::Ok( this,
                "Only notes that start and end on the same day can have frequencies.",
                "Note Can't Have Frequency" );
		    return;
        }
	}


	// put up the frequency dialog
    if (itsFreqP == NULL) {

        TTime start( itsStartDateCP->Get());
        start.CopyTime( itsStartTimeCP->Get());
        itsFreqP = new TFreq( start );
    }

    TTime date = itsStartDateCP->Get();
	PTFrequencyDialog aDialog = new TFrequencyDialog(
        this, IDD_FREQUENCY, itsFreqP, &date );
	if (IDOK == GetApplicationObject()->ExecDialog( aDialog )) {

        UpdateFrequency();

        // copy the frequency's start date into the note
        itsStartDateCP->CopyDate( date );
        if (itsEndCP->HasDate()) {

            itsEndCP->CopyDate( date );
        }
        itsDirty = TRUE;
    }
}

//******************************************************************************
// ColorButton1
//******************************************************************************
void TNoteView::ColorButton1( RTMessage )
{
itsColor.Set( kRed );
UpdateNoteStyle();
//InvalidateRect( GetDlgItem( HWindow, IDD_NOTECOLOR ), NULL, TRUE );
itsDirty = TRUE;
}

//******************************************************************************
// ColorButton2
//******************************************************************************
void TNoteView::ColorButton2( RTMessage )
{
itsColor.Set( kRedGray );
UpdateNoteStyle();
//InvalidateRect( GetDlgItem( HWindow, IDD_NOTECOLOR ), NULL, TRUE );
itsDirty = TRUE;
}

//******************************************************************************
// ColorButton3
//******************************************************************************
void TNoteView::ColorButton3( RTMessage )
{
itsColor.Set( kGray );
UpdateNoteStyle();
//InvalidateRect( GetDlgItem( HWindow, IDD_NOTECOLOR ), NULL, TRUE );
itsDirty = TRUE;
}

//******************************************************************************
// ColorButton4
//******************************************************************************
void TNoteView::ColorButton4( RTMessage )
{
itsColor.Set( kBlueGray );
UpdateNoteStyle();
//InvalidateRect( GetDlgItem( HWindow, IDD_NOTECOLOR ), NULL, TRUE );
itsDirty = TRUE;
}

//******************************************************************************
// ColorButton5
//******************************************************************************
void TNoteView::ColorButton5( RTMessage )
{
itsColor.Set( kBlue );
UpdateNoteStyle();
//InvalidateRect( GetDlgItem( HWindow, IDD_NOTECOLOR ), NULL, TRUE );
itsDirty = TRUE;
}

//*****************************************************************************
// NoteDone
//
// mark note as <done>
//*****************************************************************************
void TNoteView::NoteDone( RTMessage )
{
    itsMarkedDone= ( itsDoneToggleCP->GetCheck() == BF_CHECKED );

    SendMessage( itsTextCP->HWindow, WM_SETFONT,
        (itsMarkedDone) ? TFontMap::GetScreenFont( kBigStrikeFont ):
                          TFontMap::GetScreenFont( kBigFont ),
        (DWORD) 0 );

    itsDirty= TRUE;
}

//*****************************************************************************
// BNClicked
//
//  A button was pressed and is sending the noteview a notification message.
//  Take this opportunity to make sure that the various smart controls
//  are updated, even though the focus will not have been lost, since
//  focus is not lost when buttons are pushed.
//*****************************************************************************
void TNoteView::BNClicked( RTMessage  )
{
    itsRateCP->Validate();
    itsValueCP->Validate();
    itsStartDateCP->Validate();
    itsStartTimeCP->Validate();
}


//////////////////////////////////////////////////////////////////////////////
///////////////// notification handlers ////////////////////////
/////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------
//	NoteStartDate
//
//	When the focus is lost in the start date text box
//  update the end time
// -------------------------------------------------
void TNoteView::NoteStartDate( RTMessage Msg )
{
    if (Msg.LP.Hi == EN_KILLFOCUS) {

        if (itsStartDateCP->HasDate())
            {
            itsStartTimeCP->SetMode( TRUE, itsTimeUnit <= k1Hour );
            }
        else
            {
            itsStartTimeCP->SetMode( FALSE, itsTimeUnit <= k1Hour );
            itsStartTimeCP->Clear();
            if (itsFreqP)
                {
                itsFreqP->Clear();
                }
            }

        UpdateEndTime();
        UpdateNoteStyle();
        itsDirty = TRUE;
    }
}

// -------------------------------------------------
//	NoteStartTime
//
//	When the focus is lost in the start time text box
//  update the end time
// -------------------------------------------------
void TNoteView::NoteStartTime( RTMessage Msg )
{
    if (Msg.LP.Hi == EN_KILLFOCUS) {

        if( !itsStartTimeCP->Validate() )
            return;

        // if there was no date, then put today in
        if (itsStartTimeCP->HasTime() &&
           !itsStartDateCP->HasDate())
            {
            itsStartDateCP->SetNow();
            itsStartTimeCP->SetMode( TRUE, itsTimeUnit <= k1Hour );
            }

        UpdateEndTime();
        UpdateNoteStyle();
        itsDirty = TRUE;
    }
}

// -------------------------------------------------
//	NoteRate
//
//	When the focus is lost in the rate box update the value
// -------------------------------------------------
void TNoteView::NoteRate( RTMessage Msg )
{
    if( Msg.LP.Hi == EN_KILLFOCUS ) {

        UpdateValue();
        itsDirty = TRUE;
    }
}


// -------------------------------------------------
//  WMHScroll  (duration box)
//
//  Modify the duration and end boxes when the scroll
//  bar moves. Notice that the scroll bar's position
//  is always the same. The scroll bar is used only
//  for its lineup and linedown boxes, which control
//  itsScrollPosition.
// -------------------------------------------------
void TNoteView::WMHScroll( RTMessage Msg )
{
    //this causes the inputs to be validated,
    // and the WM_CANCELMODE msg to the scroll bar
    // keeps it from staying depressed.

    if( !itsValueCP->Validate() ||
        !itsRateCP->Validate() ||
        !itsStartDateCP->Validate() ||
        !itsStartTimeCP->Validate() )
        {
        SendMessage( itsDurationScrollCP->HWindow, WM_CANCELMODE, 0, 0 );
        return;
        }

    switch( Msg.WParam ) {

        case SB_LINEUP:

            itsScrollPosition = max( 0, itsScrollPosition - 1 );
            break;

        case SB_LINEDOWN:

            itsScrollPosition = min( 2000000000L, itsScrollPosition + 1 );
            break;

        default:

            return;
    }

    UpdateDuration();
    itsDirty = TRUE;

    //avoid setting focus on scroll bar -- it flashes
    SetFocus( itsDurationCP->HWindow );
}


// -------------------------------------------------
//	NoteUnits
//
//	The units box was changed:
//
//      - reposition the scroll bar so that it corresponds to the old duration
//      - recalculate the duration based on the new units
//      - update the end time
// -------------------------------------------------
void TNoteView::NoteUnits( RTMessage Msg )
{
if( Msg.LP.Hi == CBN_SELCHANGE )
	{
    if (IsWorkTime())
        {
        //convert to equivalent worktime
        long units= itsDurationCP->GetNumUnits( itsTimeUnit );
        TWorkTime w( itsTimeUnit, units );

        //stash converted value into duration control
        itsTimeUnit = (TTimeUnit) itsDurationBoxCP->GetSelIndex();
        itsDurationCP->Set( itsTimeUnit, w.GetNumUnits( itsTimeUnit ) );
        itsDurationCP->SetTimeUnit( itsTimeUnit );
        }
    else
        {
        itsTimeUnit = (TTimeUnit) itsDurationBoxCP->GetSelIndex();
        itsDurationCP->SetTimeUnit( itsTimeUnit );
        }

    SHOW_OR_HIDE_TIME

    //if we're switching to a mode where we need to supply a
    // start time, set it to the start of the workday.
    if( itsTimeUnit <= k1Hour &&
        itsStartDateCP->HasDate() &&
        !itsStartTimeCP->HasTime() )
        {
        itsStartTimeCP->SetMode( TRUE, TRUE );
        TTime time( itsStartDateCP->Get() );
        time.SetToMidnight();
        time.ForwardToWorkTime();
        itsStartTimeCP->Set( time );
        }
    else
        {
        itsStartTimeCP->SetMode( itsStartDateCP->HasDate(),
            itsTimeUnit <= k1Hour);
        }

    itsEndCP->UsesHours(   itsTimeUnit <= k1Hour );

    UpdateRateUnits();
    UpdateScrollPosition();
    UpdateDuration();
    itsDirty = TRUE;
	}
}



//*****************************************************************************
// user changed category selection
//*****************************************************************************
void TNoteView::WMCatSelChg( RTMessage )
{
char buf[ 100 ];

// update category list....

pCatBoxData->Store( itsCatListP );
wsprintf( buf, "%d assigned", itsCatListP->getItemsInContainer() );
itsCatTitleCP->SetCaption( buf );
itsDirty = TRUE;

}


//*****************************************************************************
// The note's units field was changed (outside of noteview),
// rendering the rate obsolete.
// Alert the user.
//*****************************************************************************
void TNoteView::WMUnitChange( RTMessage )
{

TMessageBox::Ok( this,
    "Check the 'rate' field: the time units of this note have changed.",
    "Time Unit Change" );
}



// ------------------------------------------------------------------------------------------------------------------------------------------
// --------------                                    ----------------------------------------------------------------------------------------                                                                      ------------------
// --------------           Update Functions         ----------------------------------------------------------------------------------------                                                                      ------------------
// --------------                                    ----------------------------------------------------------------------------------------                                                                      ------------------
// ------------------------------------------------------------------------------------------------------------------------------------------
//*****************************************************************************

//*****************************************************************************
void TNoteView::UpdateRateUnits( void )
{
    TSafStr str;
    if( itsTimeUnit <= k1Hour )
        str.KSET( "/hr" );
    else if( itsTimeUnit == k1Day )
        str.KSET( "/day" );
    else if( itsTimeUnit == k1Week )
        str.KSET( "/wk" );
    else
        {
        SYSERR( ERR_GEN );
        }

    itsRateUnitsCP->SetCaption( (char *)(const char *) str );
}

//*****************************************************************************
// update text; show/hide control
//*****************************************************************************
void TNoteView::UpdateFrequency( void )
{

if( itsFreqP && itsFreqP->IsRealFreq() )
    {
    char s[255];
    itsFreqP->GetFreqText( s );
    itsFrequencyCP->SetText( s );

    itsFrequencyCP->Show( SW_SHOW );
    itsFrequencyTitleCP->Show( SW_SHOW );

    itsRateCP->Show( SW_HIDE );
    itsRateTitleCP->Show( SW_HIDE );
    itsRateUnitsCP->Show( SW_HIDE );
    itsValueCP->Show( SW_HIDE );
    itsValueTitleCP->Show( SW_HIDE );

    // if it's got a frequency, it's got no rate/value
    itsRateCP->Clear();
    itsValueCP->Clear();
    }
else
    {
    itsFrequencyCP->Show( SW_HIDE );
    itsFrequencyTitleCP->Show( SW_HIDE );

    itsRateCP->Show( SW_SHOW );
    itsRateTitleCP->Show( SW_SHOW );
    itsRateUnitsCP->Show( SW_SHOW );
    itsValueCP->Show( SW_SHOW );
    itsValueTitleCP->Show( SW_SHOW );
    }
}

// ----------------------------------------------------------------
//	UpdateEndTime
//
//	Update the end time because of a new start time or duration
// ----------------------------------------------------------------
void TNoteView::UpdateEndTime( void )
{
    TTime     start;
    TDuration duration;

    start = itsStartDateCP->Get();
    start.CopyTime( itsStartTimeCP->Get() );
    duration = itsDurationCP->Get();

    if (duration.IsZero() || !start.HasDate()) {

        itsEndCP->Clear();
        return;
    }
    else if ( !IsWorkTime()) {

        // elapsed time
        TTime end( start, duration );
        itsEndCP->Set( end );
    }
    else {

        // work time
        TTime     end;
        TWorkTime worktime( itsTimeUnit,
            duration.GetNumUnits( itsTimeUnit ) );
        worktime.CalculateEnd( start, end );
        itsEndCP->Set( end );
    }
}

// ---------------------------------------------------------------
//	UpdateDuration
//
//	Calculate the scale and value of the duration from the scroll
//  bar value and the units in the units box
// ---------------------------------------------------------------
void TNoteView::UpdateDuration( void )
{
    TTimeUnit  unit = (TTimeUnit) itsDurationBoxCP->GetSelIndex();

    itsStartTimeCP->SetMode( itsStartDateCP->HasDate(),
        unit <= k1Hour );
    itsEndCP->UsesHours(   unit <= k1Hour );
    itsDurationCP->Set( unit, (int) itsScrollPosition );

    // since the duration changed, make sure to update
    // the end time, value and note style
    UpdateEndTime();
    UpdateValue();
    UpdateNoteStyle();
}

// -------------------------------------------------
// UpdateScrollPosition
//
// The duration changed through some method other than the scroll bar:
// update the "position" of the scroll bar
// -------------------------------------------------
void TNoteView::UpdateScrollPosition( void )
{
    itsScrollPosition = itsDurationCP->GetNumUnits( itsTimeUnit );
}


//*****************************************************************************
// UpdateValue
//*****************************************************************************
void TNoteView::UpdateValue( void )
{
    if (itsRateCP->HasValue() && !itsDurationCP->IsZero()) {

        long factor;

        switch( itsTimeUnit ) {

            case k1Min:     factor = 60;    break;
            case k5Min:     factor = 12;    break;
            case k6Min:     factor = 10;    break;
            case k10Min:    factor =  6;    break;
            case k12Min:    factor =  5;    break;
            case k15Min:    factor =  4;    break;
            case k20Min:    factor =  3;    break;
            case k30Min:    factor =  2;    break;
            case k1Hour:    factor =  1;    break;
            case k1Day:     factor =  1;    break;
            case k1Week:    factor =  1;    break;
        }

        TValue value;
        long length    = itsDurationCP->GetNumUnits( itsTimeUnit );
        value          = itsRateCP->Get() * length;
        value         /= factor;
        itsValueCP->Set( value );
    }
    else if (itsRateCP->HasValue() ) {

        itsValueCP->Clear();
    }
}

//*****************************************************************************

//*****************************************************************************
void TNoteView::UpdateNoteStyle( void )
{
    TNoteStyle style;

    //classify note by # of dates specified
    int key = (itsStartDateCP->HasDate() ? 2 : 0);
    key    += (itsDurationCP->IsZero() ? 0 : 1);

    switch( key )
        {
                                                  //  date     duration
                                                  //  ----     --------
        case 0: style = kItemStyle;      break;   //  no       no
        case 1: style = kFloatStyle;     break;   //  no       yes
        case 2: style = kMilestoneStyle; break;   //  yes      no
        case 3: style = kTaskStyle;      break;   //  yes      yes
        }

    //update note
    itsStyle= style;

    //update mark control
    itsMarkCP->Set( itsColor, itsStyle );
}


// ------------------------------------------------------------------------------------------------------------------------------------------
// --------------                                    ----------------------------------------------------------------------------------------                                                                      ------------------
// --------------       Generic Object Functions     ----------------------------------------------------------------------------------------                                                                      ------------------
// --------------                                    ----------------------------------------------------------------------------------------                                                                      ------------------
// ------------------------------------------------------------------------------------------------------------------------------------------


//*****************************************************************************
// GetWindowClass
//*****************************************************************************
void TNoteView::GetWindowClass( WNDCLASS & AWndClass )
{
TDialog::GetWindowClass( AWndClass );
AWndClass.hbrBackground= TGDIMap::GetScreenBrush( kHbrBkView );
}

// ------------------------------------------------------------------------------------------------------------------------------------------
// --------------                                    ----------------------------------------------------------------------------------------                                                                      ------------------
// --------------      Windows Message Handlers      ----------------------------------------------------------------------------------------                                                                      ------------------
// --------------                                    ----------------------------------------------------------------------------------------                                                                      ------------------
// ------------------------------------------------------------------------------------------------------------------------------------------

//*****************************************************************************
// hook for unit change msg
//*****************************************************************************
void TNoteView::WMActivate( RTMessage Msg )
{
TDialog::WMActivate( Msg );

if (Msg.WParam != WA_INACTIVE && itsNoteP != NULL && itsNoteP->CheckRateFlag() )
    {
    PostMessage( HWindow, WM_UNITCHANGE, 0, 0 );
    itsNoteP->ClearRateFlag();
    }
}


//*****************************************************************************
// WMCtlColor
//
// assign colors to controls
//*****************************************************************************
void TNoteView::WMCtlColor( RTMessage Msg )
{
HDC hdc= (HDC) Msg.WParam;

if( Msg.LP.Hi == CTLCOLOR_STATIC )
    {
    Msg.Result= TGDIMap::SetColorEnvironment( hdc, kEnvBkCtl );
    }
else if( Msg.LP.Hi == CTLCOLOR_DLG )
    {
    HBRUSH hBrush;
    POINT point;
    //don't need to change text/bkgnd colors

    hBrush= TGDIMap::GetScreenBrush( kHbrBkDlg );

    UnrealizeObject( hBrush );
    point.x=0; point.y= 0;
    ClientToScreen( (HANDLE) Msg.LP.Lo, &point );
    SetBrushOrg( hdc, point.x, point.y );

    Msg.Result= (DWORD) hBrush;
    }
else if( Msg.LP.Hi == CTLCOLOR_BTN )
    {
    WORD childID= GetDlgCtrlID( (HANDLE) Msg.LP.Lo );

    if( childID == IDD_NOTEINK )
        {
        Msg.Result= TGDIMap::SetColorEnvironment( hdc, kEnvBkCtl );
        }
    else if( childID == IDD_NOTEDONE )
        {
        Msg.Result= TGDIMap::SetColorEnvironment( hdc, kEnvBkCtl );
        }
    else
        DefWndProc( Msg );
    }
else
    DefWndProc( Msg );
}

//*****************************************************************************
// FN() Assumes we set smallsize before fullsize!
//****************************************************************************

//declarations
static BOOL CALLBACK EnumChildFn( HWND, long );
#define ENABLE_MORE  0x80000000L
#define DISABLE_MORE 0x00000000L

void TNoteView::SetDlgSize( BOOL bFullSize )
{
static RECT oldRect;

RECT rcMoreGroup;
GetWindowRect( itsMoreGroupCP->HWindow, &rcMoreGroup );

if( bFullSize )
    {
    long parm= ENABLE_MORE | rcMoreGroup.top;

    // enable/disable windows in moregroup
    EnumChildWindows( HWindow, (WNDENUMPROC) EnumChildFn, parm );

    SetWindowPos( HWindow, NULL, 0, 0,
                oldRect.right - oldRect.left,
                oldRect.bottom - oldRect.top,
                SWP_NOZORDER | SWP_NOMOVE );
    }
else
    {
    GetWindowRect( HWindow, &oldRect );

    //resize dlg window to cover moregroup
    SetWindowPos( HWindow, NULL, 0, 0,
                oldRect.right - oldRect.left,
                rcMoreGroup.top - oldRect.top + 12,
                SWP_NOZORDER | SWP_NOMOVE );

    long parm= DISABLE_MORE | rcMoreGroup.top;

    // enable/disable windows in moregroup
    EnumChildWindows( HWindow, (WNDENUMPROC) EnumChildFn, parm );
    }


}

//*****************************************************************************

//*****************************************************************************
BOOL CALLBACK EnumChildFn( HWND h, long parm )
{
RECT rc;
BOOL bEnable= (parm & ENABLE_MORE) != 0;
parm &= ~ENABLE_MORE;

GetWindowRect( h, &rc );

if( rc.top >= parm )     //in moregroup
    {
    ShowWindow( h, (bEnable ? SW_SHOW : SW_HIDE ) );
    EnableWindow( h, bEnable );
    }

return 1;
}



//*****************************************************************************
// SetFromDialDialog
//
// Set the note's time and duration based on the
// values in the dial dialog
//*****************************************************************************
void TNoteView::SetFromDialDialog( RTTime start, RTDuration duration )
{
    #if 0       //let it be rounded up to next interval
    // set to single minutes
    itsTimeUnit = k1Min;
    itsDurationBoxCP->SetSelIndex( 0 );
    #endif

    itsStartDateCP->SetNow();
    itsStartTimeCP->Set( start );

    // switch into elapsed time...
    itsWorkElapsedCP->SetSelIndex( 0 );

    //round up duration to next time unit
    long length= duration.GetNumUnits( itsTimeUnit );
    if( length == 0 )
        length= 1;
    itsDurationCP->Set( itsTimeUnit, length );

    UpdateEndTime( );

    itsDirty= TRUE;

    UpdateNoteStyle( );
    UpdateScrollPosition( );
    UpdateValue( );
    UpdateFrequency( );
    UpdateRateUnits( );
}
