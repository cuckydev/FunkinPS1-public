/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025

	- Hash.h -
	Hashing functions
*/

#pragma once

/// @brief Hash namespace
/// @details This is an implementation of a Fowler–Noll–Vo hash function
/// @see https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function
namespace Hash
{
	/// @brief Hash type
	typedef uint32_t Hash;
		
	/// @brief FNV prime value
	static const Hash FNV32_PRIME = 0x01000193;
	/// @brief FNV initial value
	static const Hash FNV32_IV    = 0x811C9DC5;

	/// @brief Hash a string literal
	/// @param literal String literal
	/// @param max_length Maximum length of the string literal
	/// @param accumulator Accumulator value
	/// @return Hash value
	constexpr static inline Hash FromConst(const char *const literal, size_t max_length = 0xFFFFFFFF, Hash accumulator = FNV32_IV)
	{
		if (*literal && max_length)
			return FromConst(&literal[1], max_length - 1, (accumulator ^ Hash(*literal)) * FNV32_PRIME);
		return accumulator;
	}

	/// @brief Hash a buffer
	/// @param data Buffer data
	/// @param length Buffer length
	/// @return Hash value
	static inline Hash FromBuffer(const uint8_t *data, size_t length)
	{
		Hash accumulator = FNV32_IV;
		while (length-- > 0)
			accumulator = (accumulator ^ Hash(*data++)) * FNV32_PRIME;
		return accumulator;
	}

	/// @brief Hash a string
	/// @param string String
	/// @return Hash value
	static inline Hash FromString(const char *string)
	{
		Hash accumulator = FNV32_IV;
		while (*string != '\0')
			accumulator = (accumulator ^ Hash(*string++)) * FNV32_PRIME;
		return accumulator;
	}
}

/// @brief Hash literal operator
/// @param literal Literal string
/// @param length Literal length
/// @return Hash value
constexpr static inline Hash::Hash operator"" _h(const char *const literal, size_t length)
{
	return Hash::FromConst(literal, length);
}
