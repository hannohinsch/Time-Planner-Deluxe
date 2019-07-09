#ifndef __snoop__
#define __snoop__

_CLASSDEF( TSnoop )

class _CLASSTYPE TSnoop
{
    public:
        TSnoop();
        virtual ~TSnoop();

        static void CheckForEmpty( void );
        static void Identify( void );

    protected:
        typedef enum {
            kCatNew,
            kCatDel,
            kCatVis,
            kNoteChg,
            kNoteSetChg,
            kTimeChg,                       //current time changed
            kScaleChg                       //time scale changed
            } TSnoopMsg;

        void Notify( TSnoopMsg msg, RObject aObject );
        virtual void Listen( TSnoopMsg, RObject ) {};

    private:
        static TNSCollection observers;
};

#endif
