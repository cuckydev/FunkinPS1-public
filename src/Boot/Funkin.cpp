/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- Funkin.cpp -
	Funkin boot
*/

#include "Boot/Funkin.h"

#include "Boot/Random.h"

#include <CKSDK/TTY.h>
#include <CKSDK/OS.h>
#include <CKSDK/GPU.h>
#include <CKSDK/CD.h>
#include <CKSDK/ISO.h>
#include <CKSDK/DLL.h>
#include <CKSDK/Timer.h>

#include <CKSDK/ExScreen.h>

// Main globals
KEEP Hash::Hash g_scene_dll;
KEEP std::unique_ptr<void*> g_scene_userdata;

KEEP CDP::CDP g_all_cdp;
KEEP CDP::CDP g_data_cdp;
static CDP::CDP dll_cdp;

// Symbol code
namespace Symbol
{
	// Symbol constants
	static constexpr unsigned BUCKET_BITS = 5;
	static constexpr unsigned BUCKETS = 1 << BUCKET_BITS;

	// Symbol data
	static std::unique_ptr<char[]> sym_data;

	// Symbol callback
	static void SetSymbol(std::unique_ptr<char[]> sym)
	{
		// Use symbol pointer
		sym_data = std::move(sym);
		
		// Adjust symbol addresses
		uintptr_t sym_addr = (uintptr_t)(sym_data.get());

		uint32_t *symp = (uint32_t*)(sym_data.get());
		for (unsigned i = 0; i < BUCKETS; i++)
			*symp++ += sym_addr;
		
		for (unsigned i = 0; i < BUCKETS; i++)
		{
			uint32_t syms = *symp++;
			for (uint32_t j = syms; j != 0; j--)
			{
				symp[0] += sym_addr;
				symp += 2;
			}
		}
	}

	static void *SymbolCallback(const char *name)
	{
		uint32_t hash = CKSDK::ELF::ElfHash(name);

		uint32_t *sym = (uint32_t*)(sym_data.get());

		uint32_t *symp = (uint32_t*)(sym[hash & (BUCKETS - 1)]);
		uint32_t syms = *symp++;
		
		for (uint32_t j = syms; j != 0; j--)
		{
			char *namep = (char*)(symp[0]);
			if (__builtin_strcmp(name, namep) == 0)
				return (void*)symp[1];
			symp += 2;
		}

		return nullptr;
	}
}

// Entry point
extern "C" void main()
{
	// Initialize GPU buffer and screen
	static CKSDK::GPU::Word buffer[0x4000];
	CKSDK::GPU::SetBuffer(buffer, std::size(buffer), OT::Length);
	CKSDK::GPU::SetScreen(g_width, g_height, 0, 0, 0, 0, 0, g_height);

	// Setup GTE for 2D screen
	gte_SetGeomOffset(g_width / 2, g_height / 2);
	gte_SetGeomScreen(256);

	// Initialize random seed
	Random::Seed(CKSDK::OS::TimerCtrl(2).value);

	// Read CDPs
	g_all_cdp.Read(CKSDK::ISO::g_all);
	dll_cdp.Read(g_all_cdp.Search("dll.cdp"_h));
	g_data_cdp.Read(g_all_cdp.Search("data.cdp"_h));

	// Read symbol file
	{
		CKSDK::CD::File sym_file = g_all_cdp.Search("Funkin.sym"_h);
		std::unique_ptr<char[]> sym(new char[sym_file.Size()]);

		CKSDK::CD::ReadSectors(nullptr, sym.get(), sym_file, CKSDK::CD::Mode::Speed);
		CKSDK::CD::ReadSync();

		Symbol::SetSymbol(std::move(sym));
		CKSDK::DLL::SetSymbolCallback(Symbol::SymbolCallback);
	}

	// Start game state
	g_scene_dll = "Menu.dll"_h;

	// Game loop
	while (1)
	{
		// Read scene DLL
		CKSDK::CD::File dll_file = dll_cdp.Search(g_scene_dll);
		std::unique_ptr<char[]> dll_data(new char[dll_file.Size()]);

		CKSDK::CD::ReadSectors(nullptr, dll_data.get(), dll_file, CKSDK::CD::Mode::Speed);
		CKSDK::CD::ReadSync();

		// Run DLL
		{
			CKSDK::DLL::DLL dll(std::move(dll_data), dll_file.size);
			CKSDK::OS::Function<void> dll_entry = (void(*)())dll.GetSymbol("Entry");
			dll_entry();
		}
	}
}
