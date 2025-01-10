/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- Wipe.cpp -
	Screen wipe
*/

#include "Boot/Wipe.h"

#include <CKSDK/GPU.h>

#include <CKSDK/TTY.h>

#include "Funkin.h"

namespace Wipe
{
	// Wipe constants
	static constexpr int32_t WIPE_SPAN = 96;

	// Wipe globals
	static int32_t wipe_state = 0;
	static int32_t wipe_y = 0;

	// Wipe packet
	struct WipePacket
	{
		CKSDK::GPU::FillPrim<> fill;
		CKSDK::GPU::DrawModePrim mode = CKSDK::GPU::DrawModePrim(0, CKSDK::GPU::SemiMode_Sub, CKSDK::GPU::BitDepth_4Bit, false, true, false);
		CKSDK::GPU::PolyPrim<true, true, false> span;
	};

	// Wipe functions
	KEEP bool Draw()
	{
		// Update wipe
		switch (wipe_state)
		{
			case 1:
			{
				// Scroll wipe
				wipe_y += 8;
				if (wipe_y >= (int32_t)g_height)
					wipe_state = -1;
				int32_t span_y = wipe_y + WIPE_SPAN;
				int32_t fill_y = wipe_y;

				// Setup wipe packet
				WipePacket &packet = CKSDK::GPU::AllocPacket<WipePacket>(0);
				
				packet.fill.c = CKSDK::GPU::Color(0x00, 0x00, 0x00);
				packet.fill.xy = CKSDK::GPU::ScreenCoord(0, 0);
				packet.fill.wh = CKSDK::GPU::ScreenDim(g_width, (fill_y > 0) ? fill_y : 0);

				packet.span.SetSemi(true);
				
				packet.span.v0.c = CKSDK::GPU::Color(0x00, 0x00, 0x00);
				packet.span.v0.xy = CKSDK::GPU::ScreenCoord(0, span_y);

				packet.span.v1.c = CKSDK::GPU::Color(0x00, 0x00, 0x00);
				packet.span.v1.xy = CKSDK::GPU::ScreenCoord(g_width, span_y);

				packet.span.v2.c = CKSDK::GPU::Color(0xFF, 0xFF, 0xFF);
				packet.span.v2.xy = CKSDK::GPU::ScreenCoord(0, fill_y);

				packet.span.v3.c = CKSDK::GPU::Color(0xFF, 0xFF, 0xFF);
				packet.span.v3.xy = CKSDK::GPU::ScreenCoord(g_width, fill_y);

				return wipe_state == -1;
			}
			case -1:
			{
				// Scroll wipe
				wipe_y -= 8;
				if (wipe_y < -WIPE_SPAN)
				{
					wipe_state = 0;
					return false;
				}
				int32_t span_y = g_height - wipe_y - WIPE_SPAN;
				int32_t fill_y = g_height - wipe_y;

				// Setup wipe packet
				WipePacket &packet = CKSDK::GPU::AllocPacket<WipePacket>(0);

				packet.fill.c = CKSDK::GPU::Color(0x00, 0x00, 0x00);
				packet.fill.xy = CKSDK::GPU::ScreenCoord(0, fill_y);
				packet.fill.wh = CKSDK::GPU::ScreenDim(g_width, (g_height > fill_y) ? (g_height - fill_y) : 0);

				packet.span.SetSemi(true);

				packet.span.v0.c = CKSDK::GPU::Color(0x00, 0x00, 0x00);
				packet.span.v0.xy = CKSDK::GPU::ScreenCoord(0, span_y);

				packet.span.v1.c = CKSDK::GPU::Color(0x00, 0x00, 0x00);
				packet.span.v1.xy = CKSDK::GPU::ScreenCoord(g_width, span_y);

				packet.span.v2.c = CKSDK::GPU::Color(0xFF, 0xFF, 0xFF);
				packet.span.v2.xy = CKSDK::GPU::ScreenCoord(0, fill_y);

				packet.span.v3.c = CKSDK::GPU::Color(0xFF, 0xFF, 0xFF);
				packet.span.v3.xy = CKSDK::GPU::ScreenCoord(g_width, fill_y);

				return false;
			}
			default:
				return false;
		}
	}

	KEEP void In()
	{
		// Set wipe in state
		if (wipe_state != -1)
			wipe_y = g_height;
		wipe_state = -1;
	}

	KEEP void Out()
	{
		// Set wipe out state
		if (wipe_state != 1)
			wipe_y = -WIPE_SPAN;
		wipe_state = 1;
	}
}
