#include "navputter.h"

volatile uint32_t ticks;
long milliseconds_since;
extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;

#define CTC_MATCH_OVERFLOW ((F_CPU / 1000) / 8)

#define KEY_SEQ_LIST\
    KEY_SEQ( ZOOM_IN, HID_KEYBOARD_SC_EQUAL_AND_PLUS       ) \
    KEY_SEQ( ZOOM_OUT, HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE)\

#define KEY_SEQ( e, s ) e,
enum sequence_ids
{
    KEY_SEQ_LIST
};
#undef KEY_SEQ

#define MAX_KEY_SEQUENCES 32        /* design for future flexibility */
uint16_t key_sequences[ MAX_KEY_SEQUENCES ]={1,0};

uint8_t buttons_pressed=0;

enum events
{
    EVENT_NONE = 0,
    EVENT_KEY_DOWN,
    EVENT_KEY_UP
};

enum keypad_states
{
    KP_START=0,
    KP_WAIT,
    KP_READ_ROWS,
    KP_READ_COLS
};



enum states
{
    STATE_MOUSE = 0x01,   /* buttons control mouse */
};

enum leds
{
    LED_1     = 0x00000001,
    LED_2     = 0x00000002
};

enum buttons 
{
    B_TOGGLE  = 0x00000001,
    B_LEFT    = 0x00000002,
    B_RIGHT   = 0x00000004,
    B_DOWN    = 0x00000008,
    B_UP      = 0x00000010,
    B_Z_IN    = 0x00000020,
    B_Z_OUT   = 0x00000040
};

typedef struct keypress
{
    uint8_t key;
    uint8_t modifier;
}keypress_t;

#define BUFFER_FULL 0xff
#define KEY_BUF_SIZE 10
keypress_t buf[ KEY_BUF_SIZE+1 ];
uint8_t key_head;
uint8_t key_tail;

int once=0;
 
ISR (TIMER1_COMPA_vect)
{
    ticks++;
}

void nmea_input( FILE *fp, char *input )
{
    dbgprint("nmea: %s\n\r", input );
}


#define USAGE_LIST\
    USAGE( "----------------------------------------" )\
    USAGE( "Navputter internal commands." ) \
    USAGE( "This interface is for setting eeprom." )\
    USAGE( "to send nmea prefix string with a $ char")\
    USAGE( "----------------------------------------" )\
    USAGE( "Command list:")\


#define CMD_LIST \
    CMD( CMD_DUMP, dump, cmd_dump, "Dump current configuration. Ex: dump" ) \
    CMD( CMD_SEQ,  seq,  cmd_seq,  "Set a key sequence. Ex: seq 5 0x55 0x10, 0x55 0x00" ) \
    CMD( CMD_MAP,  map,  cmd_map,  "Map a key to a sequence. Ex: map 1 5" ) \
    CMD( CMD_SAVE, save, cmd_save, "Save current configuration to eeprom. Ex: save" ) \
    CMD( CMD_HELP, help, cmd_help, "Show this help. Ex: help" ) \


#define CMD(e, x, f, h) e,
enum {
    CMD_LIST
    CMD_LAST
};
#undef CMD

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
    uint8_t         key_map[ MAX_KEY_ROWS * MAX_KEY_COLS ];  
    uint16_t        key_seq[ MAX_KEY_SEQ ][ MAX_KEYS_PER_SEQ ];
}eeprom_layout_t;


#define EEPROM_KEY_MAP ((eeprom_layout_t *)NULL)->key_map
#define EEPROM_KEY_SEQ ((eeprom_layout_t *)NULL)->key_seq

/*
    0  1  2  3 
    4  5  6  7
    8  9  10 11
    12 13 14 15


    1   up arrow            seq 0 
    9   down arrow          seq 1 
    4   left arrow          seq 2 
    6   right arrow         seq 3 
    14  zoom in             seq 4
    15  zoom out            seq 5 

*/

eeprom_layout_t global_config={
    {1,4,4,0},
    {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
    {
        { HID_KEYBOARD_SC_UP_ARROW << 8, 0,0,0 },                /* up arrow is seq 0 */
        { HID_KEYBOARD_SC_DOWN_ARROW << 8, 0,0,0 },              /* down arrow is seq 1 */
        { HID_KEYBOARD_SC_LEFT_ARROW << 8, 0,0,0 },
        { HID_KEYBOARD_SC_RIGHT_ARROW << 8, 0,0,0 },
        { HID_KEYBOARD_SC_RIGHT_ARROW << 8, 0,0,0 },
        { HID_KEYBOARD_SC_KEYPAD_PLUS_AND_MINUS << 8, 0,0,0 },
        { HID_KEYBOARD_SC_KEYPAD_PLUS_AND_MINUS << 8 | HID_KEYBOARD_MODIFIER_LEFTSHIFT, 0,0,0 },
    }
};

#define EEPROM_KEYMAP_BASE ((char *)NULL + sizeof( eeprom_header_t ))


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



//    eeprom_read_block( (void *)&eeprom_header, NULL, sizeof( eeprom_header_t ) );


void cmd_dump( FILE *fp, char *str )
{
    fprintf(fp, "Navputter ver=%d, %dx%d keymap\n\r", global_config.config.version, global_config.config.rows, global_config.config.cols );
    fprintf(fp, "keymap dump:\n\r");
    
    uint8_t c, r;
    for ( r=0; r< global_config.config.rows; r++ )
    {
        for ( c=0; c< global_config.config.cols; c++ )
        {
            fprintf(fp,"%2d ", global_config.key_map[ r * global_config.config.rows + c ] );
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
    uint8_t key = atoi(str);
    char *c = strchr( str, ' ' );
    if ( !c )
    {
        fprintf(fp, "illegal format. Nothing chagned. Ex: map 1 2\n\r");
        return;
    } 
    uint8_t seq = atoi(c+1);
    global_config.key_map[key]=seq;
    fprintf(fp, "successfully set key %d to seq %d\n\r", key, seq );
    return;
}


void cmd_seq( FILE *fp, char *str )
{
    char *c = strchr( str, ' ' );
    if ( !c ) goto ERROR;
    uint8_t key_id = atoi( c+1 );
    c = strchr( c+1, ' ' );
    if ( !c ) goto ERROR;

    
    uint16_t *key_ptr = ((eeprom_layout_t *)NULL)->key_seq[key_id];
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


void run_event(uint8_t event_type, uint8_t event_number )
{
    static uint8_t last_zoom_dir=0;
    switch( event_type )
    {
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


void start_timer(void)
{ 
    // CTC mode, Clock/8
    TCCR1B |= (1 << WGM12) | (1 << CS11);
 
    // Load the high byte, then the low byte
    // into the output compare
    OCR1AH = (CTC_MATCH_OVERFLOW >> 8);
    OCR1AL = CTC_MATCH_OVERFLOW;
 
    // Enable the compare match interrupt
    TIMSK1 |= (1 << OCIE1A);
    
    // PC0/Analog 0 to Output
    DDRC |= (1 << PC0);

}

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
                    run_event( EVENT_KEY_UP, 1<<i );
                else
                    run_event( EVENT_KEY_DOWN, 1<<i );
                counts[i]++;
                buttons_pressed = ~input;
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



#define DEBOUNCE_COUNT 10
#define GHOST_DELAY 2
void read_keypad(void)
{
    static uint8_t last_row=0;
    static uint8_t last_col=0;
    static uint8_t keypad_state = KP_START;
    static uint8_t row;
    static uint8_t col;
    static uint32_t until=0;
    static uint8_t  next_state=0xff;
    switch( keypad_state )
    {
        case KP_START:
            DDRF=0x0f;
            PORTF=0x0f;
            keypad_state = KP_WAIT;
            until = ticks+GHOST_DELAY;
            next_state = KP_READ_ROWS;

            if ( (row != last_row) || (col != last_col) )
            {
              last_row = row;
              last_col = col;
              dbgprint("%x-%x",row,col);
            }
            break;
        case KP_WAIT:
            if ( ticks < until ) return;
            keypad_state = next_state;
            break;
        case KP_READ_ROWS:
            row = PINF & 0xf0;
            keypad_state = KP_WAIT;
            until = ticks+2;
            DDRF = 0xf0;
            PORTF=0xf0;
            next_state = KP_READ_COLS;
            break;
        case KP_READ_COLS:
            col = PINF & 0x0f;
            keypad_state = KP_START;
            break;
        default:
            dbgprint("puke: %d\n",keypad_state);
            break;
    }
}

int main(void)
{

    start_timer();
	SetupHardware();
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();
    initDisplay();
	fillScreen(Color565(0,0,0));
	setTextColor(Color565(255,255,255),Color565(00,00,00));
    dbgprint("hello world"); 
    FILE stream;
    FILE *fp = &stream;
    CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &stream);
   
#define MAX_LINE_SIZE 256 
    static char serial_input[MAX_LINE_SIZE+1];
    static uint16_t ix=0;
	for (;;)
	{
        lufa_main_loop();
        read_keypad();
        char c=0;
		c = fgetc(fp);

        if ( c && (c != 0xff) ) 
        {
            serial_input[ix]=c;
            ix = (ix < MAX_LINE_SIZE ) ? ix+1 : ix;
            
            if ((c=='\n') || (c=='\r'))
            {
                fprintf( fp, "u entered %s", serial_input );
                if ( serial_input[0] == '$' )
                {
                    fprintf( fp, "it is nmea %s\n\r", serial_input );
                    nmea_input( fp, serial_input );
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
