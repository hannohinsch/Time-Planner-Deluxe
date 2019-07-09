#ifndef __frame__
#define __frame__


_CLASSDEF( TPimFrame )
_CLASSDEF( TNote )
_CLASSDEF( TCat )
_CLASSDEF( TCatTree )
_CLASSDEF( TNoteSet )
_CLASSDEF( TAlarmSet )
_CLASSDEF( TCatBox )
_CLASSDEF( TNoteSet )
_CLASSDEF( TDataView )
_CLASSDEF( TCatBoxData )
_CLASSDEF( TGlobal )
_CLASSDEF( TProfile )
_CLASSDEF( TIntlFmt )
_CLASSDEF( TCursor )
_CLASSDEF( TGDIMap )
_CLASSDEF( THolidaySet )
_CLASSDEF( TFontMap )
_CLASSDEF( TPen )
_CLASSDEF( TDatePick )
_CLASSDEF( TFilter )
_CLASSDEF( TBaseGrid )
_CLASSDEF( TCSV )
_CLASSDEF( TODBitMapButton )
_CLASSDEF( TODTextButton )
_CLASSDEF( TSysState )
_CLASSDEF( TTipWin )
_CLASSDEF( TPrint )

#ifndef __msg__
#include "msg.h"
#endif
#ifndef __rc__
#include "rc.h"
#endif
#ifndef __snoop__
#include "snoop.h"
#endif
#ifndef __apptime__
#include "apptime.h"
#endif

typedef enum
    {
    kCalView, kReportView, kTimeView, kBrowView, kGrafView
    } TActiveView;

class _CLASSTYPE TPimFrame : public TWindow, TSnoop {

    public:
        TPimFrame(LPSTR ATitle);
        virtual ~TPimFrame();

        static void ShowStartMsg( HINSTANCE, int h, int w );
        static void KillStartMsg( void );

        //file fns implemented in filecmd.cpp
        void ReadMRUFile( int n, BOOL bReportError= TRUE, BOOL checkWorkTime = TRUE );
    	void RegisterMRUFile( char * fileName );
		void PrepareMRUMenu( HMENU hMenu );
        void PrepareViewMenu( HMENU hMenu );

        void ResetView( void );
        void UpdateCatBox( void );
        void UpdateView( TTimeScale, RTTime, TActiveView );

        static PTPimFrame pFrame;       //handy pointer to object
        static PTTipWin pTipWin;        //others need access to this guy

    private:
        void WMSize(RTMessage Msg)= [WM_FIRST + WM_SIZE];
        void WMCtlColor(RTMessage Msg )= [WM_FIRST + WM_CTLCOLOR];
        void WMEraseBkgnd(RTMessage)= [WM_FIRST + WM_ERASEBKGND ];
        void WMTimer(RTMessage)= [WM_FIRST + WM_TIMER ];
        void WMLocalColor(RTMessage) = [WM_FIRST + WM_LOCALCOLOR];
		void WMInitMenu(RTMessage) = [ WM_FIRST + WM_INITMENU ];
        void WMEnable(RTMessage) = [ WM_FIRST + WM_ENABLE ];
        void WMActivateApp(RTMessage)= [ WM_FIRST + WM_ACTIVATEAPP ];
        void WMSysCommand( RTMessage )= [WM_FIRST + WM_SYSCOMMAND ];
        void WMSysColorChange(RTMessage)= [WM_FIRST + WM_SYSCOLORCHANGE ];
        void WMMouseActivate(RTMessage)= [WM_FIRST + WM_MOUSEACTIVATE ];

        //button bar responses
        void TableButton(RTMessage Msg) = [ID_FIRST + IDD_BARTABLE];
        void CalendarButton(RTMessage Msg) = [ID_FIRST + IDD_BARCALENDAR];
        void TimelineButton(RTMessage Msg) = [ID_FIRST + IDD_BARTIMELINE];
        void GraphButton(RTMessage Msg) = [ID_FIRST + IDD_BARGRAPH];
        void BrowseButton(RTMessage Msg)= [ID_FIRST + IDD_BARBROWSE];
        void AddressButton(RTMessage Msg)= [ID_FIRST + IDD_ADDRESS_BUTTON];

        void FilterButton(RTMessage Msg) = [ID_FIRST + IDD_BARFILTER];
        void CatMgrButton(RTMessage Msg) = [ID_FIRST + IDD_BARCAT ];
        void NewNoteButton(RTMessage Msg) = [ID_FIRST + IDD_BARNOTE ];
        void DateNowButton(RTMessage) = [ ID_FIRST + IDD_DATENOW ];

        //timescale buttons
        void ScaleHourButton(RTMessage Msg) = [ID_FIRST + IDD_SCALEHOUR];
        void ScaleDayButton(RTMessage Msg) = [ID_FIRST + IDD_SCALEDAY];
        void ScaleWeekButton(RTMessage Msg) = [ID_FIRST + IDD_SCALEWEEK];
        void ScaleMonthButton(RTMessage Msg) = [ID_FIRST + IDD_SCALEMONTH];

        void AllCatsButton( RTMessage Msg ) = [ID_FIRST + IDD_CATSELALL];
        void NoCatsButton( RTMessage Msg ) = [ID_FIRST + IDD_CATSELNONE];
        void HeadlineButton( RTMessage ) = [ ID_FIRST + IDD_FULLTEXT ];
        void FrameMaxButton( RTMessage )= [ ID_FIRST + IDD_FRAMEMAX ];

        //menu responses
        void FileNewCmd(RTMessage Msg)      = [CM_FIRST + IDM_FILENEW];
        void FileOpenCmd(RTMessage Msg)     = [CM_FIRST + IDM_FILEOPEN];
        void FileMergeCmd(RTMessage Msg)    = [CM_FIRST + IDM_FILEMERGE];
        void FileSaveCmd(RTMessage Msg)     = [CM_FIRST + IDM_FILESAVE ];
        void FileSaveAsCmd(RTMessage Msg)   = [ CM_FIRST + IDM_FILESAVEAS ];
        void FileArchiveCmd( RTMessage Msg ) = [ CM_FIRST + IDM_FILEARCHIVE ];
        void ExitCmd(RTMessage Msg)         = [CM_FIRST + IDM_FILEEXIT ];
        void PrintCmd(RTMessage)            = [ CM_FIRST + IDM_FILEPRINT ];
        void FileImportCmd( RTMessage )     = [ CM_FIRST + IDM_FILEIMPORT ];
        void FileExportCmd( RTMessage )     = [ CM_FIRST + IDM_FILEEXPORT ];
        void File1Cmd( RTMessage )          = [ CM_FIRST + IDM_FILEFILE1 ];
        void File2Cmd( RTMessage )          = [ CM_FIRST + IDM_FILEFILE2 ];
        void File3Cmd( RTMessage )          = [ CM_FIRST + IDM_FILEFILE3 ];
        void File4Cmd( RTMessage )          = [ CM_FIRST + IDM_FILEFILE4 ];

        void View0Cmd( RTMessage )          = [ CM_FIRST + IDM_FILE_VIEW_0 + 0];
        void View1Cmd( RTMessage )          = [ CM_FIRST + IDM_FILE_VIEW_0 + 1];
        void View2Cmd( RTMessage )          = [ CM_FIRST + IDM_FILE_VIEW_0 + 2];
        void View3Cmd( RTMessage )          = [ CM_FIRST + IDM_FILE_VIEW_0 + 3];
        void View4Cmd( RTMessage )          = [ CM_FIRST + IDM_FILE_VIEW_0 + 4];
        void View5Cmd( RTMessage )          = [ CM_FIRST + IDM_FILE_VIEW_0 + 5];
        void View6Cmd( RTMessage )          = [ CM_FIRST + IDM_FILE_VIEW_0 + 6];
        void View7Cmd( RTMessage )          = [ CM_FIRST + IDM_FILE_VIEW_0 + 7];
        void View8Cmd( RTMessage )          = [ CM_FIRST + IDM_FILE_VIEW_0 + 8];
        void View9Cmd( RTMessage )          = [ CM_FIRST + IDM_FILE_VIEW_0 + 9];
        void ViewCmd( int n );

        void EditPasteCmd( RTMessage )      = [ CM_FIRST + IDM_EDITPASTE ];

        void TableCmd(RTMessage Msg) = [CM_FIRST + IDM_VIEWTABLE] { TableButton(Msg); };
        void TimelineCmd(RTMessage Msg) = [CM_FIRST + IDM_VIEWTIMELINE] { TimelineButton(Msg); };
        void CalendarCmd(RTMessage Msg) = [CM_FIRST + IDM_VIEWCALENDAR] { CalendarButton(Msg); };
        void GraphCmd(RTMessage Msg) = [CM_FIRST + IDM_VIEWGRAF] { GraphButton(Msg); };
        void BrowseCmd(RTMessage Msg) = [CM_FIRST + IDM_VIEWBROWSER] { BrowseButton(Msg); };
		void HeadlineCmd( RTMessage )= [CM_FIRST + IDM_VIEWHEADLINES ];
		void ShowBarsCmd( RTMessage )= [CM_FIRST + IDM_SHOWBARS ];
		void DayCmd(RTMessage)= [CM_FIRST + IDM_VIEWDAY];
		void WeekCmd(RTMessage) = [CM_FIRST + IDM_VIEWWEEK];
		void MonthCmd(RTMessage) = [CM_FIRST + IDM_VIEWMONTH ];
		void YearCmd(RTMessage)= [CM_FIRST + IDM_VIEWYEAR ];
		void TodayCmd(RTMessage) = [CM_FIRST + IDM_VIEWTODAY ];
		void ViewMaxCmd(RTMessage) = [CM_FIRST + IDM_VIEWMAX ];
		void SelectMemorizedViewCmd(RTMessage) = [CM_FIRST + IDM_MEMORIZED_VIEWS ];
		void MemorizeViewCmd(RTMessage) = [CM_FIRST + IDM_MEMORIZE_VIEW ];

        void HelpIndexCmd( RTMessage )= [CM_FIRST + IDM_HELPINDEX];
        void HelpOnHelpCmd( RTMessage )= [CM_FIRST + IDM_HELPONHELP ];
        void HelpAboutCmd( RTMessage ) = [CM_FIRST + IDM_HELPABOUT];
        void HelpViewCmd( RTMessage )= [ CM_FIRST + IDM_HELPVIEW ];

        void CatMgrCmd(RTMessage Msg) = [CM_FIRST + IDM_CATMGR] { CatMgrButton(Msg); };
		void AddressCmd(RTMessage) = [CM_FIRST + IDM_CATADDR ];
 		void SelectAllCmd(RTMessage)= [ CM_FIRST + IDM_CATSELECTALL];
 		void SelectNoneCmd(RTMessage)= [ CM_FIRST + IDM_CATSELECTNONE];
		void FilterCmd(RTMessage)= [CM_FIRST + IDM_CATFILTER];

        void NewNoteCmd(RTMessage Msg) = [CM_FIRST + IDM_NOTENEW] { NewNoteButton(Msg); };

        void OptionDayviewCmd(RTMessage msg)    = [CM_FIRST + IDM_OPTION_DAYVIEW];
        void OptionReportCmd(RTMessage msg)     = [CM_FIRST + IDM_OPTION_REPORT ];
        void OptionDurationFmtCmd(RTMessage msg) = [CM_FIRST + IDM_OPTION_DURFMT];
        void OptionWorkTimeCmd(RTMessage msg)   = [CM_FIRST  + IDM_OPTION_WORK_TIME];
        void OptionPreferCmd( RTMessage msg)   = [ CM_FIRST + IDM_OPTION_PREFER ];
        void OptionRestoreCmd( RTMessage msg ) = [ CM_FIRST + IDM_OPTION_RESTORE ];
        void OptionRestoreIntervalCmd( RTMessage msg ) = [ CM_FIRST + IDM_OPTION_RESTORE_INTERVAL ];
        void OptionRestoreColumnCmd( RTMessage msg ) = [ CM_FIRST + IDM_OPTION_RESTORE_COLUMNS ];
        void OptionRestoreTabsCmd( RTMessage msg ) = [ CM_FIRST + IDM_OPTION_RESTORE_TABS ];
        void OptionRestoreSortCmd( RTMessage msg ) = [ CM_FIRST + IDM_OPTION_RESTORE_SORT ];
        void OptionFontCmd( RTMessage msg )     = [ CM_FIRST + IDM_OPTIONFONT ];
        void OptionSortCmd( RTMessage msg )     = [ CM_FIRST + IDM_SORT_ORDER ];
        void OptionSortNowCmd( RTMessage )      = [ CM_FIRST + IDM_SORTNOW ];
        void OptionHolidaysCmd( RTMessage )     = [ CM_FIRST + IDM_HOLIDAYS ];
        void OptionGroupCats( RTMessage )       = [ CM_FIRST + IDM_GROUP_CATS ];

        void PhoneSetup( RTMessage )= [ CM_FIRST + IDM_PHONE_SETUP ];
        void PhoneHangup( RTMessage )= [ CM_FIRST + IDM_PHONE_HANGUP ];

        //handlers
        void CalendarHandler(RTMessage Msg) = [ID_FIRST + IDD_CALMONTH];
        void ListHandler( RTMessage Msg ) = [ WM_FIRST + WM_CATSELCHG ];

        void TimeDateUpdate( RTMessage Msg ) = [WM_FIRST + WM_WININICHANGE];

        void SetupWindow( void );
        void GetWindowClass( WNDCLASS & AWndClass );
        BOOL CanClose( void );
        BOOL ConfirmClean( void );
        void CreateNewFile( void );

        BOOL HealthCheck( void );

        void Listen( TSnoopMsg, RObject );
        void static DisplayUpdateAction( Pvoid, Pvoid );
        void Paint( HDC hdc, PAINTSTRUCT & paintInfo);
        void NoteReset( void );
        void SetTitle( void );
        void SetTime( void );
		void ShowCurrentFileName( void );
        void Layout( void );
        void UpdateButtons( BOOL changeTime = TRUE );
        void UpdateCalendars( void );
        void DrawDropShadow( HDC hdc, RECT &aRect );

        // import/export functions (impexp.cpp)
        BOOL ExportFile( char *fName, BOOL headlinesOnly );
        int  ExportNote( ofstream &os, RTNote note, BOOL headlinesOnly );
        BOOL ImportFile( char *fName );
        int  ImportNote( ifstream &is, PTCSV csvP );

        //file functions (filecmd.cpp)
        BOOL DoFileOpen( char *fName, BOOL checkWorkTime );
        BOOL DoFileSave();
        BOOL DoFileSaveAs();

        PTBaseGrid pView;

        // "life-of-app" objects...
        PTProfile         pProfile;
        PTIntlFmt         pIntlFmt;
        PTCursor          pCursor;
        PTGDIMap          pGDIMap;
        PTHolidaySet      pHolidays;
        PTFontMap         pFontMap;
        PTPen             pPen;
        PTGlobal          pGlobal;
        PTPrint           pPrint;

		BOOL              bMemMessage;

        //button bar
        PTODBitMapButton pCalViewButton;
        PTODBitMapButton pTabViewButton;
        PTODBitMapButton pTimeViewButton;
        PTODBitMapButton pGrafViewButton;
        PTODBitMapButton pBrowViewButton;

        PTODBitMapButton pNoteButton;
        PTODTextButton pAddrButton;

        //controls
        PTCatBox pBox;
        PTODTextButton pCatAll;
        PTODTextButton pCatNone;
        PTCatBoxData pCatBoxData;
        PTStatic pTitle;
        PTStatic pCatTitle;
        PTDatePick pCalendar;
        PTODTextButton pNowButton;
        PTODTextButton pCatButton;
        PTODTextButton pFilterButton;
        PTCheckBox pFullTextButton;
        PTButton pMaxButton;
        PTStatic pTimeTitle;
        PTStatic pDateTitle;
        PTStatic pDOWTitle;


        PTRadioButton pDayButton;
        PTRadioButton pWeekButton;
        PTRadioButton pMonthButton;
        PTRadioButton pYearButton;
        PTGroupBox pGroupBox;

        TTime   currentTime;

        RECT viewRect;
        RECT catRect;
        RECT calRect;
        RECT barRect;

        BOOL bMaxFrame;
        BOOL itsActive;

		static char currentFile[ MAXPATH ];

        static HANDLE itsHStartMsg;
        static HANDLE itsHStartFont;
    };

#endif
