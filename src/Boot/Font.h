#pragma once

#include "Boot/Funkin.h"

namespace Font
{
	// Operator to remap ASCII string to character indices
	template <typename CharT>
	constexpr static inline CharT bold_transform(CharT c)
	{
		if (c == '\0')
			return '\xFF';
		else if (c == ' ')
			return '\x00';
		else if (c >= 'a' && c <= 'z')
			return '\x01' + c - 'a';
		else if (c >= 'A' && c <= 'Z')
			return '\x01' + c - 'A';
	}

	static int32_t BoldWidth(const char *str)
	{
		int32_t w = 0;
		for (; *str != '\xFF'; str++, w += 14);
		return w;
	}

	static void BoldPrint(const void *bold_spr, const char *str, int32_t x, int32_t y)
	{
		// Draw characters
		x += 7;
		for (; *str != '\xFF'; str++, x += 14)
		{
			unsigned char c = *str;
			Sprite::Draw(x, y, OT::UI, bold_spr, c, Color::White());
		}
	}

	static void BoldPrintCenter(const void *bold_spr, const char *str, int32_t x, int32_t y)
	{
		BoldPrint(bold_spr, str, x - BoldWidth(str) / 2, y);
	}
}

template <typename CharT, CharT... Chars>
constexpr static inline const char *operator"" _bold()
{
	static constexpr char transformed[] = { Font::bold_transform<CharT>(Chars)..., '\xFF' };
	return transformed;
}
