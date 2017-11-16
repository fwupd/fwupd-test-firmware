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

#include <xc.h>
#include <string.h>

#include "usb_ch9.h"
#include "usb_config.h"
#include "usb_dfu.h"
#include "usb.h"

#include "ch-config.h"
#include "ch-errno.h"
#include "ch-flash.h"

#pragma config XINST	= OFF		/* turn off extended instruction set */
#pragma config STVREN	= ON		/* Stack overflow reset */
#pragma config PLLDIV	= 3		/* (12 MHz crystal used on this board) */
#pragma config WDTEN	= ON		/* Watch Dog Timer (WDT) */
#pragma config CP0	= OFF		/* Code protect */
#pragma config OSC	= HSPLL		/* HS oscillator, PLL enabled, HSPLL used by USB */
#pragma config CPUDIV	= OSC1		/* OSC1 = divide by 1 mode */
#pragma config IESO	= OFF		/* Internal External (clock) Switchover */
#pragma config FCMEN	= ON		/* Fail Safe Clock Monitor */
#pragma config T1DIG	= ON		/* secondary clock Source */
#pragma config LPT1OSC	= OFF		/* low power timer*/
#pragma config WDTPS	= 2048		/* Watchdog Timer Postscaler */
#pragma config DSWDTOSC	= INTOSCREF	/* DSWDT uses INTOSC/INTRC as reference clock */
#pragma config RTCOSC	= T1OSCREF	/* RTCC uses T1OSC/T1CKI as reference clock */
#pragma config DSBOREN	= OFF		/* Zero-Power BOR disabled in Deep Sleep */
#pragma config DSWDTEN	= OFF		/* Deep Sleep Watchdog Timer Enable */
#pragma config DSWDTPS	= 8192		/* Deep Sleep Watchdog Timer Postscale Select 1:8,192 (8.5 seconds) */
#pragma config IOL1WAY	= OFF		/* The IOLOCK bit (PPSCON<0>) can be set and cleared as needed */
#pragma config MSSP7B_EN = MSK7		/* 7 Bit address masking */
#pragma config WPFP	= PAGE_1	/* Write Protect Program Flash Page 0 */
#pragma config WPEND	= PAGE_0	/* Write/Erase protect Flash Memory pages */
#pragma config WPCFG	= OFF		/* Write/Erase Protection of last page Disabled */
#pragma config WPDIS	= OFF		/* Write Protect Disable */

/* flash the LEDs when in bootloader mode */
#define	BOOTLOADER_FLASH_INTERVAL	0x8000

static uint16_t _led_counter = 0x0;

/* we can flash the EEPROM or the FLASH */
static uint8_t _alt_setting = 0;
static uint8_t _did_upload_or_download = FALSE;
static uint8_t _do_reset = FALSE;
static CHugConfig _cfg;

#define CH_STATUS_LED_RED		0x02
#define CH_STATUS_LED_GREEN		0x01
#define CH_EEPROM_ADDR_WRDS		0x8000

/* This is the state machine used to switch between the different bootloader
 * and firmware modes:
 *
 *  [0. BOOTLOADER] -> [1. FIRMWARE]
 *      ^------------------/
 *
 * Rules for bootloader:
 *  - Run the firmware if !RCON.RI [RESET()] && !RCON.TO [WDT] && AUTO_BOOT=1
 *  - On USB reset, boot the firmware if we've read or written firmware
 *  - If state is dfuDNLD and AUTO_BOOT=1, set AUTO_BOOT=0
 *
 * Rules for firmware:
 *  - On USB reset in appDETACH, do reset() to get back to bootloader
 *  - If GetStatus is serviced and AUTO_BOOT=0, set AUTO_BOOT=1
 *
 */

static void
chug_boot_runtime(void)
{
	uint16_t runcode_start = 0xffff;

	chug_flash_read(CH_EEPROM_ADDR_WRDS, (uint8_t *) &runcode_start, 2);
	if (runcode_start == 0xffff)
		chug_errno_show(CH_ERROR_DEVICE_DEACTIVATED, TRUE);
	asm("ljmp 0x8000");
	chug_errno_show(CH_ERROR_NOT_IMPLEMENTED, TRUE);
}

int8_t
chug_usb_dfu_write_callback(uint16_t addr, uint8_t *data, uint16_t len, void *context)
{
	int8_t rc;

	/* a USB reset will take us to firmware mode */
	_did_upload_or_download = TRUE;

	/* erase EEPROM and write */
	if (_alt_setting == 0x00) {

		/* check this looks like a valid firmware by checking the
		 * interrupt vector values; these will be 0x1200 for proper
		 * firmware or 0x0000 if the firmware doesn't handle them */
		if (addr == 0x0000) {
			uint16_t *vectors = (uint16_t *) (data + 4);
			if ((vectors[0] != 0x0000 && vectors[0] != 0x1200) ||
			    (vectors[1] != 0x0000 && vectors[1] != 0x1200)) {
				usb_dfu_set_status(DFU_STATUS_ERR_FILE);
				return -1;
			}
		}

		/* set the auto-boot flag to false */
		if (_cfg.flash_success) {
			_cfg.flash_success = FALSE;
			chug_config_write(&_cfg);
		}

		/* we have to erase in chunks of 1024 bytes, e.g. every 16 blocks */
		if (addr % CH_FLASH_ERASE_BLOCK_SIZE == 0) {
			rc = chug_flash_erase(addr + CH_EEPROM_ADDR_WRDS,
					      CH_FLASH_ERASE_BLOCK_SIZE);
			if (rc != 0) {
				usb_dfu_set_status(DFU_STATUS_ERR_ERASE);
				return -1;
			}
		}

		/* write */
		rc = chug_flash_write(addr + CH_EEPROM_ADDR_WRDS, data, len);
		if (rc != 0) {
			usb_dfu_set_status(DFU_STATUS_ERR_WRITE);
			return -1;
		}
		return 0;
	}

	/* invalid */
	return -1;
}

int8_t
chug_usb_dfu_read_callback(uint16_t addr, uint8_t *data, uint16_t len, void *context)
{
	uint8_t rc;

	/* invalid */
	if (_alt_setting != 0x00)
		return -1;

	/* a USB reset will take us to firmware mode */
	_did_upload_or_download = TRUE;

	/* read from EEPROM */
	rc = chug_flash_read(addr + CH_EEPROM_ADDR_WRDS, data, len);
	if (rc != CH_ERROR_NONE)
		return -1;

	return 0;
}

int
main(void)
{
	/* enable the PLL and wait 2+ms until the PLL locks
	 * before enabling USB module */
	uint16_t pll_startup_counter = 1200;
	OSCTUNEbits.PLLEN = 1;
	while (pll_startup_counter--);

	/* default all pins to digital */
	ANCON0 = 0xFF;
	ANCON1 = 0xFF;

	/* set RA0, RA1 to output (freq scaling),
	 * set RA2, RA3 to output (color select),
	 * set RA5 to input (frequency counter),
	 * (RA6 is "don't care" in OSC2 mode)
	 * set RA7 to input (OSC1, HSPLL in) */
	TRISA = 0xf0;

	/* set RB0 to RB3 to input (h/w revision) others input (unused) */
	TRISB = 0xff;

	/* set RC0 to RC2 to input (unused) */
	TRISC = 0xff;

	/* set RD0 to RD7 to input (unused) */
	TRISD = 0xff;

	/* set RE0, RE1 output (LEDs) others input (unused) */
	TRISE = 0x3c;

	/* set the LED state initially */
	PORTE = CH_STATUS_LED_GREEN;

/* Configure interrupts, per architecture */
#ifdef USB_USE_INTERRUPTS
	INTCONbits.PEIE = 1;
	INTCONbits.GIE = 1;
#endif

	/* read config */
	chug_config_read(&_cfg);

	/* boot to firmware mode if all okay */
	if (RCONbits.NOT_TO && RCONbits.NOT_RI && _cfg.flash_success == 0x01) {
		PORTE = 0x03;
		chug_boot_runtime();
	}

	/* set to known initial status suitable for a bootloader */
	usb_dfu_set_state(DFU_STATE_DFU_IDLE);

	usb_init();

	while (1) {
		/* clear watchdog */
		CLRWDT();

		/* boot back into firmware */
		if (_do_reset)
			chug_boot_runtime();

		/* flash the LEDs */
		if (--_led_counter == 0) {
			PORTE ^= 0x03;
			_led_counter = BOOTLOADER_FLASH_INTERVAL;
		}
#ifndef USB_USE_INTERRUPTS
		usb_service();
#endif
	}

	return 0;
}

int8_t
chug_unknown_setup_request_callback(const struct setup_packet *setup)
{
	return process_dfu_setup_request(setup);
}

void
chug_usb_reset_callback(void)
{
	/* boot into the firmware if we ever did reading or writing */
	if (_did_upload_or_download)
		_do_reset = TRUE;
}

void interrupt high_priority
isr()
{
#ifdef USB_USE_INTERRUPTS
	usb_service();
#endif
}
