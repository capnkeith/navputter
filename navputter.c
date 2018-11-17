
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
* 
* port mappings:
* 
* Navputter currently has a 4x4 keypad ( PORTF ), 
* An uTFT_ST7735 display ( PORTC 0-3 )
* A radial encoder ( PIND0,1 )
* misc buttons ( PIND2, 3 )
* 2 LEDS ( PIND4, PIND5 )
*
* Everything is configured in navputter.h, except for the display which is
* configured inside uTFT_ST7735.h 
*
******************************************************************************
*
* Navputter operation:
*
* Navputter has 3 modes, selectable with the upper left button (default config). 
* These modes are 'slow key mode', 'fast key mode', and 'mouse mode'. If you
* have the LEDs connected they will light left, right, or both respectivley.
*
* In the 3 modes buttons are mapped to key sequences or mouse moves in the
* different maps. key_map => slow mode, key_map2 => fast mode, mouse_map =>
* mouse mode. 
*
*
******************************************************************************
*
* Building:
*
* Navputter is currently set up to build as a demo of LUFA. To build navputter
* first get and build lufa:
*
*    git clone https://github.com/abcminiuser/lufa lufa
*    cd lufa
*    make
*
* Now pull Navputter into the correct location:
*
*    cd Demos/Device/ClassDriver
*    git clone https://github.com/capnkeith/navputter navputter
*    cd navputter
*    make
*
* Now program your AVR
*
*    make avrdude
*
* 
* 
******************************************************************************
* 
* Debugging:
*   if DEBUG is defined you can use the dbgprint which is printf for 
*   the lcd, or your can fprintf( gfp, ... ) to write out th serial adapter
*   if you are debugging nmea you will be glad for the lcd. I think it is 
*   possible to use an actual debugger somehow...
* 
******************************************************************************
TODO:

* Currently key repeat does not working. I think it is best to handle it in
  the uC.

******************************************************************************
*

Key Mappings:

Navputter is fully eeprom customizable, but the first time after a fresh 
program, or if you write 0xff to eeprom location 0, the eeprom will be reset
to factory default which corresponds rougly to this function map below

   ---------------------------------------------------
    toggle      |   up      | anchor    |routstart
   ---------------------------------------------------
    left        |markboat   | right     |routend 
   ---------------------------------------------------
    zim         | down      | zout      |routenext 
   ---------------------------------------------------
    daynight    |track      |follow     |drop cursor
   ---------------------------------------------------

  flow key set maps like this:
  
   ---------------------------------------------------
   |INT        | alt+up    | A         |crrl+r 
   ---------------------------------------------------
   |alt+lt     | ctrl+o    | alt+rt    | esc 
   ---------------------------------------------------
   |alt+ +     | alt+down  | alt + -   | ctrl+n
   ---------------------------------------------------
   | f5        |  ?        | F2        | ctrl+m     
   ---------------------------------------------------

  fast key set maps like this:


   ---------------------------------------------------
   |INT         | up        | A         |  ctrl+r 
   ---------------------------------------------------
   |lt          | ctrl+o    | rt        | esc  
   ---------------------------------------------------
   |  +         | down      | -         | ctrl+n  
   ---------------------------------------------------
   | f5         |  ?        |F2         | ctrl+m     
   ---------------------------------------------------

mouse map is like this:

   ---------------------------------------------------
   |INT         | mouse up  | A         | ctrl_r
   ---------------------------------------------------
   |mouseleft   | ctrl+o    |mousert    | esc 
   ---------------------------------------------------
   |  alt+ +    | mouse down| alt + -   | ctrl+n 
   ---------------------------------------------------
   | f5         | ?         | f2        | ctrl_+m
   ---------------------------------------------------


*/



#include "navputter.h"

/* this is the serial interface. */
extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;


uint16_t out_key_buffer[ MAX_KEY_BUFFER_SZ ];   /* key presses going out the USB */

                                                /* I don't know if volitle is required or not... */
volatile uint8_t out_key_head = 0;              /* head index for the keypress buffer */
volatile uint8_t out_key_tail = 0;              /* tail index for the keypress buffer */

volatile uint32_t global_ticks=0;               /* milliseconds since boot */
uint8_t global_mouse_dir=0;                     /* bitmask of all current mouse directions and clicks */
uint8_t global_mouse_mode = KEY_SLOW_MODE;      /* current mode ( slow key, fast key, mouse ) */
FILE *gfp=NULL;

/*
 * simple circular buffer for holding keypresses until they can be transmitted
 * out the usb. Push the key and modifier. Key goes in high byte, modifiers in the low order byte.
 */
void push_key( uint8_t key, uint8_t mod )
{
    if ( out_key_head == MAX_KEY_BUFFER_SZ )
    {
        if ( out_key_tail == 0 )
        {
            printf("key buffer full");
            return;  
        }
        else
        {
            out_key_buffer[out_key_head] = (uint16_t)key << 8 | mod;
            out_key_head = 0;
        }
    }
    else
    {
        if ( out_key_head + 1 == out_key_tail )
        {
            printf("key buffer full\n");
            return;
        }
        else
        {
            out_key_buffer[out_key_head] = (uint16_t)key << 8 | mod;
            out_key_head = ( out_key_head == MAX_KEY_BUFFER_SZ ) ? 0 : out_key_head+1;
        }
    }
}

/*
 * pop function for the vserial driver to retrieve the next keypress. Returns 0 if no more
 * keys queued ortherwise key code in high order 2 bytes, modifier in low two bytes
 */
uint16_t pop_key(void)
{
    if ( out_key_tail == out_key_head ) 
        return 0;
    uint16_t key = out_key_buffer[ out_key_tail ];
    out_key_tail = ( out_key_tail == MAX_KEY_BUFFER_SZ ) ? 0 : out_key_tail+1;
    return key; 
}



#define USAGE_LIST\
    USAGE( "----------------------------------------" )\
    USAGE( "Navputter internal commands." ) \
    USAGE( "This interface is for setting eeprom." )\
    USAGE( "to send nmea prefix string with a $ char")\
    USAGE( "----------------------------------------" )\
    USAGE( "Command list:")\


#define CMD( e,x,f,h ) #x,

char *cmd_strs[] = { CMD_LIST };
#undef CMD

typedef struct eeprom_header
{
    uint8_t version;
    uint8_t rows;
    uint8_t cols;
    uint8_t unused;
}eeprom_header_t;


#define MAX_KEY_ROWS        4
#define MAX_KEY_COLS        4
#define MAX_KEY_SEQ         32
#define MAX_KEYS_PER_SEQ    4


typedef struct eeprom_layout
{
    eeprom_header_t config;
    uint8_t         key_map[ MAX_KEY_ROWS][ MAX_KEY_COLS ];             /* slow key map */
    uint8_t         key_map2[ MAX_KEY_ROWS][ MAX_KEY_COLS ];            /* fast key map */ 
    uint8_t         mouse_map[ MAX_KEY_ROWS][ MAX_KEY_COLS ];           /* mouse map */
    uint8_t         button_map[TOTAL_BUTTONS];                          /* radial encoder and misc buttons */
    uint16_t        key_seq[ MAX_KEY_SEQ ][ MAX_KEYS_PER_SEQ ];         /* key sequences */
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
    },
    

 
    /* mouse map */
    {
        { 0,                    MOUSE_DIR_UP,       2,                  3   }, 
        { MOUSE_DIR_LEFT,       5,                  MOUSE_DIR_RIGHT,    7   }, 
        { 8,                    MOUSE_DIR_DOWN,     10,                 11  },
        { MOUSE_LT_CLICK,       MOUSE_RT_CLICK,     14,                 15  },
    },             

/* map misc buttons and the radial encoder to different functions */
    { 10, 8, 0, 2 },
 
    /* key sequences */

    {
        { INT_CMD, IC_TOGGLE_MOUSE_KEYBOARD, 0, 0 },                                            /* key 0 is key slow / key fast / mouse toggle */
        { HID_KEYBOARD_MODIFIER_LEFTALT, HID_KEYBOARD_SC_UP_ARROW << 8 | HID_KEYBOARD_MODIFIER_LEFTALT, HID_KEYBOARD_MODIFIER_LEFTALT, 0 },               /* slow up arrow key */
		{ HID_KEYBOARD_SC_A << 8, 0, 0, 0 },                                                    /* bottom features */
		{ HID_KEYBOARD_SC_R << 8 | HID_KEYBOARD_MODIFIER_LEFTCTRL, 0, 0, 0 },                    /* route start */

        { HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_SC_LEFT_ARROW << 8 | HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_MODIFIER_RIGHTALT,0 },             /* slow left arrow key */
		{ HID_KEYBOARD_SC_O << 8 | HID_KEYBOARD_MODIFIER_LEFTCTRL, 0, 0, 0 },                   /* mark at boat */
        { HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_SC_RIGHT_ARROW << 8 | HID_KEYBOARD_MODIFIER_RIGHTALT, HID_KEYBOARD_MODIFIER_RIGHTALT,0 },            /* slow right arrow key */
		{ HID_KEYBOARD_SC_ESCAPE << 8, 0, 0, 0 },                                                  /* route end */

		{ HID_KEYBOARD_MODIFIER_LEFTALT, HID_KEYBOARD_SC_EQUAL_AND_PLUS<<8 | HID_KEYBOARD_MODIFIER_LEFTALT, HID_KEYBOARD_MODIFIER_LEFTALT, 0 },         /* zoom in slow */
        { HID_KEYBOARD_SC_DOWN_ARROW << 8   | HID_KEYBOARD_MODIFIER_LEFTALT, 0, 0, 0 },         /* down arrow key */
		{ HID_KEYBOARD_MODIFIER_LEFTALT, HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE<<8 | HID_KEYBOARD_MODIFIER_LEFTALT, HID_KEYBOARD_MODIFIER_LEFTALT, 0 },   /* zoom out slow */
		{ HID_KEYBOARD_SC_N << 8 | HID_KEYBOARD_MODIFIER_LEFTCTRL, 0, 0, 0 },                    /* routept next */

		{ HID_KEYBOARD_SC_F5 << 8, 0, 0, 0 },                                                   /* day/night */
		{ HID_KEYBOARD_SC_A << 8, 0, 0, 0 },                                                    /* should be tracking but no hotkey */
		{ HID_KEYBOARD_SC_F2 << 8, 0, 0, 0 },                                                   /* follow */
		{ HID_KEYBOARD_SC_M << 8 | HID_KEYBOARD_MODIFIER_LEFTCTRL, 0, 0, 0 },                    /* mark at cursor pos */
 
        { HID_KEYBOARD_SC_UP_ARROW << 8, 0, 0,0 },                                              /* fast up */
        { HID_KEYBOARD_SC_LEFT_ARROW << 8, 0,0,0 },                                             /* fast left */
        { HID_KEYBOARD_SC_RIGHT_ARROW << 8, 0,0,0 },                                            /* fast right */
        { HID_KEYBOARD_SC_DOWN_ARROW << 8, 0,0,0 },                                             /* fast down */

		{ HID_KEYBOARD_SC_EQUAL_AND_PLUS<<8, 0, 0, 0 },                                         /* zoom in fast */
		{ HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE<<8, 0, 0, 0 },                                   /* zoom out fast */
    }
};

/* 
 * this is our timer interrupt service routine. This is a real interrupt service routine, so
 * don't try and do much in here. global_ticks is milliseconds since power on
 */

ISR (TIMER1_COMPA_vect)
{
    global_ticks++;
}



void cmd_help( FILE *fp, char *str )
{
#define USAGE(s) fprintf(fp, "%s\n\r", s );
USAGE_LIST
#undef USAGE

#define CMD(e, x, f, h) fprintf( fp, "cmd: %s   usage %s\n\r",#x,h );
CMD_LIST
#undef CMD
fprintf( fp, "Enter Command:\n\r");
}


void cmd_dump( FILE *fp, char *str )
{
    fprintf(fp, "Navputter ver=%d, %dx%d keymap\n\r", global_config.config.version, global_config.config.rows, global_config.config.cols );
    fprintf(fp, "keymap dump:\n\r");
    
    uint8_t c, r;
    for ( r=0; r< global_config.config.rows; r++ )
    {
        for ( c=0; c< global_config.config.cols; c++ )
        {
            fprintf(fp,"%2d ", global_config.key_map[ r ][ c ] );
        }
        fprintf(fp, "\n\r" );
    }
    fprintf(fp, "key sequences. Key map referrers to these:\n\r");
    for ( r=0; r< MAX_KEY_SEQ; r++ )
    {
        fprintf( fp, "%d: ", r );
        for (c=0; c< MAX_KEYS_PER_SEQ; c++ )
        {
            fprintf( fp, "%2x %2x,", 
                (global_config.key_seq[r][c] & 0xff00) >> 8,
                (global_config.key_seq[r][c] & 0x00ff) );
        }
        fprintf( fp, "\n\r");
    }
}

void cmd_map( FILE *fp, char *str )
{
    uint8_t row = atoi(str);
    char *c = strchr( str, ' ' );
    if ( !c ) goto ERROR;
    uint8_t col = atoi(c+1);
    if ( !c ) goto ERROR;
    uint8_t seq = atoi(c+1);
    global_config.key_map[row][col]=seq;
    fprintf(fp, "successfully set key %d %d to seq %d\n\r", row, col, seq );
    return;
ERROR:
    fprintf(fp, "Arr, illegal format. Nothing chagned. Ex: map 1 2 2\n\r");
    return;
}


void cmd_seq( FILE *fp, char *str )
{
    char *c = strchr( str, ' ' );
    if ( !c ) goto ERROR;
    uint8_t key_id = atoi( c+1 );
    c = strchr( c+1, ' ' );
    if ( !c ) goto ERROR;

    
    uint8_t i;
    uint16_t key;
    uint16_t mod;
    for ( i=0; i< MAX_KEYS_PER_SEQ; i++ )
    {
        if ( (*(c+1) == '0') && (tolower(*(c+2)) == 'x') )
            sscanf( c+1, "%x", &key );
        else
            key = atoi(c+1);
        c = strchr( c+1, ' ' );
        if ( !c ) goto ERROR;
        if ( (*(c+1) == '0') && (tolower(*(c+2)) == 'x') )
            sscanf( c+1, "%x", &mod );
        else 
            mod = atoi(c+1);
        fprintf(fp, "key=%d, mod=%d\n\r", key, mod );
        global_config.key_seq[ key_id ][ i  ] = (key << 8) | mod;
        c = strchr( c, ',' );
        if ( !c ) break;
    }
    fprintf( fp, "sequence %d set successfully\n\r", key_id);
    return;
ERROR:
    fprintf( fp, "Arr, illegal format. No change.\n\r");
}

void cmd_save( FILE *fp, char *str )
{
    eeprom_write_block( (void *)&global_config, NULL, sizeof( global_config ) );
    fprintf( fp, "config written to eeprom.\n\r");
}

void cmd_input( FILE *fp, char *str )
{
#define CMD(e,x,f, h) \
    if ( strncmp( #x, str, strlen(#x)) == 0 ) \
    { \
        f(fp,str); \
    }  

CMD_LIST
#undef CMD
}

void reset_factory_default(void)
{
    dbgprint("reset to factory default\n");
    eeprom_write_block( (void *)&global_config, NULL, sizeof( global_config ) );
}

void set_leds( uint8_t leds )
{
    uint8_t i;
#define FIRST_LED_BIT  4        /* this is ugly */
#define LED( e, bit, dir, port )                    \
    if ( leds & (1 << (bit-FIRST_LED_BIT)) )\
        port |= 1<<bit;    \
    else \
        port &= ~(1<<bit);
LED_LIST
#undef LED

}

void run_internal_cmd( uint16_t cmd )
{
    switch(cmd)
    {
        case IC_TOGGLE_MOUSE_KEYBOARD:
            global_mouse_mode = ( global_mouse_mode < MOUSE_MODE ) ? global_mouse_mode+1 : KEY_SLOW_MODE;
            global_mouse_dir = 0;
            dbgprint( "mouse mode %d\n", global_mouse_mode );
            set_leds( global_mouse_mode+1 );
            break;
        default:
            dbgprint("unkown cmd %d\n", cmd );
            break;
    }
}

void push_seq( uint8_t seq )
{
    uint8_t i;

    for (i=0; i<MAX_KEYS_PER_SEQ; i++ )
    {
        if ( global_config.key_seq[ seq ][ i ] )
        {
            uint8_t key_out = (global_config.key_seq[ seq ][i] & 0xff00 ) >> 8;
            uint8_t mod_out = (global_config.key_seq[ seq ][i] & 0x00ff );
            push_key( key_out, mod_out );
        }
        else return;
    }
}


void handle_keyseq( uint8_t event, uint8_t seq )
{
    if ( seq & MOUSE_MOVE )  
    {
        handle_mouseseq( event, seq );
    }
    else
    {
        if ( event == EVENT_KEYPAD_DOWN )
        {
            if ( global_config.key_seq[seq][0] == INT_CMD )                
                run_internal_cmd( global_config.key_seq[seq][1] );
            else
            {
                dbgprint("pressed key %d\n", seq );
                push_seq(seq);
            }
        }
    }
}

void handle_mouseseq( uint8_t event, uint8_t mousedir )
{
    if ( !(mousedir & MOUSE_MOVE) )            
    {
        handle_keyseq( event,  mousedir );
    }
    else     
    {
        mousedir &= ~MOUSE_MOVE;           /* just the direction bit now */
        if ( event == EVENT_KEYPAD_DOWN )   
            global_mouse_dir |= mousedir;   /* set the mouse direction(s) when key is pressed  */
        else
            global_mouse_dir &= ~mousedir;  /* clear the mouse direction(s) when key is released */
    }
}

void handle_buttonseq( uint8_t event, uint8_t number )
{
    if ( number & MOUSE_MOVE )
    {
        handle_mouseseq( event, number );
    }
    else
    {
        if ( event == EVENT_KEY_DOWN )
        {
            uint8_t seq = global_config.button_map[number];
            if ( global_config.key_seq[seq][0] == INT_CMD )                
                run_internal_cmd( global_config.key_seq[seq][1] );
            else
            {
                dbgprint("pressed key %d\n", seq );
                push_seq(seq);
            }
        }
    }
}                   


void run_event(uint8_t event_type, uint16_t event_number )
{
    static uint8_t last_event_number = 0xff;
    static uint8_t last_event_type = 0xff;

    switch( event_type )
    {
        case EVENT_KEYPAD_UP:
        {
            uint8_t row = (uint8_t)((0xff00 & event_number) >> 8);
            uint8_t col = (uint8_t)(0x00ff & event_number);
            dbgprint("keypad up\n");
            if ( global_mouse_mode == KEY_SLOW_MODE )
            {
                uint8_t seq = global_config.key_map[row][col];
                handle_keyseq( EVENT_KEYPAD_UP, seq );
            }
            else if ( global_mouse_mode == KEY_FAST_MODE )
            {
                uint8_t seq = global_config.key_map2[row][col];
                handle_keyseq( EVENT_KEYPAD_UP, seq );
            }
            else    
            {
                uint8_t seq = global_config.mouse_map[row][col];
                handle_mouseseq( EVENT_KEYPAD_UP, seq );
            }
        }
        break;
        case EVENT_KEYPAD_DOWN:
        {
            uint8_t row = (uint8_t)((0xff00 & event_number) >> 8);
            uint8_t col = (uint8_t)(0x00ff & event_number);
            dbgprint("keypad down\n");
            if ( global_mouse_mode == KEY_SLOW_MODE )
            {
                uint8_t seq = global_config.key_map[row][col];
                handle_keyseq( EVENT_KEYPAD_DOWN, seq );
            }
            else if ( global_mouse_mode == KEY_FAST_MODE )
            {
                uint8_t seq = global_config.key_map2[row][col];
                handle_keyseq( EVENT_KEYPAD_DOWN, seq );
            }
            else    
            {
                uint8_t seq = global_config.mouse_map[row][col];
                handle_mouseseq( EVENT_KEYPAD_DOWN, seq );
            }
        }
        break;
        case EVENT_KEY_UP:
        {
            if ( (event_number == B_Z_IN) || (event_number == B_Z_OUT) )
            {
                dbgprint("%d up    \n", event_number );
                if ( last_event_type == event_type )
                {
                    if ( last_event_number != event_number )
                    {
                        handle_buttonseq( EVENT_KEY_DOWN, last_event_number );
                    }
                } 
                last_event_number = event_number;
                last_event_type = event_type;
            }
        }
        break;
        case EVENT_KEY_DOWN:
        {
            if ( (event_number == B_Z_IN) || (event_number == B_Z_OUT) )
            {
                dbgprint("%d down   \n", event_number );
                if ( last_event_type == event_type )
                {
                    if ( last_event_number != event_number )
                    {
                        handle_buttonseq( EVENT_KEY_DOWN, last_event_number );
                    }
                }
                last_event_number = event_number;
                last_event_type = event_type;
            }
            else handle_buttonseq( event_type, event_number );
        }
        break;
/*
            if ( (event_number == B_Z_IN) || (event_number == B_Z_OUT) )
            {
            dbgprint("z up %d     \n", event_number ); 
                if ( last_zoom_dir == 0xff )
                {
                    last_zoom_dir = event_number;
                }
                else
                {
                    if ( last_zoom_dir == B_Z_IN )
                        handle_keyseq( EVENT_KEYPAD_DOWN, global_config.button_map[ B_Z_IN ] );
                    else
                        handle_keyseq( EVENT_KEYPAD_DOWN, global_config.button_map[ B_Z_OUT ] );
                }
            }
            else if ( event_number == B_TOGGLE ) 
            {
            }
*/
        break;
     }
}


/*
 * millisecond timer. We just increment the 'ticks' global 
 */
void start_timer(void)
{ 
    // CTC mode, Clock/8
    TCCR1B |= (1 << WGM12) | (1 << CS11);
 
    // Load the high byte, then the low byte
    // into the output compare
    OCR1AH = (CTC_MATCH_OVERFLOW >> 4); /* 8 */
    OCR1AL = CTC_MATCH_OVERFLOW;
 
    // Enable the compare match interrupt
    TIMSK1 |= (1 << OCIE1A);
    
    // PC0/Analog 0 to Output
    DDRC |= (1 << PC0);

}

/* TODO - rewrite this with the global_ticks
*/



/*
 * keypads have rows and cols multiplexed, so to get multipile keys at once we 
 * cycle through rows one millisecond at a time, letting the signal settle for
 * one millisecond. The we trigger up/down events for each changed row/col
 */
 
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
            DDRF = 1<<cur_row;
            PORTF &= 0xf0;
            PORTF |= 1<<cur_row;
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
            cur_cols = (PINF & 0xf0) >> 4;
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

void init_eeprom(void)
{
    eeprom_header_t hdr;
    eeprom_read_block((void *)&hdr,NULL,sizeof(hdr));
    if ( hdr.version == 0xff )
    {
        reset_factory_default();   
    }
    else
    {
        eeprom_read_block((void *)&global_config,NULL,sizeof(global_config));
        dbgprint("read eeprom ver=%d\n", global_config.config.version );
    }
}


#define DEBOUNCE_COUNT 10
void poll_buttons(void)
{
    static uint32_t last = 0;
    static uint8_t  button_value[TOTAL_BUTTONS]={0};
    static uint8_t  button_active[TOTAL_BUTTONS]={0};
    uint8_t tmp;
    if ( last == global_ticks ) return;
    last = global_ticks;

    #define BUTTON( num, bit, ddr, pin ) \
    tmp = (pin & (1<<bit)); \
    if ( !button_active[num] )\
    {\
        button_active[num] = 1;\
        button_value[num] = tmp;\
    }\
    else\
    {\
        if (( button_value[num] == tmp ) && ( button_active[num] == 1 ))\
        {\
            button_active[num]=2; \
            uint8_t event = (tmp)?EVENT_KEY_UP:EVENT_KEY_DOWN;\
            run_event( event, (uint16_t)num ); \
        }\
        else\
        {\
            if ( tmp != button_value[num] )\
            {\
                button_active[num] = 0;\
            }\
        }\
    }\

    BUTTON_LIST
    #undef BUTTON
}



int main(void)
{

    start_timer();
	SetupHardware();
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();
    initDisplay();
    init_eeprom();
	fillScreen(Color565(0,0,0));
	setTextColor(Color565(255,255,255),Color565(00,00,00));
    FILE stream;
    FILE *fp = &stream;
    CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &stream);
    gfp = fp; 
#define MAX_LINE_SIZE 256 
    static char serial_input[MAX_LINE_SIZE+1];
    static uint16_t ix=0;
        
    #define BUTTON(e,bit,dd,pin) dd &= ~(1<<bit);        /* set all button pins to input */
    BUTTON_LIST 
    #undef BUTTON

    #define LED(e,bit,dd,port) dd |= (1<<bit); port &= ~(1<<bit); /* set all LEDs to output */
    LED_LIST
    #undef LED
    set_leds( global_mouse_mode+1);

    fprintf(fp, "Navputter starting Cap'n\n\r");
	for (;;)
	{
        lufa_main_loop();
        read_keypad();
        
        poll_buttons();
        char c=0;
  /* Must throw away unused bytes from the host, or it will lock up while waiting for the device */

		c = fgetc(fp);
        if ( c && (c != 0xff) ) 
        {
            fprintf( fp, "%c", c );
            serial_input[ix]=c;
            ix = (ix < MAX_LINE_SIZE ) ? ix+1 : ix;
            
            if ((c=='\n') || (c=='\r'))
            {
                if ( serial_input[0] == '$' )
                {
                    fflush(fp);
                    nmea_input( fp, serial_input );
                    serial_input[ix]=0; 
                    ix=0;
                }
                else
                {
                    if ( serial_input[0] >= 'a' && serial_input[0] <= 'z' )
                        cmd_input( fp, serial_input );
                }
                serial_input[ix]=0; 
                ix=0;
                dbgprint( ">%s<", serial_input);
            }
        }
    }
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
