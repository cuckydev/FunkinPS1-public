/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025

	- Profiler.cpp -
	Frame time profiler
*/

#include "Boot/Profiler.h"

#ifdef ENABLE_PROFILER
#include "Boot/Timer.h"

#include <CKSDK/GPU.h>
#include <CKSDK/Mem.h>

namespace Profiler
{
	// Profiler state
	static constexpr int32_t BAR_X = (320 - 256) / 2;
	static constexpr int32_t BAR_Y = 240 - 48;
	static constexpr int32_t BAR_WIDTH = 256;
	static constexpr int32_t BAR_HEIGHT = 4;

	static constexpr Timer::FixedTime BAR_TIME = 1.0 / 30.0;

	static Timer::FixedTime start_time = 0;

	// Profiler functions
	KEEP void StartFrame()
	{
		start_time = Timer::GetTime();
	}
	
	KEEP void EndFrame()
	{
		// Display
		struct BarPacket
		{
			CKSDK::GPU::FillPrim<> back;
			CKSDK::GPU::FillPrim<> front;
		};

		BarPacket &packet = CKSDK::GPU::AllocPacket<BarPacket>(0);

		packet.back.c = CKSDK::GPU::Color(0x20, 0x20, 0x20);
		packet.back.xy = CKSDK::GPU::ScreenCoord(BAR_X, BAR_Y);
		packet.back.wh = CKSDK::GPU::ScreenDim(BAR_WIDTH, BAR_HEIGHT);

		Timer::FixedTime delta = Timer::GetTime() - start_time;
		int32_t pixels = (int32_t)(delta * BAR_WIDTH / BAR_TIME);
		if (pixels < 0)
			pixels = 0;
		if (pixels > BAR_WIDTH)
		{
			packet.front.c = CKSDK::GPU::Color(0xFF, 0x00, 0x10);
			pixels = BAR_WIDTH;
		}
		else
		{
			packet.front.c = CKSDK::GPU::Color(0x10, 0xFF, 0x20);
		}

		packet.front.xy = CKSDK::GPU::ScreenCoord(BAR_X, BAR_Y);
		packet.front.wh = CKSDK::GPU::ScreenDim(pixels, BAR_HEIGHT);

		// Memory profile
		size_t mem_used, mem_total, mem_blocks;
		CKSDK::Mem::Profile(&mem_used, &mem_total, &mem_blocks);
		CKSDK::TTY::Out("Memory ");
		CKSDK::TTY::OutHex<4>(mem_used);
		CKSDK::TTY::Out("/");
		CKSDK::TTY::OutHex<4>(mem_total);
		CKSDK::TTY::Out(" (");
		CKSDK::TTY::OutHex<4>(mem_blocks);
		CKSDK::TTY::Out(")\n");
	}
}
#endif
