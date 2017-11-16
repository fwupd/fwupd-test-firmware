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

#include <string.h>

#include "ch-config.h"
#include "ch-errno.h"
#include "ch-flash.h"

#define CH_CONFIG_ADDRESS_WRDS		0x5c00

uint8_t
chug_config_read(CHugConfig *cfg)
{
	uint8_t rc;

	/* read config block */
	rc = chug_flash_read(CH_CONFIG_ADDRESS_WRDS,
			     (uint8_t *) cfg,
			     sizeof(CHugConfig));
	if (rc != CH_ERROR_NONE)
		return rc;

	/* no config block, so set to defaults */
	if (cfg->flash_success == 0xff)
		memset(cfg, 0x00, sizeof(CHugConfig));
	return CH_ERROR_NONE;
}

uint8_t
chug_config_write(CHugConfig *cfg)
{
	uint8_t rc;

	/* erase config block (and some extra) then write */
	rc = chug_flash_erase(CH_CONFIG_ADDRESS_WRDS, CH_FLASH_ERASE_BLOCK_SIZE);
	if (rc != CH_ERROR_NONE)
		return rc;
	return chug_flash_write(CH_CONFIG_ADDRESS_WRDS, (uint8_t *) cfg, sizeof(CHugConfig));
}

uint8_t
chug_config_has_signing_key(CHugConfig *cfg)
{
	if (cfg->signing_key[0] != 0 ||
	    cfg->signing_key[1] != 0 ||
	    cfg->signing_key[2] != 0 ||
	    cfg->signing_key[3] != 0)
		return TRUE;
	return FALSE;
}

uint8_t
chug_config_self_test (void)
{
	CHugConfig cfg;

	/* write dummy data */
	cfg.serial_number = 0xdeadbeef;
	cfg.pcb_errata = 0x1234;
	cfg.flash_success = 0x56;
	chug_config_write(&cfg);

	/* set it to zero */
	memset(&cfg, 0x00, sizeof(CHugConfig));

	/* read it back */
	chug_config_read(&cfg);
	if (cfg.serial_number != 0xdeadbeef)
		return -1;
	if (cfg.pcb_errata != 0x1234)
		return -1;
	if (cfg.flash_success != 0x56)
		return -1;
	return 0;
}
