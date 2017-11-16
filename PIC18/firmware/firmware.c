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

#include "usb.h"

#include <xc.h>
#include <string.h>

#include "usb_config.h"
#include "usb_ch9.h"
#include "usb_hid.h"
#include "usb_dfu.h"

#include "ch-config.h"
#include "ch-errno.h"
#include "ch-flash.h"

static CHugConfig		 _cfg;
static ChError			 _last_error = CH_ERROR_NONE;
static ChCmd			 _last_error_cmd = CH_CMD_RESET;
static uint16_t			 _integration_time = 0x0;
static uint8_t			 _chug_buf[CH_EP0_TRANSFER_SIZE];
static uint16_t			 _heartbeat_cnt = 0;

#define CH_SRAM_ADDRESS_WRDS		0x6000

void
chug_usb_dfu_set_success_callback(void *context)
{
	/* set the auto-boot flag to true */
	if (_cfg.flash_success != 0x01) {
		uint8_t rc;
		_cfg.flash_success = TRUE;
		rc = chug_config_write(&_cfg);
		if (rc != CH_ERROR_NONE)
			chug_errno_show(rc, FALSE);
	}
}

static void
chug_set_error(ChCmd cmd, ChError status)
{
	_last_error = status;
	_last_error_cmd = cmd;
}

static void
chug_set_leds_internal(uint8_t leds)
{
	/* the first few boards on the P&P machines had the
	 * LEDs soldered the wrong way around */
	if ((_cfg.pcb_errata & CH_PCB_ERRATA_SWAPPED_LEDS) > 0) {
		PORTEbits.RE0 = (leds & CH_STATUS_LED_GREEN);
		PORTEbits.RE1 = (leds & CH_STATUS_LED_RED) >> 1;
	} else {
		PORTEbits.RE0 = (leds & CH_STATUS_LED_RED) >> 1;
		PORTEbits.RE1 = (leds & CH_STATUS_LED_GREEN);
	}
}

static void
chug_set_leds(uint8_t leds)
{
	_heartbeat_cnt = 0xffff;
	chug_set_leds_internal(leds);
}

static uint8_t
chug_get_leds_internal(void)
{
	return (PORTEbits.RE1 << 1) + PORTEbits.RE0;
}

static void
chug_heatbeat(uint8_t leds)
{
	/* disabled */
	if (_heartbeat_cnt == 0xffff)
		return;

	/* do pulse up -> down -> up */
	if (_heartbeat_cnt <= 0xff) {
		uint8_t j;
		uint8_t _duty = _heartbeat_cnt;
		if (_duty > 127)
			_duty = 0xff - _duty;
		for (j = 0; j < _duty * 2; j++)
			chug_set_leds_internal(leds);
		for (j = 0; j < 0xff - _duty * 2; j++)
			chug_set_leds_internal(0);
	}

	/* this is the 'pause' btween the bumps */
	if (_heartbeat_cnt++ > 0xc000)
		_heartbeat_cnt = 0;
}

static int8_t
_send_data_stage_cb(bool transfer_ok, void *context)
{
	/* error */
	if (!transfer_ok) {
		chug_errno_show(CH_ERROR_INVALID_ADDRESS, FALSE);
		return -1;
	}
	return 0;
}

int
main(void)
{
	uint8_t dfu_interfaces[] = { 0x01 };

	/* read config */
	chug_config_read(&_cfg);
	usb_dfu_set_state(DFU_STATE_APP_IDLE);
	usb_init();

	/* we have to tell the USB stack which interfaces to use */
	dfu_set_interface_list(dfu_interfaces, 1);

	while (1) {
		/* clear watchdog */
		CLRWDT();
		usb_service();
		chug_heatbeat(CH_STATUS_LED_RED);
	}

	return 0;
}

static int8_t
_recieve_crypto_key_cb(bool transfer_ok, void *context)
{
	/* error */
	if (!transfer_ok) {
		chug_errno_show(CH_ERROR_DEVICE_DEACTIVATED, FALSE);
		return -1;
	}

	/* save to EEPROM */
	memcpy(_cfg.signing_key, _chug_buf, sizeof(uint32_t) * 4);
	chug_config_write(&_cfg);
	return 0;
}

static int8_t
chug_handle_set_crypto_key(const struct setup_packet *setup)
{
	/* check size */
	if (setup->wLength != sizeof(uint32_t) * 4) {
		chug_set_error(CH_CMD_SET_CRYPTO_KEY, CH_ERROR_INVALID_LENGTH);
		return -1;
	}

	/* already set */
	if (chug_config_has_signing_key(&_cfg)) {
		chug_set_error(CH_CMD_SET_CRYPTO_KEY, CH_ERROR_WRONG_UNLOCK_CODE);
		return -1;
	}

	usb_start_receive_ep0_data_stage(_chug_buf, setup->wLength,
					 _recieve_crypto_key_cb, NULL);
	return 0;
}

int8_t
process_chug_setup_request(struct setup_packet *setup)
{
	int32_t tmp;

	if (setup->REQUEST.destination != DEST_INTERFACE)
		return -1;
	if (setup->REQUEST.type != REQUEST_TYPE_CLASS)
		return -1;
	if (setup->wIndex != CH_USB_INTERFACE)
		return -1;

	/* process commands */
	switch (setup->bRequest) {

	/* device->host */
	case CH_CMD_GET_SERIAL_NUMBER:
		memcpy(_chug_buf, &_cfg.serial_number, 2);
		usb_send_data_stage(_chug_buf, 2, _send_data_stage_cb, NULL);
		return 0;
	case CH_CMD_GET_LEDS:
		_chug_buf[0] = chug_get_leds_internal();
		usb_send_data_stage(_chug_buf, 1, _send_data_stage_cb, NULL);
		return 0;
	case CH_CMD_GET_PCB_ERRATA:
		_chug_buf[0] = _cfg.pcb_errata;
		usb_send_data_stage(_chug_buf, 1, _send_data_stage_cb, NULL);
		return 0;
	case CH_CMD_GET_ERROR:
		_chug_buf[0] = _last_error;
		_chug_buf[1] = _last_error_cmd;
		usb_send_data_stage(_chug_buf, 2, _send_data_stage_cb, NULL);
		return 0;

	/* host->device */
	case CH_CMD_SET_SERIAL_NUMBER:
		_cfg.serial_number = setup->wValue;
		chug_config_write(&_cfg);
		usb_send_data_stage(NULL, 0, _send_data_stage_cb, NULL);
		return 0;
	case CH_CMD_SET_LEDS:
		chug_set_leds(setup->wValue);
		usb_send_data_stage(NULL, 0, _send_data_stage_cb, NULL);
		return 0;
	case CH_CMD_SET_CRYPTO_KEY:
		return chug_handle_set_crypto_key(setup);

	/* actions */
	case CH_CMD_CLEAR_ERROR:
		chug_set_error(CH_CMD_LAST, CH_ERROR_NONE);
		usb_send_data_stage(NULL, 0, _send_data_stage_cb, NULL);
		return 0;
	default:
		chug_set_error(setup->bRequest, CH_ERROR_UNKNOWN_CMD);
	}
	return -1;
}

int8_t
chug_unknown_setup_request_callback(const struct setup_packet *setup)
{
	if (process_dfu_setup_request(setup) == 0)
		return 0;
	if (process_chug_setup_request((struct setup_packet *) setup) == 0)
		return 0;
	return -1;
}

void
chug_usb_reset_callback(void)
{
	/* reset back into DFU mode */
	if (usb_dfu_get_state() == DFU_STATE_APP_DETACH)
		RESET();
}

void interrupt high_priority
isr()
{
#ifdef USB_USE_INTERRUPTS
	usb_service();
#endif
}
