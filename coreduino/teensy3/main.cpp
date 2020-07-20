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

usb_serial_class myser;
usb_mouse_class mymouse;
usb_keyboard_class mykey;
IntervalTimer mytimer;

uint32_t global_ticks=0;

void event( int event, int p1, int p2 );

enum mouse_directions
{
    NP_MOUSE_UP=0,
    NP_MOUSE_DOWN,
    NP_MOUSE_LEFT,
    NP_MOUSE_RIGHT,
    NP_TOTAL_MOUSE_DIRS
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


enum constants
{ 
    SETTLE_KEY_BOUNCE = 20, // milliseconds
    KEY_POLL_INTERVAL = 10,     // milliseconds
    INVALID_KEYSTATE= 0xff,
};



#define MAX_KEY_ROWS        4
#define MAX_KEY_COLS        4
typedef struct eeprom_header
{
    uint8_t version;
    uint8_t rows;
    uint8_t cols;
    uint8_t flip_rows;
    uint8_t flip_cols;
    uint8_t mouse_step;
    uint16_t unused2; 
}eeprom_header_t;

typedef struct key_map
{
    uint8_t     action;
    uint8_t     p1; 
    uint8_t     p2;
    uint8_t     p3;
}key_map_t;

typedef struct eeprom_layout
{   
    eeprom_header_t config;
    uint8_t         keystate[ MAX_KEY_COLS ];
    uint8_t         last_keystate[ MAX_KEY_COLS ];
    uint8_t         keypress[MAX_KEY_ROWS][ MAX_KEY_COLS ];
    key_map_t       cur_map[MAX_KEY_ROWS][MAX_KEY_COLS];
    uint8_t         mouse_moves[NP_TOTAL_MOUSE_DIRS];
}eeprom_layout_t;


#define EEPROM_KEY_MAP ((eeprom_layout_t *)NULL)->key_map
#define EEPROM_KEY_SEQ ((eeprom_layout_t *)NULL)->key_seq



enum key_actions
{
    KA_NO_ACTION  = 0,       /* do nothing on keypress      */
    KA_KEY_ACTION,           /* inject a keystroke          */
    KA_MOUSE_UP,             /* move mouse up               */
    KA_MOUSE_DOWN,           /* move mouse down             */
    KA_MOUSE_LEFT,           /* move mouse left             */
    KA_MOUSE_RIGHT,          /* move mouse right            */
    KA_REPORT_KEY,           /* show key pressed on serial  */
};


key_map_t   temp_map[MAX_KEY_ROWS][MAX_KEY_COLS] =
{
        {{KA_REPORT_KEY,'1'}, {KA_REPORT_KEY,'2'}, {KA_REPORT_KEY,'3'}, {KA_REPORT_KEY, 'A'}},
        {{KA_REPORT_KEY,'4'}, {KA_REPORT_KEY,'5'}, {KA_REPORT_KEY,'6'}, {KA_REPORT_KEY, 'B'}},
        {{KA_REPORT_KEY,'7'}, {KA_REPORT_KEY,'8'}, {KA_REPORT_KEY,'9'}, {KA_REPORT_KEY, 'C'}},
        {{KA_REPORT_KEY,'*'}, {KA_REPORT_KEY,'0'}, {KA_REPORT_KEY,'#'}, {KA_REPORT_KEY, 'D'}},
};

key_map_t   base_map[MAX_KEY_ROWS][MAX_KEY_COLS] =
{
        {{KA_REPORT_KEY,'1'}, {KA_MOUSE_UP,'2'}, {KA_REPORT_KEY,'3'}, {KA_REPORT_KEY, 'A'}},
        {{KA_MOUSE_LEFT,'4'}, {KA_REPORT_KEY,'5'}, {KA_MOUSE_RIGHT,'6'}, {KA_REPORT_KEY, 'B'}},
        {{KA_REPORT_KEY,'7'}, {KA_MOUSE_DOWN,'8'}, {KA_REPORT_KEY,'9'}, {KA_REPORT_KEY, 'C'}},
        {{KA_REPORT_KEY,'*'}, {KA_REPORT_KEY,'0'}, {KA_REPORT_KEY,'#'}, {KA_REPORT_KEY, 'D'}},
};



eeprom_layout_t global_config={
    {1,4,4,1,1,1},              /* version 1, 4 rows, 4 cols, flip rows 1, flip cols 1, mouse step 1 */

    { INVALID_KEYSTATE},
    { 0 },
    { 0 },
    { 0 },
    { 0 },
 
};


#define dbgprint(str, ... )\
{\
        char tmpbuf[1024];\
        sprintf(tmpbuf,str,##__VA_ARGS__);\
        if ( myser.availableForWrite() ) myser.write(tmpbuf, strlen(tmpbuf));\
}


void tickme(void)
{
    global_ticks++;
}




void process_keypad_event( uint8_t event, uint8_t row, uint8_t col )
{
    row = (global_config.config.flip_rows)?global_config.config.rows - row - 1:row;
    col = (global_config.config.flip_cols)?global_config.config.cols - col - 1:col;
    uint8_t action = global_config.cur_map[row][col].action;
    dbgprint("# row=%d, col=%d, action=%d\n", row, col, action )
    switch(action)
    {
        case KA_KEY_ACTION:
            break;
        case KA_MOUSE_LEFT:
            global_config.mouse_moves[ NP_MOUSE_LEFT ] = ( event == EVENT_KEYPAD_DOWN ) ? global_config.config.mouse_step : 0; 
            break;
        case KA_MOUSE_RIGHT:
            global_config.mouse_moves[ NP_MOUSE_RIGHT ]= ( event == EVENT_KEYPAD_DOWN ) ? global_config.config.mouse_step : 0; 
            break;
        case KA_MOUSE_UP:
            global_config.mouse_moves[ NP_MOUSE_UP ] = ( event == EVENT_KEYPAD_DOWN ) ? global_config.config.mouse_step : 0; 
            dbgprint("set move moves (up) to %d\n", global_config.mouse_moves[ NP_MOUSE_UP ] );
            break;
        case KA_MOUSE_DOWN:
            global_config.mouse_moves[ NP_MOUSE_DOWN ] = ( event == EVENT_KEYPAD_DOWN ) ? global_config.config.mouse_step : 0; 
            break;
        case KA_REPORT_KEY:
            dbgprint("# report %s : %d,%d = %c\n\r", (event == EVENT_KEYPAD_DOWN)?"DOWN":"UP", row, col, global_config.cur_map[row][col].p1 );
            break;
        default:
            dbgprint("ERROR: unknown event %d in process_keypad_event() %s %d\n", event, __FILE__, __LINE__ );
            break;
    }
}


#define NOT_SET 0xff


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



void serial_init(void)
{
    int i;
    uint8_t blink=1;
    for (i=0; i<10; i++ )
    {
        if ( myser ) break;
        _delay_ms(100);
        digitalWriteFast(13, blink );
        blink ^= 1;
    }
    _delay_ms( 1000 );
    myser.clear();
    myser.flush();

#define _BW_(str) dbgprint( str"\r\n" ); _delay_ms(20);
    _BIG_WHALE_
#undef _BW_
}

void process_mouse( void )
{

    mymouse.move( 
        global_config.mouse_moves[ NP_MOUSE_RIGHT] - global_config.mouse_moves[ NP_MOUSE_LEFT],
        global_config.mouse_moves[ NP_MOUSE_DOWN ] -  global_config.mouse_moves[ NP_MOUSE_UP ], 0, 0
    );
}



#define ONE_SECOND 1000

extern "C" int main(void)
{
        int i =0;
        uint8_t flash_state=1;
        uint32_t last_flash=0;

        // To use Teensy 3.0 without Arduino, simply put your code here.
        // For example:
        usb_init();
        while( !usb_configuration );
        mymouse.begin();
        mymouse.screenSize(1024,768,1);
        myser.begin(9600);       
        pinMode(13, OUTPUT);
        DDRD =0x0f;
        init_keys();
        serial_init();

        mytimer.begin(tickme, 1000 );
        uint32_t last_tick=0;
        digitalWriteFast(13, flash_state);
        while (1) {
                if ( global_ticks != last_tick )
                {
                        last_tick = global_ticks;
                        readkeypad();   
                        process_mouse();
                }
                if ( global_ticks - last_flash > ONE_SECOND )
                {
                        last_flash = global_ticks;
                        flash_state ^= 1;
                        digitalWriteFast(13, flash_state);
                }       
//              mymouse.move(-1,-1,0,0);
//              digitalWriteFast(13, HIGH);
                if ( i++ < 10 ) mykey.write('0'+i);
//              PORTB |= 1<<5;
//              delay(50);
//              mykey.write(0);
//              PORTB &= ~(1<<5);
//              digitalWriteFast(13, LOW);
//              delay(50);
        }

}

