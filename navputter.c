#include "VirtualSerialMouse.h"
#include "myutil.h"
#include "compat/minmea_compat_windows.h"

void poll_buttons(void);
void run_event(uint8_t event_type, uint8_t event_number );
void reset_factory_default(void);
void start_timer(void);

extern USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface;
#define MAX_KEYSTROKES 4
#define CTC_MATCH_OVERFLOW ((F_CPU / 1000) / 8)

#define USAGE_LIST\
    USAGE( "----------------------------------------" )\
    USAGE( "Navputter internal commands." ) \
    USAGE( "----------------------------------------" )\
    USAGE( "Command list:")\

#define CMD_LIST \
    CMD( CMD_DUMP, dump, cmd_dump, "dump all keys. Ex: dump" ) \
    CMD( CMD_SET, set, cmd_set, "Set one key. Ex: set 5 0x55 0x10, 0x55 0x00" ) \
    CMD( CMD_HELP, help, cmd_help, "Show this help" ) \

#define CMD(e, x, f, h) e,
enum {
    CMD_LIST
    CMD_LAST
};
#undef CMD

#define CMD( e,x,f,h ) #x,

char *cmd_strs[] = { CMD_LIST };
#undef CMD


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

 
volatile uint32_t ticks;
long milliseconds_since;
 
ISR (TIMER1_COMPA_vect)
{
    ticks++;
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


void reset_factory_default()
{
    dbgprint("reset to factory default\n");
#if 0
    #define KEY_SEQ(e,s) \
    {\
        key_sequences[e]=s;\
        dbgprint( "ks[%d]= %d\n", e, s);\
        eeprom_write_word( (void *)((char *)NULL + e), s );\
    }
    KEY_SEQ_LIST
    #undef KEY_SEQ 
#endif
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

enum keypad_states
{
    KP_START=0,
    KP_WAIT,
    KP_READ_ROWS,
    KP_READ_COLS
};


void send_zoom_in( void )
{
    dbgprint("zoom in");
}


void send_zoom_out( void )
{
    dbgprint("zoom out");
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


void cmd_help( FILE *fp, char *str )
{
#define USAGE(s) fprintf(fp, "%s\n\r", s );
USAGE_LIST
#undef USAGE

#define PAD_SZ 10
#define CMD(e, x, f, h) fprintf( fp, "cmd: %s    usage: %s\n\r",#x,h );
CMD_LIST
#undef CMD
fprintf( fp, "Enter Command:\n\r");
}

typedef struct eeprom_header
{
    uint8_t version;
    uint8_t rows;
    uint8_t cols;
    uint8_t unused;
}eeprom_header_t;

#define EEPROM_KEYMAP_BASE ((char *)NULL + sizeof( eeprom_header_t ))


eeprom_header_t global_config = { 0, 4, 4, 0 };         /* 4 x 4 by default */

//    eeprom_read_block( (void *)&eeprom_header, NULL, sizeof( eeprom_header_t ) );


void cmd_dump( FILE *fp, char *str )
{
    uint16_t *ptr = (uint16_t *)EEPROM_KEYMAP_BASE;
    fprintf(fp, "Navputter ver=%d, %dx%d keymap\n\r", global_config.version, global_config.rows, global_config.cols );
    fprintf(fp, "keymap dump:\n\r");

    uint16_t map[MAX_KEYSTROKES];
    uint8_t ix;
    uint8_t i;
    for ( i=0; i< global_config.cols * global_config.rows; i++ )
    {
        ix=0;
        map[ix++] = eeprom_read_word( ptr++ );
        map[ix++] = eeprom_read_word( ptr++ );
        map[ix++] = eeprom_read_word( ptr++ );
        map[ix++] = eeprom_read_word( ptr++ );
        fprintf( fp, "key %d : %x %x %x %x\n\r", i, map[0], map[1], map[2], map[3] );
    }
}

void cmd_set( FILE *fp, char *str )
{
    char *c = strchr( str, ' ' );
    if ( !c ) goto ERROR;
    uint8_t key_id = atoi( c+1 );
    fprintf( fp, "c+1 is %s, id is %d\n\r", c+1, key_id);
    c = strchr( c+1, ' ' );
    if ( !c ) goto ERROR;

    
    uint16_t *key_ptr = (uint16_t *)(EEPROM_KEYMAP_BASE + (key_id * MAX_KEYSTROKES * 2));
    uint8_t i;
    for ( i=0; i< MAX_KEYSTROKES; i++ )
    {
        uint16_t key = strtoul( c+1, &c, 16 );
        uint16_t mod = strtoul( c+1, &c, 16 );
        fprintf( fp, "writing key %d - %x %x %x (%p)\n\r", key_id, key, mod, (key<<8)|mod, key_ptr );
        eeprom_write_word( key_ptr++, (key << 8) | mod ); 
        c = strchr( c, ',' );
        if ( !c ) break;
    }
    return;
ERROR:
    fprintf( fp, "illagal format. No change.\n\r");
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



int main(void)
{
    start_timer();                                                  /* millisecond timer               */
	SetupHardware();                                                /* setup serial mouse and keyboard */
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);                          /* for keyboard leds (unused)      */
	GlobalInterruptEnable();                                        /* enable interrupts               */
    initDisplay();                                                  /* enable the st7735               */
	fillScreen(Color565(0,0,0));                                    /* clear the screen                */
	setTextColor(Color565(255,255,255),Color565(00,00,00));         /* set default text color (white)  */

    FILE stream;
    FILE *fp = &stream;
    CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &stream); /* open a file pointer to the serial */
   
#define MAX_LINE_SIZE 256                                           /* just a guess, how long is nmea anyhow */
    static char serial_input[MAX_LINE_SIZE+1];                      /* buffer to accumulate commands and nmea */
    static uint16_t ix=0;
	for (;;)
	{
        lufa_mainloop();                                            /* keep cranking lufa */
        read_keypad();                                              /* read the key state */
        char c=0;
		c = fgetc(fp);                                              /* pull a byte from the serial */

        if ( c && (c != 0xff) )                                     /* if there is a byte */
        {
            serial_input[ix]=c;
            ix = (ix < MAX_LINE_SIZE ) ? ix+1 : ix;
            
            if ((c=='\n') || (c=='\r'))                             /* read until eoln. Either one, skip the other */
            {
                if ( serial_input[0] == '$' )                       /* NMEA starts with a $ */
                {
                    nmea_input( fp, serial_input );
                }
                else
                {
                    if ( serial_input[0] >= 'a' && serial_input[0] <= 'z' ) /* filter trailing eoln */
                        cmd_input( fp, serial_input );
                }
                serial_input[ix]=0; 
                ix=0;
            }
        }
    }
}

