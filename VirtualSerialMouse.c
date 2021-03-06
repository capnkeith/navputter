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
 *  Main source file for the VirtualSerialMouse demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "VirtualSerialMouse.h"

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */

extern uint8_t key_out;
extern uint8_t modifier_out;
extern uint8_t global_mouse_mode;
uint8_t global_mouse_dir;
#define MAX_KEY_BUFFER_SZ 8
uint16_t out_key_buffer[ MAX_KEY_BUFFER_SZ ];   /* key presses going out the USB */

                                                /* I don't know if volitle is required or not... */
volatile uint8_t out_key_head = 0;              /* head index for the keypress buffer */
volatile uint8_t out_key_tail = 0;              /* tail index for the keypress buffer */

extern void ser_print( const char *str, ... );
extern void ser_push( uint8_t c );
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

/*
 * simple circular buffer for holding keypresses until they can be transmitted
 * out the usb. Push the key and modifier. Key goes in high byte, modifiers in the low order byte.
 */
void push_key( uint16_t keypair )
{
    if ( out_key_head == MAX_KEY_BUFFER_SZ )
    {
        if ( out_key_tail == 0 )
        {
            return;  
        }
        else
        {
            out_key_buffer[out_key_head] = keypair;
            out_key_head = 0;
        }
    }
    else
    {
        if ( out_key_head + 1 == out_key_tail )
        {
            return;
        }
        else
        {
            out_key_buffer[out_key_head] = keypair;
            out_key_head = ( out_key_head == MAX_KEY_BUFFER_SZ ) ? 0 : out_key_head+1;
        }
    }
}



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




USB_ClassInfo_CDC_Device_t *get_serial_cdc_interface(void)
{
    return &VirtualSerial_CDC_Interface;
} 





/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
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
	USB_Init();
}


/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;
	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Device_HID_Interface);
	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
	USB_Device_EnableSOFEvents();
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


enum report_states
{
    SEND_KEY = 0,
    CLEAR_KEY
};

uint8_t key_state = SEND_KEY;


void get_mouse_status( int8_t *y, int8_t *x, uint8_t *buttons );

bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
    USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;
    if ( key_state == SEND_KEY )
    {
        uint16_t keypair = pop_key();
        key_state = CLEAR_KEY;
        if ( keypair )
        {
            uint8_t mod = 0;
            uint8_t key = 0;
            CONVERT_KEYPAIR( keypair, key, mod );
            KeyboardReport->Modifier =mod;
		    KeyboardReport->KeyCode[0] = key;
        }
        else
        { 
            goto PROCESS_MOUSE;    
        }
    }
    else
    {
        KeyboardReport->Modifier = 0;
		KeyboardReport->KeyCode[0] = 0;
        key_state = SEND_KEY;
    }
	*ReportID   = HID_REPORTID_KeyboardReport;
	*ReportSize = sizeof(USB_KeyboardReport_Data_t);
    return false;

PROCESS_MOUSE:
    {
		USB_MouseReport_Data_t* MouseReport = (USB_MouseReport_Data_t*)ReportData;
        get_mouse_status( &MouseReport->Y, &MouseReport->X, &MouseReport->Button );        
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

void lufa_main_loop(void)
{
	CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
    uint8_t c = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
    if ( c != 0xff )
    {
        ser_push(c);
    } 
    HID_Device_USBTask(&Device_HID_Interface);
    USB_USBTask();
}
