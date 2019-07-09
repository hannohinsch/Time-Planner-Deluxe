#ifndef __oem__
#define __oem__

_CLASSDEF( TOEM )

class _CLASSTYPE TOEM
{
public:
    static WORD aboutBoxID;
    static char *version;
    static char *signOnMsg;
    static char *frameTitle;
    static char *fileExt;
    static char *backupExt;
    static char *helpFileName;
    static char *profileName;
    static char *profileSection;
    static BOOL bAddressBook;
    static BOOL bFolderTabs;
    static BOOL bTrialVersion;
    static int  maxCats;
    static char *maxCatMsg;
    static int  maxNotes;
    static char *maxNoteMsg;
};

#endif
