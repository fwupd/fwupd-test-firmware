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

#include <config.h>

#include "usb_config.h"
#include "usb.h"
#include "usb_ch9.h"
#include "usb_dfu.h"

#include "ch-config.h"

/* Configuration Packet */
struct configuration_1_packet {
	struct configuration_descriptor		config;
	struct interface_descriptor		interface;
	struct interface_descriptor		interface_dfu;
	struct dfu_functional_descriptor	dfu_runtime;
};

/* Device Descriptor */
const struct device_descriptor chug_device_descriptor =
{
	sizeof(struct device_descriptor),
	DESC_DEVICE,
	0x0200,					/* USB 2.0, 0x0110 = USB 1.1 */
	0x00,					/* Device class */
	0x00,					/* Device Subclass */
	0x00,					/* Protocol */
	EP_0_LEN,				/* bMaxPacketSize0 */
	0x273f,					/* VID */
	0x1009,					/* PID */
	FWVER_MAJOR * 0x100 + FWVER_MINOR,	/* firmware version */
	1,					/* Manufacturer string index */
	2,					/* Product string index */
	3,					/* Serial string index */
	NUMBER_OF_CONFIGURATIONS
};

#define CH_USB_INTERFACE_SUBCLASS			0x01
#define CH_USB_INTERFACE_PROTOCOL			0x01

/* Configuration Packet Instance */
static const struct configuration_1_packet configuration_1 =
{
	{
	/* Members from struct configuration_descriptor */
	sizeof(struct configuration_descriptor),
	DESC_CONFIGURATION,
	sizeof(configuration_1),
	0x02,					/* bNumInterfaces */
	0x01,					/* bConfigurationValue */
	0x02,					/* iConfiguration */
	0b10000000,
	100/2,					/* 100/2 indicates 100mA */
	},

	{
	/* CHUG Descriptor */
	sizeof(struct interface_descriptor),
	DESC_INTERFACE,
	0x00,					/* InterfaceNumber */
	0x00,					/* AlternateSetting */
	0x00,					/* bNumEndpoints (num besides endpoint 0) */
	DEVICE_CLASS_VENDOR_SPECIFIC,		/* bInterfaceClass */
	CH_USB_INTERFACE_SUBCLASS,		/* bInterfaceSubclass */
	CH_USB_INTERFACE_PROTOCOL,		/* bInterfaceProtocol */
	0x00,					/* iInterface */
	},

	{
	/* DFU Runtime Descriptor (runtime) */
	sizeof(struct interface_descriptor),
	DESC_INTERFACE,
	0x01,					/* InterfaceNumber */
	0x00,					/* AlternateSetting */
	0x00,					/* bNumEndpoints (num besides endpoint 0) */
	DFU_INTERFACE_CLASS,			/* bInterfaceClass */
	DFU_INTERFACE_SUBCLASS,			/* bInterfaceSubclass */
	DFU_INTERFACE_PROTOCOL_RUNTIME,		/* bInterfaceProtocol */
	0x00,					/* iInterface */
	},

	{
	/* DFU Functional Descriptor (runtime) */
	sizeof(struct dfu_functional_descriptor),
	DESC_DFU_FUNCTIONAL_DESCRIPTOR,		/* bDescriptorType */
	DFU_ATTRIBUTE_CAN_UPLOAD |
	DFU_ATTRIBUTE_CAN_DOWNLOAD |
	DFU_ATTRIBUTE_WILL_DETACH |
	DFU_ATTRIBUTE_MANIFESTATON_TOLERANT,	/* bmAttributes */
	0x00,					/* wDetachTimeOut (ms) */
	DFU_TRANSFER_SIZE,			/* wTransferSize */
	0x0110,					/* bcdDFUVersion */
	},

};

/* String Descriptors */
static const struct {uint8_t bLength;uint8_t bDescriptorType; uint16_t lang; } str00 = {
	sizeof(str00),
	DESC_STRING,
	0x0409					/* US English */
};
static const struct {uint8_t bLength;uint8_t bDescriptorType; uint16_t chars[12]; } vendor_string = {
	sizeof(vendor_string),
	DESC_STRING,
	{'H','u','g','h','s','k','i',' ','L','t','d','.'}
};
static const struct {uint8_t bLength;uint8_t bDescriptorType; uint16_t chars[8]; } product_string = {
	sizeof(product_string),
	DESC_STRING,
	{'C','o','l','o','r','H','u','g'}
};

/* this is good enough for a *lot* of devices :) */
#define SERIAL_MAX_LEN		8

/* Get String function */
int16_t usb_application_get_string(uint8_t string_number, const void **ptr)
{
	if (string_number == 0) {
		*ptr = &str00;
		return sizeof(str00);
	} else if (string_number == 1) {
		*ptr = &vendor_string;
		return sizeof(vendor_string);
	} else if (string_number == 2) {
		*ptr = &product_string;
		return sizeof(product_string);
	} else if (string_number == 3) {
		static struct {uint8_t bLength;uint8_t bDescriptorType; uint16_t chars[SERIAL_MAX_LEN]; } serial = {
			sizeof(serial),
			DESC_STRING,
			{'0','0','0','0','0','0','0','0'}
		};
		CHugConfig cfg;
		uint8_t i;
		uint32_t tmp;

		/* read a serial number from the config block */
		chug_config_read(&cfg);
		tmp = cfg.serial_number;

		/* poor mans asprintf */
		for (i = 0; i < SERIAL_MAX_LEN; i++) {
			serial.chars[SERIAL_MAX_LEN - (i + 1)] = '0' + tmp % 10;
			tmp /= 10;
		}

		*ptr = &serial;
		return sizeof(serial);
	}

	return -1;
}

/* Configuration Descriptor List
 */
const struct configuration_descriptor *usb_application_config_descs[] =
{
	(struct configuration_descriptor*) &configuration_1,
};
