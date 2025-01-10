/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- MMP.h -
	Memory package class
*/

#pragma once

#include <CKSDK/CKSDK.h>

#include "Boot/Hash.h"

namespace MMP
{
	// Memory package functions
	void *Search(const void *ptr, Hash::Hash hash);
}
