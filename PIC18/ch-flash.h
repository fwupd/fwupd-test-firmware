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

#ifndef __CH_FLASH_H
#define __CH_FLASH_H

#include <xc.h>
#include <stdint.h>

/* flash constants */
#define	CH_FLASH_ERASE_BLOCK_SIZE		0x400	/* 1024 bytes */
#define	CH_FLASH_WRITE_BLOCK_SIZE		0x040	/* 64 bytes */

uint8_t		 chug_flash_erase	(uint16_t	 addr,
					 uint16_t	 len);

uint8_t		 chug_flash_write	(uint16_t	 addr,
					 const uint8_t	*data,
					 uint16_t	 len);

uint8_t		 chug_flash_read	(uint16_t	 addr,
					 uint8_t	*data,
					 uint16_t	 len);

#endif /* __CH_FLASH_H */
