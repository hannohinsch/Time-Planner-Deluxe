#ifndef __noteview__
#define __noteview__

_CLASSDEF(TCatBoxData)
_CLASSDEF(TNote)
_CLASSDEF(TCat)
_CLASSDEF(TCatBox)
_CLASSDEF(TPenEdit)
_CLASSDEF(TTimeStatic)
_CLASSDEF(TTimeOnlyEdit)
_CLASSDEF(TDateOnlyEdit)
_CLASSDEF(TValueEdit)
_CLASSDEF(TDurationStatic)
_CLASSDEF(TNoteView)
_CLASSDEF(TCatList)
_CLASSDEF(TMarkCtl)
_CLASSDEF(TFreq )
_CLASSDEF(TODTextButton )

#ifndef __snoop__
#include "snoop.h"
#endif
#ifndef __rc__
#include "rc.h"
#endif
#ifndef __apptime__
#include "apptime.h"
#endif
#ifndef __msg__
#include "msg.h"
#endif
#ifndef __ink__
#include "ink.h"
#endif
#ifndef __color__
#include "color.h"
#endif
#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __alarm__
#include "alarm.h"
#endif
#ifndef __ourdlg__
#include "ourdlg.h"
#endif

// transfer buffer stuff
const MAX_UNIT_LEN  = 50;
const MAX_FREQ_LEN  = 50;
const MAX_NOTE_LEN  = 1024;
const MAX_TIME_LEN  = 100;
const MAX_DUR_LEN   = 100;
const MAX_VALUE_LEN = 100;
const MAX_RATE_LEN  = 10;

class _CLASSTYPE TNoteView : public TOurDialog, TSnoop {

public:

    TNoteView( PTWindowsObject AParent, WORD AResID, PTNote pNewNote );
    virtual ~TNoteView();

    static PTNoteView CurrentNoteViewP;
    void SetFromDialDialog( RTTime start,
        RTDuration duration );

private:

    // inherited dialog functions
    void SetupWindow();
    virtual void ShutDownWindow( void );
    virtual void ShutDownWindow( int returnValue );
    void ShowAddressDialog( void );

    // note text handlers
    void Guess(           RTMessage Msg ) = [ ID_FIRST + IDD_NOTEGUESS    ];
    void NoteInk(         RTMessage Msg ) = [ ID_FIRST + IDD_NOTEINK    ];
    void NoteRecog(       RTMessage Msg ) = [ ID_FIRST + IDD_NOTERECOG  ];
    void NoteClear(       RTMessage Msg ) = [ ID_FIRST + IDD_NOTECLEAR  ];
	void CatMgr(          RTMessage Msg ) = [ ID_FIRST + IDD_NOTECATMGR ];
	void NoteAddress(     RTMessage Msg ) = [ ID_FIRST + IDD_NOTE_ADDRESS ];

    // note storage button handlers
    void NoteTemplate(    RTMessage Msg ) = [ ID_FIRST + IDD_NOTETEMPLATE ];
    void NoteDelete(      RTMessage Msg ) = [ ID_FIRST + IDD_NOTEDELETE ];
    void NoteDone(        RTMessage Msg ) = [ ID_FIRST + IDD_NOTEDONE   ];
    void Ok(              RTMessage Msg ) = [ ID_FIRST + IDOK ];
    void NoteMore(        RTMessage Msg ) = [ ID_FIRST + IDD_NOTEMORE ];

    // unit/rate button handlers
    void NoteUnits(       RTMessage Msg ) = [ ID_FIRST + IDD_NOTEUNITS  ];
    void NoteRate(        RTMessage Msg ) = [ ID_FIRST + IDD_NOTERATE   ];

    // time/duration control handlers
   	void StartTimeButton( RTMessage Msg ) = [ ID_FIRST + IDD_STARTPICK  ];
    void StartNow(        RTMessage Msg ) = [ ID_FIRST + IDD_STARTNOW   ];
	void EndNow(          RTMessage Msg ) = [ ID_FIRST + IDD_ENDNOW     ];
    void NoteStartDate(   RTMessage Msg ) = [ ID_FIRST + IDD_NOTE_START_DATE  ];
    void NoteStartTime(   RTMessage Msg ) = [ ID_FIRST + IDD_NOTE_START_TIME  ];
    void FrequencyButton( RTMessage Msg ) = [ ID_FIRST + IDD_NOTEFREQ   ];
    void WorkElapsed(     RTMessage Msg ) = [ ID_FIRST + IDD_NOTEWORKELAPSED   ];

    // alarm control handlers
    void AlarmButton(     RTMessage Msg ) = [ ID_FIRST + IDD_ALARM_BUTTON];

    // color button handlers (actually means priority, not color)
    void ColorButton1(    RTMessage Msg ) = [ ID_FIRST + IDD_NOTERED    ];
    void ColorButton2(    RTMessage Msg ) = [ ID_FIRST + IDD_NOTEREDGRAY ];
    void ColorButton3(    RTMessage Msg ) = [ ID_FIRST + IDD_NOTEGRAY  ];
    void ColorButton4(    RTMessage Msg ) = [ ID_FIRST + IDD_NOTEBLUEGRAY  ];
    void ColorButton5(    RTMessage Msg ) = [ ID_FIRST + IDD_NOTEBLUE   ];

    // generic object functions
    LPSTR GetClassName() { return "NoteWin"; };
    void  GetWindowClass( WNDCLASS & AWndClass );

    // windows message handlers
    void WMCtlColor( RTMessage Msg) = [ WM_FIRST + WM_CTLCOLOR  ];
    void WMHScroll(  RTMessage Msg) = [ WM_FIRST + WM_HSCROLL   ];
    void WMCatSelChg(RTMessage Msg) = [ WM_FIRST + WM_CATSELCHG ];
    void BNClicked( RTMessage Msg )  = [ NF_FIRST + BN_CLICKED   ];

    void WMUnitChange( RTMessage ) = [ WM_FIRST + WM_UNITCHANGE ];
    void WMActivate( RTMessage ) = [ WM_FIRST + WM_ACTIVATE ];

    // initialization functions
    void     InitializeNoteText( void );
    void     InitializeButtons( void );
    void     InitializeSmartControls( void );
    void     InitializeCatBox( void );
    void     InitializeMisc( void );

    // setup functions
    void     SetupInkControls( void );
    void     SetupTimeDurationControls( void );
    void     SetupMiscControls( void );

    // buffer to note functions
	void     BufferToNote( PTNote noteP );
    void     SmartControlsToNote( PTNote noteP );
    void     MiscControlsToNote( PTNote noteP );
    void     AlarmToNote( PTNote noteP );
    void     DestroyNotesAlarm( PTNote noteP );

    // update functions
    void     UpdateNoteStyle( void );
    void     UpdateScrollPosition( void );
    void     UpdateDuration( void );
    void     UpdateValue( void );
    void     UpdateEndTime( void );
    void     UpdateFrequency( void );
    void     UpdateRateUnits( void );

    // miscellaneous
    void     DestroyNote( BOOL notify );
    void     CheckDirtyFlags( void );
    BOOL     IsOkToSave();
    PTNoteSet TimeConflicts( void );
    void     ShowFreqFields( void );
    BOOL     IsWorkTime( void );

    //MORE button functions
    void          SetDlgSize( BOOL bFullSize );

    // controls
	PTPenEdit      itsTextCP;           // main text of note
    PTCatBox       itsCatBoxCP;         // cat assign box
    PTComboBox     itsDurationBoxCP;    // time-increments (1 min, 5 min etc.)
    PTStatic       itsFrequencyCP;      // frequency text
    PTStatic       itsFrequencyTitleCP; // frequency title
    PTStatic       itsCatTitleCP;       // title of note
    PTCheckBox     itsInkToggleCP;      // use ink?
    PTButton       itsRecognizeCP;      // "recognize" ink button
    PTButton       itsClearCP;          // clear button
    PTCheckBox     itsDoneToggleCP;     // note is "done?"
    PTDurationStatic itsDurationCP;       // duration and control to display it
    PTDateOnlyEdit itsStartDateCP;      // start date control
    PTTimeOnlyEdit itsStartTimeCP;      // start time control
    PTStatic       itsStartTimeTitleCP; // "start time"
    PTTimeStatic   itsEndCP;            // end time and control to display it
    PTValueEdit    itsValueCP;          // value and control to display it
    PTStatic       itsValueTitleCP;     // "value"
    PTValueEdit    itsRateCP;           // rate and control to display it
    PTStatic       itsRateTitleCP;      // "rate"
    PTStatic       itsRateUnitsCP;      // rate units
    PTScrollBar    itsDurationScrollCP; // scrollbar to affect duration
    PTBButton      itsAlarmIconCP;      // "alarm on/off"
    PTMarkCtl      itsMarkCP;           // note "mark" (shape and color)
    PTGroupBox     itsMoreGroupCP;      // box around "more" controls
    PTButton       itsMoreBtnCP;        // more button
    PTODTextButton itsCategoryCP;       // category button
    PTODTextButton itsAddressCP;        // address button
    PTComboBox     itsWorkElapsedCP;    // work or elapsed time for this note?

    // data
    PTNote         itsNoteP;          // the note
    BOOL           itsDirty;          // noteview changed something in note?
    TTimeUnit      itsTimeUnit;       // time unit of noteview (set by pDurBox)
    long           itsScrollPosition; // see WMHScroll
    BOOL           itsInkMode;        // local copy of note's ink mode
    TInk           itsInk;            // local copy of note's ink data
    TColor         itsColor;          // local copy of note's color data
    TNoteStyle     itsStyle;          // local copy of note's style
    PTCatList      itsCatListP;       // local copy of note's catlist
    BOOL           itsMarkedDone;     // done box checked?
    PTAlarm        itsAlarmP;         // an alarm
    PTFreq         itsFreqP;          // frequency
    PTCatBoxData   pCatBoxData;

    };

#endif
