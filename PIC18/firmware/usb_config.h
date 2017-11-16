/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2011-2015 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __USB_CONFIG_H
#define __USB_CONFIG_H

/* number of endpoint numbers besides endpoint zero */
#define NUM_ENDPOINT_NUMBERS		0

/* size of endpoint */
#define EP_0_LEN			8

/* only one USB config */
#define NUMBER_OF_CONFIGURATIONS	1

/* ping-pong buffering mode */
#define PPB_MODE PPB_ALL

/* no need to use usb_service() */
//#define USB_USE_INTERRUPTS

/* objects from usb_descriptors.c */
#define USB_DEVICE_DESCRIPTOR		chug_device_descriptor
#define USB_CONFIG_DESCRIPTOR_MAP	usb_application_config_descs
#define USB_STRING_DESCRIPTOR_FUNC	usb_application_get_string

/* optional callbacks */
#define UNKNOWN_SETUP_REQUEST_CALLBACK	chug_unknown_setup_request_callback
#define USB_RESET_CALLBACK		chug_usb_reset_callback

/* DFU configuration functions */
#define USB_DFU_USE_RUNTIME
#define DFU_TRANSFER_SIZE		64	/* bytes */
#define USB_DFU_SUCCESS_FUNC		chug_usb_dfu_set_success_callback

/* we expose CHUG _and_ DFU classes */
#define MULTI_CLASS_DEVICE

/* automatically send the descriptors to bind the WinUSB driver on Windows */
#define AUTOMATIC_WINUSB_SUPPORT
#define MICROSOFT_OS_DESC_VENDOR_CODE 0x50

#endif /* __USB_CONFIG_H */
