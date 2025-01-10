/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- Random.h -
	Random number generator
*/

#pragma once

#include <CKSDK/CKSDK.h>
#include <CKSDK/Util/Fixed.h>

namespace Random
{
	// Random functions
	void Seed(uint32_t seed);

	uint32_t Next();

	template <typename T>
	T Next(T max)
	{
		// Return a random number from 0 to max
		return Next() % (max + 1);
	}

	template <typename T>
	T Next(T min, T max)
	{
		// Return a random number from min to max
		return min + (Next() % ((max + 1) - min));
	}

	// Fixed point random
	template <typename T>
	T NextFixed(T min, T max)
	{
		return T::Raw(Next(min.Raw(), max.Raw()));
	}
}
