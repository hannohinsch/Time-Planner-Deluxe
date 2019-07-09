//*****************************************************************************
// print address book
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "addrbook.h"
#include "address.h"
#include "cat.h"
#include "catlist.h"
#include "gdimap.h"
#include "paint.h"
#include "print.h"

//*****************************************************************************

//*****************************************************************************
void TAddrGrid::TAddrGrid( PTCatList catListP )
{
itsMasterListP= catListP;
itsCatListP= new TCatList( *catListP );

SetColCnt( 3 );
SetRowCnt( itsCatListP->getItemsInContainer() );

prtPainter= new TPaintPrt;

SetPrtTitle( "Address Book" );
SetDefaultPrintRange();
}

//*****************************************************************************

//*****************************************************************************
void TAddrGrid::~TAddrGrid( void )
{
delete itsCatListP;
delete prtPainter;
}

//*****************************************************************************
// prune the address list to only marked addresses, if need be
//*****************************************************************************
void TAddrGrid::PrintPrep( BOOL markedOnly )
{
if( markedOnly )
    {
    delete itsCatListP;
    itsCatListP= new TCatList;

    int n= itsMasterListP->getItemsInContainer();
    for( int i= 0; i < n; i++ )
        {
        RTCat cat= (*itsMasterListP)[ i ];
        if( cat.GetAddrMark() )
            {
            itsCatListP->add( cat );
            }
        }
    }
else
    {
    delete itsCatListP;
    itsCatListP= new TCatList( *itsMasterListP );
    }

SetRowCnt( itsCatListP->getItemsInContainer() );

}

//*****************************************************************************

//*****************************************************************************
int  TAddrGrid::CellWidth( PTPaint, int col )
{
int w;

RECT r;
TPrint::GetPrtPixels( &r );

if( col == 0 )
    w= 250;
else if( col == 1 )
    w= 225;
else
    w= r.right - r.left - 475;

return w;
}

//*****************************************************************************

//*****************************************************************************
int TAddrGrid::CellHeight( PTPaint, int row )
{
int h;

if( row >= rowCnt )
    return 0;

RTCat cat= ((*itsCatListP)[ row ]);

TSafStr str;

//
// work address
//
int work= 0;

cat.GetField( kTitleField, str );
if( !str.IsBlank() )
    work++;

cat.GetField( kWAddress1Field, str );
if( !str.IsBlank() )
    work++;

cat.GetField( kWAddress2Field, str );
if( !str.IsBlank() )
    work++;

cat.GetField( kWAddress3Field, str );
if( !str.IsBlank() )
    work++;

cat.GetField( kWCityField, str );
if( !str.IsBlank() )
    work++;

cat.GetField( kWCountryField, str );
if( !str.IsBlank() )
    work++;

//
//count phone lines
//
int phone= 0;

cat.GetField( kWorkPhoneField, str );
if( !str.IsBlank() )
    phone++;

cat.GetField( kFaxField, str );
if( !str.IsBlank() )
    phone++;

cat.GetField( kHomePhoneField, str );
if( !str.IsBlank() )
    phone++;

cat.GetField( kCellPhoneField, str );
if( !str.IsBlank() )
    phone++;

cat.GetField( kAlt1PhoneField, str );
if( !str.IsBlank() )
    phone++;

cat.GetField( kAlt2PhoneField, str );
if( !str.IsBlank() )
    phone++;

cat.GetField( kEMailField, str );
if( !str.IsBlank() )
    phone++;


//
//count home address lines
//
int home= 0;

cat.GetField( kHAddress1Field, str );
if( !str.IsBlank() )
    home++;

cat.GetField( kHAddress2Field, str );
if( !str.IsBlank() )
    home++;

cat.GetField( kHAddress3Field, str );
if( !str.IsBlank() )
    home++;

cat.GetField( kHCityField, str );
if( !str.IsBlank() )
    home++;

cat.GetField( kHCountryField, str );
if( !str.IsBlank() )
    home++;

//
//count alternate address lines
//
int alt= 0;

cat.GetField( kAAddress1Field, str );
if( !str.IsBlank() )
    alt++;

cat.GetField( kAAddress2Field, str );
if( !str.IsBlank() )
    alt++;

cat.GetField( kAAddress3Field, str );
if( !str.IsBlank() )
    alt++;

cat.GetField( kACityField, str );
if( !str.IsBlank() )
    alt++;

cat.GetField( kACountryField, str );
if( !str.IsBlank() )
    alt++;

//
// analyze line counts
//
int lines= phone + 1;

if( work >= lines )
    lines= work + 2;

if( home > 0 )
    lines+= home + 3;

if( alt > 0 )
    lines+= alt + 3;

h= 15 + lines * 15;

return h;
}

//*****************************************************************************
// PaintCell
//*****************************************************************************
void  TAddrGrid::PaintCell( RTDrawInfo di )
{
TSafStr str, str1;
RTCat cat= ((*itsCatListP)[ di.row ]);

RECT r= di.cellRect;

if( di.col == 0 )       //name and addresses
    {
    r.bottom= r.top + 15;
    di.pPainter->FillRectangle( di, r, kHbrAddrTitle );
    cat.FormatAddr( str, TRUE, FALSE, FALSE );  //name
    di.pPainter->Text( di, str, kNoHilite, kSingleLine, kBigBoldFont, &r );

    cat.FormatAddr( str, FALSE, TRUE, FALSE, kWorkAddress );

    cat.FormatAddr( str1, FALSE, TRUE, FALSE, kHomeAddress );
    if( !str1.IsBlank() )
        {
        str.Cat( "\r\nHOME ADDRESS\r\n" );
        str.Cat( str1 );
        }

    cat.FormatAddr( str1, FALSE, TRUE, FALSE, kAltAddress );
    if( !str1.IsBlank() )
        {
        str.Cat( "\r\nALTERNATE ADDRESS\r\n" );
        str.Cat( str1 );
        }

    r= di.cellRect;
    r.top+= 15;
    di.pPainter->Text( di, str, kNoHilite, kMultiLine, kBigFont, &r );
    }
else if( di.col == 1 )     //tel and fax
    {
    r.bottom= r.top + 15;
    di.pPainter->FillRectangle( di, r, kHbrAddrTitle );
    str= "Telephone numbers";
    di.pPainter->Text( di, str, kNoHilite, kSingleLine, kItalicFont, &r );

    cat.FormatAddr( str, FALSE, FALSE, TRUE );

    r= di.cellRect;
    r.top+= 15;
    di.pPainter->Text( di, str, kNoHilite, kMultiLine, kBigFont, &r );
    }
else if( di.col == 2 )     //description
    {
    r.bottom= r.top + 15;
    di.pPainter->FillRectangle( di, r, kHbrAddrTitle );
    str= "Description";
    di.pPainter->Text( di, str, kNoHilite, kSingleLine, kItalicFont, &r );

    cat.GetField( kDescriptionField, str );

    r= di.cellRect;
    r.top+= 15;
    di.pPainter->Text( di, str, kNoHilite, kMultiLine, kBigFont, &r );
    }

}

