#ifndef __fieldmap__
#define __fieldmap__

_CLASSDEF( TColor )
_CLASSDEF( TFieldMap )

const int kColorDataKey      = 0;
const int kTextDataKey       = 1;
const int kUnitDataKey       = 2;
const int kStartDateDataKey  = 3;
const int kStartTimeDataKey  = 4;
const int kDurationDataKey   = 5;
const int kValueDataKey      = 6;
const int kRateDataKey       = 7;
const int kIgnoreDataKey     = 8;
const int kNumDataKeys       = 9;

const int kMaxKeys = 50;

class _CLASSTYPE TFieldMap
    {
    public:

    TFieldMap( void );
    virtual ~TFieldMap( void );

    int      itsNumImportDataKeys;
    int      itsNumExportDataKeys;
    int      itsNumImportAddressKeys;
    int      itsNumExportAddressKeys;

    int      itsImportDataKeys[kMaxKeys];
    int      itsExportDataKeys[kMaxKeys];
    int      itsImportAddressKeys[kMaxKeys];
    int      itsExportAddressKeys[kMaxKeys];

    static PTFieldMap gFieldMapP;
    static void AppInit( void );
    static void AppCleanup( void );

    private:

    static void TFieldMap::GetKeys(
        char *name, char *defaultString,
        int lo, int hi,
        int *keyTable, int &keyCount );
    static void TFieldMap::PutKeys( char *name,
        int *keyTable, int keyCount );
    };

#endif
