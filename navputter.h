#include <stdlib.h>
#include <string.h>
#include "VirtualSerialMouse.h"
#include "uTFT_ST7735.h"
#include "myutil.h"
#include "xatoi.h"

void poll_buttons(void);
void run_event(uint8_t event_type, uint16_t event_number );
void reset_factory_default(void);
void start_timer(void);
void send_zoom_in( void );
void send_zoom_out( void );


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

enum buttons            /* support for misc buttons */
{
    B_TOGGLE  = 0x00000001,
    B_LEFT    = 0x00000002,
    B_RIGHT   = 0x00000004,
    B_DOWN    = 0x00000008,
    B_UP      = 0x00000010,
    B_Z_IN    = 0x00000020,
    B_Z_OUT   = 0x00000040
};

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
