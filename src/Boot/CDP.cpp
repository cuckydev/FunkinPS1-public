/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- CDP.cpp -
	CD package class
*/

#include "Boot/CDP.h"

#include <CKSDK/OS.h>
#include <CKSDK/ExScreen.h>

namespace CDP
{
	// CD package class
	KEEP void CDP::Read(const CKSDK::CD::File &file)
	{
		// Read header
		CKSDK::CD::ReadSectors(nullptr, s.b, file.loc, 1, CKSDK::CD::Mode::Speed);
		CKSDK::CD::ReadSync();

		// Offset header appropriately to LBA
		uint32_t cdp_lba = file.loc.Dec();
		for (uint32_t i = 0; i < s.header.files; i++)
			s.header.file[i].loc.loc = CKSDK::CD::Loc::Enc(cdp_lba + s.header.file[i].loc.lba);
	}
	
	KEEP CKSDK::CD::File CDP::Search(Hash::Hash hash)
	{
		// Search for file in header
		for (uint32_t i = 0; i < s.header.files; i++)
		{
			auto &file = s.header.file[i];
			if (file.hash == hash)
			{
				return CKSDK::CD::File{
					file.loc.loc,
					file.size
				};
			}
		}

		// Failed to find file
		CKSDK::TTY::OutHex<4>(hash);
		CKSDK::TTY::Out("\n");
		CKSDK::ExScreen::Abort("CDP::Search failed");
		return {};
	}
}
