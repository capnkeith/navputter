/* Teensyduino Core Library
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

enum events             /* main event type for do_event() */
{
    EVENT_NONE = 0,
    EVENT_KEY_DOWN,
    EVENT_KEY_UP,
    EVENT_KEYPAD_UP,
    EVENT_KEYPAD_DOWN
};

enum keypad_states      /* keypad states for reading each row */
{
    KP_STABALIZE,
    KP_WAIT,
    KP_READ_COLS,
    KP_REPORT
};

enum mouse_key_modes    /* global modes for slow keyboard / fast keyboard / mouse */
{
    KEY_SLOW_MODE=0,
    KEY_FAST_MODE,
    MOUSE_MODE
};

#define MAX_KEY_ROWS        4
#define MAX_KEY_COLS        4

typedef struct eeprom_header
{
    uint8_t version;
    uint8_t rows;
    uint8_t cols;
    uint8_t unused;
}eeprom_header_t;


typedef struct eeprom_layout
{   
    eeprom_header_t config;
    uint8_t         key_map[ MAX_KEY_ROWS][ MAX_KEY_COLS ];             /* slow key map */
    uint8_t         key_map2[ MAX_KEY_ROWS][ MAX_KEY_COLS ];            /* fast key map */
    uint8_t         mouse_map[ MAX_KEY_ROWS][ MAX_KEY_COLS ];           /* mouse map */
}eeprom_layout_t;


#define EEPROM_KEY_MAP ((eeprom_layout_t *)NULL)->key_map
#define EEPROM_KEY_SEQ ((eeprom_layout_t *)NULL)->key_seq


eeprom_layout_t global_config={
    {1,4,4,0},              /* version 1, 4 rows, 4 cols  */

    /* key map 1 ( slow map ) */
    {
        {0,1,2,3},
        {4,5,6,7},
        {8,9,10,11},
        {12,13,14,15}
    },

    /* key map 2 ( fast map ) */
    {
        {0,16,2,3},
        {17,5,18,7},
        {20,19,21,11},
        {12,13,14,15}
    }
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


void run_event( uint8_t event, uint16_t val)
{
	dbgprint("event %d val %d\n\r", event, val );
}



void read_keypad(void)
{
    static uint8_t  keypad_state = KP_STABALIZE;
    static uint8_t  next_state=0xff;
    static uint8_t  cur_row =0;
    static uint8_t  last_cols = 0xff;
    static uint32_t until=0;
    static uint8_t key_array[ MAX_KEY_COLS ]={0};
    uint8_t cur_cols;
    switch( keypad_state )
    {
        case KP_STABALIZE:
            DDRD = 1<<cur_row;
            PORTD &= 0xf0;
            PORTD |= 1<<cur_row;
            next_state = KP_READ_COLS;
            keypad_state = KP_WAIT;
            until = global_ticks + 1;
            last_cols = 0xff;
            break;

        case KP_WAIT:
            if ( global_ticks < until ) return;
            keypad_state = next_state;
            break;

        case KP_READ_COLS:
            cur_cols = (PIND & 0xf0) >> 4;
	    dbgprint("row=%d, cols %d \n\r", cur_cols, cur_row);
            if ( cur_cols == last_cols )
            {
                if ( key_array[ cur_row ] != cur_cols )
                {
                    uint8_t i;
                    for ( i=0; i< global_config.config.cols; i++ )
                    {
                        if ( (cur_cols&(1<<i)) != (key_array[cur_row]&(1<<i)))
                        {
                            uint8_t event = (cur_cols & (1 << i )) ? EVENT_KEYPAD_DOWN : EVENT_KEYPAD_UP;
                            run_event( event, (uint16_t)cur_row << 8 | (uint16_t)i );
                        }
                    }
                    key_array[ cur_row ] = cur_cols;
                }
                cur_row++;
                if ( cur_row >= MAX_KEY_ROWS )
                {
                    cur_row = 0;
                }
                keypad_state = KP_STABALIZE;
            }
            else
            {
                keypad_state = KP_WAIT;
                next_state = KP_READ_COLS;
                last_cols = cur_cols;
                until = global_ticks + 1;
            }
            break;
        default:
            dbgprint("puke: %d\n",keypad_state);
            break;
    }
}

#define ONE_SECOND 1000

extern "C" int main(void)
{
	int i =0;
#ifdef USING_MAKEFILE

	// To use Teensy 3.0 without Arduino, simply put your code here.
	// For example:
	usb_init();
	while( !usb_configuration );
	mymouse.begin();
	mymouse.screenSize(1024,768,1);
	myser.begin(300);	
	pinMode(13, OUTPUT);
//	pinMode(0, INPUT);
	DDRD &= ~1;
	dbgprint("hello world");
	myser.write("Hello 2", strlen("Hello 2"));
	mytimer.begin(tickme, 1000 );
	uint32_t last_flash=0;
	uint32_t last_tick=0;
	uint8_t flash_state=1;
	digitalWriteFast(13, flash_state);
	while (1) {
		if ( global_ticks != last_tick )
		{
			last_tick = global_ticks;
			read_keypad();	
		}
		if ( global_ticks - last_flash > ONE_SECOND )
		{
			last_flash = global_ticks;
			flash_state ^= 1;
			digitalWriteFast(13, flash_state);
		}	
//		mymouse.move(-1,-1,0,0);
//		digitalWriteFast(13, HIGH);
		if ( i++ < 10 ) mykey.write('0'+i);
//		PORTB |= 1<<5;
//		delay(50);
//		mykey.write(0);
//		PORTB &= ~(1<<5);
//		digitalWriteFast(13, LOW);
//		delay(50);
	}


#else
	// Arduino's main() function just calls setup() and loop()....
	setup();
	while (1) {
		loop();
		yield();
	}
#endif
}

