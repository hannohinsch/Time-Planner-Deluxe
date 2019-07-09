#ifndef __message__
#define __message__

_CLASSDEF( TMessageBox )


class _CLASSTYPE TMessageBox {

    public:

    static void Ok(          PTWindowsObject parent,
        const char *message, const char *title );
    static int  OkAndCancel( PTWindowsObject parent,
        const char *message, const char *title );
    static int  YesNoCancel( PTWindowsObject parent,
        const char *message, const char *title );
    static int  ThreeButton( PTWindowsObject parent,
        const resId, const char *message, const char *title,
        const char *yesText, const char *noText );
};

#endif
