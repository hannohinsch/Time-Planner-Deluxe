//*****************************************************************************
//filecmd.cpp -- part of TPimFrame implementation
// (file operations)
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "frame.h"

#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __profile__
#include "profile.h"
#endif
#ifndef __error__
#include "error.h"
#endif
#ifndef __archived__
#include "archived.h"
#endif
#ifndef __impexpd__
#include "impexpd.h"
#endif
#ifndef __csv__
#include "csv.h"
#endif
#ifndef  __cursor__
#include "cursor.h"
#endif
#ifndef __noteset__
#include "noteset.h"
#endif
#ifndef __note__
#include "note.h"
#endif
#ifndef __oem__
#include "oem.h"
#endif
#ifndef __newdlg__
#include "newdlg.h"
#endif
#ifndef __memview__
#include "memview.h"
#endif
#ifndef __fldmapd__
#include "fldmapd.h"
#endif

//needed for file access() function
#include "io.h"

char TPimFrame::currentFile[ MAXPATH ]= "";

//
// file dialogs for .htp, .csv extensions...
//
_CLASSDEF( THTPFileDialog )
class _CLASSTYPE THTPFileDialog : public TOurFileDialog {
public:
    THTPFileDialog(PTWindowsObject AParent, int ResourceId,
                   LPSTR AFilePath, DWORD helpID= 0 )
     : TOurFileDialog(AParent, ResourceId, AFilePath, NULL )
       {
       itsHelpTopic= helpID;
       strcpy(Extension, TOEM::fileExt);
       };
};


//*****************************************************************************

//*****************************************************************************
void TPimFrame::File1Cmd( RTMessage )
{
ReadMRUFile( 1 );
ASSERT( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::File2Cmd( RTMessage )
{
ReadMRUFile( 2 );
ASSERT( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::File3Cmd( RTMessage )
{
ReadMRUFile( 3 );
ASSERT( ERR_GEN, HealthCheck() );
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::File4Cmd( RTMessage )
{
ReadMRUFile( 4 );
ASSERT( ERR_GEN, HealthCheck() );
}


//*****************************************************************************

//*****************************************************************************
BOOL TPimFrame::DoFileSave( void )
{
BOOL ok= FALSE;

if( !currentFile[0] )
	{
    ok= DoFileSaveAs();
	}
else
	{
    ASSERT( ERR_GEN, HealthCheck() );
    ok= TSysState::WriteFile( currentFile );
    if( !ok )
        {
        TMessageBox::Ok( this, "Unable to write file.", "Write Error" );
        }
    }

return ok;
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::FileSaveCmd( RTMessage )
{
DoFileSave();
}

//*****************************************************************************

//*****************************************************************************
BOOL TPimFrame::DoFileSaveAs( void )
{
BOOL ok= FALSE;
char fName[MAXPATH];

ASSERT( ERR_GEN, HealthCheck() );

strcpy( fName, currentFile );

if ( IDOK == GetApplication()->ExecDialog(new THTPFileDialog(this, SD_FILESAVE, fName ) ))
	{
    UpdateWindow( HWindow );        //immediately get rid of dialog box

    //write file
    //if file already exists, ask for confirmation
    if( 0 == access( fName, 0 )) {

        char buf[200];
        wsprintf( buf, "File '%s'\nalready exists.\n\nOverwrite it?", fName );
        if( IDOK != TMessageBox::OkAndCancel( this, buf, "Confirm File Overwrite" ))
            return FALSE;
        }

    ok= TSysState::WriteFile( fName );
    if( ok )
		{
		strcpy( currentFile, fName );
		ShowCurrentFileName();
		RegisterMRUFile( currentFile );
		}
	else
		{
        TMessageBox::Ok( this, "Unable to write file.", "Write Error" );
        }
    }

return ok;
}

//*****************************************************************************

//*****************************************************************************
void TPimFrame::FileSaveAsCmd( RTMessage )
{
DoFileSaveAs();
}

//*****************************************************************************
// Respond to "New"  (clears everything)
//*****************************************************************************
void TPimFrame::FileNewCmd(RTMessage)
{
if( ConfirmClean() )
	{
    CreateNewFile();
    }
}


//*****************************************************************************
// CreateNewFile
//
//  Creates either a completely empty file, or a
//  file with no notes in it that uses all other
//  features of another file
//*****************************************************************************
void TPimFrame::CreateNewFile( void )
{
    BOOL shouldCopy;
    char fName[MAXPATH];
    wsprintf( fName, "*%s", TOEM::fileExt );

    PTNewDialog dialog = new TNewDialog( this, fName, &shouldCopy );
    if ( IDOK == GetApplication()->ExecDialog( dialog ))
        {
        if (shouldCopy)
            {
            BOOL ok= TSysState::ReadFile( fName, TRUE );
            if (ok)
                {
                TSysState::pAllNotes->flush( TXArray::kDelete );
                }
            }
        else {

	        //housekeeping
            TSysState::FlushSystem();
        }

        // set the catbox's filter again since we just made a new filter
        UpdateCatBox();

        currentFile[0]= '\0';
	    ShowCurrentFileName();
        UpdateButtons();
        NoteReset();
        }
}

//*****************************************************************************
// returns TRUE if opened ok
//*****************************************************************************
BOOL TPimFrame::DoFileOpen( char *fName, BOOL checkWorkTime )
{
BOOL ok= TSysState::ReadFile( fName, checkWorkTime );

ASSERT( ERR_GEN, HealthCheck() );

if( ok )
    {
    //set current file
    strcpy( currentFile, fName );
    ShowCurrentFileName();
    RegisterMRUFile( currentFile );
    TSysState::bSystemDirty= FALSE;
    UpdateButtons();
    UpdateCatBox();
    NoteReset();
    }

return ok;
}


//*****************************************************************************
//
//*****************************************************************************
void TPimFrame::FileOpenCmd(RTMessage)
{
char fName[MAXPATH];

wsprintf( fName, "*%s", TOEM::fileExt );

if( ConfirmClean() )
	{
    if ( IDOK == GetApplication()->ExecDialog(new THTPFileDialog(this, SD_FILEOPEN, fName) ))
		{
        UpdateWindow( HWindow );        //immediately get rid of dialog box

        //read new stuff
        if( !DoFileOpen( fName, TRUE ) )
			{
            TMessageBox::Ok( this, "Unable to read file.", "Read Error" );
            }
        }
	}
}


//*****************************************************************************
//  Merge
//*****************************************************************************
void TPimFrame::FileMergeCmd(RTMessage)
{
char fName[ MAXPATH ];
wsprintf( fName, "*%s", TOEM::fileExt );


        PTHTPFileDialog pDlg= new THTPFileDialog(this, SD_FILEOPEN, fName, _HelpMerge );
        pDlg->SetCaption( "Merge File" );
        if ( IDOK == GetApplication()->ExecDialog( pDlg )) 
            {

            UpdateWindow( HWindow );        //immediately get rid of dialog box
            //read new stuff
            if( TSysState::MergeFile( fName ) )
                {
                UpdateButtons();
                UpdateCatBox();
                NoteReset();
                }
            else
                {
                TMessageBox::Ok( this, "Unable to merge file.", "Merge Error");
                }
            }

ASSERT( ERR_GEN, HealthCheck() );
}


//*****************************************************************************
// Archive
//
//     save the currently selected notes, all of the categories, but no
//     memorized notes
//*****************************************************************************
void TPimFrame::FileArchiveCmd( RTMessage )
{
BOOL rc;
BOOL shouldDelete;

char fName[MAXPATH];
wsprintf( fName, "*%s", TOEM::fileExt );

int numNotes= TSysState::pSelNotes->getItemsInContainer();

//since we archive only selected notes, make sure some notes are
// selected!
if( 0 == numNotes )
    {
    TMessageBox::Ok( this,
        "First, select the notes you want to archive.\n\n(No notes currently selected)",
        "No Selected Notes" );
    return;
    }


if ( IDOK == GetApplication()->ExecDialog(
    new TArchiveD(this, fName, &shouldDelete ) ))
   {

    UpdateWindow( HWindow );        //immediately get rid of dialog box

    //if file already exists, ask for confirmation
    if( 0 == access( fName, 0 ))
        {
        char buf[200];
        wsprintf( buf, "File '%s'\nalready exists.\n\nOverwrite it?", fName );
        if( IDOK != TMessageBox::OkAndCancel( this,
            buf, "Confirm File Overwrite" ) )
            return;
        }

    rc= TSysState::ArchiveFile( fName );

    if( !rc )
        {
        TMessageBox::Ok( this, "Unable to write file.", "Write Error" );
        return;
        }


    // should we delete the notes too?
    if (shouldDelete)
        {

        int n= TSysState::pSelNotes->getItemsInContainer();

        char buf[200];
        wsprintf( buf, "%d notes archived.\n\nPurge notes from system?", n );
        if( IDOK == TMessageBox::OkAndCancel( this,
            buf, "Confirm Note Purge" ) )
            {

            //FIXME this should be function in sysstate
            TCursor::Busy( TRUE );
            for (int i=0; i < n; i++)
                {
                PTNote pNote = &(*TSysState::pSelNotes)[0];
                TSysState::pAllNotes->detach( *pNote );
                TSysState::pSelNotes->detach( *pNote );
                delete pNote;
                }
            TSysState::bSystemDirty = TRUE;

            // regroup and mark dirty bit
            NoteReset();
            TCursor::Busy( FALSE );
            }

        }
    else
        {       //offer confirmation msg
        char buf[200];
        wsprintf( buf, "%d notes archived.", numNotes );
        TMessageBox::Ok( this,  buf, "Notes Archived" );
        }
    }

}


//*****************************************************************************
// check system dirty bit; confirm manual override if not clean
//*****************************************************************************
BOOL TPimFrame::ConfirmClean( void )
{
WORD answer;
BOOL ok= TRUE;

if( TSysState::bSystemDirty )
	{
    char buf[100];
    if( 0 < strlen( currentFile ) )
        wsprintf( buf, "File '%s'\nhas been modified.\n\nSave changes?", currentFile );
    else
        wsprintf( buf, "Untitled file has been modified.\n\nSave changes?" );

    answer= TMessageBox::YesNoCancel( this, buf, "Changed File" );
    if( answer == IDNO )
        ok= TRUE;
    else if( answer == IDYES )
        {
        ok= DoFileSave( );
        }
	else
        ok= FALSE;
	}

return ok;
}

//*****************************************************************************
// Import
//*****************************************************************************
void TPimFrame::FileImportCmd( RTMessage )
{
    // get the path name
    char fName[MAXPATH];
    strcpy( fName, "*.csv" );
    PTImportDialog pDlg= new TImportDialog(
        this, fName );
    pDlg->SetCaption( "Import CSV File" );
    if ( IDOK != GetApplication()->ExecDialog( pDlg ) ) {

        return;
    }

    if ( IDOK != GetApplication()->ExecDialog(
       new TImportMapDialog( this, fName ))) {

       return;
    }

    UpdateWindow( HWindow );        //immediately get rid of dialog box

    if (!ImportFile( fName )) {

        TMessageBox::Ok( this, "Unable to import file.", "Read Error" );
    }
    else {

        NoteReset();
    }

    ASSERT( ERR_GEN, HealthCheck() );

}


//*****************************************************************************
// Export
//*****************************************************************************
void TPimFrame::FileExportCmd( RTMessage )
{

    // get the path name
    char fName[MAXPATH];
    BOOL headlinesOnly;

    int numNotes= TSysState::pSelNotes->getItemsInContainer();

    //since we export only selected notes, make sure some notes are
    // selected!
    if( 0 == numNotes ) {

        TMessageBox::Ok( this,
            "First, select the notes you want to export.\n\n(No notes currently selected)",
            "No Selected Notes" );
        return;
    }

    strcpy( fName, "" );
    if ( IDOK != GetApplication()->ExecDialog(
        new TExportDialog(this, fName, &headlinesOnly ) )) {

        return;
    }

    UpdateWindow( HWindow );        //immediately get rid of dialog box

    //if file already exists, ask for confirmation
    if( 0 == access( fName, 0 )) {

        char buf[200];
        wsprintf( buf, "File '%s'\nalready exists.\n\nOverwrite it?", fName );
        if( IDOK != TMessageBox::OkAndCancel( this,
            buf, "Confirm File Overwrite" ) )
            return;
        }

    if (ExportFile( fName, headlinesOnly )) {

        char buf[200];
        wsprintf( buf, "%d notes exported.", numNotes );
        TMessageBox::Ok( this, buf, "Notes Exported" );

    }
    else{
        TMessageBox::Ok( this, "Unable to export file.", "Write Error" );
    }
}

//-----------------------------------------------------------------
// Arg was just used as file name; push on MRU stack
//
//-----------------------------------------------------------------
void TPimFrame::RegisterMRUFile( char * fileName )
{
TSafStr temp1;
TSafStr temp2;

temp1.Set( TProfile::MRUFile1 );
TProfile::MRUFile1.Set( fileName );

//if str was already first in stack, exit
if( temp1 == fileName )
	return;
	
//now push other files down MRU stack

temp2.Set( TProfile::MRUFile2 );
TProfile::MRUFile2.Set( temp1 );
if( temp2 == fileName )
	return;
	
temp1.Set( TProfile::MRUFile3 );
TProfile::MRUFile3.Set( temp2 );
if( temp1 == fileName )
	return;
	
TProfile::MRUFile4.Set( temp1 );
}

//-----------------------------------------------------------------
// Add current MRU file stack to file menu
//
//-----------------------------------------------------------------
void TPimFrame::PrepareMRUMenu( HMENU hMenuBar )
{
char buf[ 100 ];

//get handle of file menu
HMENU hMenu= GetSubMenu( hMenuBar,0 );

if( TProfile::MRUFile1.Len() )
	{
	wsprintf( buf, "&1 %s", TProfile::MRUFile1.Addr() );
	if( -1 == (int) GetMenuState( hMenu, IDM_FILEFILE1, MF_BYCOMMAND ) )
		{
		InsertMenu( hMenu, IDM_FILEEXIT, 
					MF_BYCOMMAND | MF_STRING, IDM_FILEFILE1, 
					buf );

		//also insert separator
		InsertMenu( hMenu, IDM_FILEEXIT, 
					MF_BYCOMMAND | MF_SEPARATOR,
					0, buf );
		}
	else
		{
		ModifyMenu( hMenu, IDM_FILEFILE1, 
					MF_BYCOMMAND | MF_STRING, IDM_FILEFILE1,
					buf );
		}

    #if 0
    //gray menu item if file doesn't exist
    EnableMenuItem( hMenu, IDM_FILEFILE1,  MF_BYCOMMAND |
        ((0 == access( TProfile::MRUFile1, 0 ))? MF_ENABLED: MF_GRAYED ));
    #endif
    }

if( TProfile::MRUFile2.Len() )
	{
	wsprintf( buf, "&2 %s", TProfile::MRUFile2.Addr() );
	if( -1 == (int) GetMenuState( hMenu, IDM_FILEFILE2, MF_BYCOMMAND ) )
		{
		int pos= GetMenuItemCount( hMenu ) - 2;	//insert before sep
		InsertMenu( hMenu, pos, 
					MF_BYPOSITION | MF_STRING, IDM_FILEFILE2, 
					buf );
		}
	else
		{
		ModifyMenu( hMenu, IDM_FILEFILE2, 
					MF_BYCOMMAND | MF_STRING, IDM_FILEFILE2,
					buf );
		}

    }

if( TProfile::MRUFile3.Len() )
	{
	wsprintf( buf, "&3 %s", TProfile::MRUFile3.Addr() );
	if( -1 == (int) GetMenuState( hMenu, IDM_FILEFILE3, MF_BYCOMMAND ) )
		{
		int pos= GetMenuItemCount( hMenu ) - 2;	//insert before sep
		InsertMenu( hMenu, pos, 
					MF_BYPOSITION | MF_STRING, IDM_FILEFILE3, 
					buf );
		}
	else
		{
		ModifyMenu( hMenu, IDM_FILEFILE3, 
					MF_BYCOMMAND | MF_STRING, IDM_FILEFILE3,
					buf );
		}

	}

if( TProfile::MRUFile4.Len() )
	{
	wsprintf( buf, "&4 %s", TProfile::MRUFile4.Addr() );
	if( -1 == (int) GetMenuState( hMenu, IDM_FILEFILE4, MF_BYCOMMAND ) )
		{
		int pos= GetMenuItemCount( hMenu ) - 2;	//insert before sep
		InsertMenu( hMenu, pos, 
					MF_BYPOSITION | MF_STRING, IDM_FILEFILE4, 
					buf );
		}
	else
		{
		ModifyMenu( hMenu, IDM_FILEFILE4, 
					MF_BYCOMMAND | MF_STRING, IDM_FILEFILE4,
					buf );
		}

    }

}

//-----------------------------------------------------------------
// Read MRU file
//
//-----------------------------------------------------------------
void TPimFrame::ReadMRUFile( int n, BOOL bReportErrors, BOOL checkWorkTime )
{
char const *p;

switch( n )
	{
	case 1:
		p= TProfile::MRUFile1.Addr();
		break;
	case 2:
		p= TProfile::MRUFile2.Addr();
		break;
	case 3:
		p= TProfile::MRUFile3.Addr();
		break;
	case 4:
		p= TProfile::MRUFile4.Addr();
		break;
	default:
		break;
	}

//give sensible error if file doesn't exist
if( 0 != access( p, 0 ))
    {
    char buf[100];
    wsprintf( buf, "File '%s'\nwas not found.", p );
    if( bReportErrors )
        TMessageBox::Ok( this, buf, "File Not Found" );
    return;
    }


if( ConfirmClean() )
	{
    if( TSysState::ReadFile( (char *) p, checkWorkTime ) )
		{
		strcpy( currentFile, p );
		ShowCurrentFileName();
        RegisterMRUFile( currentFile );     //bring it to top of MRU list
        UpdateCatBox();
        TSysState::bSystemDirty= FALSE;
        UpdateButtons();
        NoteReset();
        }
    else if( bReportErrors )
		{
        TMessageBox::Ok( this, "Unable to read file.", "Read Error" );
        }
	}
}

