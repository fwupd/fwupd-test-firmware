/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2015 Richard Hughes <richard@hughsie.com>
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

#ifndef __CH_CONFIG_H
#define __CH_CONFIG_H

#include <xc.h>
#include <stdint.h>

/**
 * CHugConfig:
 *
 * A 64 byte config block shared between bootloader and all firmwares.
 *
 * If adding members to this struct then add them at the end and decrement the
 * padding -- this is to ensure the bootloader always restores enough data for
 * any firmware.
 *
 * Do not remove or re-order items in this struct. Think of it like an ABI.
 **/
typedef struct {
	uint32_t	 signing_key[4];
	uint32_t	 serial_number;
	uint16_t	 pcb_errata;
	uint8_t		 flash_success;
	int32_t		 wavelength_cal[4];
	uint8_t		 padding[17];
} CHugConfig;

uint8_t		 chug_config_read		(CHugConfig	*cfg);
uint8_t		 chug_config_write		(CHugConfig	*cfg);
uint8_t		 chug_config_has_signing_key	(CHugConfig	*cfg);
uint8_t		 chug_config_self_test		(void);

#endif /* __CH_CONFIG_H */
