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
#include "usb_hid.h"
#include "usb_dfu.h"

#include "ch-config.h"

/* Configuration Packet */
struct configuration_1_packet {
	struct configuration_descriptor		config;
	struct interface_descriptor		interface_eeprom;
	struct dfu_functional_descriptor	dfu_eeprom;
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
	0x100a,					/* PID */
	FWVER_MAJOR * 0x100 + FWVER_MINOR,	/* firmware version */
	1,					/* Manufacturer string index */
	2,					/* Product string index */
	0,					/* Serial string index */
	NUMBER_OF_CONFIGURATIONS
};

/* Configuration Packet Instance */
static const struct configuration_1_packet configuration_1 =
{
	{
	/* Members from struct configuration_descriptor */
	sizeof(struct configuration_descriptor),
	DESC_CONFIGURATION,
	sizeof(configuration_1),
	0x01,					/* bNumInterfaces */
	0x01,					/* bConfigurationValue */
	0x00,					/* iConfiguration */
	0b10000000,
	150,					/* 300mA */
	},
	{
	/* DFU Runtime Descriptor (for EEPROM) */
	sizeof(struct interface_descriptor),
	DESC_INTERFACE,
	0x00,					/* InterfaceNumber */
	0x00,					/* AlternateSetting */
	0x00,					/* bNumEndpoints (num besides endpoint 0) */
	DFU_INTERFACE_CLASS,			/* bInterfaceClass */
	DFU_INTERFACE_SUBCLASS,			/* bInterfaceSubclass */
	DFU_INTERFACE_PROTOCOL_DFU,		/* bInterfaceProtocol */
	0x04,					/* iInterface */
	},
	{
	/* DFU Functional Descriptor (for EEPROM) */
	sizeof(struct dfu_functional_descriptor),
	DESC_DFU_FUNCTIONAL_DESCRIPTOR,		/* bDescriptorType */
	DFU_ATTRIBUTE_CAN_UPLOAD |
	DFU_ATTRIBUTE_CAN_DOWNLOAD |
	DFU_ATTRIBUTE_WILL_DETACH |
	DFU_ATTRIBUTE_MANIFESTATON_TOLERANT,	/* bmAttributes */
	0x64,					/* wDetachTimeOut (ms) */
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
static const struct {uint8_t bLength;uint8_t bDescriptorType; uint16_t chars[14]; } product_string = {
	sizeof(product_string),
	DESC_STRING,
	{'C','o','l','o','r','H','u','g',' ','[','D','F','U',']'}
};
static const struct {uint8_t bLength;uint8_t bDescriptorType; uint16_t chars[18]; } dfu_string = {
	sizeof(dfu_string),
	DESC_STRING,
	{'@','F','l','a','s','h',' ','/','0','x','0','/','1','*','1','6','K','e'}
};

static const struct {uint8_t bLength;uint8_t bDescriptorType; uint16_t chars[23]; } dfu_string_xtea = {
	sizeof(dfu_string_xtea),
	DESC_STRING,
	{'@','F','l','a','s','h','|','X','T','E','A',' ','/','0','x','0','/','1','*','1','6','K','e'}
};

/* Get String function */
int16_t
usb_application_get_string(uint8_t string_number, const void **ptr)
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
		/* FIXME: read a serial number out of EEPROM */
		return -1;
	} else if (string_number == 4) {
		CHugConfig cfg;

		/* is the device only accepting signed firmware */
		chug_config_read(&cfg);
		if (chug_config_has_signing_key(&cfg)) {
			*ptr = &dfu_string_xtea;
			return sizeof(dfu_string_xtea);
		} else {
			*ptr = &dfu_string;
			return sizeof(dfu_string);
		}
	}

	return -1;
}

/* Configuration Descriptor List
 */
const struct configuration_descriptor *usb_application_config_descs[] =
{
	(struct configuration_descriptor*) &configuration_1,
};
