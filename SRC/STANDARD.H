#ifndef __standard__
#define __standard__

//owl requires this!
#define WIN31

#include <windows.h>
#include <commdlg.h>

#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#include <_defs.h>

//C runtime
#include <ctype.h>
#include <string.h>

// NB always put local copies first, so they really get included! Otherwise,
// other headers may, as a side effect, load non-local copies first, and
// then our local copies won't be included! (hmh)

//classlib
#include <object.h>

//owl
#include "safepool.h"           //local copy
#include "objstrm.h"            //local copy
#include <owl.h>
#include <dir.h>
#include <listbox.h>
#include <combobox.h>
#include <edit.h>
#include <static.h>
#include <button.h>
#include <dialog.h>
#include <radiobut.h>
#include <scrollba.h>
#include <tcollect.h>

//bwcc
#include <bwcc.h>
#include <bbutton.h>
#include <bstatic.h>

//our message box replacement
#include "message.h"

#endif
