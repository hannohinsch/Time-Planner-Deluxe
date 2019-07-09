#ifndef __address__
#define __address__

_CLASSDEF( TAddress )

#ifndef __safstr__
#include "safstr.h"
#endif
#ifndef __csv__
#include "csv.h"
#endif

typedef enum
    {

       kFirstNameField      = 0,
       kLastNameField       = 1,
       kCompanyField        = 2,
       kWAddress1Field       = 3,
       kWAddress2Field       = 4,
       kWAddress3Field       = 5,
       kWCityField           = 6,
       kWStateField          = 7,
       kWZipField            = 8,
       kWCountryField        = 9,
       kWorkPhoneField      = 10,
       kFaxField            = 11,
       kDescriptionField    = 12,
       kLastVersion1Field   = kDescriptionField,

       kMrMsField           = 13,
       kTitleField          = 14,
       kHomePhoneField      = 15,
       kCellPhoneField      = 16,
       kAlt1PhoneField      = 17,
       kAlt2PhoneField      = 18,
       kEMailField          = 19,

       //home address
       kHAddress1Field       = 20,
       kHAddress2Field       = 21,
       kHAddress3Field       = 22,
       kHCityField           = 23,
       kHStateField          = 24,
       kHZipField            = 25,
       kHCountryField        = 26,

       //alt address
       kAAddress1Field       = 27,
       kAAddress2Field       = 28,
       kAAddress3Field       = 29,
       kACityField           = 30,
       kAStateField          = 31,
       kAZipField            = 32,
       kACountryField        = 33,

       kAllFields            = 34,
       kIgnoreAddressField   = 34

    } TAddressField;

//work, home, and alternate address
typedef enum
    {
    kHomeAddress,
    kWorkAddress,
    kAltAddress
    } TWHA;

const int kNumDataFields = (int) kACountryField + 1;

class _CLASSTYPE TAddress: public TStreamable
    {

   public:

       TAddress( void );
       TAddress( RTAddress address );
       TAddress( StreamableInit );
       virtual ~TAddress( void );

       void GetField( TAddressField field, char *buf, int len, TWHA wha= kWorkAddress );
       void GetField( TAddressField field, RTSafStr str, TWHA wha= kWorkAddress );
       void FormatAddr( RTSafStr str, BOOL name, BOOL address, BOOL extra, TWHA wha= kWorkAddress );
       void SetField( TAddressField field, const char *buf, TWHA wha= kWorkAddress );
       BOOL Matches( const char *match );
       TAddress &operator =( const TAddress &other );

       void SetMark( BOOL b ) { itsMark= b; };
       BOOL GetMark( void ) { return itsMark; };

       // streamable functions
       static PTStreamable build();

       TCSVError Export( ofstream &os, BOOL fieldNames= FALSE );
       TCSVError Import( ifstream &is, PTCSV csvP );

       const Pchar streamableName() const { return "TAddress"; };
       void write( Ropstream );
       Pvoid read( Ripstream );

   private:

       void CondFmt( RTSafStr, const char *, TAddressField, const char * );
       TAddressField TranslateWHA( TAddressField, TWHA );
       BOOL WordAppearsInAddress( const char *word );

       PTSafStr  itsFieldPtrs[ kNumDataFields ];
       BOOL      itsMark;
    };

    inline Ripstream operator >> (Ripstream is, RTAddress cl)
        { return is >> (RTStreamable) cl; }

    inline Ripstream operator >> (Ripstream is, RPTAddress cl)
        { return is >> (RPvoid) cl; }

    inline Ropstream operator << (Ropstream os, RTAddress cl)
        { return os << (RTStreamable) cl; }

    inline Ropstream operator << (Ropstream os, PTAddress cl)
        { return os << (PTStreamable) cl; }



#endif
