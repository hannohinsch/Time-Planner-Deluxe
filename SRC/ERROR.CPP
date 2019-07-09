//*****************************************************************************
// misc. system error routines
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#ifndef __frame__
#include "frame.h"
#endif

#include "error.h"

//static variables
TErrCode     TError::itsErrNum;
char *       TError::itsErrFile= "";
int          TError::itsErrLine= 0;
LPCATCHBUF   TError::itsCatchBufP= NULL;
BOOL         TError::itsMemMessageB= FALSE;

//*****************************************************************************
//  puts up message box, then continues
//*****************************************************************************
void TError::SysError( TErrCode errNum, char *errFile, int errLine )
{
char err[200];

if( errNum < 0 )        //Borland's EM_XXXX values are all < 0
    {
    GetApplicationObject()->Error( errNum );
    }
else // ours are all > 0
    {
    wsprintf( err, "err= %d; file= %s; line= %d", (int) errNum, errFile, errLine );
	MessageBox( NULL, err, "System Error", MB_ICONHAND | MB_SYSTEMMODAL );
    }
}

//*****************************************************************************
//  puts up message box, then continues
//*****************************************************************************
void TError::SysErrorStr( char *message, char *errFile, int errLine )
{
    char err[200];

    wsprintf( err, "%s\nfile= %s; line= %d", message, errFile, errLine );
	MessageBox( NULL, err, "System Error", MB_ICONHAND | MB_SYSTEMMODAL );
}

//*****************************************************************************
//  throws to surrounding catch
//*****************************************************************************
void TError::SysThrow( TErrCode errNum, char *errFile, int errLine )
{
TError::itsErrNum= errNum;
TError::itsErrFile= errFile;
TError::itsErrLine= errLine;

//always throws error 1; real code is kept in TError::itsErrNum
if( TError::itsCatchBufP )
    Throw( TError::itsCatchBufP, 1 );
else // we tried to throw an error w/o handler -> very bad news (quit app)
    {
    char err[ 200 ];
    wsprintf( err, "%s\nfile= %s; line= %d", "Missing error handler", errFile, errLine );
	MessageBox( NULL, err, "System Error", MB_ICONHAND | MB_SYSTEMMODAL );
    if( TPimFrame::pFrame )
        TPimFrame::pFrame->CloseWindow();
    }
}


//*****************************************************************************
// process error info in error variables
//*****************************************************************************
void TError::ProcessSysCatch( void )
{
char err[200];
char *pMsg;

if( itsErrNum < 0 )        //Borland's EM_XXXX values are all < 0
    {
    GetApplicationObject()->Error( itsErrNum );
    }
else // ours are all > 0
    {
    //FIXME enumerate all the error codes here
    switch( itsErrNum )
        {
        case ERR_NEW:
            pMsg= "Out of Memory!";
            break;
        case ERR_HEADER:
            pMsg= "File does not appear to be a valid project file";
            break;
        case ERR_VERSION:
            pMsg=  "Incompatible file version";
            break;
        default:
            pMsg= "System Error";
            break;
        }

    wsprintf( err, "%s\nerr= %d; file= %s; line= %d", pMsg, itsErrNum, itsErrFile, itsErrLine );
    MessageBox( NULL, err, "Error", MB_ICONHAND | MB_SYSTEMMODAL );
    }
}

//*****************************************************************************
//
//*****************************************************************************
LPCATCHBUF TError::SetCatchBuf( LPCATCHBUF pBuf )
{
LPCATCHBUF pOldBuf= itsCatchBufP;

itsCatchBufP= pBuf;

return pOldBuf;
}

//*****************************************************************************
// return TRUE if safety pool intact; if low, (optionally) tell user
//*****************************************************************************
BOOL TError::MemOK( BOOL bTell )
{
BOOL rc;

PTApplication pApp= GetApplicationObject();

// try to allocate safety pool -- some other app may have freed mem,
// in which case we probably don't know about it
// harmless if pool is already allocated
pApp->RestoreMemory();

if( pApp->LowMemory() )
	{
	rc= FALSE;
	if( bTell )
		{
		MessageBox(NULL, "Low memory! -- save your file right away!", "Low Memory",
									 MB_SYSTEMMODAL | MB_ICONHAND);
        itsMemMessageB= TRUE;
		}
    }
else
	{
	rc= TRUE;
    if( itsMemMessageB && bTell )
		{
        MessageBox(NULL, "There now seems to be enough memory to go on.", "Memory OK",
								 MB_OK | MB_ICONEXCLAMATION);
        itsMemMessageB= FALSE;
        }
	}

return rc;
}

#if 0
//*****************************************************************************
//check for cat objects that are not in global tree
//*****************************************************************************
void TError::ChkOrphan( void )
{
static int lastCount= 0;
int count= 0;

RArrayIterator i= (RArrayIterator) TGlobal::pAllNotes->pNoteArray->initIterator();
while( (int) i )
	{
	RTNote note= (RTNote) i++;

	if( NOOBJECT != (RObject) note )
		{
		//check each cat in catlist, looking for orphans
		RArrayIterator j= (RArrayIterator) note.pCatList->pCatArray->initIterator();
		while( (int) j )
			{
			RTCat cat= (RTCat) j++;

			if( NOOBJECT != (RObject) cat )
				{
				if( ! TGlobal::pCategories->IsCatInTree( cat ) )
					count++;
				}
			}

		delete &j;

		}
	}

delete &i;

if( count != lastCount )
	{
	char buf[100];
	wsprintf( buf, "changed orphan count: %d", count );
    TMessageBox::Ok( this, buf, "Orphan cats discovered!" );
	lastCount= count;
	}
}
#endif

