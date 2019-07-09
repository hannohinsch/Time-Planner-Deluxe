//*****************************************************************************
// ink object
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "ink.h"

#ifndef __classid__
#include "classid.h"
#endif
#ifndef __error__
#include "error.h"
#endif
#ifndef __pen__
#include "pen.h"
#endif
#ifndef __gdimap__
#include "gdimap.h"
#endif

//*****************************************************************************

//*****************************************************************************
TInk::TInk( void )
{
hInk=NULL;
bCompressed= FALSE;
}

//*****************************************************************************

//*****************************************************************************
TInk::TInk( RTInk ink )
{
    *this = ink;
}


//*****************************************************************************

//*****************************************************************************
TInk::~TInk()
{
if( TPen::bPenActive && hInk != NULL )
    {
    DestroyPenData( hInk );
    hInk= (WORD) NULL;
    }
}

//*****************************************************************************
// Operator =
//
//  Creates new ink object, doesn't just point to old one
//*****************************************************************************
RTInk TInk::operator =( RTInk rhs )
{

    bCompressed = rhs.bCompressed;

    // throw away the old ink
    if (hInk != NULL) {

        DestroyPenData( hInk );
    }

    // assign the new ink
    if (TPen::bPenActive && rhs.hInk) {

        hInk = (*TPen::lpfnDuplicatePenData) (rhs.hInk, GMEM_MOVEABLE);
    }
    else {

        hInk = NULL;
    }

    return( *this );
}

//*****************************************************************************

//*****************************************************************************
BOOL TInk::HasInk( void )
{
return (TPen::bPenActive && hInk != NULL);
}

//*****************************************************************************

//*****************************************************************************
void TInk::Compress( void )
{
if( TPen::bPenActive && hInk != NULL && !bCompressed)
    {
    HPENDATA hCompInk= (* TPen::lpfnCompactPenData)( hInk, PDTT_ALL | PDTT_COMPRESS );

    if( hCompInk != NULL )
        {
        hInk= hCompInk; //FIXME is this ok? or does hInk need to be destroyed?
        bCompressed= TRUE;
        }
    }
}

//*****************************************************************************

//*****************************************************************************
void TInk::Decompress( void )
{
if( TPen::bPenActive && hInk != NULL && bCompressed)
    {
    HPENDATA hCompInk= (*TPen::lpfnCompactPenData)( hInk, PDTT_DECOMPRESS );

    if( hCompInk != NULL )
        {
        hInk= hCompInk; //FIXME is this ok? or does hInk need to be destroyed?
        bCompressed= FALSE;
        }
    }
}

//*****************************************************************************

//*****************************************************************************
void TInk::Destroy( void )
{
if( TPen::bPenActive && hInk != NULL )
    {
    DestroyPenData( hInk );
    hInk= (WORD) NULL;
    }
}

//*****************************************************************************
// how big would the ink be if displayed by Show() ?
// adds 4 pixel margin
//*****************************************************************************
void TInk::Measure( RECT *pBound )
{
PENDATAHEADER penHeader;
HPENDATA hDup;

SetRect( pBound, 0, 0, 0, 0 );
if( TPen::bPenActive && hInk )
    {
    Decompress();

    //make local copy
    hDup= (*TPen::lpfnDuplicatePenData)( hInk, GMEM_MOVEABLE );

    //scale local copy into display coordinates
    (*TPen::lpfnMetricScalePenData)( hDup, PDTS_DISPLAY );

    //get bounding rect
    (*TPen::lpfnGetPenDataInfo)( hDup, &penHeader, NULL, (DWORD) 0 );

    DestroyPenData( hDup );

    *pBound= penHeader.rectBound;
    pBound->bottom= (pBound->bottom - pBound->top)/ 2 + 4;
    pBound->top= 0;

    pBound->right = (pBound->right - pBound->left) /2 + 4;
    pBound->left= 0;
    }
}

//*****************************************************************************
//FIXME -- pass in DrawInfo, so can figure out if going to printer
//*****************************************************************************
void TInk::Show( HDC hdc, RECT * bound)
{

if( TPen::bPenActive )
    {
    SaveDC( hdc );

    Decompress();

    //make local copy
    HPENDATA hDup;
    hDup= (*TPen::lpfnDuplicatePenData)( hInk, GMEM_MOVEABLE );

    //scale local copy into display coordinates
    (*TPen::lpfnMetricScalePenData)( hDup, PDTS_DISPLAY );

    //get offset of ink points in original rect
    PENDATAHEADER penHeader;
    (*TPen::lpfnGetPenDataInfo)( hDup, &penHeader, NULL, (DWORD) 0 );

    //scale to half size
    RECT r= penHeader.rectBound;
    r.bottom -= (r.bottom - r.top)/2;
    r.right -= (r.right - r.left)/2;
    (*TPen::lpfnResizePenData)( hDup, &r );

    //place into top left of bounding rect position (leave 2 pixel margins)
    int top= bound->top + 2 - penHeader.rectBound.top;
    int left= bound->left + 2 - penHeader.rectBound.left;
    (*TPen::lpfnOffsetPenData)( hDup, left, top );

    //draw only what fits into rect
    IntersectClipRect( hdc, bound->left, bound->top, bound->right, bound->bottom );

    //draw in black
    //FIXME uses screen pen going to printer, too!
    HPEN hOldPen= SelectObject( hdc, TGDIMap::GetScreenPen( kInkPen ) );

    (*TPen::lpfnDrawPenData)( hdc, NULL, hDup );

    SelectObject( hdc, hOldPen );

    DestroyPenData( hDup );
    RestoreDC( hdc, -1 );
    }
}

//*****************************************************************************

//*****************************************************************************
void TInk::ShowFirstLine( HDC , RECT * )
{
}


//*****************************************************************************
// get handle to ink; decompress first if necessary
//*****************************************************************************
HANDLE TInk::GetInkHandle( void )
{
Decompress();
return hInk;        //might be NULL
}

//*****************************************************************************
// put ink (probably from hedit or bedit control) into ink object
//*****************************************************************************
void TInk::SetInkHandle( HANDLE hCtlInk )
{
if( TPen::bPenActive )
    {
    //destroy old ink
    if( hInk != NULL )
        {
        DestroyPenData( hInk );
        hInk= (WORD) NULL;
        }

    //save latest ink
    if( hCtlInk )
        {
        hInk= (*TPen::lpfnDuplicatePenData) (hCtlInk, GMEM_MOVEABLE);
        }
    }
}

//*****************************************************************************
//
//*****************************************************************************
classType TInk::isA( void ) const
{
return IDInk;
}

char * TInk::nameOf( void ) const
{
return "TInk";
}

hashValueType TInk::hashValue( void ) const
{
return 0; //FIXME
}

int TInk::isEqual(const Object & obj) const
{
return( this == & obj );
}

void TInk::printOn(ostream far&) const
{
//later
}

//*****************************************************************************
// stream support
//*****************************************************************************
PTStreamable TInk::build( void )
{
return( new TInk( streamableInit ) );
}

TStreamableClass RegTInk( "TInk", TInk::build, __DELTA(TInk) );

//*****************************************************************************

//*****************************************************************************
Pvoid TInk::read( Ripstream is )
{
DWORD size;
char *p;
int version;

is >> version;

if( version == 1 )
	{
	is >> size;
	if( size > 0 )
		{
		is >> bCompressed;

		hInk= GlobalAlloc( GMEM_MOVEABLE, size );
		p= (char *) GlobalLock( hInk );

		//Microsoft sez ink can't be more than 64k bytes, hence size_t cast is ok
		is.readBytes( p, (size_t) size );
		GlobalUnlock( hInk );
		}
	else
		{
		hInk= NULL;
		bCompressed= FALSE;
		}
	 }
else
	SYSERR( ERR_VER );

return this;
}

//*****************************************************************************

//*****************************************************************************
void TInk::write( Ropstream os )
{

os << (int) 1;  //version

//pendata
if( hInk == NULL )
    {
    os << (DWORD) 0;        //size
    }
else
    {
    Compress();

    char *p= GlobalLock( hInk );
    DWORD size= GlobalSize( hInk );

    // errors are caught locally so we can clean up, then throw to caller
    CATCHBUF cBuf;
    LPCATCHBUF pOldCBuf= TError::SetCatchBuf( cBuf );
    TErrCode rc= (TErrCode) Catch( cBuf );
    if( rc == 0 )
        {
        os << (DWORD) size;                 //size
        os << bCompressed;                  //compressed?
        //can't be more than 64k
        os.writeBytes( p, (size_t) size );           //data bytes
        }

    //restore saved error handler
    TError::SetCatchBuf( pOldCBuf );

    GlobalUnlock( hInk );

    //signal error to caller
    if( rc )
        SYSTHROW( rc );
    }

}

