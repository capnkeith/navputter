
#include "VirtualSerialMouse.h"
#include <string.h>
#include "uTFT_ST7735.h"
#include "myutil.h"
#include "xatoi.h"

void poll_buttons(void);
void run_event(uint8_t event_type, uint8_t event_number );
void reset_factory_default(void);
void start_timer(void);
void send_zoom_in( void );
void send_zoom_out( void );
