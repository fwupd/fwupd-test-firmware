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

#include "ch-flash.h"
#include "ch-errno.h"

static void
chug_flash_load_table_at_addr(uint32_t addr)
{
	DWORD_VAL tmp;
	tmp.Val = addr;
	TBLPTRU = tmp.byte.UB;
	TBLPTRH = tmp.byte.HB;
	TBLPTRL = tmp.byte.LB;
}

uint8_t
chug_flash_erase(uint16_t addr, uint16_t len)
{
	uint16_t i;
	uint8_t enable_int = FALSE;

	/* check this is aligned */
	if (addr % CH_FLASH_ERASE_BLOCK_SIZE > 0)
		return CH_ERROR_INVALID_ADDRESS;

	/* disable interrupts if set */
	if (INTCONbits.GIE) {
		INTCONbits.GIE = 0;
		enable_int = TRUE;
	}

	/* erase in chunks */
	for (i = addr; i < addr + len; i += CH_FLASH_ERASE_BLOCK_SIZE) {
		chug_flash_load_table_at_addr(i);
		EECON1bits.WREN = 1;
		EECON1bits.FREE = 1;
		EECON2 = 0x55;
		EECON2 = 0xAA;
		EECON1bits.WR = 1;
	}

	/* re-enable interrupts */
	if (enable_int)
		INTCONbits.GIE = 1;
	return CH_ERROR_NONE;
}


uint8_t
chug_flash_write(uint16_t addr, const uint8_t *data, uint16_t len)
{
	uint16_t cnt = 0;
	uint16_t i;
	uint8_t enable_int = FALSE;

	/* check this is aligned */
	if (addr % CH_FLASH_WRITE_BLOCK_SIZE > 0)
		return CH_ERROR_INVALID_ADDRESS;

	/* disable interrupts if set */
	if (INTCONbits.GIE) {
		INTCONbits.GIE = 0;
		enable_int = TRUE;
	}

	/* write in chunks */
	for (i = 0; i < len; i += CH_FLASH_WRITE_BLOCK_SIZE) {
		chug_flash_load_table_at_addr(addr + i);
		for (cnt = 0; cnt < CH_FLASH_WRITE_BLOCK_SIZE; cnt++) {
			/* don't read past the small buffer */
			if (cnt + i < len)
				TABLAT = *data++;
			else
				TABLAT = 0xff;
			asm("TBLWTPOSTINC");
		}
		chug_flash_load_table_at_addr(addr + i);
		EECON1bits.WREN = 1;
		EECON2 = 0x55;
		EECON2 = 0xAA;
		EECON1bits.WR = 1;
		EECON1bits.WREN = 0;
	}

	/* re-enable interrupts */
	if (enable_int)
		INTCONbits.GIE = 1;
	return CH_ERROR_NONE;
}

uint8_t
chug_flash_read(uint16_t addr, uint8_t *data, uint16_t len)
{
	chug_flash_load_table_at_addr(addr);
	while (len--) {
		asm("TBLRDPOSTINC");
		*data++ = TABLAT;
	}
	return CH_ERROR_NONE;
}
