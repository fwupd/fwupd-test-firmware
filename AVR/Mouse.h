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
 *  Header file for Mouse.c.
 */

#ifndef _MOUSE_H_
#define _MOUSE_H_

	/* Includes: */
		#include <avr/io.h>
		#include <avr/wdt.h>
		#include <avr/interrupt.h>
		#include <avr/power.h>
		#include <avr/interrupt.h>
		#include <stdbool.h>
		#include <string.h>

		#include "Descriptors.h"

		#include <LUFA/Drivers/Board/LEDs.h>
		#include <LUFA/Drivers/Board/Buttons.h>
		#include <LUFA/Drivers/USB/USB.h>
		#include <LUFA/Platform/Platform.h>

	/* Macros: */
		/** LED mask for the library LED driver, to indicate that the USB interface is not ready. */
		#define LEDMASK_USB_NOTREADY      LEDS_LED1

		/** LED mask for the library LED driver, to indicate that the USB interface is enumerating. */
		#define LEDMASK_USB_ENUMERATING  (LEDS_LED2 | LEDS_LED3)

		/** LED mask for the library LED driver, to indicate that the USB interface is ready. */
		#define LEDMASK_USB_READY        (LEDS_LED2 | LEDS_LED4)

		/** LED mask for the library LED driver, to indicate that an error has occurred in the USB interface. */
		#define LEDMASK_USB_ERROR        (LEDS_LED1 | LEDS_LED3)

	/* Type Defines: */
		/** Type define for a non-returning function pointer to the loaded application. */
		typedef void (*AppPtr_t)(void) ATTR_NO_RETURN;

	/* Enums: */
		/** DFU bootloader states. Refer to the DFU class specification for information on each state. */
		enum DFU_State_t
		{
			appIDLE                      = 0,
			appDETACH                    = 1,
			dfuIDLE                      = 2,
			dfuDNLOAD_SYNC               = 3,
			dfuDNBUSY                    = 4,
			dfuDNLOAD_IDLE               = 5,
			dfuMANIFEST_SYNC             = 6,
			dfuMANIFEST                  = 7,
			dfuMANIFEST_WAIT_RESET       = 8,
			dfuUPLOAD_IDLE               = 9,
			dfuERROR                     = 10
		};

		/** DFU command status error codes. Refer to the DFU class specification for information on each error code. */
		enum DFU_Status_t
		{
			OK                           = 0,
			errTARGET                    = 1,
			errFILE                      = 2,
			errWRITE                     = 3,
			errERASE                     = 4,
			errCHECK_ERASED              = 5,
			errPROG                      = 6,
			errVERIFY                    = 7,
			errADDRESS                   = 8,
			errNOTDONE                   = 9,
			errFIRMWARE                  = 10,
			errVENDOR                    = 11,
			errUSBR                      = 12,
			errPOR                       = 13,
			errUNKNOWN                   = 14,
			errSTALLEDPKT                = 15
		};

	/* Function Prototypes: */
		void SetupHardware(void);

		void EVENT_USB_Device_Connect(void);
		void EVENT_USB_Device_Disconnect(void);
		void EVENT_USB_Device_ConfigurationChanged(void);
		void EVENT_USB_Device_ControlRequest(void);
		void EVENT_USB_Device_StartOfFrame(void);

		bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
		                                         uint8_t* const ReportID,
		                                         const uint8_t ReportType,
		                                         void* ReportData,
		                                         uint16_t* const ReportSize);
		void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
		                                          const uint8_t ReportID,
		                                          const uint8_t ReportType,
		                                          const void* ReportData,
		                                          const uint16_t ReportSize);

#endif
