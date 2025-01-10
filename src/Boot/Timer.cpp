/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025

	- Timer.cpp -
	Timer code
*/

#include "Timer.h"

#include <CKSDK/OS.h>

namespace Timer
{
	// Timer constants
	static constexpr uint32_t TIMER_HZ = 100;

	// Timer callback
	static volatile uint32_t tick, last_tick;
	static void TimerCallback()
	{
		// Update timer
		tick = tick + 1;
	}

	// Timer functions
	KEEP void Start()
	{
		// Disable interrupts to avoid putting our timer in a bad state
		CKSDK::OS::DisableIRQ();

		// Initialize timer state
		tick = 0;
		last_tick = 0;

		// Set timer callback
		CKSDK::Timer::Set(TIMER_HZ, TimerCallback);

		// Re-enable interrupts
		CKSDK::OS::EnableIRQ();
	}

	KEEP FixedTime GetTime()
	{
		return FixedTime(tick) / TIMER_HZ;
	}

	KEEP FixedTime Update()
	{
		// Calculate difference between ticks
		uint32_t now_tick = tick;
		uint32_t now_last_tick = last_tick;
		last_tick = now_tick;

		// Calculate delta time
		uint32_t delta_tick = now_tick - now_last_tick;
		FixedTime delta = FixedTime(delta_tick) / TIMER_HZ;
		return delta;
	}
}
