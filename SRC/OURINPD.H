#ifndef __ourinpd__
#define __ourinpd__

#ifndef __ourdlg__
#include "ourdlg.h"
#endif
#ifndef __safstr__
#include "safstr.h"
#endif

class _CLASSTYPE TOurInputDialog : public TOurDialog
{
public:

    TOurInputDialog( PTWindowsObject parent,
              const char *title,
              const char *prompt,
              RTSafStr buffer,
              DWORD helpID= 0 );

protected:

    virtual void SetupWindow( void );
    virtual void Ok( RTMessage );

private:

    TSafStr     itsPrompt;
    TSafStr     itsTitle;
    PTSafStr    itsBufferP;

    PTStatic    itsPromptCP;
    PTEdit      itsBufferCP;

};

#endif
