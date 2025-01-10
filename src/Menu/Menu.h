#pragma once

#include "Boot/Funkin.h"

#include <CKSDK/TTY.h>
#include <CKSDK/GPU.h>
#include <CKSDK/SPI.h>

#include "Boot/MMP.h"
#include "Boot/Loader.h"
#include "Boot/Character.h"
#include "Boot/Random.h"
#include "Boot/Wipe.h"
#include "Boot/Timer.h"
#include "Boot/Profiler.h"
#include "Boot/DATracker.h"

namespace Menu
{
	// Menu globals
	extern CDP::CDP menu_cdp;
	extern std::unique_ptr<char[]> perm_mmp;

	extern Timer::FixedTime song_time;
	extern uint32_t song_beat;

	// Menu substate class
	class Substate
	{
		public:
			virtual ~Substate() = default;

			virtual Substate *Loop(Timer::FixedTime dt) = 0;
	};
}
