//*****************************************************************************
// object observer mechanism
//*****************************************************************************
#include "standard.h"
#pragma hdrstop

#ifndef  __cursor__
#include "cursor.h"
#endif
#include "frame.h"
#include "snoop.h"

TNSCollection TSnoop::observers(10, 10);

//*****************************************************************************
// add this object to the list of observers
//*****************************************************************************
TSnoop::TSnoop( void )
{
observers.insert( (void *) (PTSnoop) this );
}


//*****************************************************************************
// remove object from list of observers
//*****************************************************************************
TSnoop::~TSnoop( void )
{
observers.remove( (void *) (PTSnoop) this );
}

//*****************************************************************************
// call all Listen() fns
//*****************************************************************************
void TSnoop::Notify( TSnoopMsg aMsg, RObject aObject )
{
int i;

TCursor::Busy( TRUE );

for( i= 0; i < observers.getCount(); i ++ )
    {
    PTSnoop pSnoop= (PTSnoop) observers.at( i );

    if( pSnoop != this )                //don't tell sender
        pSnoop->Listen( aMsg, aObject );
    }

TCursor::Busy( FALSE );
}

//*****************************************************************************
// CheckForEmpty
//
//  If there are still observers then put up a warning. This method
//  is called just before the program exits.
//*****************************************************************************
void TSnoop::CheckForEmpty( void )
{
    if (observers.getCount() > 0) {

        char s[255];
        wsprintf( s, "%d Objects still in Snoop set", observers.getCount() );
        TMessageBox::Ok( TPimFrame::pFrame,
             s,
            "Internal Error" );


        for (int i = 0; i < observers.getCount(); i++) {

            PObject object = (PObject) observers.at( i );
            wsprintf( s, "Object name: %s", object->nameOf());
            TMessageBox::Ok( TPimFrame::pFrame,
                s, "Internal Error" );
        }
    }
}

//*****************************************************************************
// Identify
//
//  Identify all of the items
//*****************************************************************************
void TSnoop::Identify( void )
{
    #if 0
    for (int i = 0; i < observers.getCount(); i++) {

        PObject object = (PObject) observers.at( i );
        classType t = object->isA();

    }
    #endif

}

