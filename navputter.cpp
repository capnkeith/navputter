/* 
******************************************************************************
* Navputer - get from here to here without hitting a whale.
******************************************************************************
* Copyright (c) 2018-2020 Seth Keith. All rights reserved.
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
char global_str_ok[] = "OK.\n\r";
char global_str_eoln[] = "\n\r";
char global_tmp_buffer[128]="";

#define EOLN     global_str_eoln 
#define OK_STR   global_str_ok

#define _KEL_( _c_, _str_, _func_, _enum_ )\
    const char global_progmem_keycode_menu_##_enum_[]  PROGMEM = _str_;
    KEYCODE_EDIT_LIST
#undef _KEL_


void navputter_class::set_seq_map( void )
{
#define ACT_KEYSEQ(a,m0,k0,m1,k1,hold,act) {a,{m0,k0,m1,k1,hold,act}}

#define KP_KEY( _sc_, _ev_, _init_, _x_, _y_ ) \
        m_seq_map[_ev_] = _init_;
        KP_KEY_LIST
#undef KP_KEY
}

void* operator new(size_t objsize) 
{ 
    return malloc(objsize); 
} 

void operator delete(void* obj) 
{ 
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
#if (F_CPU == 8000000)
    global_ticks+=4; // double ticks for half speed (3.3v) versions. 
#else
    global_ticks++;
#endif
}

void start_timer(uint32_t msecs);


/* this is the serial interface. */
extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;

void navputter_yield(void);



void ser_push( uint8_t c )
{
    SERIAL.push(c);
}

void init_keys(void)
{
    myputter.set_keymap();
    myputter.set_seq_map();
}

FILE *gfp=NULL;

#define MAX_KEY_SEQ         32
#define MAX_KEYS_PER_SEQ    4


 
/* this is the serial interface. */
extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;

void navputter_yield(void);



#define MAX_KEY_SEQ         32
#define MAX_KEYS_PER_SEQ    4


void *eeprom_start = NULL;

 

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
 
//    navputter_worker_pulse_class   work;
//    work.begin('d', 1, 10, 0, 10, 100 );
//    WORKERS.start_job( &work );
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



const char show_key_string[] PROGMEM = "show key r:%d c:%d seq:%d\n\r";

void navputter_keypad_class::press(uint8_t event, uint8_t row, uint8_t col)
{
//    row = (CONFIG.flip_rows)?CONFIG.rows - row - 1:row;
//    col = (CONFIG.flip_cols)?CONFIG.cols - col - 1:col;

    uint8_t seq = myputter.m_cur_map[row][col];
    uint8_t action = myputter.m_seq_map[seq].action;
    static uint32_t last_hold = 0;

    if ( myputter.m_seq_map[seq].params.keys.hold_action )
    {
        if (event == EVENT_KEYPAD_UP)
        {
            uint32_t delta = global_ticks - last_hold;
            delta /= 1000;
            if ( delta >= MIN_HOLD_TIME( myputter.m_seq_map[seq].params.keys.hold_time ) &&  
            ( delta <= MAX_HOLD_TIME( myputter.m_seq_map[seq].params.keys.hold_time) ) )
            {
                action = myputter.m_seq_map[seq].params.keys.hold_action;
                SERIAL.print("hold time %d actoin %d\n\r", delta, action );
            }
        }
        else
        {
            last_hold = global_ticks;
            SERIAL.print("last hold =%ld\n",last_hold);
        } 
    }       

    switch(action)
    {
        case KA_KEY_SCANCODE_ACTION:
            if ( event == EVENT_KEYPAD_DOWN )
            {
                KEY.write_scancode( myputter.m_seq_map[seq].params.keys.key_press[0] );
                KEY.write_scancode( myputter.m_seq_map[seq].params.keys.key_press[1] );
            }
        break;
        case KA_TOGGLE_KEY_ARROWS:
            if ( event == EVENT_KEYPAD_DOWN )
                CONFIG.key_arrows = (CONFIG.key_arrows < ARROW_CONFIG_FAST_KEY ) ?  CONFIG.key_arrows + 1 : 0;
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
                KEY.write_scancode( myputter.m_seq_map[seq].params.keys.key_press[0] );
            }
            else
            {
                KEY.write_scancode( myputter.m_seq_map[seq].params.keys.key_press[1] );
            }
            break;
        case KA_REPORT_KEY:
            SERIAL.print("# report %s : %d,%d = %c%s", (event == EVENT_KEYPAD_DOWN)?"DOWN":"UP", row, col, myputter.m_seq_map[seq].params.keys.key_press[0], EOLN );
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
        case KA_PULSE:
            SERIAL.print("pulse\n\r");
/*
            SERIAL.print("A pulse port=%c, pin=%d, l1=%d, l2=%d, hold1=%d, hold2=%d, cyc=%d, mulhold=%d, mulcyc=%d\n\r",
                ((myputter.m_seq_map[seq].params.raw[0] & 0xf0) >> 4) + 'B',
                myputter.m_seq_map[seq].params.raw[0] & 0x0f,
                myputter.m_seq_map[seq].params.raw[1] & 0xf0,
                myputter.m_seq_map[seq].params.raw[1] & 0x0f,
                myputter.m_seq_map[seq].params.raw[2],
                myputter.m_seq_map[seq].params.raw[3],
                myputter.m_seq_map[seq].params.raw[4],
                (myputter.m_seq_map[seq].params.raw[5] & 0xf0) >> 4,
                myputter.m_seq_map[seq].params.raw[5] & 0x0f
              );
*/
            break;
 //    navputter_worker_pulse_class   work;
//    work.begin('d', 1, 10, 0, 10, 100 );
//    WORKERS.start_job( &work );
                
        default:
            assert(0);
            break;
    }
}

void navputter_tenkey_keypad_class::begin()
{
    memset( m_keystate, INVALID_KEYSTATE, sizeof( m_keystate ) );



    /* turn all the col bits on (active low) w/ pullup resistor */
    #define _K_PIN_( _ev_, _ddr_, _port_, _pn_, _pin_ ) \
        _ddr_ |= (1<<_pin_);                               /* set col bits _pin_ as output */ \
        _port_ &= ~(1<<_pin_);                             /* set output value at bit _pin_ to 0 */  
        COL_PINS                                           /* do it for all col bits */
    #undef _K_PIN_

   
    /* set all the rows to be inputs. */
    #define _K_PIN_( _ev_, _ddr_, _port_, _pn_, _pin_ ) \
        _ddr_ &= ~(1<<_pin_);                              /* set rows to be inputs */ \
        ROW_PINS                                           /* do it for all row bits */
    #undef _K_PIN_
    init_keys();
}



void navputter_tenkey_keypad_class::poll(void)
{
    
    switch( m_keypad_state )
    {
        case KP_SET_COLS:
#define _K_PIN_( _e_, _ddr_, _port_, _pn_, _pin_ ) \
            if ( _e_ == m_col )                              /* just for for pin number m_col */ \
            {\
                _port_ &= ~(1 << _pin_);                     /* clear the bit for the pin m_col (active low)*/ \
            }\
            else\
            {\
                _port_ |= (1<<_pin_);                       /* set all other bits high (active low off) */\
            }
            COL_PINS
#undef _K_PIN_
            m_keypad_state = KP_WAIT;
            m_until = global_ticks + SETTLE_KEY_BOUNCE;
            m_next_state = KP_READ_ROWS;
            break;

        case KP_WAIT:
            if ( global_ticks < m_until ) return;
            m_keypad_state = m_next_state;
            break;

        case KP_READ_ROWS:
#define _K_PIN_( _e_, _ddr_, _port_, _pn_, _pin_ ) \
            {\
                uint8_t v;\
                v = ((_pn_ & (1<<_pin_)) >> _pin_ );\
                m_keystate[ m_col ] = m_keystate[ m_col ] & ~(1<<_e_);       /* turn bit off first */ \
                m_keystate[ m_col ] = m_keystate[ m_col ] | (v<<_e_);        /* then set bit to value of v */  \
            }
            ROW_PINS
#undef _K_PIN_

            m_col = ((m_col+1) < CONFIG.cols )? m_col + 1 : 0;
            m_keypad_state =KP_SET_COLS;
            if ( m_col == 0 ) trigger();
            break;
        default:
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

            m_keystate[m_col] = m_cur_rows;
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
        memset(m_keystate, INVALID_KEYSTATE, sizeof( m_keystate));  
        memcpy(m_last_keystate, m_keystate, sizeof( m_last_keystate ));
        memset(m_keypress, 0, sizeof( m_keypress));
        memset(m_keydown, 0, sizeof( m_keypress));

        m_keypad_state = KP_SET_COLS;
        m_next_state=0xff;
        m_col=0;
        m_until = 0;
        m_cur_rows = 0;

        DDRB = ~((1<<KEYPAD_BIT_B0)|(1<<KEYPAD_BIT_B1)|(1<<KEYPAD_BIT_B2)|(1<<KEYPAD_BIT_B3));
        PORTB |= ((1<<KEYPAD_BIT_B0)|(1<<KEYPAD_BIT_B1)|(1<<KEYPAD_BIT_B2)|(1<<KEYPAD_BIT_B3));
        DDRD = 0xff;
        PORTD = 0xf0;

        init_keys();
}

void navputter_tenkey_keypad_class::trigger()
{
	uint8_t col;
	uint8_t row;
    for ( col=0; col< CONFIG.cols; col++ )
    {
        if ( m_keystate[col] != m_last_keystate[col] )
        {
            for ( row=0; row< CONFIG.rows; row++ )
            {
                if ( !(m_keystate[col] & (1<<row)) )
                {
                    if ( !(m_keydown[col] & (1<<row)) )
                    {
						press( EVENT_KEYPAD_DOWN, row, col );
                    }
                    m_keydown[col] |= (1<<row);
                }
                else
                {
                    if (m_keydown[col] & (1<<row))
                    {
                        m_keydown[col] &= ~(1<<row);
						press( EVENT_KEYPAD_UP, row, col );
                    } 
                }
            }
            m_last_keystate[col] = m_keystate[col]; 
        }
    }
}       

void navputter_keypad_class::trigger()
{
#if 0
		uint8_t col;
		uint8_t row;
        for ( col=0; col< CONFIG.cols; col++ )
        {
            if ( m_keystate[col] != m_last_keystate[col] )
            {
				for ( row=0; row< CONFIG.rows; row++ )
				{
					if ( m_keystate[col] & (1<<row) )
					{
						if ( m_keypress[row][col]  != EVENT_KEYPAD_UP)
						{
							press( EVENT_KEYPAD_UP, row, col );
						}
						m_keypress[row][col] = EVENT_KEYPAD_UP; 
					}
					else
					{
						if ( m_keypress[row][col] != EVENT_KEYPAD_DOWN )
						{
							press( EVENT_KEYPAD_DOWN, row, col);
						}
						m_keypress[row][col] = EVENT_KEYPAD_DOWN; 
					}
				 }
            }
            m_last_keystate[col] = m_keystate[col]; 
        }
#endif
}

const char global_eeprom_string[] PROGMEM = "EEPROM %s. ver=%x%s";

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
        SERIAL.print_P( global_eeprom_string,  "RESET.", hdr_default.version, EOLN);
    }
    else
    {
        memcpy( (void *)&CONFIG, (void *)&hdr, sizeof( hdr_default ) );
        SERIAL.print_P( global_eeprom_string, "read.",  hdr.version, EOLN );
        if ( CONFIG.default_keymap != BUILT_IN_MAP )
        {
            myputter.read_eeprom_keymap();
        }
    }
}


void navputter_class::read_eeprom_keymap(void)
{
#define KEY_MAP_SIZE sizeof(m_cur_map) + sizeof(m_seq_map)
    PROM.read( (void *)m_cur_map, sizeof(m_cur_map), EEPROM_KEYMAP_START + CONFIG.default_keymap * KEY_MAP_SIZE ); 
    PROM.read( (void *)m_seq_map, sizeof(m_seq_map), EEPROM_KEYMAP_START + CONFIG.default_keymap * KEY_MAP_SIZE + sizeof(m_cur_map)); 
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
    km->begin();
    myputter.set_menu(km);
}

void navputter_eeprom_menu_class::begin(void)
{
    SERIAL.print_P( global_eeprom_string_2, EOLN, EOLN, EOLN);
    read_command();
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
                set_value();

            }
            else if ( !isdigit( c ) )
            {
                myputter.error( ERROR_EEPROM_ILLEGAL_VALUE );
                read_command();
            }
            else
            {
                m_int_value[ m_position++ ] = c;
                if ( m_position == sizeof( m_int_value ) )
                {
                    myputter.error( ERROR_EEPROM_INT_SIZE );
                    read_command();
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
                read_int();\
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
                read_command();\
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
            read_command();
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
                    read_param();\
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
                    read_param();
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
                    read_command();
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
                read_command();
            }
            break;
        case CONVERT_VALUE:
            m_count--;
            m_params[m_count] = atoxi( (const char *)m_value );
            if ( m_count == 0 )
                set_state(CALL_FUNC);
            else 
            {
                read_param();
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
            read_command();
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
    read_command();
}

void navputter_gpio_menu_class::usage(void)
{
    SERIAL.print("%sGPIO Menu:%s", EOLN, EOLN);
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
    SERIAL.print("%x.%s",v, EOLN);
    SERIAL.write( OK_STR );
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
    SERIAL.print("%x.%s",v, EOLN);
    SERIAL.write( OK_STR );
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
    //uint8_t ref = m_params[GPIO_A2D_REF_IX];
    //uint8_t admux = (ref << 6) | (apin & 0x0f);
    //SERIAL.print("setting admux to %x\n", admux );
    //ADMUX = admux;
    //ADCSRA = (1<<ADEN);
    //ADMUX = 0x40;
    ADMUX = (1<<REFS0);
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}


void navputter_gpio_menu_class::gpio_read_a2d(void)
{
    ADMUX = (1<<REFS0);
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
    uint16_t apin = m_params[GPIO_A2D_REF_IX];
    apin &= 0x07;
    ADMUX = ( ADMUX & 0xf8 ) | apin;

    ADCSRA |= (1<<ADSC);
    while(ADCSRA & (1 << ADSC));
    //{
    //    DOG.kick();
    //}
//    uint16_t v = ADC;
}


void navputter_keycode_menu_class::begin(void)
{
    memcpy( &m_key_map, myputter.get_map(),sizeof(m_key_map));
    memcpy( &m_key_seq_map, myputter.get_seq_map(),sizeof(m_key_seq_map));
    m_edit_field=0;
    read_command();
}

void navputter_keycode_menu_class::end(void)
{
    free(this);
}

void navputter_keycode_menu_class::usage(void)
{
//    uint16_t avail = (EEPROM_SIZE - EEPROM_HDR_RESERVED)-(CONFIG.key_maps * sizeof(action_map_t));
#define _KML_( _cmd_, _str_, _func_ ) SERIAL.print("%c) %s%s", _cmd_,_str_, EOLN );
    KEYCODE_MENU_LIST
#undef _KML_
}

void navputter_keycode_menu_class::set_keycode_display_indicies(void)
{
#define _KA_( _ix_, _str_, _m_ ) \
    if ( _ix_ == m_key_seq_map[ m_seq ].action ) m_ks[ m_edit_key ].action_ix = _ix_;
    KEY_ACTION_LIST
#undef _KA_

#define _MODL_( _ix_, _str_, _hid_ ) \
    if (((( m_key_seq_map[ m_seq ].params.keys.key_press[0]) & 0xff00)>>8) & _hid_ )\
    {\
        m_ks[0].mod_ix = _ix_;\
    }\
    if ((( m_key_seq_map[ m_seq ].params.keys.key_press[1] & 0xff00)>>8) & _hid_ )\
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
// FIXME
#if 0
    PROM.write( (void *)&m_key_map, sizeof( m_key_map ), EEPROM_KEYMAP_START + ix * KEY_MAP_SIZE ); 
    if ( ix == CONFIG.key_maps )
    {
        CONFIG.key_maps++;
    }
    PROM.write( &CONFIG, sizeof( CONFIG ) );
#endif
}


static const char keycode_usage_string[] PROGMEM = "Change key seq with the following keys:\n\r";
static const char keycode_usage_string2[] PROGMEM = "%c) %s\n\r";
void navputter_keycode_menu_class::keycode_edit_usage(void)
{
    SERIAL.print_P(keycode_usage_string);
#define _KEL_( _k_, _s_, _f_, _e_ ) SERIAL.print_P( keycode_usage_string2, _k_, _s_ );
    KEYCODE_EDIT_LIST
#undef _KEL_
    SERIAL.print(EOLN);
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
                read_command();
                break;
            }
            else if ( c == 'w' )
            {
                set_state( WRITE_COMMAND );
                break;
            }
            else 
            {
#define KP_KEY(_sc_, _ev_, _act_, _x_, _y_ )\
                if ( _sc_ == c )\
                {\
                    keycode_edit_usage();\
                    m_seq = _ev_;\
                    format_key_action((uint8_t)_ev_);\
                    set_keycode_display_indicies(); \
                    set_state( EDIT_KEY );\
                    break;\
                }    
                KP_KEY_LIST
#undef KP_KEY
            }
            break;
        case EDIT_KEY:
            c = SERIAL.read();
            if ( c == 0xff ) return;
#define _KEL_( _k_, _s_, _f_, _e_ ) if ( c == _k_ ) {_f_(); }
            KEYCODE_EDIT_LIST
#undef _KEL_
            keycode_edit_usage();
            if ( c != 'q' ) format_key_action(m_seq);
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
                read_command();
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
            
            }
            save_keymap_to_eeprom( c ); 
            read_command();
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
                read_command();
            }
            else
            {
                m_int_value[m_int_ix++] = c;
            }
            break;
        case EDIT_PINMAP:
            c = SERIAL.read();
            if ( c == 0xff ) return;
            #define _KPM_( _key_, _str_, _func_ ) if ( c == _key_ ) _func_();
            KEY_PIN_MENU
            #undef _KPM_
            if ( c != 'q' ) edit_pin_map_usage(); 
            break;
        case EDIT_PIN_ENTRY:
            c = SERIAL.read();
            if ( c == 0xff ) return;
            if ( isdigit(c) )
            {
                m_int_value[m_int_ix++]=c;
            }
            if ( m_int_ix >= 1 )
            {
                uint8_t i = atoi((const char *)m_int_value);
                m_key_map[m_edit_pin_row][m_edit_pin_col] = i;
            }
            else 
            {
                m_key_map[m_edit_pin_row][m_edit_pin_col] = 0xff;
            } 
            edit_pin_map();
            break;

        default: 
            myputter.error( ERROR_KEYMAP_EDITOR_INVALID_STATE );
            assert(0);
            break;
    }
        
}

void navputter_keycode_menu_class::edit_pin_field_usage(void)
{
    SERIAL.print("edit\n\r");
}


void navputter_keycode_menu_class::prev_action(void)
{
    m_key_seq_map[ m_seq ].action = ( m_key_seq_map[ m_seq ].action == 0 ) ? KA_LAST_ACTION-1 : m_key_seq_map[ m_seq ].action - 1;
}

void navputter_keycode_menu_class::next_action(void)
{
    m_key_seq_map[ m_seq ].action =  ( m_key_seq_map[ m_seq ].action >= KA_LAST_ACTION ) ? 0 : m_key_seq_map[ m_seq ].action + 1;
}



void navputter_keycode_menu_class::next_field(void)
{
    m_edit_field++;
    uint8_t found=0;

    #define _KA_KF_( _ev_, _min_, _max_, _loc_, _mask_, _shift_ ) \
        if ((_ev_ == m_edit_field))\
        {\
            m_field_min = _min_;\
            m_field_max = _max_;\
            m_field_mask = _mask_;\
            m_field_shift = _shift_;\
            m_field_ptr = &m_key_seq_map[ m_seq ]._loc_;\
            found=1;\
        }

    #define _KA_( _e_, _str_, _fields_ )\
        if ( m_key_seq_map[ m_seq ].action == _e_ )\
        {\
            _fields_\
        }\

    KEY_ACTION_LIST

    if ( !found )
    {
        m_edit_field = 0;
        KEY_ACTION_LIST
    }
    #undef _KA_
    #undef _KA_KF_
    SERIAL.print( "Editing %d (%d,%d)\n\r", m_edit_field, m_field_min, m_field_max);
}

void navputter_keycode_menu_class::next_value(void)
{
    uint8_t v = ((*m_field_ptr) & m_field_mask) >> m_field_shift;
    v = ( v < m_field_max ) ? v+1 : m_field_min;
    *m_field_ptr &= ~m_field_mask;
    *m_field_ptr |= (v << m_field_shift);
}

void navputter_keycode_menu_class::prev_value(void)
{
    uint8_t v = ((*m_field_ptr) & m_field_mask) >> m_field_shift;
    v = ( v > m_field_min ) ? v-1 : m_field_max;
    *m_field_ptr &= ~m_field_mask;
    *m_field_ptr |= (v << m_field_shift);
}

void navputter_keycode_menu_class::next_modifier(void)
{
    uint16_t km = m_key_seq_map[ m_seq ].params.keys.key_press[ m_edit_key ] & 0xff00;
    km = (km == 0) ? 0x0100 : km << 1;
    m_key_seq_map[m_seq].params.keys.key_press[m_edit_key] &= 0x00ff;
    m_key_seq_map[m_seq].params.keys.key_press[m_edit_key] |= km;
}

void navputter_keycode_menu_class::prev_modifier(void)
{
    uint16_t km = m_key_seq_map[ m_seq ].params.keys.key_press[ m_edit_key ] & 0xff00;
    km = (km == 0) ? 0x8000 : (km >> 1) & 0xff00;
    m_key_seq_map[m_seq].params.keys.key_press[m_edit_key] &= 0x00ff;
    m_key_seq_map[m_seq].params.keys.key_press[m_edit_key] |= km;
}


const char switch_keypress_message[] PROGMEM = "Now editing keypress %d%s";

void navputter_keycode_menu_class::edit_next(void)
{
    m_edit_key = ( m_edit_key == 1 ) ? 0 : 1;
    SERIAL.print_P( switch_keypress_message, m_edit_key, EOLN );
}


void navputter_keycode_menu_class::format_scancode( uint16_t sc)
{
    uint8_t mod_code = (sc & 0xff00) >> 8;
    uint8_t key_code = sc & 0x00ff;

    do {
#define _KEYL_( _lc_, _uc_, _kc_ )\
        if ( _kc_ == key_code )\
        {\
            SERIAL.print("'%s'", _lc_ );\
            break;\
        }
        KEYCODE_KEY_LIST
#undef _KEYL_
    }while(0);

    do {
#define _MODL_( _enum_, _str_, _sc_ )\
        if ( _sc_ & mod_code ) \
        {\
            SERIAL.write( "+'" _str_ "'" );\
            break;\
        }
        KEYCODE_MODIFIER_LIST
#undef _MODL_
    } while(0);
}



#define _KA_( _ev_, _str_, _m_ )\
    static const char keycode_menu_action##_ev_[] PROGMEM = _str_;
KEY_ACTION_LIST
#undef _KA_


void navputter_keycode_menu_class::format_key_action( uint8_t seq )
{
    uint16_t action=0;
    
    action = m_key_seq_map[ seq ].action;

    #define _KA_(_ev_, _str_, _m_) if ( _ev_ == action ) {SERIAL.print("%S%s", keycode_menu_action##_ev_, EOLN ); }
    KEY_ACTION_LIST
    #undef _KA_

    switch( action )
    {
        case KA_KEY_SCANCODE_ACTION:
            if ( m_key_seq_map[seq].params.keys.key_press[0] ) 
            { 
                format_scancode( m_key_seq_map[seq].params.keys.key_press[0] ); 
            }
            if ( m_key_seq_map[seq].params.keys.key_press[1] ) 
            { 
                SERIAL.write(","); 
                format_scancode( m_key_seq_map[seq].params.keys.key_press[1]); 
            }
            if ( m_key_seq_map[seq].params.keys.hold_time )    
            { 
                SERIAL.print( " (Hold from %d to %d secs  => key seq %d)", 
                    MIN_HOLD_TIME(m_key_seq_map[seq].params.keys.hold_time),
                    MAX_HOLD_TIME(m_key_seq_map[seq].params.keys.hold_time),
                    m_key_seq_map[seq].params.keys.hold_action );
            }
            break;
        default:
            break;
    }
}


void navputter_keycode_menu_class::show_keymap(void)
{
    SERIAL.write(EOLN);
#define KP_KEY( _sc_, _ev_, _act_, _x_, _y_ ) \
    SERIAL.print( "%c) - (seq %d) ", _sc_, _ev_ );\
    format_key_action(_ev_);\
    SERIAL.write(EOLN);
    KP_KEY_LIST
#undef KP_KEY 
}

#define EEPPROM_KEYMAP_START 32


static const char global_keycode_load_str[] PROGMEM = "Enter 0-%d to load keymap from eeprom.%s";
static const char global_keycode_error_no_keymaps_str[] PROGMEM = "%sNo keymaps to load.%s%s";
void navputter_keycode_menu_class::load_keymap(void)
{
    if ( CONFIG.key_maps == 0 )
    {
        SERIAL.print_P( global_keycode_error_no_keymaps_str, EOLN, EOLN, EOLN );
        read_command();
    } SERIAL.print_P( global_keycode_load_str, CONFIG.key_maps, EOLN);  
    set_state( READ_LOAD_INT );
}


const char global_progmem_keycode_editmenu_format_string_2[] PROGMEM = "Refer to keys with numbers and letters below:\n\r\n\r";
const char global_progmem_keycode_editmenu_format_string[] PROGMEM = "%s%c  ";
const char keycode_menu_str_1[] PROGMEM = "Press key number or letter to edit, 'w' to write, 'q' to quit:";
const char eoln_str[] PROGMEM = "\n\r\n\r";
void navputter_keycode_menu_class::key_edit_usage(void)
{

    SERIAL.print_P( global_progmem_keycode_editmenu_format_string_2);
#define KP_KEY( _sc_, _seq_, _act_, _x_, _y_ )\
    SERIAL.print_P( global_progmem_keycode_editmenu_format_string, ((_x_==0)||(_x_==0xff))?EOLN:"", _sc_);
    KP_KEY_LIST
#undef KP_KEY
    SERIAL.print_P( eoln_str );
    SERIAL.print_P( keycode_menu_str_1);
}

void navputter_keycode_menu_class::edit_keymap(void)
{
    show_keymap();
    key_edit_usage();
    set_state( EDIT_COMMAND );
}


const char global_save_keymap_string_1[] PROGMEM = "%sPress an integer up to %d to overwrite that slot, or%s";
const char global_save_keymap_string_2[] PROGMEM = "Press %d to add a new keymap at index %d:";
const char global_dump_keymap_string_0[] PROGMEM = "\n\rCurrent key layout:";
const char global_dump_keymap_string_1[] PROGMEM = "Keymap %d:%s%s";
const char global_dump_keymap_string_3[] PROGMEM = "\n\rKey actions by number:";
const char global_dump_keymap_string_4[] PROGMEM = "Col:    00  01  02  03  04  05%s";
const char global_dump_keymap_string_5[] PROGMEM = "Row %2.2d: ";

void navputter_keycode_menu_class::edit_pin_map(void)
{
    #define _KPM_( _key_, _str_, _func_) SERIAL.print("%c) %s%s", _key_, _str_, EOLN );
    KEY_PIN_MENU
    #undef _KPM_
    m_edit_pin_row = 0;
    m_edit_pin_col = 0;
    m_int_ix = 0;
    edit_pin_map_usage();
    set_state( EDIT_PINMAP );
}

void navputter_keycode_menu_class::edit_pin_map_usage()
{
    uint8_t r, c;
    SERIAL.print_P(global_dump_keymap_string_4, EOLN);
    for (r=0; r< MAX_KEY_ROWS; r++)
    {
        SERIAL.print_P(global_dump_keymap_string_5,r);
        for (c=0; c<MAX_KEY_COLS; c++)
        {
            if ( (m_edit_pin_row == r) && (m_edit_pin_col == c))
            {
                SERIAL.print("XX  ");
            }
            else
            {
                if ( m_key_map[r][c] != 0xff)
                    SERIAL.print("%2.2d  ",m_key_map[r][c]);
                else 
                    SERIAL.print("    " );
            }
        }
        SERIAL.print(EOLN);
    }
    SERIAL.print(EOLN);
} 

void navputter_keycode_menu_class::dump_keymap(void)
{
    show_keymap();
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
    read_command();
}


void navputter_worker_class::start( void )
{
}

void navputter_worker_class::done( void )
{
}

void navputter_worker_class::run_job( void )
{
    if ( global_ticks >= (m_start + m_duration) )
    {
        set_state( WORKER_DONE );
    }
}


void navputter_worker_pulse_class::start( void )
{
    set_state(WORKER_PULSE_ON);
}


void navputter_worker_pulse_class::pulse_on(void)
{
}

void navputter_worker_pulse_class::pulse_off(void)
{
}


#define WORK_CYCLES_INFINITE 0xff
void navputter_worker_pulse_class::run_job( void )
{
    switch( get_state() )
    {
        case WORKER_PULSE_ON:
            pulse_on();
            set_state( WORKER_WAITING );
            m_wait_until = global_ticks + m_hold_time_1;
            m_next_state = (m_cycles)? WORKER_PULSE_OFF : WORKER_DONE;
            break;
        case WORKER_PULSE_OFF:
            pulse_off();
            set_state( WORKER_WAITING );
            m_wait_until = global_ticks + m_hold_time_2;
            m_next_state = (m_cycles)? WORKER_PULSE_ON : WORKER_DONE;
            break; 

        case WORKER_WAITING:
            if ( global_ticks >= m_wait_until )
            {
                set_state( m_next_state );
                m_cycles = ( m_cycles == WORK_CYCLES_INFINITE )? m_cycles : m_cycles-1;
            }
            break;
        default:
            myputter.error( ERROR_WORKER_BAD_STATE ); 
            break;
    }
}

void navputter_work_pool_class::start_job( navputter_worker_class *job )
{
    if ( job->is_running() )
    {
        myputter.error( ERROR_JOB_RUNNING );
        return;
    }
    job->set_state( WORKER_STARTING );
    job->set_start( global_ticks );
    add_job_list(job);
    job->start();
}

void navputter_timer_class::poll(void)
{
    if ( global_ticks >= m_call_next )
    {
        m_call_next = global_ticks + m_interval;
        tick();
    }
    if ( m_last_work != global_ticks )
    {
        m_last_work = global_ticks;
        WORKERS.tick();
    }
}





