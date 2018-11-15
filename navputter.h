#ifndef __NAVPUTTER_H__
#define __NAVPUTTER_H__ 

#include <stdlib.h>
#include <string.h>
#include "VirtualSerialMouse.h"
#include "uTFT_ST7735.h"
#include "myutil.h"
#include "xatoi.h"
#include "minmea.h"


void poll_buttons(void);
void run_event(uint8_t event_type, uint16_t event_number );
void reset_factory_default(void);
void start_timer(void);
void send_zoom_in( void );
void send_zoom_out( void );
uint16_t pop_key(void);
void handle_mouseseq( uint8_t event, uint8_t mousedir );
void handle_keyseq( uint8_t event, uint8_t seq );


enum events             /* main event type for do_event() */
{
    EVENT_NONE = 0,
    EVENT_KEY_DOWN,
    EVENT_KEY_UP,
    EVENT_KEYPAD_UP,
    EVENT_KEYPAD_DOWN
};

enum keypad_states      /* keypad states for reading each row */
{
    KP_STABALIZE,
    KP_WAIT,
    KP_READ_COLS,
    KP_REPORT
};

enum mouse_key_modes    /* global modes for slow keyboard / fast keyboard / mouse */
{
    KEY_SLOW_MODE=0,
    KEY_FAST_MODE,
    MOUSE_MODE
};


/*
 * these are non keypad buttons. These are processed in poll_buttons. 
 */

#define BUTTON_LIST\
    BUTTON( B_Z_IN,     DDRD, 0, PIND )    /* radial encoder zoom in rot */     \
    BUTTON( B_Z_OUT,    DDRD, 1, PIND )    /* radial encoder zoom out dir */    \
    BUTTON( B_TOGGLE,   DDRD, 2, PIND )    /* toggle button */                  \
    BUTTON( B_MISC,     DDRD, 3, PIND )    /* some other button */              

#define BUTTON( e, ddr, num, pin ) e,
enum button_numbers
{
    BUTTON_LIST
    TOTAL_BUTTONS
};
#undef BUTTON


/*
 * these are leds. I am lighting leds for the current state <slow key>=none, <fast key>=left, <mouse>=both
 */
#define LED_LIST\
    LED( LED_1,         DDRD, 4, PORTD )    /* led 1 */                         \
    LED( LED_2,         DDRD, 5, PORTD )    /* led 2 */                         \


#define LED( e, ddr, num, pin ) e,
enum led_numbers
{
    LED_LIST 
};
#undef BUTTON


#define INT_CMD 0xffff  /* not a key press but internal command from 2nd key seq */
enum internal_commands  /* this is the list of internal commands */
{
    IC_TOGGLE_MOUSE_KEYBOARD = 0,
};

enum mouse_events       /* these are the different mouse directions and clicks in global_mouse_dir */
{
    MOUSE_DIR_UP =      0x1,
    MOUSE_DIR_DOWN =    0x2,
    MOUSE_DIR_LEFT =    0x4,
    MOUSE_DIR_RIGHT =   0x8,
    MOUSE_LT_CLICK =    0x10,
    MOUSE_RT_CLICK =    0x20,
};

#define CTC_MATCH_OVERFLOW (uint8_t)(( (uint32_t)F_CPU / (uint32_t)1000) / (uint32_t)8)
#define MAX_KEY_BUFFER_SZ 20                    /* output key buffer size */

/* this list if for non keypad switches. TODO put DDR and BIT maros in here...*/
#define KEY_SEQ_LIST\
    KEY_SEQ( ZOOM_IN, HID_KEYBOARD_SC_EQUAL_AND_PLUS       ) \
    KEY_SEQ( ZOOM_OUT, HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE)\

#define KEY_SEQ( e, s ) e,
enum sequence_ids
{
    KEY_SEQ_LIST
};
#undef KEY_SEQ


/* this is the list of all serial commands available via the usb. Use minicom or similar and
 * send these commands. The format of this list is:
 *     CMD( enum name, command string, function to call, help string )
 */

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

#endif // __NAVPUTTER_H__

