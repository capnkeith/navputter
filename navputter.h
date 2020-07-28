#ifndef __NAVPUTTER_H__
#define __NAVPUTTER_H__ 

#include <stdlib.h>
#include <string.h>


#ifdef __cplusplus
extern "C" 
{
#endif
    #include "VirtualSerialMouse.h"
    void ser_print( const char *str, ... );
    void SetupHardware(void); 
    void lufa_main_loop(void);
    void start_timer(uint32_t unused);
#ifdef __cplusplus
}
#endif
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



#define KA_TO_NP_MOUSE_DIR( ka ) ((ka)-KA_MOUSE_UP)

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
        size_t virtual write(const char *str )
        {
            return write(str, strlen(str));
        } 
        int virtual read(void) 
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

class navputter_serial_class : public usb_serial_class
{
    public:

        int virtual read(void);
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

    void virtual tick()
    {
        if ( m_callback ) m_callback();
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
class navputter_keypad_class
{
    enum
    {
        KEYPAD_BIT_B0 = 3,
        KEYPAD_BIT_B1 = 4,             
        KEYPAD_BIT_B2 = 5,             
        KEYPAD_BIT_B3 = 6,             
    };
public:
    void begin(void);

    void press(uint8_t event, uint8_t row, uint8_t col);


    void trigger();


#define READ_PIN_BIT_INTO( _pos_, _port_in_, _bit_ )\
    ((((_port_in_) & ( 1<< _bit_ )) >> _bit_) << _pos_)

    void poll(void);
private:
    uint8_t  m_keypad_state;
    uint8_t  m_next_state;
    uint8_t  m_col;
    uint32_t m_until;
    uint8_t  m_cur_rows;
};


extern void *eeprom_start;

class generic_eeprom_class
{
    public:
        void begin(void)
        {
        }

        void end(void)
        {
        }

        bool ready(void)
        {
            return eeprom_is_ready();
        }
        virtual void read( void *buf, uint32_t len );

        void write( void *buf, uint32_t len )
        {
            eeprom_write_block( buf, eeprom_start, len );
        }
};


class navputter_eeprom_class : public generic_eeprom_class
{
    public:
        void begin(void);
        void init(void);
        virtual void read( void *buf, uint32_t len );
};



#define INVALID_KEYSTATE ((uint8_t)0xff)



class navputter_class
{
public:
    void begin()
    {
        memset( &m_config, 0, sizeof( m_config ) );
        memset( m_keystate, INVALID_KEYSTATE, sizeof( m_keystate) );
        memset( m_keypress, 0, sizeof( m_keypress) );
        memset( m_cur_map, 0, sizeof( m_cur_map) );

	    SetupHardware();
	    GlobalInterruptEnable();
        m_serial.begin(9600);
        m_serial.print("hello serial\n\r");
        m_keyboard.begin();
        m_timer.begin( NULL, 1000 );
        m_watchdog.begin();
        m_keypad.begin();
        m_mouse.begin();
        while( !m_serial.available() ) m_watchdog.kick();
        m_eeprom.begin();
        m_eeprom.init();
    }
       
    void set_keymap( key_map_t *map, size_t size )
    {
        memcpy( m_cur_map, map, size );
    }

    lufa_mouse_class            m_mouse;
    navputter_timer_class       m_timer;
    navputter_watchdog_class    m_watchdog; 
    navputter_serial_class      m_serial;
    usb_keyboard_class          m_keyboard;
    navputter_keypad_class      m_keypad;
    navputter_eeprom_class      m_eeprom;

    eeprom_header_t             m_config;
    uint8_t                     m_keystate[ MAX_KEY_COLS ];
    uint8_t                     m_last_keystate[ MAX_KEY_COLS ];
    uint8_t                     m_keypress[MAX_KEY_ROWS][ MAX_KEY_COLS ];
    key_map_t                   m_cur_map[MAX_KEY_ROWS][MAX_KEY_COLS];
};

#define DOG     myputter.m_watchdog
#define MOUSE   myputter.m_mouse
#define TIMER   myputter.m_timer
#define KEY     myputter.m_keyboard
#define PAD     myputter.m_keypad
#define PROM    myputter.m_eeprom
#define SERIAL  myputter.m_serial
#define CONFIG  myputter.m_config

#endif // __NAVPUTTER_H__

