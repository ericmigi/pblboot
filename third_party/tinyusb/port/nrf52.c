/*
 * Copyright (c) 2026 Core Devices LLC
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <stdint.h>

#include <hal/nrf_clock.h>
#include <nrfx_power.h>

extern void tusb_hal_nrf_power_event(uint32_t event);

static bool detected;
static bool attached;

void pb_tinyusb_nrf52_init(void)
{
	NRF_USBD->USBPULLUP = 0;
	NRF_USBD->ENABLE = 0;
	detected = false;
	attached = false;
}

/* The DFU task is the sole owner of TinyUSB power events. Its READY handler consumes a
 * hardware event, so an interrupt must never enter it while another call is in progress.
 */
void pb_tinyusb_nrf52_poll(void)
{
	const bool removed = nrf_power_event_get_and_clear(NRF_POWER, NRF_POWER_EVENT_USBREMOVED);
	const uint32_t status = NRF_POWER->USBREGSTATUS;

	if (detected && (removed || !(status & POWER_USBREGSTATUS_VBUSDETECT_Msk))) {
		tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_REMOVED);
		detected = false;
		attached = false;
	}
	if (status & POWER_USBREGSTATUS_VBUSDETECT_Msk) {
		if (!detected) {
			tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_DETECTED);
			detected = true;
		}
		/* TinyUSB waits synchronously for USBD and HFXO. Let the DFU loop feed the
		 * watchdog and enforce its idle deadline while either one is starting.
		 */
		if (!attached && (status & POWER_USBREGSTATUS_OUTPUTRDY_Msk) &&
		    (NRF_USBD->EVENTCAUSE & USBD_EVENTCAUSE_READY_Msk) &&
		    nrf_clock_hf_is_running(NRF_CLOCK, NRF_CLOCK_HFCLK_HIGH_ACCURACY)) {
			tusb_hal_nrf_power_event(NRFX_POWER_USB_EVT_READY);
			attached = true;
		}
	}
}
