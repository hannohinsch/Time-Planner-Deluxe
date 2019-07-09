//*****************************************************************************
// contains all of the version-dependent strings, etc.
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "oem.h"

//*****************************************************************************
// the version #define is in the make file!
//*****************************************************************************

#if defined HINSCH_VERSION

WORD        TOEM::aboutBoxID= IDD_ABOUT_HINSCH;
char *      TOEM::version= "version 2.0b";
char *      TOEM::signOnMsg= "TIME PLANNER DELUXE\x99\n\nCopyright \xA9 1991-1995 H.M. Hinsch && Co., Inc.\nAll Rights Reserved.";
char *      TOEM::frameTitle= "Time Planner Deluxe";
char *      TOEM::fileExt= ".HTP";          //requires dot
char *      TOEM::backupExt= "~TP";         // no dot
char *      TOEM::helpFileName= "HTP.HLP";
char *      TOEM::profileName= "HTP.INI";
char *      TOEM::profileSection= "General";
BOOL        TOEM::bAddressBook= TRUE;
BOOL        TOEM::bFolderTabs= TRUE;
BOOL        TOEM::bTrialVersion= FALSE;
int         TOEM::maxCats= 1000;
char *      TOEM::maxCatMsg= "";
int         TOEM::maxNotes= 1000;
char *      TOEM::maxNoteMsg= "";

#else defined KEY_VERSION

char *      TOEM::version= "version 2.0";
WORD        TOEM::aboutBoxID= IDD_ABOUT_KEY;
char *      TOEM::signOnMsg= "KEY PROJECT MANAGER\x99 is a trademark of\nSoftKey Products International B.V.\nCopyright \xA9 1991-1995 H.M. Hinsch && Co., Inc.\nAll Rights Reserved.";
char *      TOEM::frameTitle= "Key Project Manager";
char *      TOEM::fileExt= ".KPM";          //requires dot
char *      TOEM::backupExt= "~PM";         // no dot
char *      TOEM::helpFileName= "KPM.HLP";
char *      TOEM::profileName= "KPM.INI";
char *      TOEM::profileSection= "General";
BOOL        TOEM::bAddressBook= FALSE;
BOOL        TOEM::bFolderTabs= FALSE;
BOOL        TOEM::bTrialVersion= FALSE;
int         TOEM::maxCats= 1000;
char *      TOEM::maxCatMsg= "";
int         TOEM::maxNotes= 1000;
char *      TOEM::maxNoteMsg= "";

#endif

