#ifndef __rptgrid__
#define __rptgrid__

_CLASSDEF( TNoteSet )
_CLASSDEF( TReportGrid )
_CLASSDEF( TXArray )
_CLASSDEF( TNoteTable )

#ifndef __value__
#include "value.h"
#endif
#ifndef __worktime__
#include "worktime.h"
#endif
#ifndef __snoop__
#include "snoop.h"
#endif
#ifndef __rulegrid__
#include "rulegrid.h"
#endif

//be sure not to mess with the order of these items --
// they're stored in the profile
typedef enum
{
    kMarkField,
    kTextField,
    kStartField,
    kEndField,
    kElapsedField,
    kWorktimeField,
    kFreqField,
    kRateField,
    kAmtField,
    kAlarmField,
    kTimeUnitField,
    kDontUseField,

    kFirstField= kMarkField,
    kLastField= kDontUseField,
    kNumFields= kLastField - kFirstField + 1

} TReportField;

const kMaxReportCols= 10;


//*****************************************************************************
// grid object, customized for table view
//*****************************************************************************

class _CLASSTYPE TReportGrid: public TRuleGrid, TSnoop {

public:
    TReportGrid( PTWindowsObject AParent );
    virtual ~TReportGrid( void );

    static void AppInit( void );

    void Listen( TSnoopMsg, RObject );
    void Reset( void );
    void ResetNoGrouping( void );
    void ResetWithGrouping( void );

    static void SetBreakLevel( int n );
    static int  GetBreakLevel( void );

    static void SetColumnField( int col, TReportField f );
    static TReportField GetColumnField( int col );

    static void SaveProfile( void );
    static void ReadProfile( void );

protected:
    BOOL IsCellSelectable( int row, int col );
    void GetDefaultFocusCell( int &row, int &col );

private:

    void SetupWindow( void );
    int  CellWidth( PTPaint, int col );
    int  CellHeight( PTPaint, int row );
    int  GridWidth( PTPaint, int /*col*/ );
    int  GridHeight( PTPaint, int /*row*/ );

    void PaintCell( RTDrawInfo di );
    void MousRtClick( RTMousInfo );
    void MousDblClick( RTMousInfo );
    void MousOver( RTMousInfo );

    int  ActiveColCnt( void );

    void PaintTitle( RTDrawInfo di );
    void PaintNote( RTDrawInfo di );
    void PaintSummary( RTDrawInfo di );
    void PaintHeader( RTDrawInfo di );

    BOOL IsNoteRow( int row );

    void InvalidateNote( RTNote );
    RTNote GetNote( int row );

    void WMVScroll(RTMessage)= [WM_FIRST + WM_VSCROLL];

    //these are static because they're initialized from
    // profile; modified by report dlg.
    static TReportField     itsColumnFields[ kMaxReportCols ];
    static int              itsBreakLevel;
    static WORD             itsFieldWidths[ kNumFields ];
    static PTSafStr         itsFieldTitles[ kNumFields ];

    int         itsRowCount;
    PTXArray    itsTotalsP;
    PTNoteTable itsNoteTableP;
    };


#endif
