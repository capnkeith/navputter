/*
             LUFA Library
     Copyright (C) Dean Camera, 2018.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2018  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the VirtualSerialMouse demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "VirtualSerialMouse.h"
#include "myutil.h"

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
static uint8_t PrevHIDReportBuffer[MAX(sizeof(USB_KeyboardReport_Data_t), sizeof(USB_MouseReport_Data_t))];
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber         = INTERFACE_ID_CDC_CCI,
				.DataINEndpoint                 =
					{
						.Address                = CDC_TX_EPADDR,
						.Size                   = CDC_TXRX_EPSIZE,
						.Banks                  = 1,
					},
				.DataOUTEndpoint                =
					{
						.Address                = CDC_RX_EPADDR,
						.Size                   = CDC_TXRX_EPSIZE,
						.Banks                  = 1,
					},
				.NotificationEndpoint           =
					{
						.Address                = CDC_NOTIFICATION_EPADDR,
						.Size                   = CDC_NOTIFICATION_EPSIZE,
						.Banks                  = 1,
					},
			},
	};

USB_ClassInfo_HID_Device_t Device_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_KeyboardMouse,
				.ReportINEndpoint             =
					{
						.Address              = HID_IN_EPADDR,
						.Size                 = HID_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevHIDReportBuffer),
			},
	};

void poll_buttons(void);
void run_event(uint8_t event_type, uint8_t event_number );
void reset_factory_default(void);
void start_timer(void);


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

    #define KEY_SEQ(e,s) \
    {\
        key_sequences[e]=s;\
        dbgprint( "ks[%d]= %d\n", e, s);\
        eeprom_write_word( (void *)((char *)NULL + e), s );\
    }
    KEY_SEQ_LIST
    #undef KEY_SEQ 

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
    CMD( CMD_DUMP, dump, cmd_dump, "dump all keys. Ex: dump" ) \
    CMD( CMD_GET, get, cmd_get, "get one key. Ex: get 1    =>    0x33 0x0, 0x33 0x10" )\
    CMD( CMD_SET, set, cmd_set, "Set one key. Ex: set 5 0x55 0x10, 0x55 0x00" ) \
    CMD( CMD_EDIT, edit, cmd_edit, "Set any number of keys. Ex : edit 3 0x55 0x0.0x55 0x10,0x55 0.0x55 0x20" ) \
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
    fprintf(fp, "dump command\n\r");
}

void cmd_get( FILE *fp, char *str )
{
    fprintf(fp, "get command\n\r");
}

void cmd_set( FILE *fp, char *str )
{
    fprintf(fp, "set command\n\r");
}

void cmd_edit( FILE *fp, char *str )
{
    fprintf(fp, "edit command\n\r");
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


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
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
		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		HID_Device_USBTask(&Device_HID_Interface);
		USB_USBTask();
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
                    fprintf( fp, "it is nmea\n\r", serial_input );
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
void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
	/* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

	/* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

	/* Hardware Initialization */
	Joystick_Init();
	LEDs_Init();
	USB_Init();
}

/** Checks for changes in the position of the board joystick, sending strings to the host upon each change. */
void CheckJoystickMovement(void)
{
	uint8_t     JoyStatus_LCL = Joystick_GetStatus();
	char*       ReportString  = NULL;
	static bool ActionSent    = false;

	if (JoyStatus_LCL & JOY_UP)
	  ReportString = "Joystick Up\r\n";
	else if (JoyStatus_LCL & JOY_DOWN)
	  ReportString = "Joystick Down\r\n";
	else if (JoyStatus_LCL & JOY_LEFT)
	  ReportString = "Joystick Left\r\n";
	else if (JoyStatus_LCL & JOY_RIGHT)
	  ReportString = "Joystick Right\r\n";
	else if (JoyStatus_LCL & JOY_PRESS)
	  ReportString = "Joystick Pressed\r\n";
	else
	  ActionSent = false;

	if ((ReportString != NULL) && (ActionSent == false))
	{
		ActionSent = true;

		CDC_Device_SendString(&VirtualSerial_CDC_Interface, ReportString);
	}
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;
	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Device_HID_Interface);
	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
	USB_Device_EnableSOFEvents();
	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
	HID_Device_ProcessControlRequest(&Device_HID_Interface);
}


/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
//	uint8_t JoyStatus_LCL    = Joystick_GetStatus();
//	uint8_t ButtonStatus_LCL = Buttons_GetStatus();

    
//	if (!(ButtonStatus_LCL & BUTTONS_BUTTON1))
	if (1)
	{

	//	USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;
/*
        if ( press == 0 )
        {

//		KeyboardReport->Modifier = HID_KEYBOARD_MODIFIER_LEFTSHIFT;

//		if (JoyStatus_LCL & JOY_UP)
		  KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_A;
		else if (JoyStatus_LCL & JOY_DOWN)
		  KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_B;

		if (JoyStatus_LCL & JOY_LEFT)
		  KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_C;
		else if (JoyStatus_LCL & JOY_RIGHT)
		  KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_D;

		if (JoyStatus_LCL & JOY_PRESS)
		  KeyboardReport->KeyCode[0] = HID_KEYBOARD_SC_E;

		*ReportID   = HID_REPORTID_KeyboardReport;
		*ReportSize = sizeof(USB_KeyboardReport_Data_t);
        press = 0xfff;
        }
        else
        {
            press--;
            KeyboardReport->Modifier=0;
        }
*/
		return false;
	}
	else
	{
		USB_MouseReport_Data_t* MouseReport = (USB_MouseReport_Data_t*)ReportData;

//		if (JoyStatus_LCL & JOY_UP)

		  MouseReport->Y = -1;
/*
		else if (JoyStatus_LCL & JOY_DOWN)
		  MouseReport->Y =  1;

		if (JoyStatus_LCL & JOY_LEFT)
		  MouseReport->X = -1;
		else if (JoyStatus_LCL & JOY_RIGHT)
		  MouseReport->X =  1;

		if (JoyStatus_LCL & JOY_PRESS)
		  MouseReport->Button |= (1 << 0);
*/

		*ReportID   = HID_REPORTID_MouseReport;
		*ReportSize = sizeof(USB_MouseReport_Data_t);
		return true;
	}
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	uint8_t  LEDMask   = LEDS_NO_LEDS;
	uint8_t* LEDReport = (uint8_t*)ReportData;

	if (*LEDReport & HID_KEYBOARD_LED_NUMLOCK)
	  LEDMask |= LEDS_LED1;

	if (*LEDReport & HID_KEYBOARD_LED_CAPSLOCK)
	  LEDMask |= LEDS_LED3;

	if (*LEDReport & HID_KEYBOARD_LED_SCROLLLOCK)
	  LEDMask |= LEDS_LED4;

	LEDs_SetAllLEDs(LEDMask);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Device_HID_Interface);
}


/** CDC class driver callback function the processing of changes to the virtual
 *  control lines sent from the host..
 *
 *  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
 */
void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t *const CDCInterfaceInfo)
{
	/* You can get changes to the virtual CDC lines in this callback; a common
	   use-case is to use the Data Terminal Ready (DTR) flag to enable and
	   disable CDC communications in your application when set to avoid the
	   application blocking while waiting for a host to become ready and read
	   in the pending data from the USB endpoints.
	*/
	bool HostReady = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR) != 0;

	(void)HostReady;
}


