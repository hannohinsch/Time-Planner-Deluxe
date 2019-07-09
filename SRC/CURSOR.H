#ifndef __cursor__
#define __cursor__

_CLASSDEF( TCursor )

typedef enum {

    kFirstCursor= 0,

    kArrowCursor= kFirstCursor,
    kBusyCursor,
    kCatSelCursor,
    kOutlineCursor,
    kDateSelCursor,
    kNoteSelCursor,
    kZoomCursor,
    kNoteBarCursor,
    kLongNoteBarCursor,
    kDragHorizCursor,
    kDragVertCursor,
    kDragBothCursor,
    kGrowUpCursor,
    kGrowDownCursor,
    kGrowLeftCursor,
    kGrowRightCursor,
    kDragIllegalCursor,
    kTimeMoveCursor,
    kNewNoteCursor,

    kNumCursors= kNewNoteCursor + 1

} TCursorStyle;

class _CLASSTYPE TCursor {

public:
    TCursor( void );
    virtual ~TCursor( void );

    static int Busy( BOOL aBusy );
    static void Set( TCursorStyle style );

private:
    void CreateAppCursors();
    void DestroyAppCursors();

    static HCURSOR cursorTable[ kNumCursors ];
    static HCURSOR hUnBusyCursor;
    static int busyCount;
};

#endif
