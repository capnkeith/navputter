 /*Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2017 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <limits.h>

#include <Arduino.h>
#include "usb_dev.h"
#include "usb_serial.h"
#include "usb_mouse.h"
#include "IntervalTimer.h"


void navputter_yield( void );
class navputter_serial_class : public usb_serial_class
{
    public:
        virtual int read() 
        {
            int i  = usb_serial_class::read();
            navputter_yield();
            return i;
        }
};


navputter_serial_class myser;
usb_mouse_class mymouse;
usb_keyboard_class mykey;
IntervalTimer mytimer;

volatile uint32_t global_ticks=0;
uint32_t last_watchticks=0;

void event( int event, int p1, int p2 );
void process_mouse( void );
char serial_read_gpio_port(void);
void serial_read_gpio(void);
void tick_watchdog(void);

enum mouse_directions
{
    NP_MOUSE_UP=0,
    NP_MOUSE_DOWN,
    NP_MOUSE_LEFT,
    NP_MOUSE_RIGHT,
    NP_TOTAL_MOUSE_DIRS
};

enum mouse_buttons
{
    MB_LEFT=0,
    MB_MIDDLE,
    MB_RIGHT,
    NP_TOTAL_MOUSE_BUTTONS
};



enum events             /* main event type for do_event() */
{
    EVENT_KEYPAD_UP=0,
    EVENT_KEYPAD_DOWN
};

enum keypad_states      /* keypad states for reading each row */
{
    KP_SET_COLS=0,
    KP_WAIT,
    KP_READ_ROWS,
};

enum mouse_key_modes    /* global modes for slow keyboard / fast keyboard / mouse */
{
    MOUSE_MODE=0,
    KEY_SLOW_MODE,
    KEY_FAST_MODE,
    PUKE
};


enum time_constants
{ 
    SETTLE_KEY_BOUNCE = 20,     // milliseconds
    KEY_POLL_INTERVAL = 10,     // milliseconds
};

#define INVALID_KEYSTATE 0xff


#define MAX_KEY_ROWS        4
#define MAX_KEY_COLS        4

#define LINE "--------------------------------------------------------------------------------"





// Add/Modify the _ED_ macros to change the format of the eeprom. The parameters below are:
//    _ED_(  
//                p1=command letter                         used for the serial menus 
//                p2= struct type,                          type in the eeprom struct itself
//                p3= default value (factory reset),        default value written on factory reset
//                p4= min value,                            minimum settable value
//                p5= max value,                            maximum settable value
//                p6= printf format,                        format string
//                p7=custom set func ptr,                   optional callback to set the value 
//                p8=help string                            descriptive string of the paramer
//      )

#define _EEPROM_DESC_\
    _ED_( 'v', version,      uint16_t,    0x0101,        0, 0xff,           "%-8x",    NULL,              "EEPROM version. Change to reset device to factory default." )\
    _ED_( 'r', rows,         uint8_t,     MAX_KEY_ROWS,  1, MAX_KEY_ROWS,   "%-8d",    NULL,              "Number of rows of keys on your keypad." )\
    _ED_( 'c', cols,         uint8_t,     MAX_KEY_COLS,  1, MAX_KEY_COLS,   "%-8d",    NULL,              "Number of columns of keys on your keypad." )\
    _ED_( 'f', flip_rows,    uint8_t,     1,             0, 1,              "%-8d",    NULL,              "Flip keypad rows." )\
    _ED_( 'l', flip_cols,    uint8_t,     1,             0, 1,              "%-8d",    NULL,              "Flip keypad columns." )\
    _ED_( 's', mouse_step,   uint8_t,     2,             1, 32,             "%-8d",    NULL,              "Mouse step." )\
    _ED_( 'k', key_maps,     uint8_t,     1,             1, 4,              "%-8d",    adjust_keymaps,    "Total keymaps in memory." )\
    _ED_( 'x', screen_x,     uint16_t,    1920,          2, 7680,           "%-8d",    NULL,              "Total keymaps in memory." )\
    _ED_( 'y', screen_y,     uint16_t,    1080,          2, 3240,           "%-8d",    NULL,              "Total keymaps in memory." )\
    _ED_( 'm', screen_mac,   uint8_t,     1,             0, 1,              "%-8d",    NULL,              "Mac Mouse." )\
// endo fo _EEPROM_DESC_ list


#define _ED_( c, _field_, _type_, d, min, max, f, func, help ) _type_ _field_;
typedef struct eeprom_header
{
    _EEPROM_DESC_
}eeprom_header_t;
#undef _ED_

char *eeprom_start = NULL;


#define EEPROM_SIZE 2048    // NOTE: this should come from teensycore ...

typedef struct eeprom_layout
{
    union
    {
        eeprom_header_t config;
        uint8_t         raw[ EEPROM_SIZE ];
    };
}eeprom_layout_t;


typedef struct key_map
{
    uint8_t     action;
    uint8_t     p1; 
    uint8_t     p2;
    uint8_t     p3;
}key_map_t;

typedef struct global_state
{   
    eeprom_header_t config;
    uint8_t         keystate[ MAX_KEY_COLS ];
    uint8_t         last_keystate[ MAX_KEY_COLS ];
    uint8_t         keypress[MAX_KEY_ROWS][ MAX_KEY_COLS ];
    key_map_t       cur_map[MAX_KEY_ROWS][MAX_KEY_COLS];
    uint8_t         mouse_moves[NP_TOTAL_MOUSE_DIRS];
    uint8_t         mouse_clicks[NP_TOTAL_MOUSE_BUTTONS];
}global_state_t;




#define dbgprint(str, ... )\
{\
        char tmpbuf[1024];\
        if ( !myser.available() )\
        {\
            sprintf(tmpbuf,str,##__VA_ARGS__);\
            if ( myser.availableForWrite() ) myser.write(tmpbuf, strlen(tmpbuf));\
        }\
}


enum key_actions
{
    KA_NO_ACTION  = 0,          /* do nothing on keypress      */
    KA_KEY_ACTION,              /* inject a keystroke          */
    KA_MOUSE_UP,                /* move mouse up               */
    KA_MOUSE_DOWN,              /* move mouse down             */
    KA_MOUSE_LEFT,              /* move mouse left             */
    KA_MOUSE_RIGHT,             /* move mouse right            */
    KA_MOUSE_LT_CLICK,          /* click left mouse button     */
    KA_MOUSE_LT_DBBL_CLICK,     /* double click left mouse button */
    KA_MOUSE_RT_CLICK,          /* click right mouse button     */
    KA_MOUSE_RT_DBL_CLICK,      /* double click right mouse button */
    KA_MOUSE_MID_CLICK,         /* click right mouse button     */
    KA_MOUSE_MID_DBL_CLICK,     /* double click right mouse button */
    KA_MOUSE_STEP,              /* cycle mouse step 1, 2, 4     */
    KA_REPORT_KEY,              /* show key pressed on serial  */
};


key_map_t   temp_map[MAX_KEY_ROWS][MAX_KEY_COLS] =
{
        {{KA_REPORT_KEY,'1'},     {KA_REPORT_KEY,'2'}, {KA_REPORT_KEY,'3'},     {KA_REPORT_KEY, 'A'}},
        {{KA_REPORT_KEY,'4'},     {KA_REPORT_KEY,'5'}, {KA_REPORT_KEY,'6'},     {KA_REPORT_KEY, 'B'}},
        {{KA_REPORT_KEY,'7'},     {KA_REPORT_KEY,'8'}, {KA_REPORT_KEY,'9'},     {KA_REPORT_KEY, 'C'}},
};

key_map_t   base_map[MAX_KEY_ROWS][MAX_KEY_COLS] =
{
        {{KA_KEY_ACTION,'+'}, {KA_MOUSE_UP,'2'}, {KA_KEY_ACTION,'-'}, {KA_REPORT_KEY, 'A'}},
        {{KA_MOUSE_LEFT,'4'}, {KA_MOUSE_STEP}, {KA_MOUSE_RIGHT,'6'}, {KA_REPORT_KEY, 'B'}},
        {{KA_REPORT_KEY,'7'}, {KA_MOUSE_DOWN,'8'}, {KA_REPORT_KEY,'9'}, {KA_REPORT_KEY, 'C'}},
        {{KA_MOUSE_LT_CLICK,'*'}, {KA_REPORT_KEY,'0'}, {KA_MOUSE_RT_CLICK,'#'}, {KA_REPORT_KEY, 'D'}},
};

void enable_watchdog(void)
{
    WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;
    WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
    delayMicroseconds(1); // Need to wait a bit..
    WDOG_STCTRLH = 0x0001; // Enable WDG
    WDOG_TOVALL = 200; // The next 2 lines sets the time-out value. This is the value that the watchdog timer compare itself to.
    WDOG_TOVALH = 0;
    WDOG_PRESC = 0; // This sets prescale clock so that the watchdog timer ticks at 1kHZ instead of the default 1kHZ/4 = 200 HZ
    last_watchticks = global_ticks = 1;
}

global_state_t global_config={
    {0x0100,4,4,1,1,1},              /* version 1.1, 4 rows, 4 cols, flip rows 1, flip cols 1, mouse step 1 */

    { INVALID_KEYSTATE},
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
};

void eeprom_init(void)
{
    eeprom_header_t hdr={0};
    eeprom_header_t hdr_default ={0};
    eeprom_initialize();
    
    while( !eeprom_is_ready() )
    {
        tick_watchdog();
    }
    eeprom_read_block((void *)&hdr, eeprom_start, sizeof( hdr ));
#define _ED_( c, _field_, t, _default_, m, mx, fmt, fnunc, str ) hdr_default._field_ = _default_;
    _EEPROM_DESC_
#undef _ED_    

    if ( hdr_default.version != hdr.version )
    {
        memcpy( (void *)&global_config.config, (void *)&hdr_default, sizeof( hdr_default ) );
        eeprom_write_block((void *)&global_config.config, eeprom_start, sizeof(global_config.config));
    }
    else
    {
        memcpy( (void *)&global_config.config, (void *)&hdr, sizeof( hdr_default ) );
    }
}

void tickme(void)
{
    global_ticks++;
}


void process_mouse_click( uint8_t button, uint8_t event )
{
    if ( event == EVENT_KEYPAD_DOWN )
    {
        global_config.mouse_clicks[ button ] = 1;
    }
    else
    {
        global_config.mouse_clicks[ button ] = 0;   
    }
    usb_mouse_buttons(global_config.mouse_clicks[MB_LEFT], global_config.mouse_clicks[MB_MIDDLE], global_config.mouse_clicks[MB_RIGHT],0,0 );
}
 


void process_keypad_event( uint8_t event, uint8_t row, uint8_t col )
{
    row = (global_config.config.flip_rows)?global_config.config.rows - row - 1:row;
    col = (global_config.config.flip_cols)?global_config.config.cols - col - 1:col;
    uint8_t action = global_config.cur_map[row][col].action;
    switch(action)
    {
        case KA_KEY_ACTION:
            if ( event == EVENT_KEYPAD_DOWN )
            {
                mykey.write(global_config.cur_map[row][col].p1);
                mykey.write(0);
            }
            else mykey.write(0); 
            break;
        case KA_MOUSE_LEFT:
            global_config.mouse_moves[ NP_MOUSE_LEFT ] = ( event == EVENT_KEYPAD_DOWN ) ? global_config.config.mouse_step : 0; 
            break;
        case KA_MOUSE_RIGHT:
            global_config.mouse_moves[ NP_MOUSE_RIGHT ]= ( event == EVENT_KEYPAD_DOWN ) ? global_config.config.mouse_step : 0; 
            break;
        case KA_MOUSE_UP:
            global_config.mouse_moves[ NP_MOUSE_UP ] = ( event == EVENT_KEYPAD_DOWN ) ? global_config.config.mouse_step : 0; 
            break;
        case KA_MOUSE_DOWN:
            global_config.mouse_moves[ NP_MOUSE_DOWN ] = ( event == EVENT_KEYPAD_DOWN ) ? global_config.config.mouse_step : 0; 
            break;
        case KA_REPORT_KEY:
            dbgprint("# report %s : %d,%d = %c\n\r", (event == EVENT_KEYPAD_DOWN)?"DOWN":"UP", row, col, global_config.cur_map[row][col].p1 );
            break;
        case KA_MOUSE_LT_CLICK:
            process_mouse_click( MB_LEFT, event );
            break;
        case KA_MOUSE_MID_CLICK:
            process_mouse_click( MB_MIDDLE, event );
            break;
        case KA_MOUSE_RT_CLICK:
            process_mouse_click( MB_RIGHT, event );
            break;
        case KA_MOUSE_STEP:
            if ( event == EVENT_KEYPAD_UP )
            {
                global_config.config.mouse_step = global_config.config.mouse_step << 1;
                if ( global_config.config.mouse_step >= 8 ) global_config.config.mouse_step = 1;
                dbgprint("mouse step %d\n", global_config.config.mouse_step ); 
            }
            break;
        default:
            dbgprint("ERROR: unknown event %d in process_keypad_event() %s %d\n", event, __FILE__, __LINE__ );
            break;
    }
}


void trigger_keypad()
{
    if ( global_config.last_keystate[0] == INVALID_KEYSTATE )
    {
        memcpy( global_config.last_keystate, global_config.keystate, global_config.config.cols );
        return;
    }
    else
    {
		int col;
		int row;
        for ( col=0; col< global_config.config.cols; col++ )
        {
            if ( global_config.keystate[col] != global_config.last_keystate[col] )
            {
				for ( row=0; row< global_config.config.rows; row++ )
				{
					if ( global_config.keystate[col] & (1<<row) )
					{
						if ( global_config.keypress[row][col]  != EVENT_KEYPAD_DOWN )
						{
							process_keypad_event( EVENT_KEYPAD_DOWN, row, col );
						}
						global_config.keypress[row][col] = EVENT_KEYPAD_DOWN; 
					}
					else
					{
						if ( global_config.keypress[row][col] != EVENT_KEYPAD_UP )
						{
							process_keypad_event( EVENT_KEYPAD_UP, row, col);
						}
						global_config.keypress[row][col] = EVENT_KEYPAD_UP; 
					}
				 }
            }
            global_config.last_keystate[col] = global_config.keystate[col]; 
        }
    }
                
}

 
void readkeypad(void)
{
    static uint8_t  keypad_state = KP_SET_COLS;
    static uint8_t  next_state=0xff;
    static uint8_t  col=0;
    static uint32_t until = 0;
    uint8_t cur_rows;

    switch( keypad_state )
    {
        case KP_SET_COLS:
            PORTD &= 0xf0;  // set low nibble to zero
            PORTD |= ~(1<<col);    // set col bit to 1

            keypad_state = KP_WAIT;
            until = global_ticks + SETTLE_KEY_BOUNCE;
            next_state = KP_READ_ROWS;
            break;

        case KP_WAIT:
            if ( global_ticks < until ) return;
            keypad_state = next_state;
            break;

        case KP_READ_ROWS:
        
            cur_rows = PIND;
            cur_rows &= 0xf0;
            cur_rows = ~(cur_rows >> 4)&0x0f;

            global_config.keystate[col] = cur_rows;
            keypad_state =KP_SET_COLS;
            col++;
            if ( col >= global_config.config.cols ) 
            {
                col=0;
                trigger_keypad();
            }
            break;
            
        default:
            dbgprint("puke on default switch %s %d\n", __FILE__, __LINE__ );
            break;
    }
}


void key_up_event( int row, int col )
{
    dbgprint("# key_up_event on %d %d\r\n", row, col);
}
      
void key_down_event( int row, int col )
{
    dbgprint("# key_down_event on %d %d\r\n", row, col);
}
      
void init_keys(void)
{
    memcpy( global_config.cur_map, base_map, sizeof( temp_map ));
}


#define _BIG_WHALE_ \
    _BW_( "Welcome to Navputter.")\
    _BW_( "Thank you LUFA and PRJC!!!." )\
    _BW_( "" )\
    _BW_( "           Watch out for me!")\
    _BW_( "     .-'       ")\
    _BW_( "'--./ /     _.---.")\
    _BW_( "'-,  (__..-`       \\")\
    _BW_( "   \\         .     |")\
    _BW_( "    `,.__.   ,__.--/")\
    _BW_( "      '._/_.'___.-`")\
    _BW_( "        /  /")\
    _BW_( "       :__/")\
// end of _BIG_WHALE_ thar she blows!



#define _SER_CMDS_\
    _SC_( 'h', "Show this help.", usage )\
    _SC_( 'f', "Show or update eeprom.", handle_eeprom )\
    _SC_( 'k', "Emulate keypad with serial commands.", serial_keypad )\
    _SC_( 'g', "Interface with GPIO via serial.", serial_gpio )
// end of _SER_CMDS_

 

void print_eeprom(void)
{
    dbgprint("\n\r\n\r%s\n\r", LINE );
    dbgprint("EEPROM Menu. Please be careful. Select field to change or reset.\r\n");
    dbgprint("%s\n\r", LINE );
    dbgprint("CMD %-12s%-8s%-8s%-8s%-8s%s\r\n", "field",  "default",  "current",  "min",  "max", "description");
    dbgprint("%s\n\r", LINE );
#define _ED_( cmd, _field_, type, _default_, _min_, _max_, _format_, func, _help_ ) \
    dbgprint("%c)  %-12s"\
        _format_    /* print current value */ \
        _format_    /* print default value */ \
        _format_    /* print min value */ \
        _format_    /* print max value */ \
        "%s\r\n",\
        cmd, \
        #_field_, \
        _default_, \
        global_config.config._field_, \
        _min_, _max_, _help_ \
    );
_EEPROM_DESC_
#undef _ED_
    dbgprint("w)  write.\r\n\r\n");
    dbgprint("q)  quit.\r\n\r\n");
    dbgprint("CMD (be careful):");
}


char serial_read_gpio_port(void)
{
    while(1)
    {
        uint8_t c = myser.read();
        if (c==0xff) continue;
        c = tolower(c);
        if ((c >='a') && ( c <= 'd')) return c;
        return 0;
    }
}

int hextoint( char *buf )
{
    int v=0;
    char c;
    while(*buf)
    {
        if ( v ) v=v<<4;
        if ( isdigit( *buf ) )
        {
            v |= (*buf - '0');
        }
        else if ( (*buf == 'x') || (*buf == 'X') )
        {
        }
        else  
        {
            c = tolower( *buf );
            v |= (c - 'a' + 0x10);
        }
        buf++;
    }
    dbgprint("hex %s is %x\n", buf, v );
    return v;
}


int serial_read_hex_int(int min, int max, uint8_t *err)
{
    *err = 1;
#define MAX_INT_SIZE 25
    char buf[MAX_INT_SIZE];
    uint8_t i = 0; 
    while(1)
    {
        uint8_t c = myser.read();
        if ( c==0xff ) continue;
        if ( ((c >= '0') && (c <= '9')) || ((c>= 'a') && (c <='f')) || ((c>='A') && (c <= 'F')))
        {
            buf[i++]=c;
            myser.write(c);
            if ( i>=MAX_INT_SIZE )
            {
                dbgprint("size error\n");
                return 0;
            } 
        }   
        else if (( c== '\n' ) || (c == '\r'))
        {
            buf[i]=0;
            int v = hextoint(buf);
            if (( v < min ) || ( v > max) )
            {
                dbgprint("ERROR: range error. Must be between %x and %x\n", min, max);
                return 0;
            }
            else
            {
                *err = 0;
                dbgprint("read hex value %x\n", v );
                return v;
            }
        }
        else
        {
            dbgprint("ERROR: not a number. 0 %d is 1 is %d\n\r", '0', '1');
            return 0;
        }                                
    }
    return 0;
} 


int serial_read_int(int min, int max, uint8_t *err)
{
    *err = 1;
#define MAX_INT_SIZE 25
    char buf[MAX_INT_SIZE];
    uint8_t i = 0; 
    dbgprint("Enter a value between %d and %d:", min, max);
    while(1)
    {
        uint8_t c = myser.read();
        if ( c==0xff ) continue;
        if ( (c >= '0') && (c <= '9') )
        {
            buf[i++]=c;
            myser.write(c);
            if ( i>=MAX_INT_SIZE )
            {
                dbgprint("number too long.\n\r");
                return 0;
            } 
        }   
        else if (( c== '\n' ) || (c == '\r'))
        {
            buf[i]=0;
            int v = atoi(buf);
            if (( v < min ) || ( v > max) )
            {
                dbgprint("\n\rRange error.\n\rEnter a number between %d and %d:", min, max );
                i=0;
            }
            else
            {
                *err = 0;
                return v;
            }
        }
        else
        {
            dbgprint("not a number. 0 %d is 1 is %d\n\r", '0', '1');
            return 0;
        }                                
    }
    return 0;
} 

int serial_get_command( char *str )
{
    while(1)
    {
        uint8_t c = myser.read();
        char *ptr;
        if ( c == 0xff ) continue;
        for ( ptr=str; *ptr !=0; ptr++ ) 
        {
            if ( *ptr == c ) 
            {
                return c;
            }
        }
    }
} 

void write_eeprom(void)
{
    eeprom_write_block((void *)&global_config.config, eeprom_start, sizeof(global_config.config));
    dbgprint("EEPROM saved.\r\n");
}

void handle_eeprom(void)
{
    char cmd_str[] = "19q";
    while(1)
    {
        int dirty=0;
        uint8_t c;
        print_eeprom();
        while(1)
        {
            c = myser.read();
            if (c == 0xff) continue;
#define _ED_( _cmd_, _field_, t, _default_, _min_, _max_, _ft_, _func_, h ) \
            if ( c == _cmd_ )\
            {\
                dbgprint("\n\r\n\rModify Paramter: \"%s\"?\n\r", #_field_ );\
                dbgprint("    1) Change\r\n");\
                dbgprint("    9) Reset\r\n" );\
                dbgprint("    q) Quit\r\n" );\
                char command_char = serial_get_command(cmd_str);\
                if ( command_char == '1' ) \
                {\
                    uint8_t err;\
                    int v  = serial_read_int(_min_,_max_,&err);\
                    if ( !err )\
                    {\
                        dirty=1;\
                        global_config.config._field_ = v;\
                    }\
                }\
                else if ( command_char == '9' )\
                {\
                    dirty=1;\
                    global_config.config._field_ = _default_;\
                    dbgprint("Value reset.\r\n");\
                    break;\
                } \
                else if ( command_char == 'q' )\
                {\
                    print_eeprom();\
                } \
                else\
                {\
                    dbgprint("What? Please 1, 9, or q\r\n");\
                } \
            }
            _EEPROM_DESC_
#undef _ED_

            if ( c == 'q' )
            {
                if ( dirty )
                {
                    while(1)
                    {
                        char yes_no[]="yn";
                        dbgprint("EEPROM has changed. Abandon change? (y/n)\n\r" );
                
                        char c = serial_get_command( yes_no ); 
                        if ( c == 'y' ) return;
                        if ( c == 'n' ) break; 
                    }
                }
            }
            else if ( c== 'w' )
            {
                write_eeprom();
                return;
            }
            print_eeprom();
        }
    }   
}
         



void serial_keypad(void)
{
    dbgprint("# Serial keypad.  \n");
    while(1)
    {
        char v = myser.read();
        if ( v == 0xff ) continue;
        uint8_t r;
        uint8_t c;
        uint8_t row, col;
        for ( r=0; r< global_config.config.rows; r++ )
        {
            for ( c=0; c< global_config.config.cols; c++ )
            {
                row = (global_config.config.flip_rows)?global_config.config.rows - r - 1:r;
                col = (global_config.config.flip_cols)?global_config.config.cols - c - 1:c;
                if ( global_config.cur_map[r][c].p1 == v )
                {
                    process_keypad_event( EVENT_KEYPAD_DOWN, row, col );
                    process_mouse();
                    process_keypad_event( EVENT_KEYPAD_UP, row,col );
                    goto NEXT_PRESS;
                }
            }
        }
NEXT_PRESS:
        if ( v == 'q' ) return;
    }
}

void serial_read_gpio(void)
{
    while(1)
    {
        uint8_t port = serial_read_gpio_port();
        if ( port == 0 ) 
        {
            dbgprint("ERROR: gpio read format incorrect. Use R<port>. Port a-d, case insenstive.\r\n");
            return;
        }   
        uint8_t v=0;
        switch( port )
        {
//            case 'a' : v = PINA;
            case 'a' : v = 0;
                dbgprint("PINA not working on teensy3.2...\n");
                break;
            case 'b' : v = PINB;
                break;
            case 'c' : v = PINC;
                break;
            case 'd' : v = PIND; 
                break;
            default:
                dbgprint("insane in membrane %s %d\n", __FILE__, __LINE__ );
                break;
        }
        dbgprint("r%c OK:0x%x\r\n",port,v);
        return;
    }
}

void serial_set_data_direction(void)
{
    uint8_t port = serial_read_gpio_port();
    if ( port == 0 )
    {
        dbgprint("ERROR: gpio data direction format incorrect. Use d<port><value>. i.e. db0f sets port D 0-3 output, 4-7 input\r\n");
        return;
    }
    uint8_t err;
    int value = serial_read_hex_int( 0x0, 0xff, &err );
    if ( err )
    {      
        dbgprint("ERROR: gpio data direction formt is in hex. d<port><hex value>.\r\n");
        dbgprint("   example: db02, or db0x02 sets port B bit 2 as an output all the others inputs\n\r");
        return;
    }
    port = tolower(port);
    switch( port )
    {
        case 'a' : /*DDRA = (value&0xff); */ dbgprint("ERROR: no port A on teensy 3.2...\n"); break;
        case 'b' : DDRB = (value&0xff); break;
        case 'c' : DDRC = (value&0xff); break;
        case 'd' : DDRD = (value&0xff); break;
        default:
            dbgprint("ERROR: illegal port %c(%d), at %s %d\n", port, port, __FILE__, __LINE__ );
            break;
    }
    dbgprint("PORT%c = %x. OK.\r\n", port, value );
}



void serial_write_gpio(void)
{
    uint8_t port = serial_read_gpio_port();
    if ( port == 0 )
    {
        dbgprint("ERROR: gpio write format incorrect. Use W<port><value>. i.e. Wb0f sets port D 0b000011111\r\n");
        return;
    }
    uint8_t err;
    int value = serial_read_hex_int( 0x0, 0xff, &err );
    if ( err )
    {      
        dbgprint("ERROR: gpio write formt is in hex. W<port><hex value>.\r\n");
        dbgprint("   example: WD02, or WD0x02 writes the low order 2 bits to port D\n\r");
        return;
    }
    port = tolower(port);
    switch( port )
    {
        case 'a' : /*DDRA = (value&0xff); */ dbgprint("ERROR: no port A on teensy 3.2...\n"); break;
        case 'b' : PORTB = (value&0xff); break;
        case 'c' : PORTC = (value&0xff); break;
        case 'd' : PORTD = (value&0xff); break;
        default:
            dbgprint("ERROR: illegal port %c(%d), at %s %d\n", port, port, __FILE__, __LINE__ );
            break;
    }
    dbgprint("PORT%c = %x. OK.\r\n", port, value );
}

void gpio_usage(void)
{
    dbgprint("Serial GPIO. Not all bits available on all micro controllers.\n");
    dbgprint("Use ports: A, B, C, D.\n\r");
    dbgprint("Commands: \r\n");
    dbgprint("    R<port>            - read entire port value as 8 bit hex (PIN) \r\n");
    dbgprint("    W<port><value>     - write 8 bit hex value to port (PORT = value) \r\n");
//    dbgprint("    r<port><bit>       - read bit from port and position (PIN & 1<<bit)\r\n");
//    dbgprint("    w<port><bit><val>  - write bit to port and position  (PORT &= ~((1<< bit) & val)\r\n");
//    dbgprint("    t<port><bit>       - toggle bit at port and position (PORT ^= 1<<bit) \r\n");
    dbgprint("    D<port><value>     - set all data direction bits of one port at oen time ( DDR = value )\r\n");
//    dbgprint("    d<port><bit><value>- set a single data direction bit of one port ( DDR &= ~((1<< bit) & value)\r\n");
    dbgprint("    h                  - this dialog\r\n");
    dbgprint("    q                  - quit\r\n");
}
 

void serial_gpio(void)
{
//    char gpio_cmds[] = "qhtwrWR";
    char gpio_cmds[] = "qhrw";
    gpio_usage();

    while(1)
    {
        char c = serial_get_command( gpio_cmds );
        switch(c)
        {
            case 'q' : return;
            case 'h' : gpio_usage(); break;
//        case 'd' : serial_set_data_direction(); break;
            case 'r' : serial_read_gpio(); break;
            case 'w' : serial_write_gpio(); break;
            default:
                dbgprint("ERROR: unknown gpio command %c  - %s %d\n", c, __FILE__, __LINE__ );
                return;
                break;
//        case 'r' : serial_read_gpio_bit(); break;
//       case 'w' : serial_write_gpio_bit(); break;
 //      case 't' : serial_toggle_gpio_bit(); break;
        }
    }
}


void usage(void)
{
    dbgprint("Main Menu Commands:\r\n");
#define _SC_( cmd, str, func ) dbgprint(" %c) - %s\n\r", (int)cmd, str );
    _SER_CMDS_
#undef _SC_
   
    dbgprint("\n\r\r\rPress command letter:");
}
 

void serial_init(void)
{
    int i;
    uint8_t blink=1;
    for (i=0; i<20; i++ )
    {
        if ( myser ) break;
        _delay_ms(100);
        digitalWriteFast(13, blink );
        tick_watchdog();
        blink ^= 1;
    }
    myser.clear();
    myser.flush();

#define _BW_(str) dbgprint( str"\r\n" );
    _BIG_WHALE_
#undef _BW_
    dbgprint("\n");
    usage();
    myser.flush();
}

void process_mouse( void )
{
    mymouse.move( 
        global_config.mouse_moves[ NP_MOUSE_RIGHT] - global_config.mouse_moves[ NP_MOUSE_LEFT],
        global_config.mouse_moves[ NP_MOUSE_DOWN ] -  global_config.mouse_moves[ NP_MOUSE_UP ], 0, 0
    );
}

void process_serial(void )
{
    uint8_t byte = myser.read();
    if ( byte != 0xff )
    {
        if ( byte == '\n' || byte == '\r' )
        {
            usage();
        }
        else
        {
#define _SC_( c, str, func ) if ( (c) == (byte) ) func();
            _SER_CMDS_
#undef _SC_
            usage();
        }
    }
}


void tick_watchdog(void)
{
    if ( !last_watchticks ) return;
    if ( last_watchticks + 2 < global_ticks ) /* required. */
    {
        last_watchticks = global_ticks;
        cli();
        WDOG_REFRESH = 0xA602;
        WDOG_REFRESH = 0xB480;
        sei();
    }
}

void navputter_yield( void )
{
    tick_watchdog();
}

#define ONE_SECOND 1000

extern "C" int main(void)
{
    mytimer.begin(tickme, 1000 );
    enable_watchdog();
    eeprom_init();
    usb_init();
    while( !usb_configuration )
    {
        _delay_ms(1);
        tick_watchdog();
    }

    mymouse.begin();
    mymouse.screenSize(global_config.config.screen_x, global_config.config.screen_y, global_config.config.screen_mac);
    myser.begin(9600);       
    pinMode(13, OUTPUT);
    DDRD =0x0f;
    init_keys();
    
    serial_init();

    uint32_t last_tick=0;
    uint32_t blink_tick=0;
    uint8_t flag=0;
    
    while (1) 
    {
        if ( myser.peek()  )
        {
            process_serial();
        }
        if ( global_ticks != last_tick )
        {
            last_tick = global_ticks;
            readkeypad();   
            process_mouse();
            tick_watchdog();    
        }
        if ( global_ticks - blink_tick >= ONE_SECOND )
        {
            blink_tick = global_ticks;
            flag ^= 0x20;
            PORTB = flag;   
            myser.flush();
            myser.clear();
        }
    }
}

