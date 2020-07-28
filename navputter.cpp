
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



navputter_class myputter={};
navputter_class *PUTT=NULL;

volatile uint32_t global_ticks=0;
ISR (TIMER1_COMPA_vect)
{
    global_ticks++;
}

void start_timer(uint32_t msecs);






key_map_t   temp_map[MAX_KEY_ROWS][MAX_KEY_COLS] =
{
        {{KA_REPORT_KEY,'1'},     {KA_REPORT_KEY,'2'}, {KA_REPORT_KEY,'3'},     {KA_REPORT_KEY, 'A'}},
        {{KA_REPORT_KEY,'4'},     {KA_REPORT_KEY,'5'}, {KA_REPORT_KEY,'6'},     {KA_REPORT_KEY, 'B'}},
        {{KA_REPORT_KEY,'7'},     {KA_REPORT_KEY,'8'}, {KA_REPORT_KEY,'9'},     {KA_REPORT_KEY, 'C'}},
        {{KA_REPORT_KEY,'*'},     {KA_REPORT_KEY,'0'}, {KA_REPORT_KEY,'#'},     {KA_REPORT_KEY, 'D'}},
};

#define SCANCODE( mod, key ) ((((uint16_t)mod)<<8)|key)
#define ZOOM_IN_KEY        {KA_KEY_SCANCODE_ACTION, SCANCODE( HID_KEYBOARD_MODIFIER_LEFTALT, HID_KEYBOARD_SC_EQUAL_AND_PLUS )}               /* alt + gives slow zoom in */
#define ZOOM_OUT_KEY       {KA_KEY_SCANCODE_ACTION, SCANCODE( HID_KEYBOARD_MODIFIER_LEFTALT, HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE)}          /* alt - gives slow zoom out */
#define FOLLOW_KEY         {KA_KEY_SCANCODE_ACTION, SCANCODE( 0, HID_KEYBOARD_SC_F2 )}                                                       /* f2 is follow */
#define ROUTE_KEY          {KA_KEY_SCANCODE_ACTION, SCANCODE(HID_KEYBOARD_MODIFIER_LEFTCTRL, HID_KEYBOARD_SC_R)}                            /* ctrl r is route */
#define COLOR_KEY          {KA_KEY_SCANCODE_ACTION, SCANCODE(HID_KEYBOARD_MODIFIER_LEFTALT,  HID_KEYBOARD_SC_C)}                            /* alt C is color change */
#define MOB_KEY            {KA_KEY_SCANCODE_ACTION, SCANCODE(HID_KEYBOARD_MODIFIER_LEFTCTRL, HID_KEYBOARD_SC_SPACE)}                        /* mob is ctrl space */ 
#define TOGGLE_KEY_ARROWS  {KA_SPECIAL_ACTION, SA_TOGGLE_KEY_ARROWS}
#define MOUSE_UP           {KA_MOUSE_UP, SCANCODE(HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_SC_UP_ARROW ), SCANCODE( 0,HID_KEYBOARD_SC_UP_ARROW)}  

key_map_t   base_map[MAX_KEY_ROWS][MAX_KEY_COLS] =
{
        { ZOOM_IN_KEY,            MOUSE_UP,             ZOOM_OUT_KEY,               FOLLOW_KEY },
        {{KA_MOUSE_LEFT,'4'},     {KA_MOUSE_STEP, '5'}, {KA_MOUSE_RIGHT,'6'},       ROUTE_KEY  },
        { TOGGLE_KEY_ARROWS,      {KA_MOUSE_DOWN,'8'},  {KA_REPORT_KEY,'9'},        COLOR_KEY  },
        {{KA_MOUSE_LT_CLICK,'*'}, {KA_REPORT_KEY,'0'},  {KA_MOUSE_RT_CLICK,'#'},    MOB_KEY    }
};


/* this is the serial interface. */
extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;

void navputter_yield(void);


extern "C" void ser_print( const char *str, ... )
{
    char buffer[256];
    va_list args;
    va_start (args, str);
    vsprintf (buffer, str, args);
    SERIAL.write(buffer);
    va_end (args);
}

void key_up_event( int row, int col )
{
    SERIAL.print("# key_up_event on %d %d\r\n", row, col);
}
      
void key_down_event( int row, int col )
{
    SERIAL.print("# key_down_event on %d %d\r\n", row, col);
}
      
void init_keys(void)
{
    myputter.set_keymap( (key_map_t *)base_map, sizeof( base_map ));
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


void *eeprom_start = NULL;

 


#define LINE "--------------------------------------------------------------------------------"





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

}


enum
{
    LUFA_MB_BUTTON_LEFT = 0,
    LUFA_MB_BUTTON_RIGHT 
};

extern "C" void get_mouse_status( int8_t *y, int8_t *x, uint8_t *buttons )
{
    *y = MOUSE.get_dir( NP_MOUSE_DOWN ) - MOUSE.get_dir( NP_MOUSE_UP );
    *x = MOUSE.get_dir( NP_MOUSE_RIGHT ) - MOUSE.get_dir( NP_MOUSE_LEFT );
    *buttons = 
            (MOUSE.get_buttons( MB_LEFT )) ? (1 << LUFA_MB_BUTTON_LEFT)  : 0  |
            (MOUSE.get_buttons( MB_RIGHT )) ? (1 << LUFA_MB_BUTTON_RIGHT ) : 0;
}

extern "C" int main(void)
{
    PUTT=&myputter;
    myputter.begin();

#define _BW_(str) SERIAL.write(str"\n\r");
    _BIG_WHALE_
#undef _BW_

	for (;;)
	{
        PAD.poll();
        TIMER.poll();
        DOG.kick();
        lufa_main_loop();
    }
}

void navputter_eeprom_class::begin(void)
{
    generic_eeprom_class::begin();
    while ( !ready() ) DOG.kick();
    SERIAL.print("eeprom ready.\n\r");
}

void navputter_eeprom_class::read( void *buf, uint32_t len )
{
    SERIAL.print("# reading eeprom %d bytes\n", len );
    eeprom_read_block( buf, eeprom_start, len );
}


int navputter_serial_class::read(void)
{
    int i  = usb_serial_class::read();
    DOG.kick();
    return i;
}

void navputter_keypad_class::press(uint8_t event, uint8_t row, uint8_t col)
{
    row = (CONFIG.flip_rows)?CONFIG.rows - row - 1:row;
    col = (CONFIG.flip_cols)?CONFIG.cols - col - 1:col;

    uint8_t action = myputter.m_cur_map[row][col].action;
    switch(action)
    {
        case KA_KEY_SCANCODE_ACTION:
            if ( event == EVENT_KEYPAD_DOWN )
            {
                KEY.write_scancode( myputter.m_cur_map[row][col].p1 );
                KEY.write_scancode( myputter.m_cur_map[row][col].p2 );
                KEY.write_scancode( myputter.m_cur_map[row][col].p3 );
            }
        break;
        case KA_SPECIAL_ACTION:
            if ( event == EVENT_KEYPAD_DOWN )
            {
                switch( myputter.m_cur_map[row][col].p1 )
                {
                    case SA_TOGGLE_KEY_ARROWS:
                        CONFIG.key_arrows = (CONFIG.key_arrows < ARROW_CONFIG_FAST_KEY ) ?  CONFIG.key_arrows + 1 : 0;
                        SERIAL.print("key arrows now %d\r\n", CONFIG.key_arrows );
                        break;
                    default:
                        SERIAL.print("unknown special action %d at %d,%d\n", myputter.m_cur_map[row][col].p1, row, col );
                        break;
                }
            }
            break;
        case KA_KEY_ACTION:
            if ( event == EVENT_KEYPAD_DOWN )
            {
                KEY.write(myputter.m_cur_map[row][col].p1);
            }
        break;
        case KA_MOUSE_LEFT:
            MOUSE.set_dir( NP_MOUSE_LEFT, (event == EVENT_KEYPAD_DOWN )?CONFIG.mouse_step:0 ); 
            break;
        case KA_MOUSE_RIGHT:
            MOUSE.set_dir( NP_MOUSE_RIGHT, (event == EVENT_KEYPAD_DOWN)?CONFIG.mouse_step:0); 
            break;
        case KA_MOUSE_UP:
            if ( CONFIG.key_arrows == ARROW_CONFIG_MOUSE )
                MOUSE.set_dir( NP_MOUSE_UP, (event == EVENT_KEYPAD_DOWN)?CONFIG.mouse_step:0); 
            else if ( CONFIG.key_arrows == ARROW_CONFIG_SLOW_KEY )
            {
                SERIAL.print("writing scancode %x\n\r", myputter.m_cur_map[row][col].p1 );
                KEY.write_scancode( myputter.m_cur_map[row][col].p1 );
            }
            else 
            {
                SERIAL.print("writing fast mouse scancode %x\n\r", myputter.m_cur_map[row][col].p2 );
                KEY.write_scancode( myputter.m_cur_map[row][col].p2 );
            }
            break;
        case KA_MOUSE_DOWN:
            MOUSE.set_dir( NP_MOUSE_DOWN, (event == EVENT_KEYPAD_DOWN)?CONFIG.mouse_step:0); 
            break;
        case KA_REPORT_KEY:
            SERIAL.print("# report %s : %d,%d = %c\n\r", (event == EVENT_KEYPAD_DOWN)?"DOWN":"UP", row, col, myputter.m_cur_map[row][col].p1 );
            break;
        case KA_MOUSE_LT_CLICK:
            MOUSE.click( MB_LEFT, event );
            break;
        case KA_MOUSE_MID_CLICK:
            MOUSE.click( MB_MIDDLE, event );
            break;
        case KA_MOUSE_RT_CLICK:
            MOUSE.click( MB_RIGHT, event );
            break;
        case KA_MOUSE_STEP:
            if ( event == EVENT_KEYPAD_UP )
            {
                CONFIG.mouse_step = CONFIG.mouse_step << 1;
                if ( CONFIG.mouse_step >= MAX_MOUSE_STEP ) CONFIG.mouse_step = 1;
            }
            break;
        default:
            SERIAL.print("ERROR: unknown event %d in navputter_keypad::press() %s %d\n", event, __FILE__, __LINE__ );
                break;
    }
}

void navputter_keypad_class::poll(void)
{
    switch( m_keypad_state )
    {
        case KP_SET_COLS:
//                PORTD &= 0xf0;  // set low nibble to zero
//                PORTD &= 0xf0;
            PORTD =~ (1<<m_col);
            m_keypad_state = KP_WAIT;
            m_until = global_ticks + SETTLE_KEY_BOUNCE;
            m_next_state = KP_READ_ROWS;
            break;

        case KP_WAIT:
            if ( global_ticks < m_until ) return;
            m_keypad_state = m_next_state;
            break;

        case KP_READ_ROWS:
            m_cur_rows = 
                READ_PIN_BIT_INTO( 0, PINB, KEYPAD_BIT_B0 ) |
                READ_PIN_BIT_INTO( 1, PINB, KEYPAD_BIT_B1 ) |
                READ_PIN_BIT_INTO( 2, PINB, KEYPAD_BIT_B2 ) |
                READ_PIN_BIT_INTO( 3, PINB, KEYPAD_BIT_B3 );

            myputter.m_keystate[m_col] = m_cur_rows;
            m_keypad_state =KP_SET_COLS;
            m_col++;
            if ( m_col >= CONFIG.cols ) 
            {
                m_col=0;
                trigger();
            }
            break;
        default:
            SERIAL.print("puke on default switch %s %d\n", __FILE__, __LINE__ );
            break;
    }
}

void navputter_keypad_class::begin(void)
{
        m_keypad_state = KP_SET_COLS;
        m_next_state=0xff;
        m_col=0;
        m_until = 0;
        m_cur_rows = 0;
        memcpy( myputter.m_last_keystate, myputter.m_keystate, CONFIG.cols );
        DDRD |= 0x0f;
        DDRB = ~((1<<KEYPAD_BIT_B0)|(1<<KEYPAD_BIT_B1)|(1<<KEYPAD_BIT_B2)|(1<<KEYPAD_BIT_B3));
        PORTB |= ((1<<KEYPAD_BIT_B0)|(1<<KEYPAD_BIT_B1)|(1<<KEYPAD_BIT_B2)|(1<<KEYPAD_BIT_B3));
        init_keys();
}

void navputter_keypad_class::trigger()
{
		uint8_t col;
		uint8_t row;
        for ( col=0; col< CONFIG.cols; col++ )
        {
            if ( myputter.m_keystate[col] != myputter.m_last_keystate[col] )
            {
				for ( row=0; row< CONFIG.rows; row++ )
				{
					if ( myputter.m_keystate[col] & (1<<row) )
					{
						if ( myputter.m_keypress[row][col]  != EVENT_KEYPAD_UP)
						{
							press( EVENT_KEYPAD_UP, row, col );
						}
						myputter.m_keypress[row][col] = EVENT_KEYPAD_UP; 
					}
					else
					{
						if ( myputter.m_keypress[row][col] != EVENT_KEYPAD_DOWN )
						{
							press( EVENT_KEYPAD_DOWN, row, col);
						}
						myputter.m_keypress[row][col] = EVENT_KEYPAD_DOWN; 
					}
				 }
            }
            myputter.m_last_keystate[col] = myputter.m_keystate[col]; 
        }
}


void navputter_eeprom_class::init(void)
{
    eeprom_header_t hdr={0};
    eeprom_header_t hdr_default ={0};
    read((void *)&hdr, sizeof( hdr ));

#define _ED_( c, _field_, t, _default_, m, mx, fmt, fnunc, str ) hdr_default._field_ = _default_;
    _EEPROM_DESC_
#undef _ED_    

    if ( hdr_default.version != hdr.version )
    {
        memcpy( (void *)&CONFIG, (void *)&hdr_default, sizeof( hdr_default ) );
        write((void *)&CONFIG, sizeof(eeprom_layout_t));
        SERIAL.print("wrote default settings to eeprom. hdrversion=%x, hdr_default=%x\n", hdr.version, hdr_default.version);
    }
    else
    {
        memcpy( (void *)&CONFIG, (void *)&hdr, sizeof( hdr_default ) );
        SERIAL.print("read version %x from eeprom\n", hdr.version );
    }
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
