/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- Random.h -
	Random number generator
*/

#include "Random.h"

namespace Random
{
	// Random globals
	static uint32_t random_seed = 0;

	// Random functions
	KEEP void Seed(uint32_t seed)
	{
		// Set random seed
		random_seed = seed;
	}

	KEEP uint32_t Next()
	{
		// Return a random number from 0x00000000 to 0xFFFFFFFF
		uint32_t a = (random_seed * 1103515245) + 12345;
		uint32_t b = (a * 1103515245) + 12345;
		return (((random_seed = b) & 0xFFFF0000) ^ (a >> 16));
	}
}
