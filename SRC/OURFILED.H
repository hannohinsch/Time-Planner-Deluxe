#ifndef __ourfiled__
#define __ourfiled__

#ifndef __ourdlg__
#include "ourdlg.h"
#endif

#ifndef __DIR_H
#include <dir.h>
#endif

#define FILESPEC	MAXPATH + MAXEXT

_CLASSDEF(TOurFileDialog)

class _EXPORT TOurFileDialog : public TOurDialog
{
public:
    LPSTR FilePath;
    char PathName[MAXPATH];
    char Extension[MAXEXT];
    char FileSpec[FILESPEC];

    TOurFileDialog(PTWindowsObject AParent, int ResourceId, LPSTR AFilePath,
                PTModule AModule = NULL);
    virtual BOOL CanClose();
    void SelectFileName();
    void UpdateFileName();
    BOOL UpdateListBoxes();

protected:
    virtual void SetupWindow();
    virtual void HandleFName(RTMessage Msg)
          = [ID_FIRST + ID_FNAME];
    virtual void HandleFList(RTMessage Msg)
          = [ID_FIRST + ID_FLIST];
    virtual void HandleDList(RTMessage Msg)
          = [ID_FIRST + ID_DLIST];

};

#endif
