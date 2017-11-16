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

#ifndef __COLORHUG_H
#define __COLORHUG_H

#include <stdint.h>

#define CH_EP0_TRANSFER_SIZE		0x400
#define CH_USB_INTERFACE		0x00

typedef enum {
	/* dummy */
	CH_CMD_RESET			= 0x24,

	/* read */
	CH_CMD_GET_LEDS			= 0x0d,
	CH_CMD_GET_ILLUMINANTS		= 0x15,
	CH_CMD_GET_SERIAL_NUMBER	= 0x0b,
	CH_CMD_GET_PCB_ERRATA		= 0x33,
	CH_CMD_GET_INTEGRAL_TIME	= 0x05,
	CH_CMD_GET_ADC_CALIBRATION_POS	= 0x51,
	CH_CMD_GET_ADC_CALIBRATION_NEG	= 0x52,
	CH_CMD_GET_CCD_CALIBRATION	= 0x53, //ish
	CH_CMD_READ_SRAM		= 0x38,

	/* write */
	CH_CMD_SET_LEDS			= 0x0e,
	CH_CMD_SET_ILLUMINANTS		= 0x16,
	CH_CMD_SET_SERIAL_NUMBER	= 0x0c,
	CH_CMD_SET_PCB_ERRATA		= 0x32,
	CH_CMD_SET_INTEGRAL_TIME	= 0x06,
	CH_CMD_SET_CCD_CALIBRATION	= 0x54, //ish
	CH_CMD_WRITE_SRAM		= 0x39,
	CH_CMD_SET_CRYPTO_KEY		= 0x70,

	/* read only */
	CH_CMD_GET_ERROR		= 0x60,
	CH_CMD_GET_TEMPERATURE		= 0x3b,

	/* action */
	CH_CMD_CLEAR_ERROR		= 0x61,
	CH_CMD_TAKE_READING_SPECTRAL	= 0x55,
	CH_CMD_TAKE_READING_XYZ		= 0x23,
	CH_CMD_LOAD_SRAM		= 0x41,
	CH_CMD_SAVE_SRAM		= 0x42,
	CH_CMD_LAST
} ChCmd;

/* any problems with the PCB */
typedef enum {
	CH_PCB_ERRATA_NONE		= 0,
	CH_PCB_ERRATA_SWAPPED_LEDS	= 1 << 0,
	CH_PCB_ERRATA_NO_WELCOME	= 1 << 1,
	CH_PCB_ERRATA_LAST		= 1 << 2
} ChPcbErrata;

/* Led colors: possible bitfield values */
typedef enum {
	CH_STATUS_LED_GREEN		= 1 << 0,
	CH_STATUS_LED_RED		= 1 << 1,
	CH_STATUS_LED_BLUE		= 1 << 2	/* Since: 0.1.29 */
} ChStatusLed;

/* Illuminants: possible bitfield values */
typedef enum {
	CH_ILLUMINANT_NONE		= 0,
	CH_ILLUMINANT_A			= 1 << 0,
	CH_ILLUMINANT_UV		= 1 << 1,
	CH_ILLUMINANT_LAST
} ChIlluminant;

/* fatal error morse code */
typedef enum {
	CH_ERROR_NONE,
	CH_ERROR_UNKNOWN_CMD,
	CH_ERROR_WRONG_UNLOCK_CODE,
	CH_ERROR_NOT_IMPLEMENTED,
	CH_ERROR_UNDERFLOW_SENSOR,
	CH_ERROR_NO_SERIAL,
	CH_ERROR_WATCHDOG,
	CH_ERROR_INVALID_ADDRESS,
	CH_ERROR_INVALID_LENGTH,
	CH_ERROR_INVALID_CHECKSUM,
	CH_ERROR_INVALID_VALUE,
	CH_ERROR_UNKNOWN_CMD_FOR_BOOTLOADER,
	CH_ERROR_NO_CALIBRATION,
	CH_ERROR_OVERFLOW_MULTIPLY,
	CH_ERROR_OVERFLOW_ADDITION,
	CH_ERROR_OVERFLOW_SENSOR,
	CH_ERROR_OVERFLOW_STACK,
	CH_ERROR_DEVICE_DEACTIVATED,
	CH_ERROR_INCOMPLETE_REQUEST,
	CH_ERROR_SELF_TEST_SENSOR,
	CH_ERROR_SELF_TEST_RED,
	CH_ERROR_SELF_TEST_GREEN,
	CH_ERROR_SELF_TEST_BLUE,
	CH_ERROR_SELF_TEST_COLOR_SELECT,
	CH_ERROR_SELF_TEST_MULTIPLIER,
	CH_ERROR_INVALID_CALIBRATION,
	CH_ERROR_SRAM_FAILED,
	CH_ERROR_OUT_OF_MEMORY,
	CH_ERROR_SELF_TEST_TEMPERATURE,
	CH_ERROR_SELF_TEST_I2C,
	CH_ERROR_SELF_TEST_ADC_VDD,
	CH_ERROR_SELF_TEST_ADC_VSS,
	CH_ERROR_SELF_TEST_ADC_VREF,
	CH_ERROR_I2C_SLAVE_ADDRESS,
	CH_ERROR_I2C_SLAVE_CONFIG,
	CH_ERROR_SELF_TEST_EEPROM,	/* since 1.2.9 */
	/*< private >*/
	CH_ERROR_LAST
} ChError;

#endif /* __COLORHUG_H */
