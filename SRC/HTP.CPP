//*****************************************************************************
//  main PIM application objects (TApplication and TMDIFrame)
//*****************************************************************************

#include "standard.h"
#pragma hdrstop

#include "rc.h"

#ifndef __frame__
#include "frame.h"
#endif
#ifndef __utility__
#include "utility.h"
#endif
#ifndef __sysstate__
#include "sysstate.h"
#endif
#ifndef __error__
#include "error.h"
#endif

#include "memchk.h"
#include "objstrm.h"
#include "oem.h"

//*****************************************************************************
// class declarations
//*****************************************************************************

_CLASSDEF(TPimApp)

class _CLASSTYPE TPimApp : public TApplication {
    public:
		TPimApp(LPSTR name, HANDLE hInstance, HANDLE hPrevInstance,
				LPSTR lpCmd, int nCmdShow)
				: TApplication(name, hInstance, hPrevInstance, lpCmd, nCmdShow)
				  {};

		void InitMainWindow( void );
		void InitInstance( void );
        void MessageLoop( void );
    };
  


//*****************************************************************************
// Construct the TPimApp's MDI frame,
//  loading its menu
//*****************************************************************************
void TPimApp::InitMainWindow( void )
{
MainWindow = new TPimFrame( TOEM::frameTitle );
}
 
//*****************************************************************************
// Initialize each MS-Windows application instance, loading an
//   accelerator table
//*****************************************************************************
void TPimApp::InitInstance( void )
{

TApplication::InitInstance();

#if 0
if ( Status == 0 )
    {
    HAccTable = LoadAccelerators(hInstance, MAKEINTRESOURCE( IDM_ACCELERATORS ) );

    if ( HAccTable == 0 )
        Status = EM_INVALIDWINDOW;
    }
#endif

}

//*****************************************************************************
// override standard message loop
//
// we use getmsg, not peekmsg
// also adds system error handler
//
// note that we never call IdleLoop(), unlike std MessageLoop
//*****************************************************************************
void TPimApp::MessageLoop( void )
{
CATCHBUF cBuf;
MSG Message;

TError::SetCatchBuf( cBuf );
if( 0 != Catch( cBuf ) )
    TError::ProcessSysCatch();

while ( GetMessage(&Message, 0, 0, 0) )
    {
    if ( !ProcessAppMsg(&Message) )
        {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
        }
    }

Status = Message.wParam;
}

//*****************************************************************************
// WinMin()
//*****************************************************************************
int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance,
		   LPSTR lpCmd, int nCmdShow)
{
    //check Windows version
    if( !IsWin31() )
        {
        MessageBox( NULL, "Requires Windows 3.1 or later.", "Unsupported Windows Version", 0 );
        return -1;
        }

    //check for VGA-sized adaptor
    WORD scrHeight= GetSystemMetrics( SM_CYSCREEN );
    WORD scrWidth = GetSystemMetrics( SM_CXSCREEN );

    if( ! ((scrHeight >= 480 && scrWidth >= 640) ||
        (scrHeight >= 640 && scrWidth >= 480) ))
        {
        MessageBox( NULL, "Requires VGA (or larger) display.\nYou may see odd screen behavior.", "Unsupported Display", 0 );
        }

    //get count of number of memory allocations made so far
    TMemChk memChk;
    memChk.Checkpoint();

    TPimFrame::ShowStartMsg( hInstance, scrHeight, scrWidth );

    //need a call to BWCC to force linker to include it
    BWCCGetVersion();

    TPimApp App( TOEM::frameTitle, hInstance, hPrevInstance,
		lpCmd, nCmdShow);

    App.Run();

    TSnoop::CheckForEmpty();

    memChk.Assert();

    return App.Status;
}

