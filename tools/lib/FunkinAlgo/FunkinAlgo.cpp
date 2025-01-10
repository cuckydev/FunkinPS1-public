/*
	[ FunkinAlgo ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- FunkinAlgo.cpp -
	Funkin image processing algorithms
*/

#include "FunkinAlgo.h"

#include <libimagequant.h>
#include <comper.h>

#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#undef STB_IMAGE_RESIZE_IMPLEMENTATION

// Quantization
void Quant::Generate(const Image &in, const RGBA *fixed, int in_l, int in_t, int in_r, int in_b, bool highbpp, bool dither)
{
	// Allocate new image
	w = in_r - in_l;
	h = in_b - in_t;
	if (w < 0 || h < 0)
		throw RuntimeError("No rect given");
	image.reset(new uint8_t[w * h]{});

	// Crop image
	std::unique_ptr<RGBA[]> crop(new RGBA[w * h]);
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
			crop[y * w + x] = in.image[(in_t + y) * in.w + (in_l + x)];

	// Create liq image
	unsigned tgt_cols = highbpp ? 256 : 16;

	liq_attr *quant_attr = liq_attr_create();
	if (quant_attr == nullptr ||
		liq_set_max_colors(quant_attr, tgt_cols) != LIQ_OK ||
		0) // liq_set_min_posterization(quant_attr, 3) != LIQ_OK)
		throw RuntimeError("Failed to set quantization attributes");

	liq_image *quant_image = liq_image_create_rgba(quant_attr, crop.get(), w, h, 0);
	if (quant_image == nullptr)
		throw RuntimeError("Failed to create quantization image");
	
	if (fixed != nullptr)
	{
		for (unsigned i = 0; i < tgt_cols; i++)
			liq_image_add_fixed_color(quant_image, *((const liq_color*)&fixed[i]));
	}

	// Quantize image
	liq_result *quant_result;
	if (liq_image_quantize(quant_image, quant_attr, &quant_result) != LIQ_OK)
		throw RuntimeError("Failed to quantize image");
	
	// Write out image
	if (liq_set_dithering_level(quant_result, dither ? 1.0 : 0.0) != LIQ_OK ||
		liq_write_remapped_image(quant_result, quant_image, image.get(), w * h) != LIQ_OK)
		throw RuntimeError("Failed to write out quantized image");

	if (fixed != nullptr)
	{
		// Remap image
		const liq_palette *quant_palette = liq_get_palette(quant_result);
		if (quant_palette == nullptr)
			throw RuntimeError("Failed to write out quantized palette");
		memcpy(palette, fixed, 256 * 4);
				
		for (int i = 0; i < (w * h); i++)
		{
			uint8_t f = image[i];
			uint8_t s = 0;
			liq_color cump = quant_palette->entries[f];
			for (unsigned j = 0; j < tgt_cols; j++)
			{
				if (fixed[j] == *((RGBA*)&cump))
				{
					s = j;
					break;
				}
			}
			image[i] = s;
		}
	}
	else
	{
		// Write out palette
		const liq_palette *quant_palette = liq_get_palette(quant_result);
		if (quant_palette == nullptr)
			throw RuntimeError("Failed to write out quantized palette");
		memcpy(palette, quant_palette->entries, 256 * 4);
	}

	// Release
	liq_result_destroy(quant_result);
	liq_image_destroy(quant_image);
	liq_attr_destroy(quant_attr);
}

// Algorithm
void Algo::Generate(const Image &in, bool semi, int in_l, int in_t, int in_r, int in_b, int a_x, int a_y, float scale)
{
	if (scale == 1.0f)
	{
		// Copy image
		anchor_x = a_x;
		anchor_y = a_y;

		int in_w = in_r - in_l;
		int in_h = in_b - in_t;
		image.w = in_w;
		image.h = in_h;
		image.image.reset(new RGBA[in_w * in_h]);
		for (int y = 0; y < in_h; y++)
			for (int x = 0; x < in_w; x++)
				image.image[(y * in_w) + x] = in.image[((in_t + y) * in.w) + (in_l + x)];
	}
	else
	{
		// Adjust anchor
		float a_x_sub = std::fmod(a_x * scale, 1.0f);
		float a_y_sub = std::fmod(a_y * scale, 1.0f);
		anchor_x = int(a_x * scale);
		anchor_y = int(a_y * scale);

		// Get resize bounds
		int in_w = in_r - in_l;
		int in_h = in_b - in_t;
		int out_w = std::ceil(in_w * scale);
		int out_h = std::ceil(in_h * scale);

		// Resize image
		image.w = out_w;
		image.h = out_h;
		image.image.reset(new RGBA[out_w * out_h]);
		stbir_resize_subpixel(in.image.get() + (in_t * in.w + in_l), in_w, in_h, in.w * 4, image.image.get(), out_w, out_h, out_w * 4, STBIR_TYPE_UINT8, 4, 3, 0, STBIR_EDGE_ZERO, STBIR_EDGE_ZERO, STBIR_FILTER_CATMULLROM, STBIR_FILTER_CATMULLROM, STBIR_COLORSPACE_SRGB, nullptr, scale, scale, a_x_sub, a_y_sub);
	}

	if (!semi)
	{
		for (int i = 0; i < (image.w * image.h); i++)
		{
			// Quantize
			if (image.image[i].a > 0x40)
				image.image[i].a = 0xFF;
			else
				image.image[i].a = 0x00;
		}
	}
}

// Cropper algorithm
void Cropper::Compile(int tx, int ty, int w, int h)
{
	// Get page
	if ((tx & 0xFF) == 0xFF)
		tx++;
	if ((ty & 0xFF) == 0xFF)
		ty++;
	int px = tx >> 8;
	int py = ty >> 8;
	
	for (int sx = (tx & 0xFF), sw = w, spx = px; sw != 0;)
	{
		// Get crop
		int cx = w - sw;
		int cw = 0xFF - sx;
		if (cw > sw)
			cw = sw;

		for (int sy = (ty & 0xFF), sh = h, spy = py; sh != 0;)
		{
			// Get crop
			int cy = h - sh;
			int ch = 0xFF - sy;
			if (ch > sh)
				ch = sh;
			
			// Push crop
			crops.emplace_back(Crop{
				tx, ty,
				spx, spy,
				sx, sy,
				cx, cy,
				cw, ch
			});
			
			// Crop window
			sy = 0;
			sh -= ch;
			spy++;
		}

		// Crop window
		sx = 0;
		sw -= cw;
		spx++;
	}
}

// Anim function
void Anim::Out(std::vector<Anim> &anims, std::ostream &stream)
{
	uint32_t poff = 4 * anims.size();
	for (auto &i : anims)
	{
		Write32(stream, poff);
		poff += i.codes.size() * 2;
	}
	for (auto &i : anims)
		for (auto &j : i.codes)
			Write16(stream, j);
}

size_t Anim::Size(std::vector<Anim> &anims)
{
	size_t size = 4 * anims.size();
	for (auto &i : anims)
		size += i.codes.size() * 2;
	return size;
}

// DMa function
DMA DMA::Image(const Quant &quant, Crop crop, bool highbpp)
{
	uint32_t tpage = crop.GetTPage(highbpp);
	uint32_t tpage_x = tpage & 0xF;
	uint32_t tpage_y = tpage >> 4;

	uint32_t vram_x, vram_w;
	if (highbpp)
	{
		if ((crop.sx & 1) != 0)
			throw RuntimeError("Bad crop sx for DMA::Image");
		vram_x = (tpage_x << 6) + (crop.sx >> 1);
		vram_w = (crop.cw + 1) >> 1;
	}
	else
	{
		if ((crop.sx & 3) != 0)
			throw RuntimeError("Bad crop sx for DMA::Image");
		vram_x = (tpage_x << 6) + (crop.sx >> 2);
		vram_w = (crop.cw + 3) >> 2;
	}
	vram_w += (vram_w & 1); // Old PS1 GPUs may have a bug with odd DMA transfers maybe i dont know it's unknown (??)

	uint32_t vram_y = (tpage_y << 8) + crop.sy;
	uint32_t vram_h = crop.ch;

	DMA dma;
	dma.x = vram_x;
	dma.y = vram_y;
	dma.w = vram_w;
	dma.h = vram_h;

	dma.size = (vram_w * 2) * vram_h;
	dma.data.reset(new uint8_t[dma.size]{});

	if (highbpp)
	{
		for (uint32_t y = 0, srcy = crop.cy; y < crop.ch; y++, srcy++)
			for (uint32_t x = 0, srcx = crop.cx; x < crop.cw; x++, srcx++)
				dma.data[(y * vram_w * 2) + x] = quant.image[(srcy * quant.w) + srcx];
	}
	else
	{
		for (uint32_t y = 0, srcy = crop.cy; y < crop.ch; y++, srcy++)
			for (uint32_t x = 0, srcx = crop.cx; x < crop.cw; x++, srcx++)
				dma.data[(y * vram_w * 2) + (x >> 1)] |= (quant.image[(srcy * quant.w) + srcx] << ((x & 1) ? 4 : 0));
	}

	dma.AlignBCR();
	return dma;
}

DMA DMA::Palette(const Quant &quant, bool highbpp)
{
	uint32_t cols = highbpp ? 256 : 16;

	DMA dma;
	dma.w = cols;
	dma.h = 1;
	dma.size = cols * 2;
	dma.data.reset(new uint8_t[dma.size]);

	uint8_t *clutp = dma.data.get();
	for (uint32_t i = 0; i < cols; i++)
	{
		uint16_t col = quant.palette[i].ToPS1();
		*clutp++ = col >> 0;
		*clutp++ = col >> 8;
	}

	dma.AlignBCR();
	return dma;
}

void DMA::AlignBCR()
{
	// Calculate BCR
	bcr = w * h;
	bcr += bcr & 1;
	bcr >>= 1;

	uint32_t bs = 1;
	while (((bcr & 1) == 0) && (bs < 16))
	{
		bs <<= 1;
		bcr >>= 1;
	}

	if (bcr >= 65536)
		throw RuntimeError("Transfer too big");
	bcr <<= 16;
	bcr |= bs;

	// Align size
	uint32_t osize = size;
	size = ((bcr >> 16) * (bcr & 0xFFFF)) * 4; // ((size + 15) / 16) * 16;
	if (osize > size)
		throw RuntimeError("Transfer smaller than data size");

	uint8_t *newdata = new uint8_t[size]{};
	memcpy(newdata, data.get(), osize);
	data.reset(newdata);

}

void DMA::Compress()
{
	uint32_t osize = size;
	size_t comper_size;
	unsigned char *comper = ClownLZSS_ComperCompress(data.get(), size, &comper_size);
	data.reset(new uint8_t[comper_size]);
	size = comper_size;
	compress = osize;
	memcpy(data.get(), comper, comper_size);
	free(comper);
}

void DMA::Out(std::vector<DMA> &dmas, std::ostream &stream)
{
	Write32(stream, dmas.size());

	uint32_t poff = 4 + (dmas.size() * (4 * 6));
	for (auto &i : dmas)
	{
		Write32(stream, poff);
		size_t isize = i.size;
		if (isize & 0xF)
			isize += 0x10 - (isize & 0xF);
		poff += isize;
		Write32(stream, isize);
		Write32(stream, i.bcr);
		Write32(stream, i.compress);
		Write16(stream, i.x); Write16(stream, i.y);
		Write16(stream, i.w); Write16(stream, i.h);
	}

	for (auto &i : dmas)
	{
		stream.write((char *)i.data.get(), i.size);
		if (i.size & 0xF)
			for (size_t j = 0; j < (0x10 - (i.size & 0xF)); j++)
				stream.put(0);
	}
}

size_t DMA::Size(std::vector<DMA> &dmas)
{
	size_t size = 4 + (dmas.size() * (4 * 6));
	for (auto &i : dmas)
	{
		size_t isize = i.size;
		if (isize & 0xF)
			isize += 0x10 - (isize & 0xF);
		size += isize;
	}
	return size;
}


// Mesh function
void Mesh::Compile(const Quant &in, bool compress, bool highbpp, int semi, int ax, int ay, int tx, int ty, int clutx, int cluty)
{
	// Generate crops
	if (highbpp)
		tx <<= 1;
	else
		tx <<= 2;
	
	Cropper cropper;
	cropper.Compile(tx, ty, in.w, in.h);

	// Crop images
	for (auto &i : cropper.crops)
	{
		// Create polygon
		{
			Poly poly;
			poly.poly.tpage = i.GetTPage(highbpp);
			if (highbpp)
				poly.poly.tpage |= (1 << 7);
			if (semi >= 0)
				poly.poly.tpage |= (semi << 5);
			poly.poly.clut = (cluty * (1024 / 16)) + (clutx / 16);

			poly.v0.x = i.cx - ax;
			poly.v0.y = i.cy - ay;
			poly.v0.z = 0;
			poly.poly.u0 = i.sx;
			poly.poly.v0 = i.sy;

			poly.v1.x = (i.cx + i.cw) - ax;
			poly.v1.y = i.cy - ay;
			poly.v1.z = 0;
			poly.poly.u1 = i.sx + i.cw;
			poly.poly.v1 = i.sy;

			poly.v2.x = i.cx - ax;
			poly.v2.y = (i.cy + i.ch) - ay;
			poly.v2.z = 0;
			poly.poly.u2 = i.sx;
			poly.poly.v2 = i.sy + i.ch;

			poly.v3.x = (i.cx + i.cw) - ax;
			poly.v3.y = (i.cy + i.ch) - ay;
			poly.v3.z = 0;
			poly.poly.u3 = i.sx + i.cw;
			poly.poly.v3 = i.sy + i.ch;

			polys.push_back(poly);
		}

		// DMA image
		{
			DMA image = std::move(DMA::Image(in, i, highbpp));
			if (compress)
				image.Compress();
			dmas.push_back(std::move(image));
		}
	}
	
	// DMA palette
	DMA palette = std::move(DMA::Palette(in, highbpp));
	palette.x = clutx;
	palette.y = cluty;
	dmas.push_back(std::move(palette));
}

void Mesh::Out(std::ostream &stream)
{
	Write32(stream, polys.size());
	
	for (auto &i : polys)
	{
		Write8(stream, i.poly.u0);
		Write8(stream, i.poly.v0);
		Write16(stream, i.poly.clut);

		Write8(stream, i.poly.u1);
		Write8(stream, i.poly.v1);
		Write16(stream, i.poly.tpage);

		Write8(stream, i.poly.u2);
		Write8(stream, i.poly.v2);
		Write16(stream, i.poly.pad1);

		Write8(stream, i.poly.u3);
		Write8(stream, i.poly.v3);
		Write16(stream, i.poly.pad2);
		
		Write16(stream, i.v0.x);
		Write16(stream, i.v0.y);

		Write16(stream, i.v0.z);
		Write16(stream, i.v0.pad);
		
		Write16(stream, i.v1.x);
		Write16(stream, i.v1.y);

		Write16(stream, i.v1.z);
		Write16(stream, i.v1.pad);
		
		Write16(stream, i.v2.x);
		Write16(stream, i.v2.y);

		Write16(stream, i.v2.z);
		Write16(stream, i.v2.pad);
		
		Write16(stream, i.v3.x);
		Write16(stream, i.v3.y);

		Write16(stream, i.v3.z);
		Write16(stream, i.v3.pad);
	}
}

size_t Mesh::Size()
{
	size_t size = 4 + (polys.size() * sizeof(Poly));
	return size;
}

// Sprite function
void Sprites::Compile(const Quant &in, bool compress, bool highbpp, int semi, int ax, int ay, int tx, int ty, int clutx, int cluty)
{
	// Generate crops
	if (highbpp)
		tx <<= 1;
	else
		tx <<= 2;

	Cropper cropper;
	cropper.Compile(tx, ty, in.w, in.h);

	// Crop images
	for (auto &i : cropper.crops)
	{
		// Create polygon
		{
			Sprite sprite;
			sprite.tpage = i.GetTPage(highbpp);
			if (highbpp)
				sprite.tpage |= (1 << 7);
			if (semi >= 0)
				sprite.tpage |= (semi << 5);
			sprite.clut = (cluty * (1024 / 16)) + (clutx / 16);

			sprite.x = i.cx - ax;
			sprite.y = i.cy - ay;
			sprite.w = i.cw;
			sprite.h = i.ch;

			sprite.u = i.sx;
			sprite.v = i.sy;

			sprites.push_back(sprite);
		}

		// DMA image
		{
			DMA image = std::move(DMA::Image(in, i, highbpp));
			if (compress)
				image.Compress();
			dmas.push_back(std::move(image));
		}
	}

	// DMA palette
	DMA palette = std::move(DMA::Palette(in, highbpp));
	palette.x = clutx;
	palette.y = cluty;
	dmas.push_back(std::move(palette));
}

void Sprites::Out(std::ostream &stream)
{
	Write32(stream, sprites.size());

	for (auto &i : sprites)
	{
		Write16(stream, i.tpage);
		Write16(stream, i.pad);
		Write16(stream, i.x);
		Write16(stream, i.y);
		Write8(stream, i.u);
		Write8(stream, i.v);
		Write16(stream, i.clut);
		Write16(stream, i.w);
		Write16(stream, i.h);
	}
}

size_t Sprites::Size()
{
	size_t size = 4 + (sprites.size() * sizeof(Sprite));
	return size;
}
