/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- Funkin.h -
	Funkin app
*/

#pragma once

#include <CKSDK/CKSDK.h>

#include "Boot/CDP.h"

#include <memory>

// Funkin globals
static constexpr uint32_t g_width = 320;
static constexpr uint32_t g_height = 240;

enum OT
{
	UI = (8 * 1) - 1,
	Foreground = (8 * 2) - 1,
	Focus = (8 * 3) - 1,
	Background = (8 * 4) - 1,

	Length
};

// Main globals
extern Hash::Hash g_scene_dll;
extern std::unique_ptr<void*> g_scene_userdata;

extern CDP::CDP g_all_cdp, g_data_cdp;
