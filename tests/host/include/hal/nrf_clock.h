#pragma once
#include <stdbool.h>

extern bool hfclk_running;
#define NRF_CLOCK 0
#define NRF_CLOCK_HFCLK_HIGH_ACCURACY 1

static inline bool nrf_clock_hf_is_running(int clock, int source)
{
	(void)clock;
	(void)source;
	return hfclk_running;
}
