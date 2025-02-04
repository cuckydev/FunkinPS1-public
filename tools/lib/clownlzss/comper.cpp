/*
	(C) 2018-2021 Clownacy
	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.
	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
	1. The origin of this software must not be misrepresented; you must not
	   claim that you wrote the original software. If you use this software
	   in a product, an acknowledgment in the product documentation would be
	   appreciated but is not required.
	2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.
*/

/* Modified 32-bit little-endian version by Clownacy */

#include "comper.h"

#include <assert.h>
#include <stddef.h>

#include "clowncommon.h"

#include "clownlzss.h"
#include "common.h"
#include "memory_stream.h"

#define TOTAL_DESCRIPTOR_BITS 32

typedef struct ComperInstance
{
	MemoryStream *output_stream;
	MemoryStream match_stream;

	unsigned long descriptor;
	unsigned int descriptor_bits_remaining;
} ComperInstance;

static void FlushData(ComperInstance *instance)
{
	size_t match_buffer_size;
	unsigned char *match_buffer;

	MemoryStream_WriteByte(instance->output_stream, (instance->descriptor >> (8 * 0)) & 0xFF);
	MemoryStream_WriteByte(instance->output_stream, (instance->descriptor >> (8 * 1)) & 0xFF);
	MemoryStream_WriteByte(instance->output_stream, (instance->descriptor >> (8 * 2)) & 0xFF);
	MemoryStream_WriteByte(instance->output_stream, (instance->descriptor >> (8 * 3)) & 0xFF);

	match_buffer_size = MemoryStream_GetPosition(&instance->match_stream);
	match_buffer = MemoryStream_GetBuffer(&instance->match_stream);

	MemoryStream_Write(instance->output_stream, match_buffer, 1, match_buffer_size);
}

static void PutMatchByte(ComperInstance *instance, unsigned int byte)
{
	MemoryStream_WriteByte(&instance->match_stream, byte);
}

static void PutDescriptorBit(ComperInstance *instance, cc_bool bit)
{
	assert(bit == 0 || bit == 1);

	if (instance->descriptor_bits_remaining == 0)
	{
		FlushData(instance);

		instance->descriptor_bits_remaining = TOTAL_DESCRIPTOR_BITS;
		MemoryStream_Rewind(&instance->match_stream);
	}

	--instance->descriptor_bits_remaining;

	instance->descriptor <<= 1;

	instance->descriptor |= bit;
}

static void DoLiteral(const unsigned char *value, void *user)
{
	ComperInstance *instance = (ComperInstance*)user;

	PutDescriptorBit(instance, 0);
	PutMatchByte(instance, value[0]);
	PutMatchByte(instance, value[1]);
	PutMatchByte(instance, value[2]);
	PutMatchByte(instance, value[3]);
}

static void DoMatch(size_t distance, size_t length, size_t offset, void *user)
{
	ComperInstance *instance = (ComperInstance*)user;

	(void)offset;

	PutDescriptorBit(instance, 1);
	PutMatchByte(instance, ((-distance << 2) >> 0) & 0xFF);
	PutMatchByte(instance, ((-distance << 2) >> 8) & 0xFF);
	PutMatchByte(instance, ((length - 1) >> 0) & 0xFF);
	PutMatchByte(instance, ((length - 1) >> 8) & 0xFF);
}

static size_t GetMatchCost(size_t distance, size_t length, void *user)
{
	(void)distance;
	(void)length;
	(void)user;

	/* This compressor optimises for speed rather than compression ratio, so the costs reflects cycles rather than bits. */

	if (length == 1)
		return 0;       /* Dictionary matches with a length of 1 are reserved for indicating the end of the file. Not that a dictionary match for 1 value would ever be worth it. */
	else if (distance == 1)
		return 2 + 12 + length * 3 + 2 + 14;
	else
		return 2 + 14 + length * 11 + 2 + 14;
}

static void FindExtraMatches(const unsigned char *data, size_t data_size, size_t offset, ClownLZSS_GraphEdge *node_meta_array, void *user)
{
	(void)data;
	(void)data_size;
	(void)offset;
	(void)node_meta_array;
	(void)user;
}

static CLOWNLZSS_MAKE_COMPRESSION_FUNCTION(CompressData, 4, 0x10000, 0x10000 / 4, FindExtraMatches, 2 + 11 + 2 + 14, DoLiteral, GetMatchCost, DoMatch)

static void ComperCompressStream(const unsigned char *data, size_t data_size, MemoryStream *output_stream, void *user)
{
	ComperInstance instance;

	(void)user;

	instance.output_stream = output_stream;
	MemoryStream_Create(&instance.match_stream, cc_true);
	instance.descriptor = 0;
	instance.descriptor_bits_remaining = TOTAL_DESCRIPTOR_BITS;

	CompressData(data, data_size, &instance);

	/* Terminator match */
	PutDescriptorBit(&instance, 1);
	PutMatchByte(&instance, 0);
	PutMatchByte(&instance, 0);
	PutMatchByte(&instance, 0);
	PutMatchByte(&instance, 0);

	instance.descriptor <<= instance.descriptor_bits_remaining;
	FlushData(&instance);

	MemoryStream_Destroy(&instance.match_stream);
}

unsigned char* ClownLZSS_ComperCompress(const unsigned char *data, size_t data_size, size_t *compressed_size)
{
	return RegularWrapper(data, data_size, compressed_size, NULL, ComperCompressStream);
}

unsigned char* ClownLZSS_ModuledComperCompress(const unsigned char *data, size_t data_size, size_t *compressed_size, size_t module_size)
{
	return ModuledCompressionWrapper(data, data_size, compressed_size, NULL, ComperCompressStream, module_size, 1);
}
