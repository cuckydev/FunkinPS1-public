/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025

	- Profiler.h -
	Frame time profiler
*/

#pragma once

#include "Boot/Funkin.h"

// #define ENABLE_PROFILER

namespace Profiler
{
	// Profiler functions
	#ifdef ENABLE_PROFILER
		void StartFrame();
		void EndFrame();
	#else
		inline void StartFrame() {}
		inline void EndFrame() {}
	#endif
}
