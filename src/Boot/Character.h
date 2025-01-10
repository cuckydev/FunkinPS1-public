/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- Character.h -
	Character code
*/

#pragma once

#include <CKSDK/CKSDK.h>
#include <CKSDK/GPU.h>
#include <CKSDK/Util/Fixed.h>

#include "Boot/Timer.h"
#include "Boot/Compress.h"

// Color helper
struct Color
{
	private:
		friend class Character;
		friend class Sprite;
		uint32_t c;

		constexpr Color(uint32_t c) : c(c) {}

	public:
		// Constructors
		constexpr Color() {}

		static constexpr Color RGB(int r, int g, int b) { return Color(((((r + 1) >> 1) & 0xFF) << 16) | ((((g + 1) >> 1) & 0xFF) << 8) | (((b + 1) >> 1) & 0xFF)); }
		static constexpr Color White() { return Color(0x808080); }

		// Comparison
		bool operator==(const Color &other) const { return c == other.c; }
};

// Character class
class Character
{
	public:
		// Animation controller
		class Animation
		{
			private:
				// Animation state
				const uint16_t *codep;
				Timer::FixedTime timer;

				uint32_t animation;
				uint32_t frame;

				bool dma, ended;

			public:
				// Static animation functions
				static uint32_t GetFrameAt(const void *chr, uint32_t i, uint32_t j);
				static const void *GetMeshAt(const void *chr, uint32_t i, uint32_t j)
				{
					return GetMesh(chr, GetFrameAt(chr, i, j));
				}

				// Animation functions
				void Set(const void *chr, uint32_t i);
				uint32_t Get() const { return animation; }

				void Tick(Timer::FixedTime time);
				
				uint32_t GetFrame() const { return frame; }
				
				void SetEnded() { ended = true; }
				bool GetEnded() const { return ended; }
				
				bool DMA() { bool result = dma; dma = false; return result; }
		};

		// Character structures
		struct MeshHeader
		{
			uint32_t polys;
		};
		static_assert(sizeof(MeshHeader) == 4);
		struct MeshPoly
		{
			union
			{
				struct
				{
					CKSDK::GPU::TexCoord uv0;
					CKSDK::GPU::TexCoord uv1;
					CKSDK::GPU::TexCoord uv2;
					CKSDK::GPU::TexCoord uv3;
				} s;
				CKSDK::GPU::Word p[4];
			};
			CKSDK::GPU::SVector v[4];

			MeshPoly() {}
			MeshPoly(const MeshPoly &other) : p{ other.p[0], other.p[1], other.p[2], other.p[3] }, v{ other.v[0], other.v[1], other.v[2], other.v[3] } {}
			MeshPoly &operator=(const MeshPoly &other) { p[0] = other.p[0]; p[1] = other.p[1]; p[2] = other.p[2]; p[3] = other.p[3]; v[0] = other.v[0]; v[1] = other.v[1]; v[2] = other.v[2]; v[3] = other.v[3]; return *this; }
		};
		static_assert(sizeof(MeshPoly) == (4 * (4 + (4 * 2))));

	private:
		// Assigned character and animation
		const void *chr;
		Animation animation;

	public:
		// Constructor
		Character() {}
		Character(const void *chr) : chr(chr) {}
		Character &operator=(const void *chr) { this->chr = chr; return *this; }

		// Static character functions
		static void Draw(int32_t x, int32_t y, size_t ot, const void *msh, Color color);
		
		static const void *GetMesh(const void *chr, uint32_t frame)
		{
			// Get mesh pointer
			const uint32_t *chrp = (uint32_t*)chr;
			chrp = (const uint32_t*)((uintptr_t)chrp + chrp[0]);
			return (const void*)((uintptr_t)chrp + chrp[frame * 2]);
		}

		static void Draw(int32_t x, int32_t y, size_t ot, const void *chr, uint32_t frame, Color color)
		{
			// Draw mesh
			Draw(x, y, ot, GetMesh(chr, frame), color);
		}

		static void DMA(const void *dma);
		static void DMA(const void *chr, uint32_t frame)
		{
			// Get DMA pointer
			const uint32_t *chrp = (const uint32_t*)chr;
			chrp = (const uint32_t*)((uintptr_t)chrp + chrp[0]);

			uint32_t dmao = chrp[frame * 2 + 1];
			if (dmao == 0)
				return;

			const uint32_t *dmap = (const uint32_t*)((uintptr_t)chrp + dmao);

			// Issue DMA
			DMA(dmap);
		}

		// Character functions
		void SetAnimation(uint32_t i)
		{
			// Set animation index
			animation.Set(chr, i);
		}

		uint32_t GetAnimation() const
		{
			// Get animation index
			return animation.Get();
		}

		void SetAnimationEnded()
		{
			// Set animation ended
			animation.SetEnded();
		}

		bool GetAnimationEnded()
		{
			// Get animation ended
			return animation.GetEnded();
		}

		void Tick(Timer::FixedTime dt)
		{
			// Tick animation and DMA if needed
			animation.Tick(dt);
		}

		void Draw(int32_t x, int32_t y, size_t ot, Color color = Color::White())
		{
			// Draw character
			if (animation.DMA())
				DMA(chr, animation.GetFrame());
			Draw(x, y, ot, chr, animation.GetFrame(), color);
		}
};

// Sprite class
class Sprite
{
	public:
		// Assigned sprite and animation
		const void *spr;
		Character::Animation animation;

		// Sprite structures
		struct SpriteHeader
		{
			uint32_t sprites;
		};

		struct SpriteSprite
		{
			union
			{
				struct
				{
					CKSDK::GPU::TexPage tpage; uint16_t pad;
					CKSDK::GPU::ScreenCoord xy;
					CKSDK::GPU::TexCoord uv;
					CKSDK::GPU::ScreenDim wh;
				} s;
				CKSDK::GPU::Word p[4];
			};
		};
		static_assert(sizeof(SpriteSprite) == (4 * 4));

	public:
		// Constructor
		Sprite() {}
		Sprite(const void *spr) : spr(spr) {}
		Sprite &operator=(const void *spr) { this->spr = spr; return *this; }

		// Static sprite functions
		static void Draw(int32_t x, int32_t y, size_t ot, const void *spr, Color color);

		static const void *GetSprite(const void *spr, uint32_t frame)
		{
			// Get sprite pointer
			const uint32_t *sprp = (const uint32_t*)spr;
			sprp = (const uint32_t*)((uintptr_t)sprp + sprp[0]);
			return (const void*)((uintptr_t)sprp + sprp[frame * 2]);
		}

		static void Draw(int32_t x, int32_t y, size_t ot, const void *spr, uint32_t frame, Color color)
		{
			// Draw mesh
			Draw(x, y, ot, GetSprite(spr, frame), color);
		}

		static void DMA(const void *dma) { Character::DMA(dma); }
		static void DMA(const void *chr, uint32_t frame) { Character::DMA(chr, frame); }

		// Sprite functions
		void SetAnimation(uint32_t i)
		{
			// Set animation index
			animation.Set(spr, i);
		}

		uint32_t GetAnimation() const
		{
			// Get animation index
			return animation.Get();
		}

		void SetAnimationEnded()
		{
			// Set animation ended
			animation.SetEnded();
		}

		bool GetAnimationEnded()
		{
			// Get animation ended
			return animation.GetEnded();
		}

		void Tick(Timer::FixedTime dt)
		{
			// Tick animation and DMA if needed
			animation.Tick(dt);
		}

		void Draw(int32_t x, int32_t y, size_t ot, Color color = Color::White())
		{
			// Draw character
			if (animation.DMA())
				DMA(spr, animation.GetFrame());
			Draw(x, y, ot, spr, animation.GetFrame(), color);
		}
};
