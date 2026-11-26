#pragma once

#include <stdbool.h>
#include <stdint.h>

enum { NRFX_POWER_USB_EVT_DETECTED, NRFX_POWER_USB_EVT_REMOVED, NRFX_POWER_USB_EVT_READY };
enum { NRF_POWER_EVENT_USBREMOVED };
#define POWER_USBREGSTATUS_VBUSDETECT_Msk 1U
#define POWER_USBREGSTATUS_OUTPUTRDY_Msk 2U
#define USBD_EVENTCAUSE_READY_Msk 1U

struct fake_power {
	uint32_t USBREGSTATUS;
	bool removed;
};
struct fake_usbd {
	uint32_t USBPULLUP, ENABLE, EVENTCAUSE;
};
extern struct fake_power *const NRF_POWER;
extern struct fake_usbd *const NRF_USBD;

static inline bool nrf_power_event_get_and_clear(struct fake_power *power, int event)
{
	(void)event;
	bool removed = power->removed;
	power->removed = false;
	return removed;
}
