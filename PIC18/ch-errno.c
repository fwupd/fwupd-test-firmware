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

#include <stdint.h>

#include "ch-errno.h"

static void
_led_delay(void)
{
	uint32_t i = 0x0;
	for (i = 0; i < 0xffff; i++)
		CLRWDT();
}

void
chug_errno_show(ChError errno, uint8_t is_fatal)
{
	uint8_t i;
	PORTE = 0;
	do {
		for (i = 0; i < 10; i++)
			_led_delay();
		for (i = 0; i < errno; i++) {
			_led_delay();
			_led_delay();
			PORTE = CH_STATUS_LED_RED;
			_led_delay();
			_led_delay();
			PORTE = 0;
		}
	} while (is_fatal);
}
