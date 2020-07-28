
/* 
******************************************************************************
* Navputer - like a putter for golf except for your sailboat.
******************************************************************************
*
* This work contains open source 3rd party libraries such as LUFA, minmea,
* and uTFT_ST7735 at least. Each of these has its own licenses, the most 
* restrictive of is LUFA. Please investigagte and respect each licnese 
* appropriately. Also if you like this work, please donate to LUFA instead, 
* LUFA is the real magic here.
*
******************************************************************************
*
* This file is an original work by Seth Keth offered for free with no licese 
* whatsoever or if you prefer you can stick a WTFPL v2 on it. 
*
******************************************************************************
*
*
*         .-'           Watch out for me!
*    '--./ /     _.---.
*    '-,  (__..-`       \
*       \          .     |
*        `,.__.   ,__.--/
*          '._/_.'___.-`
*            /  /
*            __/
*
******************************************************************************
*
* Hardware: this project is currently set up for specific hardware. For 
* easiest results start with identical harware. 
*
* Navputter assembled:
*
*   uC: Teensy++ 2.0. This is a at90usb1286.
*   4x4 keypad ( multiplexed 8 pin type )
*   simple radial encoder 
*   simple push buttons (2)
*   LEDs (2)
*
* For programming: 
*    USBASP programmer
*
******************************************************************************
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "navputter.h"

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





volatile uint32_t global_ticks=0;
ISR (TIMER1_COMPA_vect)
{
    global_ticks++;
}

void start_timer(uint32_t msecs);





global_state_t global_config={
    {0x0100,4,4,1,1,1},              /* version 1.1, 4 rows, 4 cols, flip rows 1, flip cols 1, mouse step 1 */

    { INVALID_KEYSTATE},
    { 0 },
    { 0 },
    { 0 },
};


key_map_t   temp_map[MAX_KEY_ROWS][MAX_KEY_COLS] =
{
        {{KA_REPORT_KEY,'1'},     {KA_REPORT_KEY,'2'}, {KA_REPORT_KEY,'3'},     {KA_REPORT_KEY, 'A'}},
        {{KA_REPORT_KEY,'4'},     {KA_REPORT_KEY,'5'}, {KA_REPORT_KEY,'6'},     {KA_REPORT_KEY, 'B'}},
        {{KA_REPORT_KEY,'7'},     {KA_REPORT_KEY,'8'}, {KA_REPORT_KEY,'9'},     {KA_REPORT_KEY, 'C'}},
        {{KA_REPORT_KEY,'*'},     {KA_REPORT_KEY,'0'}, {KA_REPORT_KEY,'#'},     {KA_REPORT_KEY, 'D'}},
};

key_map_t   base_map[MAX_KEY_ROWS][MAX_KEY_COLS] =
{
        {{KA_KEY_ACTION,'+'}, {KA_MOUSE_UP,'2'}, {KA_KEY_ACTION,'-'}, {KA_REPORT_KEY, 'A'}},
        {{KA_MOUSE_LEFT,'4'}, {KA_MOUSE_STEP, '5'}, {KA_MOUSE_RIGHT,'6'}, {KA_REPORT_KEY, 'B'}},
        {{KA_REPORT_KEY,'7'}, {KA_MOUSE_DOWN,'8'}, {KA_REPORT_KEY,'9'}, {KA_REPORT_KEY, 'C'}},
        {{KA_MOUSE_LT_CLICK,'*'}, {KA_REPORT_KEY,'0'}, {KA_MOUSE_RT_CLICK,'#'}, {KA_REPORT_KEY, 'D'}},
};


/* this is the serial interface. */
extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;

void navputter_yield(void);


void key_up_event( int row, int col )
{
    myser.print("# key_up_event on %d %d\r\n", row, col);
}
      
void key_down_event( int row, int col )
{
    myser.print("# key_down_event on %d %d\r\n", row, col);
}
      
void init_keys(void)
{
    memcpy( global_config.cur_map, base_map, sizeof( base_map ));
//    memcpy( global_config.cur_map, temp_map, sizeof( temp_map ));
}

FILE *gfp=NULL;

#define MAX_KEY_SEQ         32
#define MAX_KEYS_PER_SEQ    4


 
/* this is the serial interface. */
extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;

void navputter_yield(void);



#define MAX_KEY_ROWS        4
#define MAX_KEY_COLS        4
#define MAX_KEY_SEQ         32
#define MAX_KEYS_PER_SEQ    4


navputter_serial_class      myser;
usb_keyboard_class          mykey;
navputter_timer_class       mytimer;
navputter_watchdog_class    mydog; 
navputter_keypad            mykeypad; 
lufa_mouse_class             mymouse; 
void *eeprom_start = NULL;

 


#define LINE "--------------------------------------------------------------------------------"





navputter_eeprom_class      myprom;
/* 
 * this is our timer interrupt service routine. This is a real interrupt service routine, so
 * don't try and do much in here. global_ticks is milliseconds since power on
 */



void start_timer(uint32_t msecs)
{
    cli();
    DDRE = 0xFF;
    TCCR1A |= 1 << WGM12;       // Configure timer 1 for CTC mode
    TCCR1B= (1<<WGM12) | (1<<CS10) ;
    OCR1A = 20000;              // 20000
    TIMSK1 |= 1 << OCIE1A;      // Output Compare A Match Interrupt Enable
    sei();                      // enable interrupts
}
 

void navputter_yield(void)
{
}

void timer_callback(void)
{
    DDRB |= 1;
    PORTB = PINB ^ 1;
//    if ( myser.available() ) myser.print("running %ld milliseconds\n\r", global_ticks);
}

extern "C" void SetupHardware(void);
extern "C" void lufa_main_loop(void);

enum
{
    LUFA_MB_BUTTON_LEFT = 0,
    LUFA_MB_BUTTON_RIGHT 
};

extern "C" void get_mouse_status( int8_t *y, int8_t *x, uint8_t *buttons )
{
    *y = mymouse.get_dir( NP_MOUSE_DOWN ) - mymouse.get_dir( NP_MOUSE_UP );
    *x = mymouse.get_dir( NP_MOUSE_RIGHT ) - mymouse.get_dir( NP_MOUSE_LEFT );
    *buttons = 
            (mymouse.get_buttons( MB_LEFT )) ? (1 << LUFA_MB_BUTTON_LEFT)  : 0  |
            (mymouse.get_buttons( MB_RIGHT )) ? (1 << LUFA_MB_BUTTON_RIGHT ) : 0;
}

extern "C" int main(void)
{
	SetupHardware();
	GlobalInterruptEnable();
    myser.begin(9600);
    mykey.begin();
    mytimer.begin( timer_callback, 10000 );
    mydog.begin();
    mykeypad.begin();
    mymouse.begin();
    while( !myser.available() ) mydog.kick();
#define _BW_(str) myser.write(str"\n\r");
    _BIG_WHALE_
#undef _BW_

   //DDRD = (1<<5);
   // DDRB = 0;
   // PORTD = 0xff;
   // PORTB = 0xff;
    myprom.begin();
    myprom.init();

//    mykey.write('a');
//    mykey.write('A');
//    mykey.write('b');
//    mykey.write('B');
#define MAX_LINE_SIZE 256 

	for (;;)
	{
        mykeypad.poll();
        mytimer.poll();
        mydog.kick();
        lufa_main_loop();
        continue;
    }
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
