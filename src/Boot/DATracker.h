/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025

	- DATracker.h -
	Digital audio tracker
*/

#pragma once

#include <CKSDK/CKSDK.h>
#include <CKSDK/CD.h>

#include "Boot/Timer.h"

namespace DATracker
{
	// DA tracker functions
	void Callback_Loop(CKSDK::CD::IRQStatus status, const CKSDK::CD::Result &result);

	void PlayTrack(uint8_t track, CKSDK::CD::Callback end_cb);
	Timer::FixedTime Tick(Timer::FixedTime dt);
}
