#ifndef __msgbox__
#define __msgbox__

//PS OurMessageBox is defined in util.cpp

#define MESSAGEBOX(a,b,c,d) OurMessageBox(a,b,c,d)

int OurMessageBox( WORD handle, char *title, char *msg, WORD flags );

#endif
