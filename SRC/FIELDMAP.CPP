//*****************************************************************************
// import/export field map objects
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#include "fieldmap.h"
#ifndef __oem__
#include "oem.h"
#endif

PTFieldMap TFieldMap::gFieldMapP = NULL;

// -------------------------------------------------
// Constructor
// -------------------------------------------------
TFieldMap::TFieldMap( void )
{
    itsNumImportDataKeys    = 0;
    itsNumExportDataKeys    = 0;
    itsNumImportAddressKeys = 0;
    itsNumExportAddressKeys = 0;
}

// -------------------------------------------------
// Destructor
// -------------------------------------------------
TFieldMap::~TFieldMap()
{
}

// -------------------------------------------------
// AppInit
//
// Initialize the default
// -------------------------------------------------
void TFieldMap::AppInit( void )
{
    gFieldMapP = new TFieldMap;

    GetKeys( "ExportDataKeys",
        "0,1,2,3,4,5,6,7", 0, 7,
        gFieldMapP->itsExportDataKeys,
        gFieldMapP->itsNumExportDataKeys );
    GetKeys( "ImportDataKeys",
        "0,1,2,3,4,5,6,7", 0, 7,
        gFieldMapP->itsImportDataKeys,
        gFieldMapP->itsNumImportDataKeys );
    GetKeys( "ExportAddressKeys",
        "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33", 0, 34,
        gFieldMapP->itsExportAddressKeys,
        gFieldMapP->itsNumExportAddressKeys );
    GetKeys( "ImportAddressKeys",
        "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33", 0, 34,
        gFieldMapP->itsImportAddressKeys,
        gFieldMapP->itsNumImportAddressKeys );
}

// -------------------------------------------------
// AppCleanup
//
// Save the default in the profile
// -------------------------------------------------
void TFieldMap::AppCleanup( void )
{
    PutKeys( "ExportDataKeys",
        gFieldMapP->itsExportDataKeys,
        gFieldMapP->itsNumExportDataKeys );
    PutKeys( "ImportDataKeys", 
        gFieldMapP->itsImportDataKeys,
        gFieldMapP->itsNumImportDataKeys );
    PutKeys( "ExportAddressKeys",
        gFieldMapP->itsExportAddressKeys,
        gFieldMapP->itsNumExportAddressKeys );
    PutKeys( "ImportAddressKeys",
        gFieldMapP->itsImportAddressKeys,
        gFieldMapP->itsNumImportAddressKeys );

    delete gFieldMapP;
}

// -------------------------------------------------
// GetKeys
//
// Read in a table of data keys from the profile
// -------------------------------------------------
void TFieldMap::GetKeys( char *name, char *defaultString,
    int lo, int hi, int *keyTable, int &keyCount )
{
    char    s[500];
    char    *p;

    GetPrivateProfileString( TOEM::profileSection,
        name, defaultString, s, 100,
        TOEM::profileName );

    keyCount = 0;
    p = strtok( s, "," );
    while ((p) && keyCount < kMaxKeys) {

        keyTable[keyCount++] =
            min( hi, max( lo, atoi( p )));
        p = strtok( NULL, "," );
    }
}

// -------------------------------------------------
// PutKeys
//
// Write out a table of data keys from the profile
// -------------------------------------------------
void TFieldMap::PutKeys( char *name,
    int *keyTable, int keyCount )
{
    char s[500];
    char p[500];

    s[0] = '\0';
    for (int i = 0; i < keyCount; i++) {

        wsprintf( p, "%d", keyTable[i] );
        strcat( s, p );
        strcat( s, "," );    
    }

    // ditch the last comma
    s[strlen( s ) - 1] = '\0';
    WritePrivateProfileString( TOEM::profileSection,
        name, s, TOEM::profileName );
}
