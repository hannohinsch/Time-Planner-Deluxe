#ifndef __msg__
#define __msg__

//
// user defined messages
//
#define WM_CATSELCHG    WM_USER + 101  // catbox: selection changed
#define WM_LOCALCOLOR   WM_USER + 102  // frame: app colors changed
#define WM_UNITCHANGE   WM_USER + 103  // noteview: note units changed
#define WM_EDIT_INVALID WM_USER + 104  // fmtedit: invalid field
#define WM_HEADLINE_CHG WM_USER + 105  // frame: headline box clicked
#define WM_PAINTHELP    WM_USER + 106  // help button for OurDialog

//custom control notify messages
#define CN_CLICKED      1
#define CN_DBLCLICKED   2
#define CN_TIMECHANGED  3


#endif
