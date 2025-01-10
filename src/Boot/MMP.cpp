/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- MMP.cpp -
	Memory package class
*/

#include "MMP.h"

#include <CKSDK/OS.h>
#include <CKSDK/ExScreen.h>

namespace MMP
{
	// Memory package functions
	KEEP void *Search(const void *ptr, Hash::Hash hash)
	{
		// Search for file in header
		struct MMPFile
		{
			Hash::Hash hash;
			uint32_t addr;
			uint32_t size;
		};
		uint32_t files = *((uint32_t*)ptr);
		const MMPFile *const mmp = (const MMPFile*)((uint32_t*)ptr + 1);

		for (uint32_t i = 0; i < files; i++)
		{
			if (mmp[i].hash == hash)
				return (void*)((const char*)ptr + mmp[i].addr);
		}
		
		// Failed to find file
		CKSDK::TTY::OutHex<4>(hash);
		CKSDK::TTY::Out("\n");
		CKSDK::ExScreen::Abort("MMP::Search failed");
		return nullptr;
	}
}
