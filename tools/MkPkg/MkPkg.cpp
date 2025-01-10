/*
	[ MkPkg ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- MkPkg.cpp -
	Package generator
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include <cstdint>
#include <cstddef>
#include <cassert>

#if defined(CDP) || defined(__INTELLISENSE__)
#define ALIGN 2048
#endif
#ifdef MMP
#define ALIGN 16
#endif
#ifdef XAP
#define ALIGN 2336
#endif

/*
	String hashing based on a 32-bit implementation of the FNV-1a algorithm
	https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
*/
namespace Hash
{
	static const uint32_t FNV32_PRIME = 0x01000193;
	static const uint32_t FNV32_IV    = 0x811C9DC5;

	constexpr static inline uint32_t FromConst(const char *const literal, size_t max_length = 0xFFFFFFFF, uint32_t accumulator = FNV32_IV)
	{
		if (*literal && max_length)
			return FromConst(&literal[1], max_length - 1, (accumulator ^ static_cast<uint32_t>(*literal)) * FNV32_PRIME);
		return accumulator;
	}

	static inline uint32_t FromBuffer(const uint8_t *data, uint32_t length)
	{
		uint32_t accumulator = FNV32_IV;
		while (length-- > 0)
			accumulator = (accumulator ^ static_cast<uint32_t>(*data++)) * FNV32_PRIME;
		return accumulator;
	}

	static inline uint32_t FromString(const char *string)
	{
		uint32_t accumulator = FNV32_IV;
		while (*string != '\0')
			accumulator = (accumulator ^ static_cast<uint32_t>(*string++)) * FNV32_PRIME;
		return accumulator;
	}
}

constexpr static inline uint32_t operator"" _h(const char *const literal, size_t length)
{
	return Hash::FromConst(literal, length);
}

static void AlignSector(std::ofstream &stream)
{
	// Just terrible
	int i = (stream.tellp() % ALIGN);
	if (i == 0)
		return;
	for (; i < ALIGN; i++)
		stream.put(0);
	assert((stream.tellp() % ALIGN) == 0);
}

static void Write32(std::ofstream &stream, uint32_t x)
{
	stream.put(x >> 0);
	stream.put(x >> 8);
	stream.put(x >> 16);
	stream.put(x >> 24);
}

int main(int argc, char *argv[])
{
	// Check arguments
	if (argc < 2)
	{
		std::cout << "usage: " << argv[0] << " out.pkg ..." << std::endl;
		return 1;
	}
	
	// Construct file directory
	struct FileDirectory
	{
		std::string name;
		uint32_t hash;
		uint32_t lba;
		uint32_t size;
	};
	
	std::vector<FileDirectory> file_directory;
	
	uint32_t lbac = 0;
	
	for (int i = 2; i < argc; i++)
	{
		FileDirectory dir;
		dir.name = std::string(argv[i]);
		
		size_t cut_i = dir.name.find_last_of("/\\");
		std::string cut_name;
		if (cut_i != std::string::npos)
			cut_name = dir.name.substr(cut_i + 1);
		else
			cut_name = dir.name;
		
		dir.hash = Hash::FromString(cut_name.c_str());
		dir.lba = lbac;
		
		std::ifstream stream_file(dir.name, std::ios::binary | std::ios::ate);
		if (!stream_file)
		{
			std::cerr << "Failed to open file " << dir.name << std::endl;
			return 1;
		}
		
		dir.size = stream_file.tellg();
		
		#ifdef MMP
			lbac += ((dir.size + (ALIGN - 1)) / ALIGN) * ALIGN;
		#else
			lbac += (dir.size + (ALIGN - 1)) / ALIGN;
		#endif
		file_directory.push_back(dir);
	}
	
	// Check conflicts
	{
		std::unordered_map<uint32_t, std::string> conflict_map;
		for (auto &i : file_directory)
		{
			auto conflict_find = conflict_map.find(i.hash);
			if (conflict_find != conflict_map.end())
			{
				std::cerr << "Conflict between files " << i.name << " and " << conflict_find->second << std::endl;
				return 1;
			}
			conflict_map.emplace(std::make_pair(i.hash, i.name));
		}
	}
	
	// Write archive
	std::ofstream stream_arc(argv[1], std::ios::binary);
	if (!stream_arc)
	{
		std::cerr << "Failed to open archive " << argv[1] << std::endl;
		return 1;
	}
	
	#ifdef MMP
		uint32_t lbaa = (((4 + (file_directory.size() * 3 * 4)) + (ALIGN - 1)) / ALIGN) * ALIGN;
	#else
		uint32_t lbaa = 1;
	#endif
	
	#ifdef XAP
		stream_arc.put(0x00);
		stream_arc.put(0x00);
		stream_arc.put(0x89);
		stream_arc.put(0x00);
		stream_arc.put(0x00);
		stream_arc.put(0x00);
		stream_arc.put(0x89);
		stream_arc.put(0x00);
	#endif
	
	Write32(stream_arc, file_directory.size());
	for (auto &i : file_directory)
	{
		Write32(stream_arc, i.hash);
		Write32(stream_arc, i.lba + lbaa);
		#ifdef XAP
			Write32(stream_arc, (i.size / ALIGN) * 2048);
		#else
			Write32(stream_arc, i.size);
		#endif
	}
	
	if (stream_arc.tellp() >= 2048)
	{
		std::cerr << "Too many files" << std::endl;
		return 1;
	}
	
	AlignSector(stream_arc);
	
	for (auto &i : file_directory)
	{
		std::ifstream stream_file(i.name, std::ios::binary);
		if (!stream_file)
		{
			std::cerr << "Failed to open file " << i.name << std::endl;
			return 1;
		}
		
		std::unique_ptr<char[]> file_buf = std::make_unique<char[]>(i.size);
		stream_file.read(file_buf.get(), i.size);
		
		stream_arc.write(file_buf.get(), i.size);
		AlignSector(stream_arc);
	}
}
