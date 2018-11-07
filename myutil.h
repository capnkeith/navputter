
#define dbgprint( fmt, ... )\
{\
    static char buf[80];\
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

extern uint8_t dbgpos;
