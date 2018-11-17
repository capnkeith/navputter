#include "uTFT_ST7735.h"

#ifdef DEBUG
extern uint8_t dbgpos;
#define dbgprint( fmt, ... )\
{\
    char buf[80];\
    setCursor(0,dbgpos);\
    print(" ");\
    dbgpos = (dbgpos < 159) ? dbgpos + 8 : 0;\
	setCursor(0,dbgpos);\
    print("                              ");\
	setCursor(0,dbgpos);\
    sprintf( buf, fmt, ##__VA_ARGS__ );\
    print( ">" );\
    print( buf );\
    print( "    " );\
}
#else
#define dbgprint( fmt, ... )
#endif


#define printtext( x, y, fmt, ... )\
{\
    char buf[80];\
    setCursor( x, y );\
    print("                     ");\
    sprintf( buf, fmt, ##__VA_ARGS__ );\
    setCursor( x, y );\
    print(buf);\
}


