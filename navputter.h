#ifndef __NAVPUTTER_H__
#define __NAVPUTTER_H__ 

#include <stdlib.h>
#include <string.h>

extern "C" 
{
#include "VirtualSerialMouse.h"
void ser_print( const char *str, ... );
}
extern volatile uint32_t global_ticks;

enum key_arrow_states
{
    ARROW_CONFIG_MOUSE = 0,
    ARROW_CONFIG_SLOW_KEY,
    ARROW_CONFIG_FAST_KEY,
};

#define MAX_KEY_ARROW_STATE 2   /
enum special_actions
{
    SA_TOGGLE_KEY_ARROWS = 0
};

#define MAX_MOUSE_STEP 64

#define KEYMAP_LIST\
    _KM_( 'a', 0, HID_KEYBOARD_SC_A )\
    _KM_( 'b', 0, HID_KEYBOARD_SC_B )\
    _KM_( 'c', 0, HID_KEYBOARD_SC_C )\
    _KM_( 'd', 0, HID_KEYBOARD_SC_D )\
    _KM_( 'e', 0, HID_KEYBOARD_SC_E )\
    _KM_( 'f', 0, HID_KEYBOARD_SC_F )\
    _KM_( 'g', 0, HID_KEYBOARD_SC_G )\
    _KM_( 'h', 0, HID_KEYBOARD_SC_H )\
    _KM_( 'i', 0, HID_KEYBOARD_SC_I )\
    _KM_( 'j', 0, HID_KEYBOARD_SC_J )\
    _KM_( 'k', 0, HID_KEYBOARD_SC_K )\
    _KM_( 'l', 0, HID_KEYBOARD_SC_L )\
    _KM_( 'm', 0, HID_KEYBOARD_SC_M )\
    _KM_( 'n', 0, HID_KEYBOARD_SC_N )\
    _KM_( 'o', 0, HID_KEYBOARD_SC_O )\
    _KM_( 'p', 0, HID_KEYBOARD_SC_P )\
    _KM_( 'q', 0, HID_KEYBOARD_SC_Q )\
    _KM_( 'r', 0, HID_KEYBOARD_SC_R )\
    _KM_( 's', 0, HID_KEYBOARD_SC_S )\
    _KM_( 't', 0, HID_KEYBOARD_SC_T )\
    _KM_( 'u', 0, HID_KEYBOARD_SC_U )\
    _KM_( 'v', 0, HID_KEYBOARD_SC_V )\
    _KM_( 'w', 0, HID_KEYBOARD_SC_W )\
    _KM_( 'x', 0, HID_KEYBOARD_SC_X )\
    _KM_( 'y', 0, HID_KEYBOARD_SC_Y )\
    _KM_( 'z', 0, HID_KEYBOARD_SC_Z )\
    _KM_( 'A', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_A )\
    _KM_( 'B', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_B )\
    _KM_( 'C', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_C )\
    _KM_( 'D', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_D )\
    _KM_( 'E', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_E )\
    _KM_( 'F', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_F )\
    _KM_( 'G', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_G )\
    _KM_( 'H', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_H )\
    _KM_( 'I', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_I )\
    _KM_( 'J', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_J )\
    _KM_( 'K', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_K )\
    _KM_( 'L', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_L )\
    _KM_( 'M', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_M )\
    _KM_( 'N', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_N )\
    _KM_( 'O', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_O )\
    _KM_( 'P', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_P )\
    _KM_( 'Q', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_Q )\
    _KM_( 'R', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_R )\
    _KM_( 'S', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_S )\
    _KM_( 'T', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_T )\
    _KM_( 'U', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_U )\
    _KM_( 'V', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_V )\
    _KM_( 'W', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_W )\
    _KM_( 'X', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_X )\
    _KM_( 'Y', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_Y )\
    _KM_( 'Z', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_Z )\
    _KM_( '1', 0,                       HID_KEYBOARD_SC_1_AND_EXCLAMATION )\
    _KM_( '2', 0,                       HID_KEYBOARD_SC_2_AND_AT )\
    _KM_( '3', 0,                       HID_KEYBOARD_SC_3_AND_HASHMARK )\
    _KM_( '4', 0,                       HID_KEYBOARD_SC_4_AND_DOLLAR )\
    _KM_( '5', 0,                       HID_KEYBOARD_SC_5_AND_PERCENTAGE )\
    _KM_( '6', 0,                       HID_KEYBOARD_SC_6_AND_CARET )\
    _KM_( '7', 0,                       HID_KEYBOARD_SC_7_AND_AMPERSAND )\
    _KM_( '8', 0,                       HID_KEYBOARD_SC_8_AND_ASTERISK )\
    _KM_( '9', 0,                       HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS )\
    _KM_( '0', 0,                       HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS )\
    _KM_( '!', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_1_AND_EXCLAMATION )\
    _KM_( '@', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_2_AND_AT )\
    _KM_( '#', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_3_AND_HASHMARK )\
    _KM_( '$', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_4_AND_DOLLAR )\
    _KM_( '%', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_5_AND_PERCENTAGE )\
    _KM_( '^', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_6_AND_CARET )\
    _KM_( '&', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_7_AND_AMPERSAND )\
    _KM_( '*', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_8_AND_ASTERISK )\
    _KM_( '(', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS )\
    _KM_( ')', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS )\
    _KM_( '+', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_EQUAL_AND_PLUS)\
    _KM_( '=', 0,                       HID_KEYBOARD_SC_EQUAL_AND_PLUS)\
    _KM_( '-', 0,                       HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE )\
    _KM_( '_', HID_KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE )\

#undef _KM_

#define SERIAL_STARTUP_DELAY_MS 4000

#define MAX_KEY_ROWS        4
#define MAX_KEY_COLS        4

/* keep the header to 16 bytes */
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
    _ED_( 'K', key_arrows,   uint8_t,     0,             0, 1,              "%-8d",    NULL,              "Keyboard arrows by default." )\
// endo fo _EEPROM_DESC_ list


typedef struct eeprom_header
{
#define _ED_( c, _field_, _type_, d, min, max, f, func, help ) _type_ _field_;
    _EEPROM_DESC_
#undef _ED_
}eeprom_header_t;


typedef struct eeprom_layout
{
    union
    {
        eeprom_header_t config;
        uint8_t         raw[ 16 ];
    };
}eeprom_layout_t;


typedef struct key_map
{
    uint16_t     action;
    uint16_t     p1; 
    uint16_t     p2;
    uint16_t     p3;
}key_map_t;

void poll_buttons(void);
void init_keys(void);
void run_event(uint8_t event_type, uint16_t event_number );
void reset_factory_default(void);
uint16_t pop_key(void);
void handle_mouseseq( uint8_t event, uint8_t mousedir );
void handle_keyseq( uint8_t event, uint8_t seq );
void cmd_mapf( FILE *fp, char *str );
void cmd_maps( FILE *fp, char *str );
void cmd_mapm( FILE *fp, char *str );

enum key_actions
{
    KA_NO_ACTION  = 0,          /* do nothing on keypress      */
    KA_KEY_ACTION,              /* inject a keystroke          */
    KA_KEY_SCANCODE_ACTION,     /* inject a keystroke via scan codes */
    KA_SPECIAL_ACTION,          /* custom action */
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


enum time_constants
{ 
    SETTLE_KEY_BOUNCE = 10,     // milliseconds
};


enum mouse_buttons
{
    MB_LEFT=0,
    MB_MIDDLE,
    MB_RIGHT,
    NP_TOTAL_MOUSE_BUTTONS
};

typedef struct global_state
{   
    eeprom_header_t config;
    uint8_t         keystate[ MAX_KEY_COLS ];
    uint8_t         last_keystate[ MAX_KEY_COLS ];
    uint8_t         keypress[MAX_KEY_ROWS][ MAX_KEY_COLS ];
    key_map_t       cur_map[MAX_KEY_ROWS][MAX_KEY_COLS];
}global_state_t;

extern global_state_t global_config;

struct key_code
{
    uint8_t modifier;
    uint8_t key;
};

class lufa_mouse_class
{
    public:
        void begin(void) 
        {
            memset( m_mouse_moves, 0, sizeof( m_mouse_moves ) );
            memset( m_mouse_clicks, 0, sizeof( m_mouse_clicks) );
        }
        void end(void) 
        {
        }
        void set_dir( uint8_t dir, uint8_t val )
        {
            m_mouse_moves[dir] = val;
        }
        void screenSize(uint16_t width, uint16_t height, bool isMacintosh = false)
        {
        }
        void click( uint8_t button, uint8_t event )
        {
            m_mouse_clicks[button] = ( event == EVENT_KEYPAD_DOWN ) ? 1 : 0;
        } 

        uint8_t get_dir( uint8_t dir )
        {
            return m_mouse_moves[dir];
        }
        uint8_t get_buttons( uint8_t button )
        {
            return m_mouse_clicks[button];
        }
    private:    
        uint8_t         m_mouse_moves[NP_TOTAL_MOUSE_DIRS];
        uint8_t         m_mouse_clicks[NP_TOTAL_MOUSE_BUTTONS];
};

extern lufa_mouse_class mymouse;


extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;
class usb_serial_class
{
    private:
        FILE m_stream_obj;
        uint32_t m_ready_after;
    public:
        FILE *m_stream;
        void begin(long baud) 
        {
 //           USB_ClassInfo_CDC_Device_t *serial = get_serial_cdc_interface();
            CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &m_stream_obj);
            m_stream = &m_stream_obj;
            m_ready_after = global_ticks + SERIAL_STARTUP_DELAY_MS;
        }
        
        bool available(void) 
        {  
            return ( global_ticks >= m_ready_after ); 
        }
    
        void end(void){}
        void flush(void){}
        void clear(void){}

        uint8_t peek(void)
        {
            char c = getc(m_stream);
            if ( c != EOF )
            {
                ungetc(c, m_stream);
                return c;
            }
            return 0;
        }

        size_t write(uint8_t c) 
        { 
            fputc(c, m_stream);
            return 1;
        }
        size_t write(const char *str, size_t len)
        {
            fputs(str, m_stream);
            return len;
        }
        size_t write(const char *str )
        {
            return write(str, strlen(str));
        } 
        int read(void) 
        {
            return fgetc(m_stream);
        }
};

class navputter_watchdog_class 
{
    public:
        void begin()
        {
            wdt_reset();
            wdt_enable(WDTO_250MS);
        }

        void end()
        {
            wdt_reset();
            wdt_disable();
        }
    
        void kick(void)
        {
            wdt_reset();
        }
};
extern navputter_watchdog_class    mydog; 

class navputter_serial_class : public usb_serial_class
{
    public:

        virtual int read() 
        {
            int i  = usb_serial_class::read();
            mydog.kick();
            return i;
        }
        void print( const char *str, ... )
        {
            char buffer[256];
            va_list args;
            va_start (args, str);
            vsprintf (buffer, str, args);
            write(buffer);
            va_end (args);
        }
};

extern "C" void start_timer(uint32_t milliseconds);
extern navputter_serial_class      myser;





class navputter_timer_class
{
public:
    void begin( void (*t)(void), uint32_t milliseconds )
    {
        m_callback = t;
        m_interval = milliseconds;
        m_call_next = global_ticks+m_interval;
        start_timer(milliseconds);
    }
    void end()
    {
        m_call_next = 0xffffffff;
    }

    void poll()
    {
        if ( global_ticks >= m_call_next )
        {
            m_call_next = global_ticks + m_interval;
            tick();
        }
    }

    void tick()
    {
        m_callback();
    }
private:
    void (*m_callback)(void);
    uint32_t m_call_next;
    uint32_t m_interval;
};     




class usb_keyboard_class
{
    public:
        void begin(void) 
        {
            memset( m_key_codes,0, sizeof( m_key_codes ) );
#define _KM_( _char_, _modifier_, _key_ ) m_key_codes[_char_].key = _key_; m_key_codes[_char_].modifier = _modifier_;
            KEYMAP_LIST
#undef _KM_
        }
        void end(void) {}
        size_t write( uint8_t v )         
        {
            uint16_t keypair;
            CREATE_KEYPAIR( keypair, m_key_codes[v].key, m_key_codes[v].modifier );
            push_key( keypair );
            return 1;
        }
        void write_scancode( uint16_t k1 )
        {
            if ( k1 ) push_key( k1 );
        }

    private:
        struct key_code m_key_codes[0xff];
};
extern usb_keyboard_class          mykey;
class navputter_keypad
{
    enum
    {
        KEYPAD_BIT_B0 = 3,
        KEYPAD_BIT_B1 = 4,             
        KEYPAD_BIT_B2 = 5,             
        KEYPAD_BIT_B3 = 6,             
    };
public:
    void begin(void)
    {
        m_keypad_state = KP_SET_COLS;
        m_next_state=0xff;
        m_col=0;
        m_until = 0;
        m_cur_rows = 0;
        memcpy( global_config.last_keystate, global_config.keystate, global_config.config.cols );
        DDRD |= 0x0f;
        DDRB = ~((1<<KEYPAD_BIT_B0)|(1<<KEYPAD_BIT_B1)|(1<<KEYPAD_BIT_B2)|(1<<KEYPAD_BIT_B3));
        PORTB |= ((1<<KEYPAD_BIT_B0)|(1<<KEYPAD_BIT_B1)|(1<<KEYPAD_BIT_B2)|(1<<KEYPAD_BIT_B3));
        init_keys();
    }

    void press(uint8_t event, uint8_t row, uint8_t col)
    {
        row = (global_config.config.flip_rows)?global_config.config.rows - row - 1:row;
        col = (global_config.config.flip_cols)?global_config.config.cols - col - 1:col;

        uint8_t action = global_config.cur_map[row][col].action;
        switch(action)
        {
            case KA_KEY_SCANCODE_ACTION:
                if ( event == EVENT_KEYPAD_DOWN )
                {
                    mykey.write_scancode( global_config.cur_map[row][col].p1 );
                    mykey.write_scancode( global_config.cur_map[row][col].p2 );
                    mykey.write_scancode( global_config.cur_map[row][col].p3 );
                }
            break;
            case KA_SPECIAL_ACTION:
                if ( event == EVENT_KEYPAD_DOWN )
                {
                    switch( global_config.cur_map[row][col].p1 )
                    {
                        case SA_TOGGLE_KEY_ARROWS:
                            global_config.config.key_arrows = (global_config.config.key_arrows < ARROW_CONFIG_FAST_KEY ) ? global_config.config.key_arrows + 1 : 0;
                            myser.print("key arrows now %d\r\n", global_config.config.key_arrows );
                            break;
                        default:
                            myser.print("unknown special action %d at %d,%d\n", global_config.cur_map[row][col].p1, row, col );
                            break;
                    }
                }
                break;
            case KA_KEY_ACTION:
                if ( event == EVENT_KEYPAD_DOWN )
                {
                    mykey.write(global_config.cur_map[row][col].p1);
                }
            break;
            case KA_MOUSE_LEFT:
                mymouse.set_dir( NP_MOUSE_LEFT, (event == EVENT_KEYPAD_DOWN )?global_config.config.mouse_step:0 ); 
                break;
            case KA_MOUSE_RIGHT:
                mymouse.set_dir( NP_MOUSE_RIGHT, (event == EVENT_KEYPAD_DOWN)?global_config.config.mouse_step:0); 
                break;
            case KA_MOUSE_UP:
                if ( global_config.config.key_arrows == ARROW_CONFIG_MOUSE )
                    mymouse.set_dir( NP_MOUSE_UP, (event == EVENT_KEYPAD_DOWN)?global_config.config.mouse_step:0); 
                else if ( global_config.config.key_arrows == ARROW_CONFIG_SLOW_KEY )
                {
                    myser.print("writing scancode %x\n\r", global_config.cur_map[row][col].p1 );
                    mykey.write_scancode( global_config.cur_map[row][col].p1 );
                }
                else 
                {
                    myser.print("writing fast mouse scancode %x\n\r", global_config.cur_map[row][col].p2 );
                    mykey.write_scancode( global_config.cur_map[row][col].p2 );
                }
                break;
            case KA_MOUSE_DOWN:
                mymouse.set_dir( NP_MOUSE_DOWN, (event == EVENT_KEYPAD_DOWN)?global_config.config.mouse_step:0); 
                break;
            case KA_REPORT_KEY:
                myser.print("# report %s : %d,%d = %c\n\r", (event == EVENT_KEYPAD_DOWN)?"DOWN":"UP", row, col, global_config.cur_map[row][col].p1 );
                break;
            case KA_MOUSE_LT_CLICK:
                mymouse.click( MB_LEFT, event );
                break;
            case KA_MOUSE_MID_CLICK:
                mymouse.click( MB_MIDDLE, event );
                break;
            case KA_MOUSE_RT_CLICK:
                mymouse.click( MB_RIGHT, event );
                break;
            case KA_MOUSE_STEP:
                if ( event == EVENT_KEYPAD_UP )
                {
                    global_config.config.mouse_step = global_config.config.mouse_step << 1;
                    if ( global_config.config.mouse_step >= MAX_MOUSE_STEP ) global_config.config.mouse_step = 1;
                }
                break;
            default:
                myser.print("ERROR: unknown event %d in navputter_keypad::press() %s %d\n", event, __FILE__, __LINE__ );
                break;
        }
    }


    void trigger()
    {
		uint8_t col;
		uint8_t row;
        for ( col=0; col< global_config.config.cols; col++ )
        {
            if ( global_config.keystate[col] != global_config.last_keystate[col] )
            {
				for ( row=0; row< global_config.config.rows; row++ )
				{
					if ( global_config.keystate[col] & (1<<row) )
					{
						if ( global_config.keypress[row][col]  != EVENT_KEYPAD_UP)
						{
							press( EVENT_KEYPAD_UP, row, col );
						}
						global_config.keypress[row][col] = EVENT_KEYPAD_UP; 
					}
					else
					{
						if ( global_config.keypress[row][col] != EVENT_KEYPAD_DOWN )
						{
							press( EVENT_KEYPAD_DOWN, row, col);
						}
						global_config.keypress[row][col] = EVENT_KEYPAD_DOWN; 
					}
				 }
            }
            global_config.last_keystate[col] = global_config.keystate[col]; 
        }
    }


#define READ_PIN_BIT_INTO( _pos_, _port_in_, _bit_ )\
    ((((_port_in_) & ( 1<< _bit_ )) >> _bit_) << _pos_)

    void poll(void)
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

                global_config.keystate[m_col] = m_cur_rows;
                m_keypad_state =KP_SET_COLS;
                m_col++;
                if ( m_col >= global_config.config.cols ) 
                {
                    m_col=0;
                    trigger();
                }
                break;
            default:
                myser.print("puke on default switch %s %d\n", __FILE__, __LINE__ );
                break;
        }
    }
private:
    uint8_t  m_keypad_state;
    uint8_t  m_next_state;
    uint8_t  m_col;
    uint32_t m_until;
    uint8_t  m_cur_rows;
};


extern void *eeprom_start;

class
generic_eeprom_class
{
    public:
        void begin(void)
        {
            myser.write("hello generic_eeprom_class\n\r");
        }

        void end(void)
        {
        }

        bool ready(void)
        {
            return eeprom_is_ready();
        }
        void read( void *buf, uint32_t len )
        {
            myser.print("# reading eeprom %d bytes\n", len );
            eeprom_read_block( buf, eeprom_start, len );
        }
        void write( void *buf, uint32_t len )
        {
            eeprom_write_block( buf, eeprom_start, len );
        }
};


class navputter_eeprom_class : public generic_eeprom_class
{
public:
    void begin(void)
    {
        generic_eeprom_class::begin();
        while ( !ready() ) mydog.kick();
        myser.print("eeprom ready.\n\r");
    }

    void init(void)
    {
        eeprom_header_t hdr={0};
        eeprom_header_t hdr_default ={0};
        read((void *)&hdr, sizeof( hdr ));

#define _ED_( c, _field_, t, _default_, m, mx, fmt, fnunc, str ) hdr_default._field_ = _default_;
            _EEPROM_DESC_
#undef _ED_    

        if ( hdr_default.version != hdr.version )
        {
            memcpy( (void *)&global_config.config, (void *)&hdr_default, sizeof( hdr_default ) );
//            write((void *)&global_config.config, sizeof(global_config.config));
            write((void *)&global_config.config, sizeof(eeprom_layout_t));
            myser.print("wrote default settings to eeprom. hdrversion=%x, hdr_default=%x\n", hdr.version, hdr_default.version);
        }
        else
        {
            memcpy( (void *)&global_config.config, (void *)&hdr, sizeof( hdr_default ) );
            myser.print("read version %x from eeprom\n", hdr.version );
        }
    }
};



#define INVALID_KEYSTATE ((uint8_t)0xff)





extern navputter_timer_class       mytimer;

#endif // __NAVPUTTER_H__

