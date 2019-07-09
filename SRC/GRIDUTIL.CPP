//*****************************************************************************
// misc. subfunctions used by grids
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "gridutil.h"

#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __noteview__
#include "noteview.h"
#endif
#ifndef __filter__
#include "filter.h"
#endif
#ifndef __notelisd__
#include "notelisd.h"
#endif
#ifndef __frame__
#include "frame.h"
#endif
#ifndef __dragwin__
#include "dragwin.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif


// -----------------------------------------------------------------------------
// ShowNote
//
// Put up the noteview dialog box for the given note, or create a new
// one if the note is NULL
// -----------------------------------------------------------------------------
void TGridUtil::ShowNote( PTNote pNote )
{

    if (pNote == NULL) {

        pNote = TSysState::pAllNotes->CreateNote( TPimFrame::pFrame );
        if (pNote == NULL) {

            return;
        }
    }

    // show the note
    GetApplicationObject()->ExecDialog(
        new TNoteView( TPimFrame::pFrame, IDD_NOTE, pNote ));
}

//*****************************************************************************
// brings up note list
//*****************************************************************************
void TGridUtil::NotePopup( RTTime time )
{
    TFilter filter( NULL,
        TRUE,           //select from all categories
        kCatMatchAny,
        NULL,           //search text
        TRUE            //don't include notes w/o dates
         );

    filter.SetFilterDates( kDayFilter, time );

    // get all of the currently selected notes that
    // match that filter
    TNoteSet theNotes( filter, TSysState::pSelNotes );

    // popup a list if there are any notes in the set
    int numNotes = theNotes.getItemsInContainer();
    if (numNotes > 0) {

        TNoteListDialog *noteListDialog = new TNoteListDialog(
            TPimFrame::pFrame, IDD_NOTELIST_DIALOG,
            theNotes, " Notes at this time:" );

        // if we didn't cancel out of the box then show a note
        int selection = GetApplicationObject()->ExecDialog( noteListDialog );
        if (selection != kCanceledNotelist) {

		    GetApplicationObject()->ExecDialog(new TNoteView(
                TPimFrame::pFrame, IDD_NOTE, &theNotes[selection] ));
        }
    }
}


// -------------------------------------------------------------------------
// PaintBackground
//
//
// -------------------------------------------------------------------------
void TGridUtil::PaintBackground( RTDrawInfo di, RTTime time, BOOL weekInOneCell, BOOL checkTime )
{
    switch (TSysState::timeScale) {

        case SCALE_HOUR:
        case SCALE_DAY:

             // holiday?
             if (time.IsHoliday()) {

                 di.pPainter->FillRectangle( di, di.cellRect, kHbrBkHolidays );
             }
             // weekend?
             else if (!time.IsWorkDay()) {

                 di.pPainter->FillRectangle( di, di.cellRect, kHbrBkOffDays );
             }
             // during the day?
             else if (checkTime && !time.IsWorkTime()) {

                 di.pPainter->FillRectangle( di, di.cellRect, kHbrBkOffDays );
             }

             break;

        case SCALE_WEEK:

            if (weekInOneCell) {

                //NOTE partial cell backgournd disabled!!!
            }
            else if (time.IsHoliday()) {

                di.pPainter->FillRectangle( di, di.cellRect, kHbrBkHolidays );
            }
            else if (!time.IsWorkDay()) {

                di.pPainter->FillRectangle( di, di.cellRect, kHbrBkOffDays );
            }
            break;

        default:

            break;
    }
}

// -------------------------------------------------------------------------
// PaintBackground
//
// If you only have the day of the week, then you only have the day
// of the week...
//
// This only happens in the month grid right now
// -------------------------------------------------------------------------
void TGridUtil::PaintBackground( RTDrawInfo di, int dow )
{
    if (!TProfile::IsWorkDay[ dow ]) {

        di.pPainter->FillRectangle( di, di.cellRect, kHbrBkOffDays );
    }
}

//*****************************************************************************
// DrawMarquee
//
// Turn the marquee on or off (only does something if it's necessary)
//*****************************************************************************
void TGridUtil::DrawMarquee( PTWindowsObject parent, RECT &r, BOOL on )
{
    static BOOL dragMarquee = FALSE;

    if (on && !dragMarquee) {

        // marquee is being turned on
        dragMarquee = TRUE;
    }
    else if ((!on) && dragMarquee) {

        // marquee is being turned off
        dragMarquee = FALSE;
    }
    else {

        // marquee doesn't need to be changed
        return;
    }

    // invert the rectangle
    HANDLE dc = GetDC( parent->HWindow );
    DrawFocusRect( dc, &r );
    ReleaseDC( parent->HWindow, dc );
}

//*****************************************************************************
// handles captured mouse, cursor
//*****************************************************************************
void TGridUtil::OurTrackPopupMenu(HMENU hMenu, UINT fuFlags, int x, int y,
                                  int reserved, HWND hWnd, const RECT* lprc )
{
SetCursor( NULL );
HANDLE hCap= GetCapture();
ReleaseCapture();

TrackPopupMenu( hMenu, fuFlags, x, y, reserved, hWnd, lprc );

SetCapture( hCap );
}
