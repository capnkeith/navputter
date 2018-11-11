
/* 
******************************************************************************
* Navputer - like a putter for golf except for your sailboat.
******************************************************************************
* This is an original work by Seth Keth offered for free with no licese 
* whatsoever or if you prefer you can stick a WTFPL v2 on it. 
*/

#include "navputter.h"

/* this is the serial interface. */
extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;


uint16_t out_key_buffer[ MAX_KEY_BUFFER_SZ ];   /* key presses going out the USB */
uint8_t out_key_head = 0;                       /* head index for the keypress buffer */
uint8_t out_key_tail = 0;                       /* tail index for the keypress buffer */
volatile uint32_t global_ticks=0;               /* milliseconds since boot */
uint8_t global_mouse_dir=0;                     /* bitmask of all current mouse directions and clicks */
uint8_t global_mouse_mode = KEY_SLOW_MODE;      /* current mode ( slow key, fast key, mouse ) */
FILE *gfp=NULL;

/* 
 * this is our timer interrupt service routine. This is a real interrupt service routine, so
 * don't try and do much in here. global_ticks is milliseconds since power on
 */

ISR (TIMER1_COMPA_vect)
{
    global_ticks++;
}


/*
 * simple circular buffer for holding keypresses until they can be transmitted
 * out the usb. Push the key and modifier. Key goes in high byte, mods low order.
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
    {
        return 0;
    }
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
    uint8_t         key_map[ MAX_KEY_ROWS][ MAX_KEY_COLS ];  
    uint8_t         key_map2[ MAX_KEY_ROWS][ MAX_KEY_COLS ];  
    uint8_t         mouse_map[ MAX_KEY_ROWS][ MAX_KEY_COLS ];  
    uint16_t        key_seq[ MAX_KEY_SEQ ][ MAX_KEYS_PER_SEQ ];
}eeprom_layout_t;


#define EEPROM_KEY_MAP ((eeprom_layout_t *)NULL)->key_map
#define EEPROM_KEY_SEQ ((eeprom_layout_t *)NULL)->key_seq

/*
 * This is the factory defaul settting for key mappings. If you flash eeprom will get
 * written to this value, or you can set the version ( eeprom location 0 ) to 0xff and
 * eeprom will get factory rest on the next power up 
 */

eeprom_layout_t global_config={
    {1,4,4,0},              /* version 1, 4 rows, 4 cols  */
    {                       /* key map 1 ( slow map ) */
        {0,1,2,3},
        {4,5,6,7},
        {8,9,10,11},
        {12,13,14,15}
    },                      /* key map 2 ( fast map ) */
    {
        {0,16,2,3},
        {17,5,18,7},
        {8,19,10,11},
        {12,13,14,15}
    },      
    {                       /* mouse map */
        { 0, MOUSE_DIR_UP, 0, 0 }, 
        { MOUSE_DIR_LEFT, 0, MOUSE_DIR_RIGHT, 0 }, 
        { 0, MOUSE_DIR_DOWN, 0, 0 },
        { MOUSE_LT_CLICK, MOUSE_RT_CLICK, 0, 0 }
    },             
    {                       /* key sequences */
        { INT_CMD, IC_TOGGLE_MOUSE_KEYBOARD, 0, 0 },            /* key 0 is key slow / key fast / mouse toggle */
        { HID_KEYBOARD_SC_UP_ARROW << 8, 0,0,0 },               /* up arrow key */
		{ HID_KEYBOARD_SC_A << 8, 0, 0, 0 },                    /* 'a' */
		{ HID_KEYBOARD_SC_A << 8, 0, 0, 0 },                    /* 'a' */

        { HID_KEYBOARD_SC_LEFT_ARROW << 8, 0,0,0 },             /* right arrow key */
		{ HID_KEYBOARD_SC_A << 8, 0, 0, 0 },                    /* 'a' */
        { HID_KEYBOARD_SC_RIGHT_ARROW << 8, 0,0,0 },            /* left arrow key */
		{ HID_KEYBOARD_SC_A << 8, 0, 0, 0 },                    /* 'a' */

		{ HID_KEYBOARD_SC_A << 8, 0, 0, 0 },                    /* 'a' */
        { HID_KEYBOARD_SC_DOWN_ARROW << 8, 0,0,0 },             /* down arrow key */
		{ HID_KEYBOARD_SC_A << 8, 0, 0, 0 },                    /* 'a' */
		{ HID_KEYBOARD_SC_A << 8, 0, 0, 0 },                    /* 'a' */

		{ HID_KEYBOARD_SC_KEYPAD_PLUS_AND_MINUS << 8, 0, 0, 0 },/* 'a' */
		{ HID_KEYBOARD_SC_KEYPAD_PLUS_AND_MINUS << 8 | HID_KEYBOARD_MODIFIER_LEFTSHIFT, 0, 0, 0 }, /* 'a' */
		{ HID_KEYBOARD_SC_A << 8, 0, 0, 0 },                    /* 'a' */
		{ HID_KEYBOARD_SC_A << 8, 0, 0, 0 },                    /* 'a' */

        { HID_KEYBOARD_SC_UP_ARROW << 8 | HID_KEYBOARD_MODIFIER_LEFTSHIFT, 0,0,0 },       /* speed key up arrow seq */
        { HID_KEYBOARD_SC_LEFT_ARROW << 8 | HID_KEYBOARD_MODIFIER_LEFTSHIFT, 0,0,0 },     /* speed key left seq */
        { HID_KEYBOARD_SC_RIGHT_ARROW << 8 | HID_KEYBOARD_MODIFIER_LEFTSHIFT, 0,0,0 },    /* speed key right seq */
        { HID_KEYBOARD_SC_DOWN_ARROW << 8 | HID_KEYBOARD_MODIFIER_LEFTSHIFT, 0,0,0 },     /* speed key down seq */
    }
};


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
    fprintf(fp, "illegal format. Nothing chagned. Ex: map 1 2 2\n\r");
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
    fprintf( fp, "illegal format. No change.\n\r");
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
void run_internal_cmd( uint16_t cmd )
{
    switch(cmd)
    {
        case IC_TOGGLE_MOUSE_KEYBOARD:
            global_mouse_mode = ( global_mouse_mode < MOUSE_MODE ) ? global_mouse_mode+1 : KEY_SLOW_MODE;
            global_mouse_dir = 0;
            dbgprint( "mouse mode %d\n", global_mouse_mode );
            break;
        default:
            dbgprint("unkown cmd %d\n", cmd );
            break;
    }
}


void run_event(uint8_t event_type, uint16_t event_number )
{
    static uint8_t last_zoom_dir=0;
    switch( event_type )
    {
        case EVENT_KEYPAD_UP:
        {
            uint8_t row = (uint8_t)((0xff00 & event_number) >> 8);
            uint8_t col = (uint8_t)(0x00ff & event_number);
            uint8_t seq = global_config.key_map[row][col];

            if ( global_config.key_seq[ seq ][0] == INT_CMD )
            {
                run_internal_cmd( global_config.key_seq[seq][1] );
            }
            global_mouse_dir &= ~global_config.mouse_map[row][col];
        }
        break;
        case EVENT_KEYPAD_DOWN:
        {
            uint8_t row = (uint8_t)((0xff00 & event_number) >> 8);
            uint8_t col = (uint8_t)(0x00ff & event_number);
            uint8_t seq = global_config.key_map[row][col];
            if ( global_config.key_seq[ seq ][ 0 ] )
            {
                uint8_t key_out = (global_config.key_seq[ seq ][0] & 0xff00 ) >> 8;
                uint8_t mod_out = (global_config.key_seq[ seq ][0] & 0x00ff );
                mod_out = (global_config.key_seq[seq][0] & 0x00ff );
                dbgprint("seq %d, key %x mod %x\n", seq, key_out, mod_out );
                push_key( key_out, mod_out );
            }
            else
            {
                dbgprint("seq %d no key\n", seq );
            }
            global_mouse_dir |= global_config.mouse_map[row][col];
        }
        break;
        case EVENT_KEY_UP:
            if ( event_number & (B_Z_IN|B_Z_OUT) )
                last_zoom_dir = 0;
        break;
        case EVENT_KEY_DOWN:
            if ( event_number & (B_Z_IN|B_Z_OUT) )
            {
                if ( !last_zoom_dir )
                    last_zoom_dir = event_number;
                else
                {
                    if ( last_zoom_dir == B_Z_IN )
                        send_zoom_in();
                    else
                        send_zoom_out();
                }
            }
            else if ( event_number == B_TOGGLE ) 
            {
            }
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

#define DEBOUNCE_COUNT 10
void poll_buttons(void)
{
    static uint8_t last_input=0;
    uint8_t input = PIND & 0x7f;
    uint8_t i;
    static uint8_t counts[8] = {0};
    for ( i=0; i<8; i++ )
    {
        if ( ( input & (1<<i) ) == ( last_input & (1<<i) ) )
        {
            if ( counts[i] == DEBOUNCE_COUNT )
            {
                if ( input & (1<<i) )
                    run_event( EVENT_KEY_UP, (uint16_t)1<<i );
                else
                    run_event( EVENT_KEY_DOWN, (uint16_t)1<<i );
                counts[i]++;
            }
            else if ( counts[i] < DEBOUNCE_COUNT ) counts[i]++;
        }
        else
        {
            counts[i]=0;
            last_input &= ~(1<<i);
            last_input |= input & (1<<i);
        }
    }
}

void send_zoom_in( void )
{
    dbgprint("zoom in");
}


void send_zoom_out( void )
{
    dbgprint("zoom out");
}


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
    dbgprint("hello world"); 
    FILE stream;
    FILE *fp = &stream;
    CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &stream);
    gfp = fp; 
#define MAX_LINE_SIZE 256 
    static char serial_input[MAX_LINE_SIZE+1];
    static uint16_t ix=0;
        

    fprintf(fp, "Navputter starting Cap'n\n\r");
    uint32_t start = global_ticks;
	for (;;)
	{
        lufa_main_loop();
        read_keypad();
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
