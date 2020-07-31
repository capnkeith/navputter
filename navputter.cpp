
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

#include <assert.h>
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
    _SC_( 'g', "Interface with GPIO via serial.", serial_gpio )\
    _SC_( 'w', "Write config to eeprom.", write_eeprom )
// end of _SER_CMDS_
void* operator new(size_t objsize) { 
    return malloc(objsize); 
} 

void operator delete(void* obj) { 
    free(obj); 
} 

uint8_t atoxi( const char *p)
{
        /*
         * Look for 'x' as second character as in '0x' format
         */
        if ((p[1] == 'x')  || (p[1] == 'X'))
                return(strtol(&p[2], (char **)0, 16));
        else
                return(strtol(p, (char **)0, 16));
}


navputter_class myputter={};
navputter_class *PUTT=NULL;

volatile uint32_t global_ticks=0;
ISR (TIMER1_COMPA_vect)
{
    global_ticks++;
}

void start_timer(uint32_t msecs);



#if 0
key_map_t   temp_map[MAX_KEY_ROWS][MAX_KEY_COLS] =
{
        {{KA_REPORT_KEY,'1'},     {KA_REPORT_KEY,'2'}, {KA_REPORT_KEY,'3'},     {KA_REPORT_KEY, 'A'}},
        {{KA_REPORT_KEY,'4'},     {KA_REPORT_KEY,'5'}, {KA_REPORT_KEY,'6'},     {KA_REPORT_KEY, 'B'}},
        {{KA_REPORT_KEY,'7'},     {KA_REPORT_KEY,'8'}, {KA_REPORT_KEY,'9'},     {KA_REPORT_KEY, 'C'}},
        {{KA_REPORT_KEY,'*'},     {KA_REPORT_KEY,'0'}, {KA_REPORT_KEY,'#'},     {KA_REPORT_KEY, 'D'}},
};
#endif

#define SCANCODE( mod, key ) ((((uint16_t)mod)<<8)|key)
#define ZOOM_IN_KEY        {KA_KEY_SCANCODE_ACTION, SCANCODE( HID_KEYBOARD_MODIFIER_LEFTALT, HID_KEYBOARD_SC_EQUAL_AND_PLUS )}               /* alt + gives slow zoom in */
#define ZOOM_OUT_KEY       {KA_KEY_SCANCODE_ACTION, SCANCODE( HID_KEYBOARD_MODIFIER_LEFTALT, HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE)}          /* alt - gives slow zoom out */
#define FOLLOW_KEY         {KA_KEY_SCANCODE_ACTION, SCANCODE( 0, HID_KEYBOARD_SC_F2 )}                                                       /* f2 is follow */
#define ROUTE_KEY          {KA_KEY_SCANCODE_ACTION, SCANCODE(HID_KEYBOARD_MODIFIER_LEFTCTRL, HID_KEYBOARD_SC_R)}                            /* ctrl r is route */
#define COLOR_KEY          {KA_KEY_SCANCODE_ACTION, SCANCODE(HID_KEYBOARD_MODIFIER_LEFTALT,  HID_KEYBOARD_SC_C)}                            /* alt C is color change */
#define MOB_KEY            {KA_KEY_SCANCODE_ACTION, SCANCODE(HID_KEYBOARD_MODIFIER_LEFTCTRL, HID_KEYBOARD_SC_SPACE)}                        /* mob is ctrl space */ 
#define TOGGLE_KEY_ARROWS  {KA_SPECIAL_ACTION, SA_TOGGLE_KEY_ARROWS}                                                                        /* togger mouse, slow key, fast key */

/* these mouse moves have 2 additional arguments, a slow key scancode, and a fast key scancodes used in the key arrow modes instead of mouse moves.
   so, MOUSE_UP will be a mouse action in key_arrow mode 0, an ALT up arrow in key_arrow mode 1 and an up arrow in key mode 2.                      */ 
#define MOUSE_UP           {KA_MOUSE_UP,    SCANCODE(HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_SC_UP_ARROW ),    SCANCODE( 0,HID_KEYBOARD_SC_UP_ARROW)}  
#define MOUSE_LEFT         {KA_MOUSE_LEFT,  SCANCODE(HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_SC_LEFT_ARROW ),  SCANCODE( 0,HID_KEYBOARD_SC_LEFT_ARROW)}  
#define MOUSE_RIGHT        {KA_MOUSE_RIGHT, SCANCODE(HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_SC_RIGHT_ARROW ), SCANCODE( 0,HID_KEYBOARD_SC_LEFT_ARROW)}  
#define MOUSE_DOWN         {KA_MOUSE_DOWN,  SCANCODE(HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_SC_DOWN_ARROW ),  SCANCODE( 0,HID_KEYBOARD_SC_DOWN_ARROW)}  

key_map_t   base_map[MAX_KEY_ROWS][MAX_KEY_COLS] =
{
        { ZOOM_IN_KEY,            MOUSE_UP,             ZOOM_OUT_KEY,               FOLLOW_KEY },
        { MOUSE_LEFT,             {KA_MOUSE_STEP, '5'}, MOUSE_RIGHT,                ROUTE_KEY  },
        { TOGGLE_KEY_ARROWS,      MOUSE_DOWN,           {KA_REPORT_KEY,'9'},        COLOR_KEY  },
        {{KA_MOUSE_LT_CLICK,'*'}, {KA_REPORT_KEY,'0'},  {KA_MOUSE_RT_CLICK,'#'},    MOB_KEY    }
};


/* this is the serial interface. */
extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;

void navputter_yield(void);


void ser_push( uint8_t c )
{
    SERIAL.push(c);
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
//    DDRB |= 1;
//    PORTB = PINB ^ 1;

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
#ifndef LEAN_N_MEAN
    myputter.big_whale();
#endif
    SERIAL.write("\n\r");
    SERIAL.write("\n\r");

    navputter_main_menu_class *main_menu = new navputter_main_menu_class();
    myputter.set_menu( main_menu ); 
 
    PORTB |= 1; 
	for (;;)
	{
        PAD.poll();
        TIMER.poll();
        DOG.kick();
        lufa_main_loop();
        myputter.poll();
    }
}

void navputter_eeprom_class::begin(void)
{
    generic_eeprom_class::begin();
    while ( !ready() ) DOG.kick();
}

void navputter_eeprom_class::read( void *buf, uint32_t len )
{
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
            }
        break;
        case KA_SPECIAL_ACTION:
            if ( event == EVENT_KEYPAD_DOWN )
            {
                switch( myputter.m_cur_map[row][col].p1 )
                {
                    case SA_TOGGLE_KEY_ARROWS:
                        CONFIG.key_arrows = (CONFIG.key_arrows < ARROW_CONFIG_FAST_KEY ) ?  CONFIG.key_arrows + 1 : 0;
                        break;
                    default:
                        break;
                }
            }
            break;
        break;
        case KA_MOUSE_LEFT:
        case KA_MOUSE_RIGHT:
        case KA_MOUSE_UP:
        case KA_MOUSE_DOWN:
            if ( CONFIG.key_arrows == ARROW_CONFIG_MOUSE )
            { 
                MOUSE.set_dir( KA_TO_NP_MOUSE_DIR(action), (event == EVENT_KEYPAD_DOWN )?CONFIG.mouse_step:0 ); 
            }
            else if ( CONFIG.key_arrows == ARROW_CONFIG_SLOW_KEY )
            {
                KEY.write_scancode( myputter.m_cur_map[row][col].p1 );
            }
            else
            {
                KEY.write_scancode( myputter.m_cur_map[row][col].p2 );
            }
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
            assert(0);
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
            assert(0);
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



#ifndef LEAN_N_MEAN
void navputter_class::big_whale(void)
{
#define _BW_(str) SERIAL.write(str"\n\r");
    _BIG_WHALE_
#undef _BW_
}
#endif

void navputter_menu_base_class::begin(void)
{
        SERIAL.write("Base Menu:");
}

void navputter_menu_base_class::end(void)
{
        SERIAL.write("Base Menu Done.");
        free( this );
}

void navputter_menu_base_class::poll(void)
{
    char c = SERIAL.read();
    if ( c != 0xff )
    {
    }
}

void navputter_menu_base_class::usage(void)
{
}

void navputter_main_menu_class::begin(void)
{
}

void navputter_main_menu_class::end(void)
{
    free( this );
}

void navputter_main_menu_class::poll(void)
{
    char c = SERIAL.read();
    if ( c != 0xff )
    {
#define _SC_( cmd, str, func ) if ( cmd == c ) { func(); goto DONE;}
    _SER_CMDS_
#undef _SC_
        usage();    
    }
DONE:
    return;
}

void navputter_main_menu_class::usage(void)
{
    SERIAL.print("Main Menu Commands:\r\n");
#define _SC_( cmd, str, func ) SERIAL.print(" %c) - %s\n\r", (int)cmd, str ); 
    _SER_CMDS_
#undef _SC_
    SERIAL.print("\n\r\r\rPress command letter:");
}

void navputter_main_menu_class::handle_eeprom(void)
{
    myputter.clear_menu();
    navputter_eeprom_menu_class *em = new navputter_eeprom_menu_class( );
    myputter.set_menu(em);
}


void navputter_main_menu_class::serial_keypad(void)
{
    SERIAL.print("handle keypad\n\r");
}

void navputter_main_menu_class::serial_gpio(void)
{
    myputter.clear_menu();
    navputter_gpio_menu_class *gm = new navputter_gpio_menu_class();
    assert(gm);
    myputter.set_menu(gm);
}

void navputter_main_menu_class::write_eeprom(void)
{
    PROM.write( &CONFIG, sizeof( CONFIG ) );
    SERIAL.print("\n\rConfig written to EEPROM.\n\r");
}

void navputter_eeprom_menu_class::begin(void)
{
    SERIAL.write("EEPROM Menu\n\r\n\r");
    set_state(READ_COMMAND);
}

void navputter_eeprom_menu_class::end(void)
{
    free( this );
}

void navputter_eeprom_menu_class::poll(void)
{
    uint8_t c;
    switch( get_state() )
    {
        case READ_INT:
            c = SERIAL.read();
            if ( c == 0xff ) return;
            SERIAL.write("READ INT\n\r");
            if ( (c == '\n') || (c=='\r'))
            {
                m_int_value[m_position]=0;
                set_state( SET_VALUE );
            }
            else if ( !isdigit( c ) )
            {
                SERIAL.write("Illegal value.\n\r");
                usage();
                set_state( READ_COMMAND );
            }
            else
            {
                m_int_value[ m_position++ ] = c;
                if ( m_position == sizeof( m_int_value ) )
                {
                    SERIAL.write("number too long.\n\r");
                    set_state( READ_COMMAND );
                }
                else SERIAL.write(c);
            }
            break;
            
        case READ_COMMAND:
            m_command = SERIAL.read();
            if ( m_command == 0xff ) return;
            #define _ED_(_cmd_, _field_, _type_, _default_, _min_, _max_, _fmt_, _func_, _help_) \
            if ( m_command == _cmd_ ) \
            {\
                set_state( READ_INT );\
                m_position =0;\
                SERIAL.print("integer:\n\r");\
                return;\
            }
                _EEPROM_DESC_
            #undef _ED_
             
            if ( m_command == 'q' )
            {
                myputter.clear_menu();
                navputter_main_menu_class *mm = new navputter_main_menu_class();
                myputter.set_menu(mm);
                return;
            }
            else
            {
                usage();
            }
            break; 
        case SET_VALUE:
            SERIAL.write("SET VALUE STATE..\n\r");
            #define _ED_(_cmd_, _field_, _type_, _default_, _min_, _max_, _fmt_, _func_, _help_) \
            if ( m_command == _cmd_ ) \
            {\
                set_state( READ_COMMAND );\
                SERIAL.write("reading int..\n\r");\
                int i = atoi((const char *)m_int_value);\
                SERIAL.print("reading int. %d.\n\r",i);\
                SERIAL.print("read in %d\n\r",i);\
                if ( (i < _min_) || (i > _max_) )\
                {\
                    SERIAL.print("Range Error.\n\r");\
                    return;\
                }\
                SERIAL.print("set value %d\n\r", i );\
                CONFIG._field_ = (__typeof__(CONFIG._field_))i;\
                break;\
            } 
                _EEPROM_DESC_
            #undef _ED_
            usage();
            set_state( READ_COMMAND );
            break;
        default:
            SERIAL.write("ILLEGAL STATE!..\n\r");\
            return;
    }
}

void navputter_eeprom_menu_class::usage(void)
{
    SERIAL.print("EEPROM Settings:\r\n");
    SERIAL.print("CMD %-15s %-8s\n\r", "Field", "Value" );
    #define _ED_( _cmd_, _field_, _type_, _default_, _min_, _max_, _fmt_, _func_, _help_ ) SERIAL.print("%c)%-15s" _fmt_ "\n\r", _cmd_, #_field_,  CONFIG._field_ );
        _EEPROM_DESC_
    #undef _ED_
    SERIAL.print("q) quit.\n\r");
    SERIAL.print("\n\r\r\rPress Command to Change Value:");
}

void navputter_gpio_menu_class::poll(void)
{
    uint8_t c=0;
    switch( get_state() )
    {
        case READ_COMMAND:
            c = SERIAL.read();
            if ( c == 0xff ) return;
            SERIAL.write(c);
#define _GM_( _cmd_, _func_, _count_, _desc_ ) \
            if ( _cmd_ == c ) \
            {\
                m_cmd = c;\
                m_count = _count_;\
                m_pos = 0;\
                set_state( READ_PORT );\
                return;\
            }
            GPIO_MENU
#undef _GM_
            gpio_error( GPIO_ERROR_BAD_COMMAND );
            usage();
        break;
        case READ_PORT:
            c = SERIAL.read();
            if (c == 0xff ) return;
            SERIAL.write(c);
            if ( c >= 'a' && c <= 'd' )
            {
                SERIAL.print("port %c\n\r",c);
                m_port = c;
                if ( m_count )
                {
                    set_state( READ_PARAM );
                }
                else 
                {
                    SERIAL.write("call\n\r");
                    set_state( CALL_FUNC );
                }
            }
            break;
        case READ_PARAM:
            c = SERIAL.read();
            if ( c == 0xff ) return;
            SERIAL.write(c);
            if ( isxdigit(c) )
            {
                m_value[ m_pos++ ] = c;
                SERIAL.print("hex, pos=%d\n\r", m_pos);
                if ( m_pos == sizeof( m_value ) )
                {
                    gpio_error( GPIO_ERROR_BAD_SIZE );
                    set_state(READ_COMMAND);
                    return;
                }  
#define HEX_WORD_READ_SIZE 2
                if (m_pos == HEX_WORD_READ_SIZE)
                {
                    SERIAL.write("convert\n\r");
                    set_state( CONVERT_VALUE );
                }
            }
            else
            {
                gpio_error( GPIO_ERROR_BAD_SIZE );
                set_state(READ_COMMAND);
            }
            break;
        case CONVERT_VALUE:
            m_count--;
            m_params[m_count] = atoxi( (const char *)m_value );
            SERIAL.print("convert[%d]=%x\n\r", m_count, m_params[m_count]);
            if ( m_count == 0 )
                set_state(CALL_FUNC);
            else 
            {
                m_pos=0;
                set_state(READ_PARAM);
            }
            break;
        case CALL_FUNC:
#define _GM_( _cmd_, _func_, _count_, _desc_ ) \
            if ( _cmd_ == m_cmd ) \
            {\
                _func_();\
            }
            GPIO_MENU
#undef _GM_
            set_state( READ_COMMAND );\
            break; 
        default:
            gpio_error( GPIO_ERROR_BAD_CASE );
            break;
    }

    if ( c == 'q' )
    {
        myputter.clear_menu();
        navputter_main_menu_class *mm = new navputter_main_menu_class();
        assert(mm);
        myputter.set_menu(mm);  
    } 
}
void navputter_gpio_menu_class::gpio_error(uint8_t err)
{
    SERIAL.print("ERR:%d\n",err);
}

void navputter_gpio_menu_class::end(void)
{
    free(this);
}

void navputter_gpio_menu_class::begin(void)
{
    set_state( READ_COMMAND );
    SERIAL.print("\n\rGPIO Menu:\n\r");
    DDRB  |= 1; 
    PORTB |= 1;
    strcpy( m_ok, "OK.\n\r" );
}

void navputter_gpio_menu_class::usage(void)
{
#if LEAN_N_MEAN 
    #define _GM_( _cmd_, _func_,  _params_, _desc_ ) SERIAL.write( _cmd_ ); SERIAL.write( "<PORT>" ); if (_params_) SERIAL.write("<value>  - "); SERIAL.write( "\n\r" );
        GPIO_MENU
    #undef _GM_
    SERIAL.print( "&<port>[value|mask]\n\r");
#else
    #define _GM_( _cmd_, _func_,  _params_, _desc_ ) SERIAL.print( "%c)       %s\n", _cmd_, _desc_ );
        GPIO_MENU
    #undef _GM_
    SERIAL.print("Enter Command:\n\r");
#endif
}

void navputter_gpio_menu_class::gpio_in_mask(void)
{
    uint8_t v=0;
#define _GP_( ascii, port, pin, dir ) \
    if ( m_port == ascii )\
    {\
        v = pin & m_params[0];\
    }
    GPIO_PORTS
#undef _GP_
    SERIAL.write("OK:");
    SERIAL.print("%x.\n\r",v);
}    


void navputter_gpio_menu_class::gpio_in(void)
{
    uint8_t v=0;
#define _GP_( ascii, port, pin, dir ) \
    if ( m_port == ascii )\
    {\
        v = pin;\
    }
    GPIO_PORTS
#undef _GP_
    SERIAL.write("OK:");
    SERIAL.print("%x.\n\r",v);
}    

void navputter_gpio_menu_class::gpio_in_bit(void)
{
    uint8_t v=0;
#define _GP_( ascii, port, pin, dir ) \
    if ( m_port == ascii )\
    {\
        v = ((pin & (1<< m_params[0]))>>m_params[0]);\
    }
    GPIO_PORTS
#undef _GP_
    SERIAL.write("OK:");
    SERIAL.print("%x.\n\r",v);
}    


void navputter_gpio_menu_class::gpio_xor(void)
{
#define _GP_( ascii, port, pin, dir ) \
    if ( ascii == m_port )\
    {\
        port = port ^ 1;\
        SERIAL.write(m_ok);\
    }
    GPIO_PORTS
#undef _GP_
}

void navputter_gpio_menu_class::gpio_ddr(void)
{
#define _GP_( ascii, port, pin, dir ) \
    if ( ascii == m_port )\
    {\
        dir = m_params[0];\
        SERIAL.write(m_ok);\
    }
    GPIO_PORTS
#undef _GP_
}

void navputter_gpio_menu_class::gpio_set_ddr_bit(void)
{
#define _GP_( ascii, port, pin, dir ) \
    if ( ascii == m_port )\
    {\
        dir |= 1<<m_params[0];\
        SERIAL.write(m_ok);\
    }
    GPIO_PORTS
#undef _GP_
}

void navputter_gpio_menu_class::gpio_clear_ddr_bit(void)
{
#define _GP_( ascii, port, pin, dir ) \
    if ( ascii == m_port )\
    {\
        dir &= ~(1<<m_params[0]);\
        SERIAL.write(m_ok);\
    }
    GPIO_PORTS
#undef _GP_
}

void navputter_gpio_menu_class::gpio_clear_bit(void)
{
#define _GP_( ascii, port, pin, dir ) \
    if ( ascii == m_port )\
    {\
        port = port & ~(1<<m_params[0]);\
        SERIAL.print( m_ok );\
    }
    GPIO_PORTS
#undef _GP_
}

void navputter_gpio_menu_class::gpio_set_bit(void)
{
#define _GP_( ascii, port, pin, dir ) \
    if ( ascii == m_port )\
    {\
        port = (1<<m_params[0]);\
        SERIAL.write( m_ok );\
    }
    GPIO_PORTS
#undef _GP_
}

void navputter_gpio_menu_class::gpio_and(void)
{
#define _GP_( ascii, port, pin, dir ) \
    if ( ascii == m_port )\
    {\
        port = port & m_params[0];\
        SERIAL.write( m_ok );\
    }
    GPIO_PORTS
#undef _GP_
}




void navputter_gpio_menu_class::gpio_or(void)
{
#define _GP_( ascii, port, pin, dir ) \
    if ( ascii == m_port )\
    {\
        port = port | m_params[0];\
        SERIAL.write( m_ok );\
    }
    GPIO_PORTS
#undef _GP_
}



void navputter_gpio_menu_class::gpio_out(void)
{
#define _GP_( ascii, port, pin, dir ) \
    if ( ascii == m_port )\
    {\
        SERIAL.write( m_ok );\
        port = m_params[0];\
    }
    GPIO_PORTS
#undef _GP_
}


/** Configures the board hardware and chip peripherals for the demo's functionality. */
