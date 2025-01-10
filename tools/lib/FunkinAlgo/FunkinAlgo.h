/*
	[ FunkinAlgo ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- FunkinAlgo.h -
	Funkin image processing algorithms
*/

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <memory>

#include "stb_image.h"

// Constants
static const unsigned int TILE_DIM = 32;
static const unsigned int TILE_FIT = (256 / TILE_DIM) - 1;

// Exception types
class RuntimeError : public std::runtime_error
{
	public:
		RuntimeError(std::string what_arg = "") : std::runtime_error(what_arg) {}
};

// Writes
static void Write8(std::ostream &stream, uint8_t x)
{
	stream.put(char(x >> 0));
}

static void Write16(std::ostream &stream, uint16_t x)
{
	stream.put(char(x >> 0));
	stream.put(char(x >> 8));
}

static void Write32(std::ostream &stream, uint32_t x)
{
	stream.put(char(x >> 0));
	stream.put(char(x >> 8));
	stream.put(char(x >> 16));
	stream.put(char(x >> 24));
}

// String hashing
namespace Hash
{
	typedef uint32_t Hash;

	static const Hash FNV32_PRIME = 0x01000193;
	static const Hash FNV32_IV    = 0x811C9DC5;

	constexpr static inline Hash FromConst(const char *const literal, size_t max_length = 0xFFFFFFFF, Hash accumulator = FNV32_IV)
	{
		if (*literal && max_length)
			return FromConst(&literal[1], max_length - 1, (accumulator ^ (Hash)(*literal)) * FNV32_PRIME);
		return accumulator;
	}

	static inline Hash FromBuffer(const uint8_t *data, size_t length)
	{
		Hash accumulator = FNV32_IV;
		while (length-- > 0)
			accumulator = (accumulator ^ (Hash)(*data++)) * FNV32_PRIME;
		return accumulator;
	}

	static inline Hash FromString(const char *string)
	{
		Hash accumulator = FNV32_IV;
		while (*string != '\0')
			accumulator = (accumulator ^ (Hash)(*string++)) * FNV32_PRIME;
		return accumulator;
	}
}

constexpr static inline Hash::Hash operator"" _h(const char *const literal, size_t length)
{
	return Hash::FromConst(literal, length);
}

// Image
struct RGBA
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;

	bool operator==(RGBA _x) const
	{ return r == _x.r && g == _x.g && b == _x.b && a == _x.a; }
	
	uint16_t ToPS1() const
	{
		// Return transparent
		if (a < 0x40)
			return 0;

		// 8-bit to 5-bit
		// https://github.com/stenzek/duckstation/blob/master/src/core/gpu_types.h#L135
		// https://stackoverflow.com/a/9069480
		uint16_t cr = (((uint16_t)r * 249) + 1014) >> 11;
		uint16_t cg = (((uint16_t)g * 249) + 1014) >> 11;
		uint16_t cb = (((uint16_t)b * 249) + 1014) >> 11;

		// Translucency 
		uint16_t v = 0;
		if (a < 0xC0)
			v |= 0x8000;
		else if ((cr | cg | cb) == 0)
			return (1 << 10); // Our eyes don't see blue as well as green or red
		
		// Out
		v |= cr << 0;
		v |= cg << 5;
		v |= cb << 10;
		return v;
	}
};
static_assert(sizeof(RGBA) == 4);

struct Image
{
	int w = 0, h = 0;
	std::unique_ptr<RGBA[]> image;

	void Decode(std::string name)
	{
		int c = 0;
		stbi_uc *image_data = stbi_load(name.c_str(), &w, &h, &c, 0);
		if (image_data == nullptr)
			throw RuntimeError(std::string("Failed to decode ") + name);
		image.reset(new RGBA[w * h]);
		std::cout << "comp is " << c << std::endl;
		switch (c)
		{
			case 0:
				throw RuntimeError(std::string("Failed to decode (comp is 0) ") + name);
			case 1:
			{
				for (int i = 0; i < w * h; i++)
				{
					image[i].r = image_data[i];
					image[i].g = image_data[i];
					image[i].b = image_data[i];
					image[i].a = 0xFF;
				}
				break;
			}
			case 2:
			{
				for (int i = 0; i < w * h; i++)
				{
					image[i].r = image_data[i * 2 + 0];
					image[i].g = image_data[i * 2 + 0];
					image[i].b = image_data[i * 2 + 0];
					image[i].a = image_data[i * 2 + 1];
				}
				break;
			}
			case 3:
			{
				for (int i = 0; i < w * h; i++)
				{
					image[i].r = image_data[i * 3 + 0];
					image[i].g = image_data[i * 3 + 1];
					image[i].b = image_data[i * 3 + 2];
					image[i].a = 0xFF;
				}
				break;
			}
			case 4:
			{
				for (int i = 0; i < w * h; i++)
				{
					image[i].r = image_data[i * 4 + 0];
					image[i].g = image_data[i * 4 + 1];
					image[i].b = image_data[i * 4 + 2];
					image[i].a = image_data[i * 4 + 3];
				}
				break;
			}
		}
		// memcpy(image.get(), image_data, w * h * 4);
		stbi_image_free(image_data);
	}

	void Flip()
	{
		RGBA *p = image.get();
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w / 2; x++)
			{
				int mx = w - x - 1;
				if (mx < 0 || mx >= w)
					throw RuntimeError("ummm");
				RGBA temp = p[x];
				p[x] = p[mx];
				p[mx] = temp;
			}
			p += w;
		}
	}
};

// Quantization
class Quant
{
	public:
		// Image
		int w = 0, h = 0;
		std::unique_ptr<uint8_t[]> image;
		RGBA palette[256];

	public:
		// Quant functions
		void Generate(const Image &in, const RGBA *fixed, int in_l, int in_t, int in_r, int in_b, bool highbpp, bool dither);
};

// Algorithms
class Algo
{
	public:
		// Image
		Image image;
		int anchor_x, anchor_y;

	public:
		// Algo functions
		void Generate(const Image &in, bool semi, int in_l, int in_t, int in_r, int in_b, int a_x, int a_y, float scale);

		void Flip()
		{
			anchor_x = image.w - anchor_x;
			image.Flip();
			// if (image.w & 1)
			//	anchor_x--;
		}
};

// Cropper algo
struct Crop
{
	int tx, ty; // VRAM coordinate
	int px, py; // Page coordinate
	int sx, sy; // Texture page coordinate
	int cx, cy; // Image crop coordinate
	int cw, ch; // Image crop size

	uint32_t GetTPage(bool highbpp)
	{
		uint32_t tpage_x = px;
		if (highbpp)
			tpage_x <<= 1;
		uint32_t tpage_y = py;
		uint32_t tpage = (tpage_y << 4) | tpage_x;
		return tpage;
	}
};

class Cropper
{
	public:
		std::vector<Crop> crops;

	public:
		void Compile(int tx, int ty, int w, int h);
};

// Mesh
struct Vector
{
	int16_t x, y, z, pad = 0;
};

struct Poly
{
	struct
	{
		uint8_t u0, v0; uint16_t clut;
		uint8_t u1, v1; uint16_t tpage;
		uint8_t u2, v2; uint16_t pad1 = 0;
		uint8_t u3, v3; uint16_t pad2 = 0;
	} poly;
	Vector v0, v1, v2, v3;
};
static_assert(sizeof(Poly) == (4 * (4 + (4 * 2))));

struct DMA
{
	uint32_t x = 0, y = 0, w = 0, h = 0;
	uint32_t compress = 0;
	std::unique_ptr<uint8_t[]> data;
	uint32_t size = 0, bcr = 0;

	static DMA Image(const Quant &quant, Crop crop, bool highbpp);
	static DMA Palette(const Quant &quant, bool highbpp);

	void AlignBCR();

	void Compress();
	
	static void Out(std::vector<DMA> &dmas, std::ostream &stream);
	static size_t Size(std::vector<DMA> &dmas);
};

class Anim
{
	public:
		std::vector<uint16_t> codes;

	public:
		void Frame(unsigned i, unsigned length) { if (!length) length++; codes.push_back((0 << 14) | (length << 9) | i); }
		void Back(unsigned length) { codes.push_back((1 << 14) | length); }
		void End() { Back(1); }

		static void Out(std::vector<Anim> &anims, std::ostream &stream);
		static size_t Size(std::vector<Anim> &anims);
};

class Mesh
{
	public:
		// Data
		std::vector<Poly> polys;
		std::vector<DMA> dmas;

	public:
		// Mesh function
		void Compile(const Quant &in, bool compress, bool highbpp, int semi, int ax, int ay, int tx, int ty, int clutx, int cluty);
		void Out(std::ostream &stream);
		size_t Size();
};

// Sprites
struct Sprite
{
	uint16_t tpage; uint16_t pad = 0;
	int16_t x, y;
	uint8_t u, v; uint16_t clut;
	uint16_t w, h;
};

class Sprites
{
	public:
		// Data
		std::vector<Sprite> sprites;
		std::vector<DMA> dmas;

	public:
		// Sprites functions
		void Compile(const Quant &in, bool compress, bool highbpp, int semi, int ax, int ay, int tx, int ty, int clutx, int cluty);
		void Out(std::ostream &stream);
		size_t Size();
};
