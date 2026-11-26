#include <assert.h>
#include <hal/nrf_clock.h>
#include <nrfx_power.h>

static struct fake_power power;
static struct fake_usbd usbd;
struct fake_power *const NRF_POWER = &power;
struct fake_usbd *const NRF_USBD = &usbd;
bool hfclk_running;

static unsigned int detected, ready, removed;
void pb_tinyusb_nrf52_init(void);
void pb_tinyusb_nrf52_poll(void);

void tusb_hal_nrf_power_event(uint32_t event)
{
	switch (event) {
	case NRFX_POWER_USB_EVT_DETECTED:
		assert(!usbd.ENABLE);
		usbd.ENABLE = 1;
		usbd.EVENTCAUSE = 0;
		detected++;
		break;
	case NRFX_POWER_USB_EVT_READY:
		assert(usbd.ENABLE && usbd.EVENTCAUSE && hfclk_running);
		usbd.EVENTCAUSE = 0;
		usbd.USBPULLUP = 1;
		ready++;
		break;
	case NRFX_POWER_USB_EVT_REMOVED:
		assert(usbd.ENABLE);
		usbd.ENABLE = 0;
		usbd.USBPULLUP = 0;
		removed++;
		break;
	default:
		assert(0);
	}
}

int main(void)
{
	usbd.USBPULLUP = usbd.ENABLE = 1;
	pb_tinyusb_nrf52_init();
	assert(!usbd.USBPULLUP && !usbd.ENABLE);

	power.USBREGSTATUS = POWER_USBREGSTATUS_VBUSDETECT_Msk;
	pb_tinyusb_nrf52_poll();
	pb_tinyusb_nrf52_poll();
	assert(detected == 1 && ready == 0);

	power.USBREGSTATUS |= POWER_USBREGSTATUS_OUTPUTRDY_Msk;
	pb_tinyusb_nrf52_poll();
	usbd.EVENTCAUSE = USBD_EVENTCAUSE_READY_Msk;
	pb_tinyusb_nrf52_poll();
	assert(ready == 0);
	hfclk_running = true;
	pb_tinyusb_nrf52_poll();
	pb_tinyusb_nrf52_poll();
	assert(ready == 1 && usbd.USBPULLUP);

	/* Unplug and replug occurred between two DFU task iterations. */
	power.removed = true;
	pb_tinyusb_nrf52_poll();
	assert(removed == 1 && detected == 2 && ready == 1 && !usbd.USBPULLUP);
	usbd.EVENTCAUSE = USBD_EVENTCAUSE_READY_Msk;
	pb_tinyusb_nrf52_poll();
	assert(ready == 2 && usbd.USBPULLUP);

	power.USBREGSTATUS = 0;
	pb_tinyusb_nrf52_poll();
	pb_tinyusb_nrf52_poll();
	assert(removed == 2 && !usbd.USBPULLUP);
	return 0;
}
