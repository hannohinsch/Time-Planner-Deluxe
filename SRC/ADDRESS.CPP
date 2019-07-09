//*****************************************************************************
// address object
//*****************************************************************************
#include "standard.h"
#include "rc.h"
#pragma hdrstop

#include "address.h"
#ifndef __csv__
#include "csv.h"
#endif
#ifndef __fieldmap__
#include "fieldmap.h"
#endif
#ifndef __fldmapd__
#include "fldmapd.h"
#endif

// -----------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------
TAddress::TAddress( void )
{
for( int i= 0; i < kNumDataFields; i++ )
    {
    itsFieldPtrs[i]= new TSafStr();
    }
itsMark= FALSE;
}

// -----------------------------------------------------------------
// Copy Constructor
// -----------------------------------------------------------------
TAddress::TAddress( RTAddress address )
{
for( int i= 0; i < kNumDataFields; i++ )
    {
    itsFieldPtrs[i]= new TSafStr( *address.itsFieldPtrs[i] );
    }

itsMark= address.itsMark;
}

// -----------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------
void TAddress::TAddress( StreamableInit s )
{
for( int i= 0; i < kNumDataFields; i++ )
    {
    itsFieldPtrs[i]= new TSafStr( s );
    }

itsMark= FALSE;
}

// -----------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------
TAddress::~TAddress( void )
{
for( int i= 0; i < kNumDataFields; i++ )
    {
    delete (PTSafStr) itsFieldPtrs[i];
    }
}

//*****************************************************************************

//*****************************************************************************
void TAddress::CondFmt( RTSafStr s, const char *sep1, TAddressField f, const char *sep2 )
{
if( !itsFieldPtrs[ f ]->IsBlank() )
    {
    s.Cat( sep1 );
    s.Cat( *itsFieldPtrs[ f ] );
    s.Cat( sep2 );
    }
}

//*****************************************************************************
// format address fields into a safstr
//*****************************************************************************
void TAddress::FormatAddr( RTSafStr addr, BOOL name, BOOL address, BOOL extra, TWHA wha )
{
addr.Clear();

TAddressField addr1, addr2, addr3, city, state, zip, country;

switch( wha )
    {
    case kHomeAddress:
        addr1= kHAddress1Field;
        addr2= kHAddress2Field;
        addr3= kHAddress3Field;
        city= kHCityField;
        state= kHStateField;
        zip= kHZipField;
        country= kHCountryField;
        break;

    case kAltAddress:
        addr1= kAAddress1Field;
        addr2= kAAddress2Field;
        addr3= kAAddress3Field;
        city= kACityField;
        state= kAStateField;
        zip= kAZipField;
        country= kACountryField;
        break;

    case kWorkAddress:
    default:
        addr1= kWAddress1Field;
        addr2= kWAddress2Field;
        addr3= kWAddress3Field;
        city= kWCityField;
        state= kWStateField;
        zip= kWZipField;
        country= kWCountryField;
        break;
    }

if( name )
    {
    CondFmt( addr, "", kMrMsField, " " );

    addr.Cat( *itsFieldPtrs[ kFirstNameField ] );
    addr.Cat( " " );
    addr.Cat( *itsFieldPtrs[ kLastNameField ] );
    }

if( name && address )
    addr.Cat( "\r\n" );

if( address )
    {
    if( wha == kWorkAddress && !itsFieldPtrs[ kTitleField ]->IsBlank() )
        {
        addr.Cat( *itsFieldPtrs[ kTitleField ] );
        addr.Cat( "\r\n" );
        }

    if( wha == kWorkAddress && !itsFieldPtrs[ kCompanyField ]->IsBlank() )
        {
        addr.Cat( *itsFieldPtrs[ kCompanyField ] );
        addr.Cat( "\r\n" );
        }

    if( !itsFieldPtrs[ addr1 ]->IsBlank() )
        {
        addr.Cat( *itsFieldPtrs[ addr1 ] );
        addr.Cat( "\r\n" );
        }

    if( !itsFieldPtrs[ addr2 ]->IsBlank() )
        {
        addr.Cat( *itsFieldPtrs[ addr2 ] );
        addr.Cat( "\r\n" );
        }

    if( !itsFieldPtrs[ addr3 ]->IsBlank() )
        {
        addr.Cat( *itsFieldPtrs[ addr3 ] );
        addr.Cat( "\r\n" );
        }

    BOOL cityLine= FALSE;
    if( !itsFieldPtrs[ city ]->IsBlank() )
        {
        addr.Cat( *itsFieldPtrs[ city ] );
        cityLine= TRUE;
        }

    if( !itsFieldPtrs[ state ]->IsBlank() )
        {
        addr.Cat( ", " );
        addr.Cat( *itsFieldPtrs[ state ] );
        cityLine= TRUE;
        }

    if( !itsFieldPtrs[ zip ]->IsBlank() )
        {
        addr.Cat( "  " );
        addr.Cat( *itsFieldPtrs[ zip ] );
        cityLine= TRUE;
        }

    if( cityLine )
        addr.Cat( "\r\n" );

    if( !itsFieldPtrs[ country ]->IsBlank() )
        {
        addr.Cat( *itsFieldPtrs[ country ] );
        addr.Cat( "\r\n" );
        }
    }

if( address && extra )
    {
    addr.Cat( "\r\n" );
    }

if( extra )
    {
    CondFmt( addr, "Work:  ",  kWorkPhoneField, "\r\n" );
    CondFmt( addr, "Fax:  ",   kFaxField, "\r\n" );
    CondFmt( addr, "Cell:  ",  kCellPhoneField, "\r\n" );
    CondFmt( addr, "Home:  ",  kHomePhoneField, "\r\n" );
    CondFmt( addr, "Alt1:  ", kAlt1PhoneField, "\r\n" );
    CondFmt( addr, "Alt2:  ", kAlt2PhoneField, "\r\n" );
    CondFmt( addr, "E-Mail:  ", kEMailField, "" );
    }

}

// -----------------------------------------------------------------
// GetField
// -----------------------------------------------------------------
void TAddress::GetField( TAddressField field, char *buf, int len, TWHA wha )
{
TSafStr str;
GetField( field, str, wha );
str.Get( buf, len );
}

// -----------------------------------------------------------------
// GetField
// -----------------------------------------------------------------
void TAddress::GetField( TAddressField field, RTSafStr str, TWHA wha )
{
    if( field < kNumDataFields )
        {
        field= TranslateWHA( field, wha );
        str.Set( *itsFieldPtrs[ field ] );
        }
    else
        str.Clear();
}

//*****************************************************************************
// translate from the default "work" address identifiers to the
// proper work, home, or alternate ids.
//*****************************************************************************
TAddressField TAddress::TranslateWHA( TAddressField field, TWHA wha )
{
if( wha == kHomeAddress || wha == kAltAddress )
    {
    switch( field )
        {
        case kWAddress1Field:
            if( wha == kHomeAddress )
                field= kHAddress1Field;
            else
                field= kAAddress1Field;
            break;

        case kWAddress2Field:
            if( wha == kHomeAddress )
                field= kHAddress2Field;
            else
                field= kAAddress2Field;
            break;

        case kWAddress3Field:
            if( wha == kHomeAddress )
                field= kHAddress3Field;
            else
                field= kAAddress3Field;
            break;

        case kWCityField:
            if( wha == kHomeAddress )
                field= kHCityField;
            else
                field= kACityField;
            break;

        case kWStateField:
            if( wha == kHomeAddress )
                field= kHStateField;
            else
                field= kAStateField;
            break;

        case kWZipField:
            if( wha == kHomeAddress )
                field= kHZipField;
            else
                field= kAZipField;
            break;

        case kWCountryField:
            if( wha == kHomeAddress )
                field= kHCountryField;
            else
                field= kACountryField;
            break;

        default:
            break;
        }
    }

return field;
}

// -----------------------------------------------------------------
// SetField
// -----------------------------------------------------------------
void TAddress::SetField( TAddressField field, const char *buf, TWHA wha )
{
    if( field < kNumDataFields )
        {
        field= TranslateWHA( field, wha );
        itsFieldPtrs[ field ]->Set( buf );
        }
}

// -----------------------------------------------------------------
// Matches
// -----------------------------------------------------------------
BOOL TAddress::Matches( const char *match )
{
    char temp[1024];

    strcpy( temp, match );
    char *word = strtok( temp, " " );
    while (word) {

        if (!WordAppearsInAddress( word )) {

            return FALSE;
        }
        word = strtok( NULL, " " );
    }

    return TRUE;
}

// -----------------------------------------------------------------
// WordAppearsInAddress
// -----------------------------------------------------------------
BOOL TAddress::WordAppearsInAddress( const char *word )
{
    for( int i= 0; i < kNumDataFields; i++ )
        {
        if( itsFieldPtrs[i]->Contains( word ) )
            return TRUE;
        }

    return FALSE;
}

// -----------------------------------------------------------------
// Operator =
// -----------------------------------------------------------------
TAddress &TAddress::operator =( const TAddress &other )
{
    for( int i= 0; i < kNumDataFields; i++ )
        {
        itsFieldPtrs[i]->Set( *other.itsFieldPtrs[i] );
        }

    return( *this );
}




//*****************************************************************************
// stream support
//*****************************************************************************
PTStreamable TAddress::build( void )
{
return( new TAddress( streamableInit ) );
}

TStreamableClass RegTAddress( "TAddress", TAddress::build, __DELTA(TAddress) );

//*****************************************************************************
// Read
//
//    Reads in fields only if it's a real address
//*****************************************************************************
Pvoid TAddress::read( Ripstream is )
{
    int version;

    is >> version;

    if( version == 7 )
        {    // read itsMark
        int idx;

        is >> itsMark;

        is >> idx;

        for( int i= kNumDataFields - 1 ; 0 <= i; i-- )
            {
            if( i <= idx )
                itsFieldPtrs[i]->read( is );
            else
                itsFieldPtrs[i]->StreamFix();

            }

        }
    else if( version == 6 )
        {    // written backwards
        int idx;

        is >> idx;

        for( int i= kNumDataFields - 1 ; 0 <= i; i-- )
            {
            if( i <= idx )
                itsFieldPtrs[i]->read( is );
            else
                itsFieldPtrs[i]->StreamFix();

            }

        }
    else if( version == 5 )
        {    // many new fields for v2

        for( int i= 0; i < kNumDataFields; i++ )
            {
            itsFieldPtrs[i]->read( is );
            }

        }
    else if( version == 4 )
        {  // added separate first, last name

        for( int i= 0; i < kNumDataFields; i++ )
            {
            if( i > kLastVersion1Field)
                itsFieldPtrs[ i ]->StreamFix();
            else
                itsFieldPtrs[i]->read( is );
            }

        }
    else if (version == 3)
        {     //write fields directly

        for( int i= 0; i < kNumDataFields; i++ )
            {
            if( i > kLastVersion1Field || i == kLastNameField )
                itsFieldPtrs[ i ]->StreamFix();
            else
                itsFieldPtrs[i]->read( is );
            }

        }
    else if (version == 2)
        {     //added address2, and 3 (version 1.00)

        for( int i= 0; i < kNumDataFields; i++ )
            {
            if( i > kLastVersion1Field || i == kLastNameField )
                itsFieldPtrs[ i ]->StreamFix();
            else
                is >> *itsFieldPtrs[i];
            }

        }
    else
        {
        SYSERR( ERR_VER );
        }

    return this;
}

//*****************************************************************************
// Write
//
//*****************************************************************************
void TAddress::write( Ropstream os )
{
    os << (int) 7;      //version

    os << itsMark;

    //write backwards, skipping all empty fields before first active field
    BOOL writeEm= FALSE;
    for( int i= kNumDataFields - 1 ; 0 <= i; i-- )
        {
        if( !writeEm && !itsFieldPtrs[ i ]->IsBlank() )
            {
            writeEm= TRUE;
            os << i;
            }

        if( writeEm )
            itsFieldPtrs[i]->write( os );
        }


    if( !writeEm )
        os << (int) -1;            //all fields were empty
}


//*****************************************************************************
// Export
//
//    Writes out fields in csv format
//*****************************************************************************
TCSVError TAddress::Export( ofstream &os, BOOL fieldNames )
{
    TCSV csv;

    int numFields = TFieldMap::gFieldMapP->itsNumExportAddressKeys;
    for (int i = 0; i < numFields; i++) {

        TSafStr s;
        BOOL eol = (i == numFields - 1);
        TAddressField field = (TAddressField) TFieldMap::gFieldMapP->itsExportAddressKeys[i];
        if( fieldNames )
            s.Set( ::addressFieldNames[ field ] );
        else
            GetField( field, s );

        csv.WriteString( os, s, eol );
    }

    return( kCSVOk );
}

//*****************************************************************************
// Import
//
//    Reads ins fields in csv format
//*****************************************************************************
#define CHECK_ERROR( ERROR )        \
                                    \
    switch( ERROR ) {               \
                                    \
        case kCSVOk:                \
                                    \
            break;                  \
                                    \
        case kCSVErrorCancel:       \
        case kCSVErrorSkip:         \
                                    \
            return( ERROR );        \
    }

#define CHECK_EOL_ERROR( LASTFIELD, IS, EOL )         \
    if (LASTFIELD) {                                  \
        CHECK_ERROR( csvP->ShouldBeEOL( IS, EOL ) );  \
    }                                                 \
    else {                                            \
        CHECK_ERROR( csvP->ShouldntBeEOL( IS, EOL )); \
    }

TCSVError TAddress::Import( ifstream &is, PTCSV csvP )
{
    BOOL lastField;
    BOOL eol;
    for (int i = 0; i < TFieldMap::gFieldMapP->itsNumImportAddressKeys; i++)
        {
        lastField = (i == TFieldMap::gFieldMapP->itsNumImportAddressKeys - 1);
        eol = FALSE;
        TSafStr s;
        TAddressField field = (TAddressField)
            TFieldMap::gFieldMapP->itsImportAddressKeys[i];
        CHECK_ERROR( csvP->ReadString( is, s, eol ));
        SetField( field, s );
        CHECK_EOL_ERROR( lastField, is, eol );
        }

    // there's an error if we're at the eol, but we're
    // not out of fields yet
    CHECK_EOL_ERROR( lastField, is, eol );

    return( kCSVOk );
}
