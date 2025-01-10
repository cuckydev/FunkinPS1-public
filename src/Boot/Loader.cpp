/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- Loader.cpp -
	Loader screen
*/

#include "Boot/Loader.h"

#include "Boot/Wipe.h"
#include "Boot/Character.h"

#include <CKSDK/TTY.h>

namespace Loader
{
	// Loader data
	alignas(uint32_t) static const uint8_t loader_chr[] = {
		#include "chr/Loader.chr.h"
	};

	// Loader callback
	static uint32_t bump = 0;

	static void VBlankCallback()
	{
		/*
		// Direct flip buffer
		GPU::DirectFlip();
		
		// Start direct flip
		GPU::DirectStart();

		// Bump screen
		if (bump < 0x140)
			bump = 0x2400;
		else
			bump = bump * 16 / 17;

		{
			GPU::Matrix mat = {};
			mat.m[0][0] = 0x1000;
			mat.m[1][1] = 0x1000;
			mat.m[2][2] = 0x1000;
			mat.t[2] = 256;
			
			gte_SetGeomOffset(g_width / 2, g_height / 2);
			gte_SetGeomScreen(256 + (bump >> 8));
			gte_SetTransMatrix(&mat);
			gte_SetRotMatrix(&mat);
		}
		
		// Draw loading screen
		Character::Draw(loader_chr, 0, GPU::OT::UI - 1);
		
		{
			uint32_t *bg_pri = GPU::AllocPacket(GPU::OT::UI, 3);
			bg_pri[1] = (GPU::GP0_FillRect << 24) | (0xCA << 0) | (0xFF << 8) | (0x4D << 16);
			bg_pri[2] = 0;
			bg_pri[3] = (g_width << 0) | (g_height << 16);
		}
		*/
	}

	// Loader functions
	KEEP void Start()
	{
		/*
		// DMA image
		Character::DMA(loader_chr, 0, true);

		// Start loading screen
		bump = 0;
		GPU::DirectStart();
		GPU::SetVBlankCallback(VBlankCallback);
		*/
	}

	KEEP void End()
	{

	}
}
