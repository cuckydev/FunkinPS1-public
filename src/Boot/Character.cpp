/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- Character.cpp -
	Character code
*/

#include "Boot/Character.h"

#include <CKSDK/TTY.h>

#include <memory>

// Animation static functions
KEEP uint32_t Character::Animation::GetFrameAt(const void *chr, uint32_t i, uint32_t j)
{
	// Get code pointer
	const uint32_t *chrp = (const uint32_t*)chr + 1;
	const uint16_t *codep = (const uint16_t*)((uintptr_t)chrp + chrp[i]);

	// Decode frame code
	return codep[j] & 0x1FF;
}

// Animation controller
KEEP void Character::Animation::Set(const void *chr, uint32_t i)
{
	// Get code pointer
	const uint32_t *chrp = (const uint32_t *)chr + 1;
	codep = (const uint16_t *)((uintptr_t)chrp + chrp[i]);

	// Start animation
	animation = i;

	dma = true;
	ended = false;
	timer = 0;
	Tick(0);
}

KEEP void Character::Animation::Tick(Timer::FixedTime time)
{
	// Decrement timer
	timer -= time;

	while (1)
	{
		// Check if timer expired
		if (timer > 0)
			break;

		// Process codes
		while (1)
		{
			uint16_t code = *codep;
			uint32_t op = (code >> 14) & 3;

			if (op == 0) // Frame
			{
				uint32_t next_frame = code & 0x1FF;
				uint32_t last_frame = frame;
				frame = next_frame;
				timer += Timer::FixedTime(1.0 / 24.0) * ((code >> 9) & 0x1F);
				codep++;
				if (next_frame != last_frame)
					dma = true;
				break;
			}
			else
			{
				switch (op)
				{
					case 1: // Back
					{
						ended = true;
						codep -= (code & 0x3FFF);
						break;
					}
				}
			}
		}
	}
}

// Character static functions
KEEP void Character::Draw(int32_t x, int32_t y, size_t ot, const void *msh, Color color)
{
	// Set GTE transform
	gte_ldtx(x);
	gte_ldty(y);

	// Get primitive word
	uint32_t priw;
	if (color == Color::White())
		priw = (CKSDK::GPU::GP0_Poly | CKSDK::GPU::GP0_Poly_Quad | CKSDK::GPU::GP0_Poly_Tex | CKSDK::GPU::GP0_Poly_Raw | CKSDK::GPU::GP0_Poly_Semi) << 24; // 0x808080;
	else
		priw = ((CKSDK::GPU::GP0_Poly | CKSDK::GPU::GP0_Poly_Quad | CKSDK::GPU::GP0_Poly_Tex | CKSDK::GPU::GP0_Poly_Semi) << 24) | color.c;

	// Get buffer pointers
	CKSDK::GPU::Buffer *bufferp = CKSDK::GPU::g_bufferp;

	CKSDK::GPU::Word *prip = bufferp->prip;
	CKSDK::GPU::Tag *otp = &bufferp->GetOT(ot);
	CKSDK::GPU::Word *linkp = (CKSDK::GPU::Word*)otp->Ptr();

	// Get mesh pointer
	MeshHeader header = *(const MeshHeader*)msh;
	const MeshPoly *mshp = (const MeshPoly*)((uintptr_t)msh + sizeof(MeshHeader));

	// Transform and write primitives
	while (header.polys-- > 0)
	{
		// Link this primitive
		new (prip) CKSDK::GPU::Tag(linkp, 9);
		linkp = prip++;

		// Load first 3 vectors
		gte_ldv3c(&mshp->v[0]);

		// Begin transform
		gte_rtpt();

		// Copy poly to primitive buffer
		// Annoyingly, we have to coerce GCC into using two registers
		// by using two variables
		// Oh well..
		uint32_t c0, c1;

		prip[0] = priw;

		c0 = mshp->p[0];
		c1 = mshp->p[1];
		prip[2] = c0;
		prip[4] = c1;

		c0 = mshp->p[2];
		c1 = mshp->p[3];
		prip[6] = c0;
		prip[8] = c1;

		// Read transformation result
		gte_stsxy3(&prip[1], &prip[3], &prip[5]); // x0 y0 x1 y1 x2 y2

		// Transform last vertex
		gte_ldv0(&mshp->v[3]);
		gte_rtps();

		// Increment pointers
		prip += 9;
		mshp++;

		// Read transformation result
		gte_stsxy2(&prip[7 - 9]);
	}

	// Link primitives
	bufferp->prip = prip;
	new (otp) CKSDK::GPU::Tag(linkp, 0);
}

KEEP void Character::DMA(const void *dma)
{
	// Process DMAs
	const char *dmap = (const char*)dma;
	const uint32_t *dmad = (const uint32_t*)dma;

	uint32_t dmas = *dmad++;
	for (; dmas != 0; dmas--)
	{
		// Read DMA header
		uint32_t poff = dmad[0];
		uint32_t size = dmad[1];
		uint32_t bcr = dmad[2];
		uint32_t compress = dmad[3];
		uint32_t xy = dmad[4];
		uint32_t wh = dmad[5];
		dmad += 6;

		if (compress != 0)
		{
			// Decompress image
			std::unique_ptr<char[]> decbuf(new char[compress]);
			Compress::Decompress(dmap + poff, decbuf.get());
			CKSDK::GPU::DMAImage(decbuf.get(), xy, wh, bcr);
			CKSDK::GPU::QueueSync();
		}
		else
		{
			// Direct DMA image
			CKSDK::GPU::DMAImage(dmap + poff, xy, wh, bcr);
		}
	}
}

// Sprite static functions
KEEP void Sprite::Draw(int32_t x, int32_t y, size_t ot, const void *spr, Color color)
{
	// Get buffer pointers
	CKSDK::GPU::Buffer *bufferp = CKSDK::GPU::g_bufferp;

	CKSDK::GPU::Word *prip = bufferp->prip;
	CKSDK::GPU::Tag *otp = &bufferp->GetOT(ot);
	CKSDK::GPU::Word *linkp = (CKSDK::GPU::Word*)otp->Ptr();

	// Get primitive word
	uint32_t priw;
	if (color == Color::White())
		priw = (CKSDK::GPU::GP0_Rect | CKSDK::GPU::GP0_Rect_Semi | CKSDK::GPU::GP0_Rect_Tex | CKSDK::GPU::GP0_Rect_Raw) << 24;
	else
		priw = ((CKSDK::GPU::GP0_Rect | CKSDK::GPU::GP0_Rect_Semi | CKSDK::GPU::GP0_Rect_Tex) << 24) | color.c;

	// Get sprite pointer
	SpriteHeader header = *(const SpriteHeader*)spr;
	const SpriteSprite *sprp = (const SpriteSprite *)((uintptr_t)spr + sizeof(SpriteHeader));

	// Transform and write primitives
	while (header.sprites-- > 0)
	{
		// Link this primitive
		new (prip) CKSDK::GPU::Tag(linkp, 5);
		linkp = prip++;

		// First primitive is drawmode
		prip[0] = (CKSDK::GPU::GP0_DrawMode << 24) | sprp->s.tpage.tpage | (1 << 10);

		// Write sprite primitive
		int16_t sx = sprp->s.xy.s.x + x;
		int16_t sy = sprp->s.xy.s.y + y;

		prip[1] = (CKSDK::GPU::GP0_Rect | CKSDK::GPU::GP0_Rect_Semi | CKSDK::GPU::GP0_Rect_Tex | CKSDK::GPU::GP0_Rect_Raw) << 24;
		prip[2] = ((uint32_t)sy << 16) | (uint16_t)sx;
		prip[3] = sprp->s.uv.w;
		prip[4] = sprp->s.wh.w;

		// Increment pointers
		prip += 5;
		sprp++;
	}

	// Link primitives
	bufferp->prip = prip;
	new (otp) CKSDK::GPU::Tag(linkp, 0);
}
