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
    void ser_push( uint8_t c );
#ifdef __cplusplus
}
#endif
extern volatile uint32_t global_ticks;
#define MAX_TMP_BUFFER_SIZE 128
extern uint8_t global_tmp_buf[MAX_TMP_BUFFER_SIZE];
extern char global_tmp_buffer[];
enum key_arrow_states
{
    ARROW_CONFIG_MOUSE = 0,
    ARROW_CONFIG_SLOW_KEY,
    ARROW_CONFIG_FAST_KEY,
};

enum error_codes
{
    ERROR_GPIO_BAD_COMMAND,
    ERROR_GPIO_BAD_SIZE,
    ERROR_GPIO_BAD_CASE,
    ERROR_EEPROM_FULL,
    ERROR_EEPROM_BAD_STATE,
    ERROR_EEPROM_VALUE_RANGE,
    ERROR_EEPROM_ILLEGAL_VALUE,
    ERROR_EEPROM_INT_SIZE,
    ERROR_KEYMAP_EDITOR_INVALID_STATE,
    ERROR_KEYMAP_OUT_OF_RANGE,
    ERROR_KEYMAP_NOT_A_NUMBER,
    ERROR_KEYMAP_INVALID_ACTION,
    ERROR_JOB_RUNNING,
    ERROR_WORKER_BAD_STATE,
};

#define MAX_KEY_ARROW_STATE 2   


#define MAX_MOUSE_STEP 64


#define SERIAL_STARTUP_DELAY_MS 1000

#define MAX_KEY_ROWS        6 
#define MAX_KEY_COLS        6 



#define BUILT_IN_MAP 0xff   // use the hard coded keymap, otherwise read map from eeprom at start.


/* keep the header to 16 bytes */
#define _EEPROM_DESC_\
    _ED_( 'v', version,          uint16_t,    0x0104,        0, 0xff,           "%-8x",    no_check,              "EEPROM version. Change to reset device to factory default." )\
    _ED_( 'r', rows,             uint8_t,     MAX_KEY_ROWS,  1, MAX_KEY_ROWS,   "%-8d",    no_check,              "Number of rows of keys on your keypad." )\
    _ED_( 'c', cols,             uint8_t,     MAX_KEY_COLS,  1, MAX_KEY_COLS,   "%-8d",    no_check,              "Number of columns of keys on your keypad." )\
    _ED_( 'f', flip_rows,        uint8_t,     1,             0, 1,              "%-8d",    no_check,              "Flip keypad rows." )\
    _ED_( 'l', flip_cols,        uint8_t,     1,             0, 1,              "%-8d",    no_check,              "Flip keypad columns." )\
    _ED_( 's', mouse_step,       uint8_t,     2,             1, 32,             "%-8d",    no_check,              "Mouse step." )\
    _ED_( 'k', key_maps,         uint8_t,     0,             0, 0xff,           "%-8d",    no_check,              "Total keymaps in memory." )\
    _ED_( 'x', screen_x,         uint16_t,    1920,          2, 7680,           "%-8d",    no_check,              "Total keymaps in memory." )\
    _ED_( 'y', screen_y,         uint16_t,    1080,          2, 3240,           "%-8d",    no_check,              "Total keymaps in memory." )\
    _ED_( 'm', screen_mac,       uint8_t,     1,             0, 1,              "%-8d",    no_check,              "Mac Mouse." )\
    _ED_( 'K', key_arrows,       uint8_t,     0,             0, 1,              "%-8d",    no_check,              "Keyboard arrows by default." )\
    _ED_( 'd', default_keymap,   uint8_t,     BUILT_IN_MAP,  0, 0xff,           "%-8x",    validate_keymap,       "Default startup keymap from eeprom." )\
// endo fo _EEPROM_DESC_ list



/* 
 * for a 19 key '10 key style' keypad, number the keys like so:
   
              1  2  3  4
              5  6  7  8
              9  a  b  c
              d  e  f  g
              h  i  j  g
   
   key g is a double wide (enter key).           
 */



#define HOLD_TIME(_mint_, _maxt_ ) ((_mint_)&0x0f) | ((_maxt_)<<4)
#define SCANCODE( mod, key ) ((((uint16_t)mod)<<8)|key)

#define C_KEY        {KA_KEY_SCANCODE_ACTION, '1', SCANCODE( 0,  HID_KEYBOARD_SC_C)}

#define ZOOM_IN_KEY        {KA_KEY_SCANCODE_ACTION, '1', SCANCODE( HID_KEYBOARD_MODIFIER_LEFTALT, HID_KEYBOARD_SC_EQUAL_AND_PLUS ), 0, HOLD_TIME(3,5), KEY_SEQ_20}      
#define ZOOM_OUT_KEY       {KA_KEY_SCANCODE_ACTION, '3', SCANCODE( HID_KEYBOARD_MODIFIER_LEFTALT, HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE)} /* alt - gives slow zoom out */
#define FOLLOW_KEY         {KA_KEY_SCANCODE_ACTION, 'a', SCANCODE( 0, HID_KEYBOARD_SC_F2 )}                                              /* f2 is follow */
#define ROUTE_KEY          {KA_KEY_SCANCODE_ACTION, 'b', SCANCODE(HID_KEYBOARD_MODIFIER_LEFTCTRL, HID_KEYBOARD_SC_R)}                    /* ctrl r is route */
#define COLOR_KEY          {KA_KEY_SCANCODE_ACTION, 'c', SCANCODE(HID_KEYBOARD_MODIFIER_LEFTALT,  HID_KEYBOARD_SC_C)}                    /* alt C is color change */
#define MOB_KEY            {KA_KEY_SCANCODE_ACTION, 'd', SCANCODE(HID_KEYBOARD_MODIFIER_LEFTCTRL, HID_KEYBOARD_SC_SPACE)}                /* mob is ctrl space */ 
#define TOGGLE_KEY_ARROWS  {KA_TOGGLE_KEY_ARROWS }}   /* toggler mouse, slow key, fast key*/

/* these mouse moves have 2 additional arguments, a slow key scancode, and a fast key scancodes used in the key arrow modes instead of mouse moves.
   so, MOUSE_UP will be a mouse action in key_arrow mode 0, an ALT up arrow in key_arrow mode 1 and an up arrow in key mode 2.                      */
#define MOUSE_UP           {KA_MOUSE_UP,    '2', SCANCODE(HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_SC_UP_ARROW ),    SCANCODE( 0,HID_KEYBOARD_SC_UP_ARROW)}  
#define MOUSE_LEFT         {KA_MOUSE_LEFT,  '4', SCANCODE(HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_SC_LEFT_ARROW ),  SCANCODE( 0,HID_KEYBOARD_SC_LEFT_ARROW)}  
#define MOUSE_RIGHT        {KA_MOUSE_RIGHT, '6', SCANCODE(HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_SC_RIGHT_ARROW ), SCANCODE( 0,HID_KEYBOARD_SC_LEFT_ARROW)}  
#define MOUSE_DOWN         {KA_MOUSE_DOWN,  '8', SCANCODE(HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_SC_DOWN_ARROW ),  SCANCODE( 0,HID_KEYBOARD_SC_DOWN_ARROW)}  

#define MOUSE_LT_CLICK      {KA_MOUSE_LT_CLICK,0}
#define MOUSE_RT_CLICK      {KA_MOUSE_RT_CLICK,0}
#define SHOW_KEY            {KA_REPORT_KEY,0}
#define TOGGLE_MOUSE_SPEED  {KA_MOUSE_STEP,0}
#define POWER_CYCLE         {KA_POWER_CYCLE,0}

#if 0 /* 4x4 map */
key_map_t   base_map[4][4] =
//[MAX_KEY_ROWS][MAX_KEY_COLS] =
{
        { ZOOM_IN_KEY,            MOUSE_UP,             ZOOM_OUT_KEY,               FOLLOW_KEY },
        { MOUSE_LEFT,             {KA_MOUSE_STEP, '5'}, MOUSE_RIGHT,                ROUTE_KEY  },
        { TOGGLE_KEY_ARROWS,      MOUSE_DOWN,           {KA_REPORT_KEY,'9'},        COLOR_KEY  },
        {{KA_MOUSE_LT_CLICK,'*'}, {KA_REPORT_KEY,'0'},  {KA_MOUSE_RT_CLICK,'#'},    MOB_KEY    }
};
//[MAX_KEY_ROWS][MAX_KEY_COLS] =
key_map_t   base_map[MAX_KEY_ROWS][MAX_KEY_COLS] =
{
        { KP_KEY1, KP_KEY_2, KP_KEY_3, KP_KEY_4 },
        { KP_KEY5, KP_KEY_6, KP_KEY_7, KP_KEY_8 },
        { KP_KEY9, KP_KEY_10, KP_KEY_3, KP_KEY_4 },
};
#endif

#define SEQ_NA 0xff 

/* this is the list of all keys, their enumerated sequence name, and x,y location on the keypad grid. 
   The format is as follows:
         KP_KEY( shortcut key,  sequence enum,  action,  x locaton, y location )
 */
#define KP_KEY_LIST\
    KP_KEY('1',   KEY_SEQ_1,    ZOOM_IN_KEY,        0, 0 )\
    KP_KEY('2',   KEY_SEQ_2,    FOLLOW_KEY,         1, 0 )\
    KP_KEY('3',   KEY_SEQ_3,    ROUTE_KEY,          2, 0 )\
    KP_KEY('4',   KEY_SEQ_4,    COLOR_KEY,          3, 0 )\
    KP_KEY('5',   KEY_SEQ_5,    ZOOM_OUT_KEY,       0, 1 )\
    KP_KEY('6',   KEY_SEQ_6,    MOUSE_UP,           1, 1 )\
    KP_KEY('7',   KEY_SEQ_7,    SHOW_KEY,           2, 1 )\
    KP_KEY('8',   KEY_SEQ_8,    C_KEY,              3, 1 )\
    KP_KEY('9',   KEY_SEQ_9,    MOUSE_LEFT,         0, 2 )\
    KP_KEY('a',   KEY_SEQ_10,   TOGGLE_MOUSE_SPEED, 1, 2 )\
    KP_KEY('b',   KEY_SEQ_11,   MOUSE_RIGHT,        2, 2 )\
    KP_KEY('c',   KEY_SEQ_12,   SHOW_KEY,           3, 2 )\
    KP_KEY('d',   KEY_SEQ_13,   SHOW_KEY,           0, 3 )\
    KP_KEY('e',   KEY_SEQ_14,   MOUSE_DOWN,         1, 3 )\
    KP_KEY('f',   KEY_SEQ_15,   SHOW_KEY,           2, 3 )\
    KP_KEY('g',   KEY_SEQ_16,   MOB_KEY,            3, 3 )\
    KP_KEY('h',   KEY_SEQ_17,   MOUSE_LT_CLICK,     0, 4 )\
    KP_KEY('i',   KEY_SEQ_18,   SHOW_KEY,           1, 4 )\
    KP_KEY('j',   KEY_SEQ_19,   MOUSE_RT_CLICK,     2, 4 )\
    KP_KEY('k',   KEY_SEQ_20,   POWER_CYCLE,        0xff,0xff)\
/* end of KP_KEY_LIST */

#define KP_KEY(_sc_, _ev_, _act_, _x_, _y_) _ev_,
enum key_seq_enum
{
    KP_KEY_LIST
    MAX_KEY_SEQUENCES
};
#undef KP_KEY



/* this is the encoding for an 'obnomon' 19 key membrane found in Kadaon 19 key keypads. 
   Format is:
        _KM_( col0, col1, col2, col3, col4, col5 ) 
   Where colx is one of the key sequence enumerators or SEQ_NA for no connection.
*/

#define KEYPAD_PIN_MAP\
    _KM_( KEY_SEQ_8,    SEQ_NA,     SEQ_NA,     SEQ_NA,     KEY_SEQ_4,  SEQ_NA  )        /* row 0 */    \
    _KM_( SEQ_NA,       SEQ_NA,     KEY_SEQ_1,  KEY_SEQ_2,  KEY_SEQ_3,  SEQ_NA  )        /* row 1 */    \
    _KM_( SEQ_NA,       SEQ_NA,     SEQ_NA,     KEY_SEQ_17, KEY_SEQ_19, SEQ_NA  )        /* row 2 */   \
    _KM_( SEQ_NA,       SEQ_NA,     KEY_SEQ_13, KEY_SEQ_14, KEY_SEQ_15, KEY_SEQ_16 )    /* row 3 */    \
    _KM_( KEY_SEQ_18,   KEY_SEQ_12, KEY_SEQ_9,  KEY_SEQ_10, KEY_SEQ_11, SEQ_NA )        /* row 4 */    \
    _KM_( SEQ_NA,       SEQ_NA,     KEY_SEQ_5,  KEY_SEQ_6,  KEY_SEQ_7,  KEY_SEQ_8 )      /* row 5 */   \
/* end of KEYPAD_PIN_MAP */



#define EEPROM_HDR_RESERVED 64
/* EEPROM layout:
 *
 * byte
 * 0  - 15          - eeprom header version 1.1
 * 16 - 63          - reserved for future use
 * 64 - EEPROM END  - keymaps and key sequences.
 */

typedef struct __attribute__((packed)) eeprom_header
{
#define _ED_( c, _field_, _type_, d, min, max, f, func, help ) _type_ _field_;
    _EEPROM_DESC_
#undef _ED_
}eeprom_header_t;



typedef struct __attribute__((packed)) eeprom_layout
{
    union
    {
        eeprom_header_t config;
        uint8_t         raw[ 32 ];
    };
}eeprom_layout_t;

#define MIN_HOLD_TIME( _t_ ) ((_t_) & 0x0f)
#define MAX_HOLD_TIME( _t_ ) (((_t_) & 0xf0) >> 4)



typedef struct key_map
{
    uint8_t      action;                /* action on pressing the key */
    uint8_t      unused;
    union   
    {
        uint16_t     key_press[2];          /* up to 2 keys pressed */
        uint8_t      key_press_8[4];
    };
    uint8_t      hold_time;             /* bits 0-3 are min hold seconds bits 4-7 are max hold seconds */
    uint8_t      hold_action;           /* KA_SPECIAL_ACTION after holding between min and max time and releasing */
}key_map_t;


typedef struct act_map
{
    uint8_t     action;
    uint8_t     params[7];
}act_map_t;

union key_union
{
    key_map_t   key_seq;
    act_map_t   act_seq;
};


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

#define NO_FIELDS\
    _KA_KF_( 0, 0, 0, unused, 0, 0 )

#define KEY_FIELDS\
    _KA_KF_( KEY_VAL_0,         0, 0xff,                key_press_8[0], 0xff, 0 ) \
    _KA_KF_( KEY_MOD_0,         0, 0xff,                key_press_8[1], 0xff, 0 ) \
    _KA_KF_( KEY_VAL_1,         0, 0xff,                key_press_8[2], 0xff, 0 ) \
    _KA_KF_( KEY_MOD_1,         0, 0xff,                key_press_8[3], 0xff, 0 ) \
    _KA_KF_( KEY_HOLD_TIME_MIN, 0, 0xff,                hold_time,      0xf0, 4 ) \
    _KA_KF_( KEY_HOLD_TIME_MAX, 0, 0xff,                hold_time,      0x0f, 0 ) \
    _KA_KF_( HOLD_ACTION,       0, MAX_KEY_SEQUENCES,   hold_action,    0xff, 8 ) \
/* end of KEY_FIELDS */
enum 
{
#define _KA_KF_( _ev_, _min_, _max_, _field_, _mask_, _shift_ ) _ev_,
    KEY_FIELDS
#undef _KA_KF_
};

#define KEY_ACTION_LIST\
    _KA_( KA_NO_ACTION,             "Nothing",    NO_FIELDS)\
    _KA_( KA_KEY_SCANCODE_ACTION,   "Keypress:",  KEY_FIELDS)\
    _KA_( KA_MOUSE_UP,              "Mouse-UP",   NO_FIELDS)\
    _KA_( KA_MOUSE_DOWN,            "Mouse-DN",   NO_FIELDS)\
    _KA_( KA_MOUSE_LEFT,            "Mouse-LT",   NO_FIELDS)\
    _KA_( KA_MOUSE_RIGHT,           "Mouse-RT",   NO_FIELDS)\
    _KA_( KA_MOUSE_LT_CLICK,        "Mouse-LK",   NO_FIELDS)\
    _KA_( KA_MOUSE_RT_CLICK,        "Mouse-RK",   NO_FIELDS)\
    _KA_( KA_MOUSE_RT_DBL_CLICK,    "Mouse-RDK",  NO_FIELDS)\
    _KA_( KA_MOUSE_LT_DBL_CLICK,    "Mouse-LDK",  NO_FIELDS)\
    _KA_( KA_MOUSE_MID_CLICK,       "Mouse-MK",   NO_FIELDS)\
    _KA_( KA_MOUSE_MID_DBL_CLICK,   "Mouse-MDK",  NO_FIELDS)\
    _KA_( KA_MOUSE_STEP,            "M Speed",    NO_FIELDS)\
    _KA_( KA_REPORT_KEY,            "Show Key",   NO_FIELDS)\
    _KA_( KA_TOGGLE_KEY_ARROWS,     "ToggleMov",  NO_FIELDS)\
    _KA_( KA_POWER_CYCLE,           "PowerCycle", NO_FIELDS)\
/* end of KEY_ACTION_LIST */


enum key_map_actions
{
#define _KA_( _e_, _s_, _m_ ) _e_,
    KEY_ACTION_LIST
#undef _KA_
    KA_LAST_ACTION
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
    SETTLE_KEY_BOUNCE = 1,
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


#define SERIAL_BUFFER_SIZE 16

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
            CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &m_stream_obj);
            m_stream = &m_stream_obj;
            m_ready_after = global_ticks + SERIAL_STARTUP_DELAY_MS;
            m_head = m_tail = 0;
        }
        
        bool available(void) 
        {  
            return ( global_ticks >= m_ready_after ); 
        }
    
        void end(void){}
        void flush(void){}
        void clear(void){}

        void push(uint8_t c)
        {
            if (((m_head + 1) == m_tail ) || (((m_head+1) == SERIAL_BUFFER_SIZE)&(m_tail==0)))
                return;
            m_serial_buffer[m_head] = c;
            m_head = ((m_head+1)==SERIAL_BUFFER_SIZE)?0:m_head+1;
        }

        uint8_t pop(void)
        {
            if ( m_head == m_tail ) return 0xff;
            uint8_t c = m_serial_buffer[m_tail];
            m_tail = ((m_tail+1)==SERIAL_BUFFER_SIZE)?0:m_tail+1;
            return c;
        }

        uint8_t peek(void)
        {
            if ( m_head == m_tail ) return 0xff;
            return m_serial_buffer[m_tail];
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
            return pop();
        }
        uint8_t m_serial_buffer[ SERIAL_BUFFER_SIZE ];
        uint8_t m_head;
        uint8_t m_tail;
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
        int print( const char *str, ... )
        {
            va_list args;
            va_start (args, str);
            vsprintf( (char *)global_tmp_buffer, str, args);
            va_end (args);
            return write( (char *)global_tmp_buffer);
        }
        int print_P( const char *str, ... )
        {
            va_list args;
            va_start (args, str);
            vsprintf_P( (char *)global_tmp_buffer, str, args);
            va_end (args);
            return write( (char *)global_tmp_buffer);
        }
};


enum worker_states
{
    WORKER_STATE_INVALID = 0,
    WORKER_STARTING,
    WORKER_PULSE_ON,
    WORKER_PULSE_OFF,
    WORKER_WAITING,
    WORKER_DONE,
    
};




class navputter_worker_class
{
public:
    void begin(uint32_t duration)
    {
        m_running = false;
        m_state = WORKER_STATE_INVALID;
        m_duration = duration; 
    }

    uint8_t get_state( void )
    {
        return m_state;
    }

    void set_state( uint8_t new_state )
    {
        m_state = new_state;
    }

    void set_start( uint32_t ticks )
    {
        m_start = ticks;
    }

    bool is_running( void ) 
    {
        return m_running;
    }

    void running( bool tf )
    {
        m_running = tf;
    }

    class navputter_worker_class *get_next_job( void ) 
    {
        return m_next;
    }

    class navputter_worker_class *get_prev_job( void ) 
    {
        return m_prev;
    }

    void set_next_job( class navputter_worker_class *job )
    {
        m_next = job;
    }
    
    void set_prev_job( class navputter_worker_class *job )
    {
        m_prev = job;
    }


    virtual void start( void );
    virtual void done( void );
    virtual void run_job( void );


private:
    uint32_t    m_start;
    uint8_t     m_state;
    bool        m_running;
    uint32_t    m_duration;
    class navputter_worker_class *m_next;
    class navputter_worker_class *m_prev;
};

    
class navputter_worker_pulse_class : public navputter_worker_class
{
public:
    void begin( 
        uint8_t     port,
        uint8_t     pin_state_1,
        uint16_t    hold_time_1,
        uint8_t     pin_state_2,
        uint16_t    hold_time_2,
        uint16_t    cycles
    )
    {
        m_port = port;
        m_pin_state_1 = pin_state_1;
        m_pin_state_2 = pin_state_2;
        m_hold_time_1 = hold_time_1;
        m_hold_time_2 = hold_time_2;
        m_cycles = cycles;
    }

    void start(void);

    virtual void run_job();

    virtual void pulse_on(void);
    virtual void pulse_off(void);

private:

    uint8_t m_port;
    uint8_t m_pin_state_1;
    uint8_t m_pin_state_2;
    uint16_t m_hold_time_1;
    uint16_t m_hold_time_2;
    uint16_t m_cycles;
    uint8_t  m_next_state;
    uint32_t m_wait_until;
};
 

class navputter_work_pool_class
{
public:
    void begin( void )
    {
        m_job_head = NULL;
        m_job_tail = NULL;
    }

    void start_job( navputter_worker_class *job );

    void tick()
    {
        navputter_worker_class *j;
        for ( j=first_job(); j; j=next_job(j) )
        {
            j->run_job();
            if ( j->get_state() == WORKER_DONE )
            {
                end_job(j);
            }
        }
    }
private:
    void end_job( navputter_worker_class *job )
    {
        job->running(false);
        remove_job(job);
        job->done();
    }    

    void add_job_list( navputter_worker_class *job )
    {
        if ( !m_job_tail )
        {
            m_job_head = m_job_tail = job;
        }
        else
        {
            m_job_tail->set_next_job( job );
            job->set_prev_job( m_job_tail );
            job->set_next_job( NULL );
            m_job_tail = job;
        }
    }

    navputter_worker_class *m_job_head;
    navputter_worker_class *m_job_tail;

    navputter_worker_class *first_job( void ) 
    {
        return m_job_head;
    }

    navputter_worker_class *next_job( navputter_worker_class *job )
    {
        return job->get_next_job();
    }

    void remove_job( navputter_worker_class *job )
    {
        navputter_worker_class *nextjob = job->get_next_job();
        navputter_worker_class *prevjob = job->get_prev_job();

        if ( m_job_head == job ) 
        {
            m_job_head = nextjob;
        }
        if ( m_job_tail == job )
        {
            m_job_tail = prevjob;
        }
        if ( nextjob ) nextjob->set_prev_job( prevjob );
        if ( prevjob ) prevjob->set_next_job( nextjob );
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

    void poll();

    void tick()
    {
        if ( m_callback ) m_callback();
    }

private:
    void (*m_callback)(void);
    uint32_t m_call_next;
    uint32_t m_last_work;
    uint32_t m_interval;
};     




class usb_keyboard_class
{
    public:
        void begin(void) 
        {
        }
        void end(void) {}

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
    uint8_t  m_keypad_state;
    uint32_t m_until;
    uint8_t  m_next_state;
    uint8_t  m_keystate[ MAX_KEY_COLS ];
    uint8_t  m_last_keystate[ MAX_KEY_COLS ];
    uint8_t  m_keypress[ MAX_KEY_COLS ];
    uint8_t  m_keydown[ MAX_KEY_COLS ];
private:
    uint8_t  m_col;
    uint8_t  m_cur_rows;
};

#define ROW_PINS \
    _K_PIN_( KEYPAD_R0, DDRD, PORTD, PIND, 0 ) \
    _K_PIN_( KEYPAD_R1, DDRD, PORTD, PIND, 1 ) \
    _K_PIN_( KEYPAD_R2, DDRD, PORTD, PIND, 2 ) \
    _K_PIN_( KEYPAD_R3, DDRD, PORTD, PIND, 3 ) \
    _K_PIN_( KEYPAD_R4, DDRD, PORTD, PIND, 4 ) \
    _K_PIN_( KEYPAD_R5, DDRD, PORTD, PIND, 6 ) 

#define COL_PINS \
    _K_PIN_( KEYPAD_C0, DDRD, PORTD, PIND, 7 ) \
    _K_PIN_( KEYPAD_C1, DDRB, PORTB, PINB, 1 ) \
    _K_PIN_( KEYPAD_C2, DDRB, PORTB, PINB, 2 ) \
    _K_PIN_( KEYPAD_C3, DDRB, PORTB, PINB, 3 ) \
    _K_PIN_( KEYPAD_C4, DDRB, PORTB, PINB, 4 ) \
    _K_PIN_( KEYPAD_C5, DDRB, PORTB, PINB, 5 ) 

class navputter_tenkey_keypad_class : public navputter_keypad_class
{
public:
    enum col_enums
    {
        #define _K_PIN_( e, ddr, port, inpin, pin ) e,
            COL_PINS
        #undef _K_PIN_
    };

    enum row_enums
    {
        #define _K_PIN_( e, ddr, port, inpin, pin ) e,
            ROW_PINS
        #undef _K_PIN_
    };

    void begin();
    void poll();
    void trigger();
private:
    uint8_t m_row_state;
    uint8_t m_col_state;
    uint8_t m_col;
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
        virtual void read( void *buf, uint32_t len, size_t offset=0 );
        virtual void write( void *buf, uint32_t len, size_t offset=0 );
};


class navputter_eeprom_class : public generic_eeprom_class
{
    public:
        void begin(void);
        void init(void);
        virtual void read( void *buf, uint32_t len, size_t offset=0 );
        virtual void write( void *buf, uint32_t len, size_t offset=0 );
};



#define INVALID_KEYSTATE ((uint8_t)0xff)


enum
{
    READ_COMMAND = 0,
    READ_INT,   
    SET_VALUE,
    READ_PORT,
    CALL_FUNC,
    READ_PARAM,
    CONVERT_VALUE,
    UNPRESS_KEY,
    EDIT_COMMAND,
    WRITE_COMMAND,
    EDIT_KEY,
    READ_LOAD_INT,
    READ_SAVE_INT,
    LOAD_KEYMAP,
    SAVE_KEYMAP,
    EDIT_PINMAP,
    EDIT_PIN_ENTRY,
};


    
class navputter_menu_base_class
{
public:
    virtual void begin(void);
    virtual void end(void);
    virtual void poll(void);
    virtual void usage(void);

    uint8_t get_state( void )
    {
        return m_state;
    }    
    void set_state( uint8_t state )
    {
        m_state = state;
    }
    void read_command(void)
    {
        set_state(READ_COMMAND);
        usage();
    }
private:
    uint8_t m_state;

};



#define _SER_CMDS_\
    _SC_( 'h', "Show this help.", usage )\
    _SC_( 'e', "Show or update eeprom.", handle_eeprom )\
    _SC_( 'g', "Interface with GPIO via serial.", serial_gpio ) \
    _SC_( 'w', "Write config to eeprom.", write_eeprom )\
    _SC_( 'k', "Show or edit additional keymaps.", handle_keymaps)
// end of _SER_CMDS_

class navputter_main_menu_class : public navputter_menu_base_class
{
public:
    virtual void begin(void);
    virtual void end(void);
    virtual void poll(void);
    virtual void usage(void);

    void serial_gpio(void);
    void handle_eeprom(void);
    void write_eeprom(void);
    void handle_keymaps(void);
};


class navputter_eeprom_menu_class : public navputter_menu_base_class
{
public:
    virtual void begin(void);
    virtual void end(void);
    virtual void poll(void);
    virtual void usage(void);
    int validate_keymap( int cmd, int val );
    int no_check( int cmd, int val );

    void read_int(void)
    {
        set_state( READ_INT );
        m_position =0;
    }
    void set_value(void)
    {
        m_int_value[m_position]=0;
        set_state( SET_VALUE );
    }

private:
    bool    m_dirty;
    int     m_position;    
    uint8_t m_command;
#define MAX_INT_SIZE 8 
    uint8_t m_int_value[ MAX_INT_SIZE ];
};

#define GPIO_MENU\
    _GM_( 'd', gpio_ddr,            1, "d<port><value>        - DDR <port>" )\
    _GM_( 'S', gpio_set_ddr_bit,    1, "S<port><value>        - DDR <port>" )\
    _GM_( 'C', gpio_clear_ddr_bit,  1, "C<port><value>        - DDR <port>" )\
    _GM_( 'i', gpio_in,             0, "i<port>               - PIN <port>" )\
    _GM_( 'I', gpio_in_mask,        1, "I<port><mask>         - PIN <port>&<mask>" )\
    _GM_( 'b', gpio_in_bit,         1, "b<port><pos>          - ((PIN <port>&(1<<pos))>>pos\n\r" )\
    _GM_( 'o', gpio_out,            1, "o<port><value>        - PORT<port> = value\n\r")\
    _GM_( '^', gpio_xor,            1, "^<port><value>        - PORT<port> = PORT<port> ^ value\n\r")\
    _GM_( '&', gpio_and,            1, "&<port><value>        - PORT<port> = PORT<port> & value\n\r")\
    _GM_( '|', gpio_or,             1, "|<port><value>        - PORT<port> = PORT<port> | value\n\r")\
    _GM_( 's', gpio_set_bit,        1, "s<port><pos>          - PORT<port> = PORT<port> | (1<<pos)\n\r")\
    _GM_( 'c', gpio_clear_bit,      1, "c<port><pos>          - PORT<port> = PORT<port> & ~(1<<pos)\n\r")\
    _GM_( 'a', gpio_read_a2d,       1, "a<pin>                - read a to d from pin \n\r")\
    _GM_( 'A', gpio_setup_a2d,      2, "A<pin><ref>           - configure a to d on pin. ref: 0=AREF, 1=AVCC, 2=reserved, 3=internal\n\r")\


#define GPIO_PORTS\
/*    _GP_( 'a', PORTA, PINA, DDRA )*/\
    _GP_( 'b', PORTB, PINB, DDRB )\
    _GP_( 'c', PORTC, PINC, DDRC )\
    _GP_( 'd', PORTD, PIND, DDRD )


class navputter_gpio_menu_class: public navputter_menu_base_class
{
public:
    virtual void begin(void);
    virtual void end(void);
    virtual void poll(void);
    virtual void usage(void);
    void gpio_ddr( void ); 
    void gpio_in( void );
    void gpio_in_mask( void );
    void gpio_in_bit( void );
    void gpio_out( void );
    void gpio_xor( void );
    void gpio_read_a2d( void );
    void gpio_setup_a2d( void );
    void gpio_and( void ); 
    void gpio_or( void );
    void gpio_set_bit( void );
    void gpio_clear_bit( void );
    void gpio_set_ddr_bit( void );
    void gpio_clear_ddr_bit( void );
    void read_param(void)
    {
        m_pos=0;
        set_state(READ_PARAM);
    } 
private:
    uint8_t m_cmd;                          // current command
    uint8_t m_pos;                          // current index in m_value for integer read
    uint8_t m_value[ MAX_INT_SIZE ];        // integer being read
#define PARAMS_MAX                   2      // 2 parameters max
    uint8_t m_params[ PARAMS_MAX ];         // array of parameters (8 bit)
    uint8_t m_count;                        // current parameter being read.
    uint8_t m_port;                         // gpio port (ascii lower case)
};

#define KEYCODE_EDIT_LIST\
    _KEL_( 'd', "next action",         next_action,    KEYVAL_NEXT_ACTION )\
    _KEL_( 'f', "prev acton",          prev_action,    KEYVAL_PREV_ACTION )\
    _KEL_( ' ', "next field",          next_field,     KEYVAL_NEXT_FIELD  )\
    _KEL_( 'j', "value next",          next_value,     KEYMOD_NEXT_VALUE )\
    _KEL_( 'k', "value prev",          prev_value,     KEYMOD_PREV_VALUE )\
    _KEL_( 'q', "quit",                quit_edit,      KEYVAL_QUIT )

enum keycode_edit_enums
{
#define _KEL_( k, s, f, _e_ ) _e_,
    KEYCODE_EDIT_LIST
#undef _KEL_
};


#define KEYCODE_MENU_LIST\
    _KML_( 'l', "load keymap", load_keymap )\
    _KML_( 's', "save keymap", save_keymap )\
    _KML_( 'e', "edit keymap", edit_keymap )\
    _KML_( 'd', "dump all keymaps", dump_keymap )\
    _KML_( 'p', "edit pin map",edit_pin_map)\
    _KML_( 'q', "quit", quit )
   

#define KEYCODE_MODIFIER_LIST\
    _MODL_( KC_MOD_NONE,    "NONE",          0 ) \
    _MODL_( KC_MOD_LSHIFT,  "L-SHIFT",       HID_KEYBOARD_MODIFIER_LEFTSHIFT ) \
    _MODL_( KC_MOD_RSHIFT,  "R-SHIFT",       HID_KEYBOARD_MODIFIER_RIGHTSHIFT) \
    _MODL_( KC_MOD_LCTRL,   "L-CTRL",        HID_KEYBOARD_MODIFIER_LEFTCTRL  ) \
    _MODL_( KC_MOD_RCTRL,   "R-CTRL",        HID_KEYBOARD_MODIFIER_RIGHTCTRL ) \
    _MODL_( KC_MOD_LALT,    "L-ALT",         HID_KEYBOARD_MODIFIER_LEFTALT   ) \
    _MODL_( KC_MOD_RALT,    "R-ALT",         HID_KEYBOARD_MODIFIER_RIGHTALT  ) \
    _MODL_( KC_MOD_LGUI,    "L-GUI",         HID_KEYBOARD_MODIFIER_LEFTGUI   ) \
    _MODL_( KC_MOD_RGUI,    "R-GUI",         HID_KEYBOARD_MODIFIER_RIGHTGUI  )

enum keycode_modifier_cycle_order
{     
#define _MODL_( _enum_, _str_, _sc_ ) _enum_,
    KEYCODE_MODIFIER_LIST
#undef _MODL_
    KC_MOD_LAST
}; 
  
#define KEYCODE_KEY_LIST \
    _KEYL_( "NONE", "NONE", 0 ) \
    _KEYL_( "a",    "A",    HID_KEYBOARD_SC_A ) \
    _KEYL_( "b",    "B",    HID_KEYBOARD_SC_B ) \
    _KEYL_( "c",    "C",    HID_KEYBOARD_SC_C ) \
    _KEYL_( "d",    "D",    HID_KEYBOARD_SC_D ) \
    _KEYL_( "e",    "E",    HID_KEYBOARD_SC_E ) \
    _KEYL_( "f",    "F",    HID_KEYBOARD_SC_F ) \
    _KEYL_( "g",    "G",    HID_KEYBOARD_SC_G ) \
    _KEYL_( "h",    "H",    HID_KEYBOARD_SC_H ) \
    _KEYL_( "i",    "I",    HID_KEYBOARD_SC_I ) \
    _KEYL_( "j",    "J",    HID_KEYBOARD_SC_J ) \
    _KEYL_( "k",    "K",    HID_KEYBOARD_SC_K ) \
    _KEYL_( "l",    "L",    HID_KEYBOARD_SC_L ) \
    _KEYL_( "m",    "M",    HID_KEYBOARD_SC_M ) \
    _KEYL_( "n",    "N",    HID_KEYBOARD_SC_N ) \
    _KEYL_( "o",    "O",    HID_KEYBOARD_SC_O ) \
    _KEYL_( "p",    "P",    HID_KEYBOARD_SC_P ) \
    _KEYL_( "q",    "Q",    HID_KEYBOARD_SC_Q ) \
    _KEYL_( "r",    "R",    HID_KEYBOARD_SC_R ) \
    _KEYL_( "s",    "S",    HID_KEYBOARD_SC_S ) \
    _KEYL_( "t",    "T",    HID_KEYBOARD_SC_T ) \
    _KEYL_( "u",    "U",    HID_KEYBOARD_SC_U ) \
    _KEYL_( "v",    "V",    HID_KEYBOARD_SC_V ) \
    _KEYL_( "w",    "W",    HID_KEYBOARD_SC_W ) \
    _KEYL_( "x",    "X",    HID_KEYBOARD_SC_X ) \
    _KEYL_( "y",    "Y",    HID_KEYBOARD_SC_Y ) \
    _KEYL_( "z",    "Z",    HID_KEYBOARD_SC_Z ) \
    _KEYL_( "1",    "!",    HID_KEYBOARD_SC_1_AND_EXCLAMATION ) \
    _KEYL_( "2",    "@",    HID_KEYBOARD_SC_2_AND_AT ) \
    _KEYL_( "3",    "#",    HID_KEYBOARD_SC_3_AND_HASHMARK ) \
    _KEYL_( "4",    "$",    HID_KEYBOARD_SC_4_AND_DOLLAR ) \
    _KEYL_( "5",    "%",    HID_KEYBOARD_SC_5_AND_PERCENTAGE ) \
    _KEYL_( "6",    "^",    HID_KEYBOARD_SC_6_AND_CARET ) \
    _KEYL_( "7",    "&",    HID_KEYBOARD_SC_7_AND_AMPERSAND ) \
    _KEYL_( "8",    "*",    HID_KEYBOARD_SC_8_AND_ASTERISK ) \
    _KEYL_( "9",    "(",    HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS ) \
    _KEYL_( "0",    ")",    HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS ) \
    _KEYL_( "ENT",  0,      HID_KEYBOARD_SC_ENTER ) \
    _KEYL_( "ESC",  0,      HID_KEYBOARD_SC_ESCAPE ) \
    _KEYL_( "BS",   0,      HID_KEYBOARD_SC_BACKSPACE ) \
    _KEYL_( "TAB",  0,      HID_KEYBOARD_SC_TAB ) \
    _KEYL_( "SPC",  0,      HID_KEYBOARD_SC_SPACE ) \
    _KEYL_( "-",    "_",    HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE ) \
    _KEYL_( "=",    "+",    HID_KEYBOARD_SC_EQUAL_AND_PLUS ) \
    _KEYL_( "[",    "{",    HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE ) \
    _KEYL_( "]",    "}",    HID_KEYBOARD_SC_CLOSING_BRACKET_AND_CLOSING_BRACE ) \
    _KEYL_( "\\",   "|",    HID_KEYBOARD_SC_BACKSLASH_AND_PIPE ) \
    _KEYL_( "?",    "~",    HID_KEYBOARD_SC_NON_US_HASHMARK_AND_TILDE ) \
    _KEYL_( ";",    ":",    HID_KEYBOARD_SC_SEMICOLON_AND_COLON ) \
    _KEYL_( "'",    "\"",   HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE ) \
    _KEYL_( "`",    "~",    HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE ) \
    _KEYL_( ",",    "<",    HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN ) \
    _KEYL_( ".",    ">",    HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN ) \
    _KEYL_( "/",    "?",    HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK ) \
    _KEYL_( "CL",   0,      HID_KEYBOARD_SC_CAPS_LOCK ) \
    _KEYL_( "F1",   0,      HID_KEYBOARD_SC_F1 ) \
    _KEYL_( "F2",   0,      HID_KEYBOARD_SC_F2 ) \
    _KEYL_( "F3",   0,      HID_KEYBOARD_SC_F3 ) \
    _KEYL_( "F4",   0,      HID_KEYBOARD_SC_F4 ) \
    _KEYL_( "F5",   0,      HID_KEYBOARD_SC_F5 ) \
    _KEYL_( "F6",   0,      HID_KEYBOARD_SC_F6 ) \
    _KEYL_( "F7",   0,      HID_KEYBOARD_SC_F7 ) \
    _KEYL_( "F8",   0,      HID_KEYBOARD_SC_F8 ) \
    _KEYL_( "F9",   0,      HID_KEYBOARD_SC_F9 ) \
    _KEYL_( "F10",  0,      HID_KEYBOARD_SC_F10 ) \
    _KEYL_( "F11",  0,      HID_KEYBOARD_SC_F11 ) \
    _KEYL_( "F12",  0,      HID_KEYBOARD_SC_F12 ) \
    _KEYL_( "HOME", 0,      HID_KEYBOARD_SC_HOME ) \
    _KEYL_( "PGUP", 0,      HID_KEYBOARD_SC_PAGE_UP ) \
    _KEYL_( "DEL",  0,      HID_KEYBOARD_SC_DELETE ) \
    _KEYL_( "END",  0,      HID_KEYBOARD_SC_END ) \
    _KEYL_( "PGDN", 0,      HID_KEYBOARD_SC_PAGE_DOWN ) \
    _KEYL_( "RT",   0,      HID_KEYBOARD_SC_RIGHT_ARROW ) \
    _KEYL_( "LT",   0,      HID_KEYBOARD_SC_LEFT_ARROW ) \
    _KEYL_( "DN",   0,      HID_KEYBOARD_SC_DOWN_ARROW ) \
    _KEYL_( "UP",   0,      HID_KEYBOARD_SC_UP_ARROW ) \
    _KEYL_( "NUML", 0,      HID_KEYBOARD_SC_NUM_LOCK ) \
    _KEYL_( "/",    0,      HID_KEYBOARD_SC_KEYPAD_SLASH ) \
    _KEYL_( "*",    0,      HID_KEYBOARD_SC_KEYPAD_ASTERISK ) \
    _KEYL_( "-",    0,      HID_KEYBOARD_SC_KEYPAD_MINUS ) \
    _KEYL_( "+",    0,      HID_KEYBOARD_SC_KEYPAD_PLUS ) \
    _KEYL_( "ENT",  0,      HID_KEYBOARD_SC_KEYPAD_ENTER ) \
    _KEYL_( "END",  "1",    HID_KEYBOARD_SC_KEYPAD_1_AND_END ) \
    _KEYL_( "DN",   "2",    HID_KEYBOARD_SC_KEYPAD_2_AND_DOWN_ARROW ) \
    _KEYL_( "PGDN", "3",    HID_KEYBOARD_SC_KEYPAD_3_AND_PAGE_DOWN ) \
    _KEYL_( "LT",   "4",    HID_KEYBOARD_SC_KEYPAD_4_AND_LEFT_ARROW ) \
    _KEYL_( "5",    0,      HID_KEYBOARD_SC_KEYPAD_5 ) \
    _KEYL_( "RT",   "6",    HID_KEYBOARD_SC_KEYPAD_6_AND_RIGHT_ARROW ) \
    _KEYL_( "HOME", "7",    HID_KEYBOARD_SC_KEYPAD_7_AND_HOME ) \
    _KEYL_( "UP",   "8",    HID_KEYBOARD_SC_KEYPAD_8_AND_UP_ARROW ) \
    _KEYL_( "PGUP", "9",    HID_KEYBOARD_SC_KEYPAD_9_AND_PAGE_UP ) \
    _KEYL_( "INS",  "0",    HID_KEYBOARD_SC_KEYPAD_0_AND_INSERT ) \
    _KEYL_( "DEL",  ".",    HID_KEYBOARD_SC_KEYPAD_DOT_AND_DELETE ) \
    _KEYL_( "PLAY", 0,      HID_KEYBOARD_SC_MEDIA_PLAY ) \
    _KEYL_( "STOP", 0,      HID_KEYBOARD_SC_MEDIA_STOP ) \
    _KEYL_( "<<",   0,      HID_KEYBOARD_SC_MEDIA_PREVIOUS_TRACK ) \
    _KEYL_( ">>",   0,      HID_KEYBOARD_SC_MEDIA_NEXT_TRACK ) \
    _KEYL_( "EJECT",0,      HID_KEYBOARD_SC_MEDIA_EJECT ) \
    _KEYL_( "VOL+", 0,      HID_KEYBOARD_SC_MEDIA_VOLUME_UP ) \
    _KEYL_( "VOL-", 0,      HID_KEYBOARD_SC_MEDIA_VOLUME_DOWN ) \
    _KEYL_( "MUTE", 0,      HID_KEYBOARD_SC_MEDIA_MUTE ) 

enum key_scancode_index
{
#define _KEYL_( _lc_, _uc_, _sc_ ) key_code_enum_##_sc_,
    KEYCODE_KEY_LIST
#undef _KEYL_
    key_code_enum_last
};    


typedef struct keyedit_state
{
    uint8_t         key_ix;
    uint8_t         mod_ix;
    uint8_t         action_ix;
}keyedit_state_t;
 

#define KEY_PIN_MENU\
    _KPM_( 'c', "Next Column", inc_pin_edit_col )\
    _KPM_( 'r', "Next Row", inc_pin_edit_row )\
    _KPM_( 'e', "Edit", edit_pin_entry )\
    _KPM_( 'q', "Quit", read_command )
    
 
class navputter_keycode_menu_class: public navputter_menu_base_class
{
public:
    virtual void    begin(void);
    virtual void    end(void);
    virtual void    poll(void);
    virtual void    usage(void);
    void            keycode_edit_usage(void);
    void            edit_keymap(void);
    void            quit(void);
    void            show_keymap(void);
    void            save_keymap(void);
    void            edit_pin_map(void);
    void            exit_pin_map(void);
    void            edit_pin_map_usage(void);
    void            load_keymap(void);
    void            format_key_action(uint8_t seq);
    void            format_scancode( uint16_t sc );
    void            key_edit_usage( void );
    void            next_modifier(void);
    void            prev_modifier(void);
    void            next_action(void);
    void            prev_action(void);
    void            prev_value(void);
    void            next_value(void);
    void            next_field(void);
    void            quit_edit(void);
    void            set_keycode_display_indicies(void);
    void            save_keymap_to_eeprom( uint8_t ix );
    void            edit_next(void);
    void            dump_keymap(void);
    void            set_action(uint8_t action);
    void            set_field(uint8_t field);
    void            inc_pin_edit_col(void)
    {
        m_edit_pin_col++;
        if ( m_edit_pin_col == MAX_KEY_COLS ) m_edit_pin_col = 0;
    }
    void            inc_pin_edit_row(void)
    {
        m_edit_pin_row++;
        if ( m_edit_pin_row == MAX_KEY_ROWS ) m_edit_pin_row = 0;
    }
    void exit_edit_entry()
    {
        set_state( EDIT_KEY );
        keycode_edit_usage();
        m_int_ix=0;
    }
    void edit_pin_entry()
    {
        set_state( EDIT_PIN_ENTRY );
        m_int_ix = 0;
        edit_pin_field_usage();
    }
    void edit_pin_field_usage(void);
 
private:
    uint8_t         m_edit_pin_row;
    uint8_t         m_edit_pin_col;
    uint8_t         m_cmd;
    uint8_t         m_row;
    uint8_t         m_col;
    uint8_t         m_edit_key;
    keyedit_state_t m_ks[2]; 
    uint8_t         m_key_map[MAX_KEY_ROWS][MAX_KEY_COLS];
    key_map_t       m_key_seq_map[MAX_KEY_SEQUENCES];
    uint8_t         m_seq;
#define MAX_INT_SIZE 8 
    uint8_t         m_int_value[ MAX_INT_SIZE ];
    uint8_t         m_int_ix;
    uint8_t         m_edit_field;
    uint8_t         m_field_min;
    uint8_t         m_field_max;
    uint8_t         m_field_mask;
    uint8_t         m_field_shift;
    uint8_t         *m_field_ptr;
};



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

class navputter_class
{
public:
    void begin()
    {
        memset( &m_config, 0, sizeof( m_config ) );
        memset( m_cur_map, 0, sizeof( m_cur_map) );
        memset( m_seq_map, 0, sizeof( m_seq_map) );
	    SetupHardware();
	    GlobalInterruptEnable();
        m_serial.begin(9600);
        m_keyboard.begin();
        m_timer.begin( NULL, 1000 );
        m_workers.begin();
        m_watchdog.begin();
        m_keypad.begin();
        m_mouse.begin();
        while( !m_serial.available() ) m_watchdog.kick();
        m_eeprom.begin();
        m_eeprom.init();
        m_cur_menu = NULL;
        m_eeprom_dirty = false;
    }
    void read_eeprom_keymap(void);
    void set_menu( class navputter_menu_base_class *menu )
    {
        if ( m_cur_menu )
        {
            m_cur_menu->end(); } m_cur_menu = menu;       
        m_cur_menu->begin();
        m_cur_menu->usage();
    }

    void clear_menu(void)
    {
        m_cur_menu->end();
        m_cur_menu=NULL;
    }
    void set_keymap( void )
    {
        int i=0;
#define _KM_(c1,c2,c3,c4,c5,c6) m_cur_map[i][0]=c1; m_cur_map[i][1]=c2; m_cur_map[i][2]=c3; m_cur_map[i][3]=c4; m_cur_map[i][4]=c5; m_cur_map[i][5]=c6; i++;
        KEYPAD_PIN_MAP
#undef _KM_
    }

    void set_seq_map( void )
    {
#define KP_KEY( _sc_, _ev_, _act_, _x_, _y_ ) \
        m_seq_map[_ev_]=_act_;
        KP_KEY_LIST
#undef KP_KEY
    }


    void poll()
    {
        if ( m_cur_menu ) m_cur_menu->poll();
    }
    void error( uint8_t err );

    void set_dirty(bool dirty=true) {m_eeprom_dirty=dirty;}
    bool is_dirty(void) {return m_eeprom_dirty;}
    uint8_t *get_map()
    {
        return  (uint8_t *)m_cur_map;
    }

    key_map_t *get_seq_map(void)
    {
        return  m_seq_map;
    }

    void usage(void);
    void big_whale(void);
    void eeprom_menu(void);

    lufa_mouse_class            m_mouse;
    navputter_timer_class       m_timer;
    navputter_work_pool_class   m_workers;
    navputter_watchdog_class    m_watchdog; 
    navputter_serial_class      m_serial;
    usb_keyboard_class          m_keyboard;
    //navputter_keypad_class      m_keypad;
    navputter_tenkey_keypad_class      m_keypad;
    navputter_eeprom_class      m_eeprom;
    navputter_menu_base_class   *m_cur_menu;
    eeprom_header_t             m_config;
    uint8_t                     m_cur_map[MAX_KEY_ROWS][MAX_KEY_COLS];
    key_map_t                   m_seq_map[MAX_KEY_SEQUENCES];
    bool                        m_eeprom_dirty;
};

#define DOG     myputter.m_watchdog
#define MOUSE   myputter.m_mouse
#define TIMER   myputter.m_timer
#define WORKERS myputter.m_workers
#define KEY     myputter.m_keyboard
#define PAD     myputter.m_keypad
#define PROM    myputter.m_eeprom
#define SERIAL  myputter.m_serial
#define CONFIG  myputter.m_config

#endif // __NAVPUTTER_H__

