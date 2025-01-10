#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>

static constexpr unsigned BUCKET_BITS = 5;
static constexpr unsigned BUCKETS = 1 << BUCKET_BITS;

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

// Implementation of hashing function used in the ELF .hash section
// https://en.wikipedia.org/wiki/PJW_hash_function
static uint32_t ElfHash(const char *str)
{
	uint32_t value = 0;
	while (*str != '\0')
	{
		value <<= 4;
		value += uint32_t(*(str++));

		uint32_t nibble = value & 0xF0000000;
		if (nibble != 0)
			value ^= nibble >> 24;

		value &= ~nibble;
	}
	return value;
}

int main(int argc, char *argv[])
{
	// Get arguments
	if (argc < 3)
	{
		std::cout << "usage: MkMap out.sym in.map" << std::endl;
		return 0;
	}

	std::string path_out(argv[1]);
	std::string path_in(argv[2]);

	// Read symbols from .map
	std::unordered_map<std::string, uint32_t> symbols;
	{
		// Open .map file
		std::ifstream stream_imap(path_in);
		if (!stream_imap)
		{
			std::cerr << "Failed to open " << path_in << std::endl;
			return 1;
		}

		// Read line by line
		std::string line;
		while (std::getline(stream_imap, line))
		{
			// Read function info
			std::istringstream iss(line);
			std::string name, type;
			uint64_t addr;

			iss >> name >> type >> std::hex >> addr;
			addr &= 0xFFFFFFFF; // Annoying hack due to nm outputting 64-bit addresses

			// Push to symbol map
			if (type == "T" || type == "D" || type == "B")
				symbols.emplace(std::make_pair(name, addr));
		}
	}

	// Put into buckets
	struct Symbol
	{
		std::string name;
		uint32_t addr;
	};
	std::vector<Symbol> buckets[BUCKETS];

	for (auto &i : symbols)
	{
		uint32_t hash = ElfHash(i.first.data());
		Symbol symbol;
		symbol.name = i.first;
		symbol.addr = i.second;
		buckets[hash & (BUCKETS - 1)].push_back(symbol);
	}

	// Write out
	{
		// Open .sym file
		std::ofstream stream_sym(path_out, std::ios::binary);
		if (!stream_sym)
		{
			std::cerr << "Failed to open " << path_out << std::endl;
			return 1;
		}

		// Write buckets
		uint32_t bucket_off = BUCKETS * 4;
		for (auto &i : buckets)
		{
			Write32(stream_sym, bucket_off);
			bucket_off += 4 + i.size() * (4 * 2);
		}
		uint32_t name_off = BUCKETS * (4 * 2) + symbols.size() * (4 * 2);
		for (auto &i : buckets)
		{
			Write32(stream_sym, i.size());
			for (auto &j : i)
			{
				Write32(stream_sym, name_off);
				name_off += j.name.size() + 1;
				Write32(stream_sym, j.addr);
			}
		}

		// Write names
		for (auto &i : buckets)
		{
			for (auto &j : i)
			{
				stream_sym.write(j.name.data(), j.name.size());
				stream_sym.put(0);
			}
		}
	}

	return 0;
}