/*
             LUFA Library
     Copyright (C) Dean Camera, 2017.
     Copyright (C) Richard Hughes, 2017.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2017  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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
 *  Main source file for the Mouse DFU demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "Mouse.h"

/** Buffer to hold the previously generated Mouse HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevMouseHIDReportBuffer[sizeof(USB_MouseReport_Data_t)];

/** Current DFU state machine state, one of the values in the DFU_State_t enum. */
static uint8_t DFU_State = appIDLE;

/** Status code of the last executed DFU command. This is set to one of the values in the DFU_Status_t enum after
 *  each operation, and returned to the host when a Get Status DFU request is issued.
 */
static uint8_t DFU_Status = OK;

/** Flag to indicate if we should switch to bootloader mode when the USB stack
 *  is idle.
 */
static bool SwitchToBootloader = false;

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Mouse_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_Mouse,
				.ReportINEndpoint             =
					{
						.Address              = MOUSE_EPADDR,
						.Size                 = MOUSE_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevMouseHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevMouseHIDReportBuffer),
			},
	};


/** LUFA DFU Class driver interface configuration and state information.
 */
USB_ClassInfo_HID_Device_t DFU_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_DFU,
			},
	};

/** Pointer to the start of the bootloader.
 */
static AppPtr_t BootloaderPtr = (AppPtr_t)0x1e000;

/** Resets all configured hardware required for the bootloader back to their original states. */
static void ResetHardware(void)
{
	/* Shut down the USB and other board hardware drivers */
	USB_Disable();
	LEDs_Disable();

	/* Disable Bootloader active LED toggle timer */
	TIMSK1 = 0;
	TCCR1B = 0;

	/* Relocate the interrupt vector table back to the application section */
	MCUCR = (1 << IVCE);
	MCUCR = 0;
}

static void
RebootToBootloader(void)
{
	/* Reset configured hardware back to their original states for the user application */
	ResetHardware();

	/* Start the user application */
	BootloaderPtr();
}

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();

	for (;;)
	{
		HID_Device_USBTask(&Mouse_HID_Interface);
		USB_USBTask();
		if (SwitchToBootloader)
			RebootToBootloader();
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
	Buttons_Init();
	USB_Init();
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

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Mouse_HID_Interface);

	USB_Device_EnableSOFEvents();

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** DFU specific requests */
enum DfuRequest {
	DFU_REQ_DETACH			= 0x00,
	DFU_REQ_DNLOAD			= 0x01,
	DFU_REQ_UPLOAD			= 0x02,
	DFU_REQ_GETSTATUS		= 0x03,
	DFU_REQ_CLRSTATUS		= 0x04,
	DFU_REQ_GETSTATE		= 0x05,
	DFU_REQ_ABORT			= 0x06,
};

void DFU_Device_ProcessControlRequest(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo)
{
	if (USB_ControlRequest.wIndex != HIDInterfaceInfo->Config.InterfaceNumber)
	  return;

	/* Activity - toggle indicator LEDs */
	LEDs_ToggleLEDs(LEDS_LED1 | LEDS_LED2);

	switch (USB_ControlRequest.bRequest)
	{
		case DFU_REQ_DNLOAD:
		case DFU_REQ_UPLOAD:
			Endpoint_ClearSETUP();
			break;
		case DFU_REQ_GETSTATUS:
			Endpoint_ClearSETUP();

			while (!(Endpoint_IsINReady()))
			{
				if (USB_DeviceState == DEVICE_STATE_Unattached)
				  return;
			}

			/* Write 8-bit status value */
			Endpoint_Write_8(DFU_Status);

			/* Write 24-bit poll timeout value */
			Endpoint_Write_8(0);
			Endpoint_Write_16_LE(0);

			/* Write 8-bit state value */
			Endpoint_Write_8(DFU_State);

			/* Write 8-bit state string ID number */
			Endpoint_Write_8(0);

			Endpoint_ClearIN();

			Endpoint_ClearStatusStage();
			break;
		case DFU_REQ_CLRSTATUS:
			Endpoint_ClearSETUP();

			/* Reset the status value variable to the default OK status */
			DFU_Status = OK;

			Endpoint_ClearStatusStage();
			break;
		case DFU_REQ_GETSTATE:
			Endpoint_ClearSETUP();

			while (!(Endpoint_IsINReady()))
			{
				if (USB_DeviceState == DEVICE_STATE_Unattached)
				  return;
			}

			/* Write the current device state to the endpoint */
			Endpoint_Write_8(DFU_State);

			Endpoint_ClearIN();

			Endpoint_ClearStatusStage();
			break;
		case DFU_REQ_DETACH:
			Endpoint_ClearSETUP();

			/* Mark as currently detaching then do the reset idle */
			DFU_State = appDETACH;
			SwitchToBootloader = true;

			Endpoint_ClearStatusStage();
			break;
		case DFU_REQ_ABORT:
			Endpoint_ClearSETUP();

			/* Reset the current state variable to the default idle state */
			DFU_State = appIDLE;

			Endpoint_ClearStatusStage();
			break;
		default:
			break;
	}
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Mouse_HID_Interface);
	DFU_Device_ProcessControlRequest(&DFU_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Mouse_HID_Interface);
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
	USB_MouseReport_Data_t* MouseReport = (USB_MouseReport_Data_t*)ReportData;

	uint8_t JoyStatus_LCL    = Joystick_GetStatus();
	uint8_t ButtonStatus_LCL = Buttons_GetStatus();

	if (JoyStatus_LCL & JOY_UP)
	  MouseReport->Y = -1;
	else if (JoyStatus_LCL & JOY_DOWN)
	  MouseReport->Y =  1;

	if (JoyStatus_LCL & JOY_LEFT)
	  MouseReport->X = -1;
	else if (JoyStatus_LCL & JOY_RIGHT)
	  MouseReport->X =  1;

	if (JoyStatus_LCL & JOY_PRESS)
	  MouseReport->Button |= (1 << 0);

	if (ButtonStatus_LCL & BUTTONS_BUTTON1)
	  MouseReport->Button |= (1 << 1);

	*ReportSize = sizeof(USB_MouseReport_Data_t);
	return true;
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
	// Unused (but mandatory for the HID class driver) in this demo, since there are no Host->Device reports
}

