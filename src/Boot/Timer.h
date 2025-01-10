/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025

	- Timer.h -
	Timer code
*/

#pragma once

#include <CKSDK/CKSDK.h>
#include <CKSDK/Timer.h>
#include <CKSDK/Util/Fixed.h>

namespace Timer
{
	// Timer types
	using FixedTime = CKSDK::Fixed::Fixed<int32_t, 16>;

	// Timer functions
	void Start();

	FixedTime GetTime();
	FixedTime Update();
}
