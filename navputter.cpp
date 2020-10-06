
/* 
******************************************************************************
* Navputer - get from here to here without hitting a whale.
******************************************************************************
* Copyright (c) 2020 Seth Keith. All rights reserved.
******************************************************************************
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
* The idea here is that your hands are free enough so you can sail and 
* lookout for whales and still have a usable opencpn mounted at your helm.
* It is better having a static slow response (putt) then a mouse moving all 
* over the place whilst you are dodging whales. 
******************************************************************************
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <avr/pgmspace.h>
#include "navputter.h"

#define EEPROM_KEYMAP_START 32
uint8_t global_tmp_buf[MAX_TMP_BUFFER_SIZE];
char global_str_ok[] = "OK.\n\r";
char global_str_eoln[] = "\n\r";

#define EOLN     global_str_eoln 
#define OK_STR   global_str_ok

#define _KEL_( _c_, _str_, _func_, _enum_ )\
    const char global_progmem_keycode_menu_##_enum_[]  PROGMEM = _str_;
    KEYCODE_EDIT_LIST
#undef _KEL_


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

#define HOLD_TIME(_mint_, _maxt_ ) ((_mint_)&0x0f) | ((_maxt_)<<4)
#define SCANCODE( mod, key ) ((((uint16_t)mod)<<8)|key)
#define ZOOM_IN_KEY        {KA_KEY_SCANCODE_ACTION, '1', SCANCODE( HID_KEYBOARD_MODIFIER_LEFTALT, HID_KEYBOARD_SC_EQUAL_AND_PLUS ), 0, HOLD_TIME(3,5), SA_POWER_CYCLE}      /* alt + gives slow zoom in */
#define ZOOM_OUT_KEY       {KA_KEY_SCANCODE_ACTION, '3', SCANCODE( HID_KEYBOARD_MODIFIER_LEFTALT, HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE)} /* alt - gives slow zoom out */
#define FOLLOW_KEY         {KA_KEY_SCANCODE_ACTION, 'a', SCANCODE( 0, HID_KEYBOARD_SC_F2 )}                                              /* f2 is follow */
#define ROUTE_KEY          {KA_KEY_SCANCODE_ACTION, 'b', SCANCODE(HID_KEYBOARD_MODIFIER_LEFTCTRL, HID_KEYBOARD_SC_R)}                    /* ctrl r is route */
#define COLOR_KEY          {KA_KEY_SCANCODE_ACTION, 'c', SCANCODE(HID_KEYBOARD_MODIFIER_LEFTALT,  HID_KEYBOARD_SC_C)}                    /* alt C is color change */
#define MOB_KEY            {KA_KEY_SCANCODE_ACTION, 'd', SCANCODE(HID_KEYBOARD_MODIFIER_LEFTCTRL, HID_KEYBOARD_SC_SPACE)}                /* mob is ctrl space */ 
#define TOGGLE_KEY_ARROWS  {KA_SPECIAL_ACTION,      '7', SA_TOGGLE_KEY_ARROWS}                                                           /* toggler mouse, slow key, fast key*/

/* these mouse moves have 2 additional arguments, a slow key scancode, and a fast key scancodes used in the key arrow modes instead of mouse moves.
   so, MOUSE_UP will be a mouse action in key_arrow mode 0, an ALT up arrow in key_arrow mode 1 and an up arrow in key mode 2.                      */ 
#define MOUSE_UP           {KA_MOUSE_UP,    '2', SCANCODE(HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_SC_UP_ARROW ),    SCANCODE( 0,HID_KEYBOARD_SC_UP_ARROW)}  
#define MOUSE_LEFT         {KA_MOUSE_LEFT,  '4', SCANCODE(HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_SC_LEFT_ARROW ),  SCANCODE( 0,HID_KEYBOARD_SC_LEFT_ARROW)}  
#define MOUSE_RIGHT        {KA_MOUSE_RIGHT, '6', SCANCODE(HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_SC_RIGHT_ARROW ), SCANCODE( 0,HID_KEYBOARD_SC_LEFT_ARROW)}  
#define MOUSE_DOWN         {KA_MOUSE_DOWN,  '8', SCANCODE(HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_SC_DOWN_ARROW ),  SCANCODE( 0,HID_KEYBOARD_SC_DOWN_ARROW)}  


key_map_t   base_map[MAX_KEY_ROWS][MAX_KEY_COLS] =
{
        { ZOOM_IN_KEY,            MOUSE_UP,             ZOOM_OUT_KEY,               FOLLOW_KEY },
        { MOUSE_LEFT,             {KA_MOUSE_STEP, '5'}, MOUSE_RIGHT,                ROUTE_KEY  },
        { TOGGLE_KEY_ARROWS,      MOUSE_DOWN,           {KA_REPORT_KEY,'9'},        COLOR_KEY  },
        {{KA_MOUSE_LT_CLICK,'*'}, {KA_REPORT_KEY,'0'},  {KA_MOUSE_RT_CLICK,'#'},    MOB_KEY    }
};


#define KEY_MAP_SIZE sizeof(base_map)


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
    myputter.big_whale();
    SERIAL.write( EOLN );
    SERIAL.write( EOLN );

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


void navputter_eeprom_class::write( void *buf, uint32_t len, size_t offset )
{
    uint8_t *dst = (uint8_t *)offset;
    uint8_t *c = (uint8_t *)buf;
    uint16_t i; 
    for ( i=0; i< len; i++ )
    {
        eeprom_update_byte( dst+i, *(c+i) );
        DOG.kick();
    }
}

void navputter_eeprom_class::begin(void)
{
    generic_eeprom_class::begin();
    while ( !ready() ) DOG.kick();
}

void navputter_eeprom_class::read( void *buf, uint32_t len, size_t offset )
{
    eeprom_read_block( buf, (void *)((uint8_t *)eeprom_start + offset), len );
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
    static uint32_t last_hold = 0;

    if ( myputter.m_cur_map[row][col].hold_action )
    {
        if (event == EVENT_KEYPAD_UP)
        {
            uint32_t delta = global_ticks - last_hold;
            delta /= 1000;
            if ( delta >= MIN_HOLD_TIME( myputter.m_cur_map[row][col].hold_time ) &&  
            ( delta <= MAX_HOLD_TIME( myputter.m_cur_map[row][col].hold_time) ) )
            {
                action = myputter.m_cur_map[row][col].hold_action;
            }
        }
        else
        {
            last_hold = global_ticks;
        } 
    }       

    switch(action)
    {
        case KA_KEY_SCANCODE_ACTION:
            if ( event == EVENT_KEYPAD_DOWN )
            {
                KEY.write_scancode( myputter.m_cur_map[row][col].key_press[0] );
                KEY.write_scancode( myputter.m_cur_map[row][col].key_press[1] );
            }
        break;
        case KA_SPECIAL_ACTION:
            if ( event == EVENT_KEYPAD_DOWN )
            {
                switch( myputter.m_cur_map[row][col].key_press[0] )
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
                KEY.write_scancode( myputter.m_cur_map[row][col].key_press[0] );
            }
            else
            {
                KEY.write_scancode( myputter.m_cur_map[row][col].key_press[1] );
            }
            break;
        case KA_REPORT_KEY:
#ifndef LEAN_N_MEAN
            SERIAL.print("# report %s : %d,%d = %c%s", (event == EVENT_KEYPAD_DOWN)?"DOWN":"UP", row, col, myputter.m_cur_map[row][col].key_press[0], EOLN );
#endif
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
//            PORTD &= 0xf0;
//            PORTD &= 0xf0;
            PORTD &= 0xf0;
            PORTD |= (0x0f & ~(1<<m_col));
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
;
        DDRB = ~((1<<KEYPAD_BIT_B0)|(1<<KEYPAD_BIT_B1)|(1<<KEYPAD_BIT_B2)|(1<<KEYPAD_BIT_B3));
        PORTB |= ((1<<KEYPAD_BIT_B0)|(1<<KEYPAD_BIT_B1)|(1<<KEYPAD_BIT_B2)|(1<<KEYPAD_BIT_B3));
        DDRD = 0xff;
        PORTD = 0xf0;
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

const char global_eeprom_reset_str[] PROGMEM = "EEPROM reset. ver=%x%s";
const char global_eeprom_read_str[] PROGMEM = "EEPROM read ver %x%s";

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
        write((void *)&CONFIG, sizeof(CONFIG));
        SERIAL.print_P( global_eeprom_reset_str,  hdr_default.version, EOLN);
    }
    else
    {
        memcpy( (void *)&CONFIG, (void *)&hdr, sizeof( hdr_default ) );
        SERIAL.print_P( global_eeprom_read_str, hdr.version, EOLN );
        if ( CONFIG.default_keymap != BUILT_IN_MAP )
        {
            PROM.read( (void *)base_map, sizeof( base_map ), EEPROM_KEYMAP_START + CONFIG.default_keymap * KEY_MAP_SIZE ); 
        }
    }
}



void navputter_class::big_whale(void)
{
#ifndef LEAN_N_MEAN
#define _BW_(str) SERIAL.pm_write(str EOLN );
    _BIG_WHALE_
#undef _BW_
#endif
}

void navputter_menu_base_class::begin(void)
{
}

void navputter_menu_base_class::end(void)
{
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
#define _SC_( cmd, str, func ) if ( cmd == c ) { func(); /*goto DONE;*/}
    _SER_CMDS_
#undef _SC_
        usage();    
    }
//DONE:
//    return;
}

const char global_main_menu_str1[] PROGMEM = "Main Menu:%s";
void navputter_main_menu_class::usage(void)
{
    SERIAL.print_P(global_main_menu_str1, EOLN );
#define _SC_( cmd, str, func ) SERIAL.print(" %c) - %s%s", (int)cmd, str, EOLN ); 
    _SER_CMDS_
#undef _SC_
    SERIAL.print("%s%sCMD:", EOLN, EOLN);
}

void navputter_main_menu_class::handle_eeprom(void)
{
    myputter.clear_menu();
    navputter_eeprom_menu_class *em = new navputter_eeprom_menu_class( );
    myputter.set_menu(em);
}

void navputter_main_menu_class::serial_gpio(void)
{
    myputter.clear_menu();
    navputter_gpio_menu_class *gm = new navputter_gpio_menu_class();
    assert(gm);
    myputter.set_menu(gm);
}

const char global_eeprom_string_1[] PROGMEM = "%sEEPROM written.%s";
const char global_eeprom_string_2[] PROGMEM = "%sEEPROM Menu%s%s";

void navputter_main_menu_class::write_eeprom(void)
{
    PROM.write( &CONFIG, sizeof( CONFIG ) );
    SERIAL.print_P( global_eeprom_string_1, EOLN, EOLN );
}

void navputter_main_menu_class::handle_keymaps(void)
{
    myputter.clear_menu();
    navputter_keycode_menu_class *km = new navputter_keycode_menu_class();
    assert(km);
    myputter.set_menu(km);
}

void navputter_eeprom_menu_class::begin(void)
{
    SERIAL.print_P( global_eeprom_string_2, EOLN, EOLN, EOLN);
    set_state(READ_COMMAND);
}

void navputter_eeprom_menu_class::end(void)
{
    free( this );
}


int navputter_eeprom_menu_class::no_check( int cmd, int val )
{
    return 0;
}


int navputter_eeprom_menu_class::validate_keymap( int cmd, int val )
{
    if ( val > CONFIG.key_maps ) return ERROR_KEYMAP_OUT_OF_RANGE;
    return 0;
}


void navputter_eeprom_menu_class::poll(void)
{
    uint8_t c;
    switch( get_state() )
    {
        case READ_INT:
            c = SERIAL.read();
            if ( c == 0xff ) return;
            if ( (c == '\n') || (c=='\r'))
            {
                m_int_value[m_position]=0;
                set_state( SET_VALUE );
            }
            else if ( !isdigit( c ) )
            {
                myputter.error( ERROR_EEPROM_ILLEGAL_VALUE );
                usage();
                set_state( READ_COMMAND );
            }
            else
            {
                m_int_value[ m_position++ ] = c;
                if ( m_position == sizeof( m_int_value ) )
                {
                    myputter.error( ERROR_EEPROM_INT_SIZE );
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
                SERIAL.print("INT:%s", EOLN);\
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
            #define _ED_(_cmd_, _field_, _type_, _default_, _min_, _max_, _fmt_, _func_, _help_) \
            if ( m_command == _cmd_ ) \
            {\
                set_state( READ_COMMAND );\
                int i = atoi((const char *)m_int_value);\
                if ( (i < _min_) || (i > _max_) )\
                {\
                    myputter.error( ERROR_EEPROM_VALUE_RANGE );\
                    return;\
                }\
                int rc = _func_(_cmd_,i);\
                if ( rc ) \
                {\
                    myputter.error( rc );\
                    return; \
                }\
                CONFIG._field_ = (__typeof__(CONFIG._field_))i;\
                break;\
            } 
                _EEPROM_DESC_
            #undef _ED_
            usage();
            set_state( READ_COMMAND );
            break;
        default:
            myputter.error( ERROR_EEPROM_BAD_STATE );
            return;
    }
}

void navputter_eeprom_menu_class::usage(void)
{
    SERIAL.print("EEPROM Settings:%s", EOLN);
    SERIAL.print("CMD %-15s %-8s%s", "Field", "Value", EOLN );
    #define _ED_( _cmd_, _field_, _type_, _default_, _min_, _max_, _fmt_, _func_, _help_ ) SERIAL.print("%c)%-15s" _fmt_ "%s", _cmd_, #_field_,  CONFIG._field_, EOLN );
        _EEPROM_DESC_
    #undef _ED_
    SERIAL.print("q) quit.%s", EOLN);
    SERIAL.print("%s%sCMD:", EOLN,EOLN);
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
                if ((c=='a')||(c=='A'))\
                {\
                    set_state( READ_PARAM);\
                    return;\
                }\
                set_state( READ_PORT );\
                return;\
            }
            GPIO_MENU
#undef _GM_
            myputter.error( ERROR_GPIO_BAD_COMMAND );
            usage();
        break;
        case READ_PORT:
            c = SERIAL.read();
            if (c == 0xff ) return;
            SERIAL.write(c);
            if ( c >= 'a' && c <= 'd' )
            {
                m_port = c;
                if ( m_count )
                {
                    set_state( READ_PARAM );
                }
                else 
                {
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
                if ( m_pos == sizeof( m_value ) )
                {
                    myputter.error( ERROR_GPIO_BAD_SIZE );
                    set_state(READ_COMMAND);
                    return;
                }  
#define HEX_WORD_READ_SIZE 2
                if (m_pos == HEX_WORD_READ_SIZE)
                {
                    set_state( CONVERT_VALUE );
                }
            }
            else
            {
                myputter.error( ERROR_GPIO_BAD_SIZE );
                set_state(READ_COMMAND);
            }
            break;
        case CONVERT_VALUE:
            m_count--;
            m_params[m_count] = atoxi( (const char *)m_value );
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
            myputter.error( ERROR_GPIO_BAD_CASE );
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

void navputter_class::error(uint8_t err)
{
    SERIAL.print("ERR:%d%s",err, EOLN);
}

void navputter_gpio_menu_class::end(void)
{
    free(this);
}

void navputter_gpio_menu_class::begin(void)
{
    set_state( READ_COMMAND );
    SERIAL.print("%sGPIO Menu:%s", EOLN, EOLN);
    DDRB  |= 1; 
    PORTB |= 1;
}

void navputter_gpio_menu_class::usage(void)
{
#if LEAN_N_MEAN 
    #define _GM_( _cmd_, _func_,  _params_, _desc_ ) SERIAL.write("\t"); SERIAL.write( _cmd_ ); SERIAL.write( "  <PORT>" ); if (_params_) SERIAL.write("<value>"); SERIAL.write( EOLN );
        GPIO_MENU
    #undef _GM_
#else
    #define _GM_( _cmd_, _func_,  _params_, _desc_ ) SERIAL.print( "%c)       %s%s", _cmd_, _desc_, EOLN );
        GPIO_MENU
    #undef _GM_
    SERIAL.print("CMD:%s", EOLN);
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
    SERIAL.write( OK_STR );
    SERIAL.print("%x.%s",v, EOLN);
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
    SERIAL.write( OK_STR );
    SERIAL.print("%x.%s",v, EOLN);
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
    SERIAL.write( OK_STR );
    SERIAL.print("%x.%s",v, EOLN);
}    


void navputter_gpio_menu_class::gpio_xor(void)
{
#define _GP_( ascii, port, pin, dir ) \
    if ( ascii == m_port )\
    {\
        port = port ^ 1;\
        SERIAL.write( OK_STR );\
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
        SERIAL.write(OK_STR);\
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
        SERIAL.write(OK_STR);\
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
        SERIAL.write(OK_STR);\
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
        SERIAL.print( OK_STR );\
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
        SERIAL.write( OK_STR );\
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
        SERIAL.write( OK_STR );\
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
        SERIAL.write( OK_STR );\
    }
    GPIO_PORTS
#undef _GP_
}



void navputter_gpio_menu_class::gpio_out(void)
{
#define _GP_( ascii, port, pin, dir ) \
    if ( ascii == m_port )\
    {\
        SERIAL.write( OK_STR );\
        port = m_params[0];\
    }
    GPIO_PORTS
#undef _GP_
}


#define GPIO_A2D_PIN_IX 1
#define GPIO_A2D_REF_IX 0

void navputter_gpio_menu_class::gpio_setup_a2d(void)
{
    uint16_t apin = m_params[GPIO_A2D_PIN_IX];
    //uint8_t ref = m_params[GPIO_A2D_REF_IX];
    //uint8_t admux = (ref << 6) | (apin & 0x0f);
    //SERIAL.print("setting admux to %x\n", admux );
    //ADMUX = admux;
    //ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
    ADCSRA = (1<<ADEN);
    ADMUX = 0x40;
    apin = 0;
    DIDR2 = ((1<<apin) & 0xff00) >> 8;
    DIDR0 = ((1<<apin) & 0x00ff);
    ADCSRA = ADCSRA | (1 << ADSC);
    while(ADCSRA & (1 << ADSC))
    {
        DOG.kick();
    }
    uint16_t v = ADCL;
    v |= (ADCH << 8 );
    SERIAL.print("# a2d read %x (%x,%x)\n\r", v, ADCH, ADCL );
}


void navputter_gpio_menu_class::gpio_read_a2d(void)
{
    ADCSRA = ADCSRA | (1 << ADSC);
    while(ADCSRA & (1 << ADSC))
    {
        DOG.kick();
    }
    uint16_t v = ADCL;
    v |= (ADCH << 8 );
    SERIAL.print("# a2d read %x (%x,%x)\n\r", v, ADCH, ADCL );
}


void navputter_keycode_menu_class::begin(void)
{
    set_state( READ_COMMAND );
    memcpy( &m_key_map, myputter.get_map(),sizeof(m_key_map));
}

void navputter_keycode_menu_class::end(void)
{
    free(this);
}

void navputter_keycode_menu_class::usage(void)
{
//    uint16_t avail = (EEPROM_SIZE - EEPROM_HDR_RESERVED)-(CONFIG.key_maps * sizeof(key_map_t));
    SERIAL.print("Keycode editor.%s", EOLN);
#define _KML_( _cmd_, _str_, _func_ ) SERIAL.print("%c) %s%s", _cmd_,_str_, EOLN );
    KEYCODE_MENU_LIST
#undef _KML_
}


void navputter_keycode_menu_class::set_keycode_display_indicies(void)
{
#define _KA_( _ix_, _str_, _desc_ ) \
    if ( _ix_ == m_key_map[ m_row ][ m_col ].action ) m_ks[ m_edit_key ].action_ix = _ix_;
    KEY_ACTION_LIST
#undef _KA_

#define _MODL_( _ix_, _str_, _hid_ ) \
    if (((( m_key_map[m_row][m_col].key_press[0]) & 0xff00)>>8) & _hid_ )\
    {\
        m_ks[0].mod_ix = _ix_;\
    }\
    if ((( m_key_map[m_row][m_col].key_press[1] & 0xff00)>>8) & _hid_ )\
    {\
        m_ks[0].mod_ix = _ix_;\
    }
    KEYCODE_MODIFIER_LIST
#undef _MODL_
}


static const char global_save_keymap_to_eeprom_str[] PROGMEM = "Keymap saved to index %d (offset %d)%s";

#define EEPROM_KEYCODE_OFFSET 32
void navputter_keycode_menu_class::save_keymap_to_eeprom( uint8_t ix )
{
    SERIAL.print_P( global_save_keymap_to_eeprom_str, ix, 0, EOLN);
    PROM.write( (void *)&m_key_map, sizeof( m_key_map ), EEPROM_KEYMAP_START + ix * KEY_MAP_SIZE ); 
    if ( ix == CONFIG.key_maps )
    {
        CONFIG.key_maps++;
    }
    PROM.write( &CONFIG, sizeof( CONFIG ) );
}
 
void navputter_keycode_menu_class::poll(void)
{
    uint8_t c;
    switch( get_state() )
    {
        case READ_COMMAND:
            c = SERIAL.read();
            if ( c==0xff ) return;
#define _KML_( _cmd_, _str_, _func_ ) \
                if ( _cmd_ == c ) \
                {\
                    _func_(); \
                    break;\
                }
                KEYCODE_MENU_LIST
#undef _KML_
                usage();
                
        break;
        case EDIT_COMMAND:
            c = SERIAL.read();
            if ( c == 0xff ) return;
            if ( c == 'q' )
            {
                set_state( READ_COMMAND );
                usage();
                break;
            }
            else if ( c == 'w' )
            {
                set_state( WRITE_COMMAND );
                break;
            }
            else 
            {
                uint8_t row,col;
                for (row=0;row<CONFIG.rows;row++)
                {
                    for (col=0;col<CONFIG.cols;col++)
                    {
                        if ( m_key_map[row][col].short_cut == c )
                        {
                            m_row = row;
                            m_col = col;
                            m_edit_key = 0;
                            m_int_ix=0;
                            set_keycode_display_indicies(); 
                            key_edit_usage();
                            set_state( EDIT_KEY );
                            break;
                        }
                    }
                }
            }
        break;
        case EDIT_KEY:
            c = SERIAL.read();
            if ( c == 0xff ) return;
#define _KEL_( _k_, _s_, _f_, _e_ ) if ( c == _k_ ) _f_(); 
            KEYCODE_EDIT_LIST
#undef _KEL_
            SERIAL.write(EOLN);
            if ( c != 'q' ) format_key_action(m_row,m_col);
            break;
        case READ_LOAD_INT:
            c = SERIAL.read();
            if ( c == 0xff ) return;
            if ( !isdigit( c ) )
            {
                if ( c == '\n' || c == '\r' )
                {
                    m_int_value[m_int_ix]=0;
                    uint8_t i = atoi((const char *)m_int_value);
                    if ( i >= CONFIG.key_maps )
                        myputter.error( ERROR_KEYMAP_OUT_OF_RANGE );
                    else
                        set_state( LOAD_KEYMAP );
                }
                myputter.error( ERROR_KEYMAP_NOT_A_NUMBER );
                set_state( READ_COMMAND );
                usage();
            }
            else
            {
                m_int_value[m_int_ix++] = c;
            }
            break;
        case LOAD_KEYMAP:
            c = atoi((const char *)m_int_value);
            break;
        case SAVE_KEYMAP:
            c = atoi((const char *)m_int_value);
            if (( c > CONFIG.key_maps )||(c<0))
            {
                myputter.error( ERROR_KEYMAP_OUT_OF_RANGE );
                set_state( READ_COMMAND );
                return;
            }
            save_keymap_to_eeprom( c ); 
            usage();
            set_state( READ_COMMAND );
            break;
        case READ_SAVE_INT:
            c = SERIAL.read();
            if ( c == 0xff ) return;
            if ( !isdigit( c ) )
            {
                if ( c == '\n' || c == '\r' )
                {
                    m_int_value[m_int_ix]=0;
                    uint8_t i = atoi((const char *)m_int_value);
                    if ( i >  CONFIG.key_maps )
                    {
                        myputter.error( ERROR_KEYMAP_OUT_OF_RANGE );
                        return;
                    }
                    else
                    {
                        set_state( SAVE_KEYMAP );
                        return;
                    }       
                }
                myputter.error( ERROR_KEYMAP_NOT_A_NUMBER );
                set_state( READ_COMMAND );
            }
            else
            {
                m_int_value[m_int_ix++] = c;
            }
            break;
        default: 
            myputter.error( ERROR_KEYMAP_EDITOR_INVALID_STATE );
            assert(0);
            break;
    }
        
}

void navputter_keycode_menu_class::fill_keymap(void)
{
    memcpy( &m_key_map, base_map, sizeof(base_map));
}


void navputter_keycode_menu_class::prev_action(void)
{
}

void navputter_keycode_menu_class::next_action(void)
{
    uint16_t first=0xffff;
    uint16_t act=0;
    m_ks[ m_edit_key ].action_ix++;
#define _KA_( _enum_, _short_, _long_ ) \
    if ( first == 0xffff ) first=_enum_;\
    if ( m_ks[ m_edit_key ].action_ix == _enum_ )\
    {\
        act = _enum_;\
        goto DONE;\
    }
KEY_ACTION_LIST
#undef _KA_
    m_ks[ m_edit_key ].action_ix=0;
    act = first;
DONE:
    m_key_map[ m_row ][ m_col ].action = act;
}

void navputter_keycode_menu_class::prev_key(void)
{
    uint16_t first=0xffff;
    uint16_t scan=0;
    m_ks[ m_edit_key ].key_ix++;
#define _KEYL_( _lc_, _uc_, _sc_ ) \
    if ( first == 0xffff ) first=_sc_;\
    if ( m_ks[ m_edit_key ].key_ix == key_code_enum_##_sc_ )\
    {\
        scan = _sc_;\
        goto DONE;\
    }
KEYCODE_KEY_LIST
#undef _KEYL_
    m_ks[ m_edit_key ].key_ix=0;
    scan = first;
DONE:
    m_key_map[ m_row ][ m_col ].key_press[m_edit_key] &= 0xff00; 
    m_key_map[ m_row ][ m_col ].key_press[m_edit_key] |= (scan);
}


void navputter_keycode_menu_class::next_key(void)
{
    uint16_t first=0xffff;
    uint16_t scan=0;
    m_ks[ m_edit_key ].key_ix++;
#define _KEYL_( _lc_, _uc_, _sc_ ) \
    if ( first == 0xffff ) first=_sc_;\
    if ( m_ks[ m_edit_key ].key_ix == key_code_enum_##_sc_ )\
    {\
        scan = _sc_;\
        goto DONE;\
    }
KEYCODE_KEY_LIST
#undef _KEYL_
    m_ks[ m_edit_key ].key_ix=0;
    scan = first;
DONE:
    m_key_map[ m_row ][ m_col ].key_press[m_edit_key] &= 0xff00; 
    m_key_map[ m_row ][ m_col ].key_press[m_edit_key] |= (scan);
}

void navputter_keycode_menu_class::next_modifier(void)
{
    uint16_t first=0xffff;
    uint16_t scan=0;
    m_ks[ m_edit_key ].mod_ix++;
#define _MODL_( _ix_, _str_, _sc_ ) \
    if ( first == 0xffff ) first=_sc_;\
    if ( m_ks[ m_edit_key ].mod_ix == _ix_ )\
    {\
        scan = _sc_;\
        goto DONE;\
    }
KEYCODE_MODIFIER_LIST
#undef _MODL_
    m_ks[ m_edit_key ].mod_ix=0;
    scan = first;
DONE:
    m_key_map[ m_row ][ m_col ].key_press[m_edit_key] &= 0x00ff; 
    m_key_map[ m_row ][ m_col ].key_press[m_edit_key] |= (scan << 8);
}


void navputter_keycode_menu_class::prev_modifier(void)
{
    uint16_t prev=0xffff;
    uint16_t scan=0;
    uint8_t  last=0;
    m_ks[ m_edit_key ].mod_ix--;
#define _MODL_( _ix_, _str_, _sc_ ) \
    prev =_sc_;\
    last = _ix_;\
    if ( m_ks[ m_edit_key ].mod_ix == _ix_ )\
    {\
        scan = _sc_;\
        goto DONE;\
    }
KEYCODE_MODIFIER_LIST
#undef _MODL_
    m_ks[ m_edit_key ].mod_ix=prev;
    scan = last;
DONE:
    m_key_map[ m_row ][ m_col ].key_press[m_edit_key] &= 0x00ff; 
    m_key_map[ m_row ][ m_col ].key_press[m_edit_key] |= (scan << 8);
}

const char switch_keypress_message[] PROGMEM = "Now editing keypress %d%s";

void navputter_keycode_menu_class::edit_next(void)
{
    m_edit_key = ( m_edit_key == 1 ) ? 0 : 1;
    SERIAL.print_P( switch_keypress_message, m_edit_key, EOLN );
}

uint16_t navputter_keycode_menu_class::format_scancode( uint16_t sc)
{
    uint16_t sz=0;
    uint8_t mod_code = (sc & 0xff00) >> 8;
    uint8_t key_code = sc & 0x00ff;
    do {
#define _KEYL_( _lc_, _uc_, _kc_ )\
        if ( _kc_ == key_code )\
        {\
            sz += SERIAL.print("'%s'", _lc_ );\
            break;\
        }
        KEYCODE_KEY_LIST
#undef _KEYL_
    }while(0);

    do {
#define _MODL_( _enum_, _str_, _sc_ )\
        if ( _sc_ & mod_code ) \
        {\
            sz += SERIAL.write( "+'" _str_ "'" );\
            break;\
        }
        KEYCODE_MODIFIER_LIST
#undef _MODL_
    } while(0);

    return sz;
}


void navputter_keycode_menu_class::format_key_action( uint8_t row, uint8_t col)
{
    uint16_t action=0;
    uint8_t  sz=0;
    do
    {
#define _KA_(e, _short_, _desc_ ) \
        if ( m_key_map[row][col].action == e ) \
        {\
            SERIAL.print( "%c)%s", m_key_map[row][col].short_cut, _short_ ); \
            sz+=strlen(_short_);\
            action = e;\
            break;\
        }
    KEY_ACTION_LIST
#undef _KA_
    }while(0);

    switch( action )
    {
        case KA_KEY_SCANCODE_ACTION:
            if ( m_key_map[row][col].key_press[0] ) { sz += format_scancode( m_key_map[row][col].key_press[0] ); }
            if ( m_key_map[row][col].key_press[1] ) { sz += SERIAL.write(","); sz += format_scancode( m_key_map[row][col].key_press[1]); }
            break;
        case KA_SPECIAL_ACTION:
            switch( m_key_map[row][col].key_press[0] )
            {
                case SA_TOGGLE_KEY_ARROWS:
                    sz += SERIAL.write("ArrowMode");
                    break;
                case SA_POWER_CYCLE:
                    SERIAL.print("power cycle command\n\r"); 
                    break;
                default:
                    myputter.error( ERROR_KEYMAP_INVALID_ACTION );
                    break;
            }
            break;
    }
#define FMT_PAD 15
    while( sz< FMT_PAD )
    {
        sz++;
        SERIAL.write(" ");
    }
}


void navputter_keycode_menu_class::show_keymap(void)
{
    uint8_t r;
    uint8_t c;

    SERIAL.write(EOLN);
    for (r=0; r< CONFIG.rows; r++ )
    {
        for ( c=0; c< CONFIG.cols; c++ )
        {
            format_key_action( r,c);
        }
        SERIAL.write(EOLN);
    }
}

#define EEPPROM_KEYMAP_START 32


static const char global_keycode_load_str[] PROGMEM = "Enter 0-%d to load keymap from eeprom.%s";
static const char global_keycode_error_no_keymaps_str[] PROGMEM = "%sNo keymaps to load.%s%s";
void navputter_keycode_menu_class::load_keymap(void)
{
    if ( CONFIG.key_maps == 0 )
    {
        SERIAL.print_P( global_keycode_error_no_keymaps_str, EOLN, EOLN, EOLN );
        usage();
        set_state( READ_COMMAND );
        return;
    }
    SERIAL.print_P( global_keycode_load_str, CONFIG.key_maps, EOLN);  
    set_state( READ_LOAD_INT );
}


const char global_progmem_keycode_editmenu_format_string[] PROGMEM = "%c) - %S%s"; 
const char keycode_menu_str_1[] PROGMEM = "%sPress menu KEY) to edit, 'w' to write, 'q' to quit:";
const char keycode_menu_str_3[] PROGMEM = "KEY:";
void navputter_keycode_menu_class::key_edit_usage(void)
{
//    SERIAL.write(EOLN);
//    SERIAL.write(EOLN);
//    format_key_action( m_row, m_col );
    SERIAL.write(EOLN);
#define _KEL_( _c_, _str_, _func_, _enum_ ) SERIAL.print_P( global_progmem_keycode_editmenu_format_string, _c_, global_progmem_keycode_menu_##_enum_, EOLN );
                KEYCODE_EDIT_LIST
#undef _KEL_ 
    SERIAL.write(EOLN);
}

void navputter_keycode_menu_class::edit_keymap(void)
{
    show_keymap();
    //key_edit_usage();
    SERIAL.print_P(keycode_menu_str_3);
    set_state( EDIT_COMMAND );
}


const char global_save_keymap_string_1[] PROGMEM = "%sPress an integer up to (non inclusive) %d to overwrite that slot, or%s";
const char global_save_keymap_string_2[] PROGMEM = "Press %d to add a new keymap at index %d:";
const char global_dump_keymap_string_0[] PROGMEM = "Current keymap";
const char global_dump_keymap_string_1[] PROGMEM = "Keymap %d:%s%s";

void navputter_keycode_menu_class::dump_keymap(void)
{
    uint8_t i;

    SERIAL.print_P( global_dump_keymap_string_0 );
    memcpy( m_key_map, base_map, sizeof( base_map ) );
    show_keymap();
    SERIAL.write( EOLN );
    for ( i=0; i< CONFIG.key_maps; i++ )
    {
        SERIAL.print_P( global_dump_keymap_string_1,i,EOLN,EOLN);
        PROM.read( m_key_map, sizeof(m_key_map), EEPROM_KEYMAP_START + i* KEY_MAP_SIZE );
        show_keymap();
        SERIAL.write( EOLN );
    }
}


void navputter_keycode_menu_class::save_keymap(void)
{
    if ( CONFIG.key_maps )
        SERIAL.print_P( global_save_keymap_string_1, EOLN, CONFIG.key_maps, EOLN );
    SERIAL.print_P( global_save_keymap_string_2, CONFIG.key_maps, CONFIG.key_maps );
    set_state( READ_SAVE_INT );
}
void navputter_keycode_menu_class::quit(void)
{
    myputter.clear_menu();
    navputter_main_menu_class *mm = new navputter_main_menu_class();
    assert(mm);
    myputter.set_menu(mm);
}

void navputter_keycode_menu_class::quit_edit(void)
{
    set_state( READ_COMMAND );
    usage();
}


/** Configures the board hardware and chip peripherals for the demo's functionality. */
