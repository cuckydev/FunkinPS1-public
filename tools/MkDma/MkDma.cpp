#include <iostream>
#include <fstream>
#include <cstdint>
#include <memory>
#include <vector>
#include <algorithm>
#include <cstring>
#include <stdexcept>

// Exception types
class RuntimeError : public std::runtime_error
{
	public:
		RuntimeError(std::string what_arg = "") : std::runtime_error(what_arg) {}
};

uint32_t Read32(std::istream &stream)
{
	unsigned char x[4];
	stream.read((char*)x, 4);
	return ((uint32_t)x[0] << 0) | ((uint32_t)x[1] << 8) | ((uint32_t)x[2] << 16) | ((uint32_t)x[3] << 24);
}

void Write32(std::ostream &stream, uint32_t x)
{
	stream.put(char(x >> 0));
	stream.put(char(x >> 8));
	stream.put(char(x >> 16));
	stream.put(char(x >> 24));
}

struct DMA
{
	std::unique_ptr<uint8_t[]> data;
	uint32_t size, bcr;
	uint32_t compress, xy, wh;

	bool operator==(const DMA &o) const
	{
		if (size != o.size || bcr != o.bcr || compress != o.compress || xy != o.xy || wh != o.wh)
			return false;
		return std::memcmp(data.get(), o.data.get(), size) == 0;
	}
};

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		std::cout << "MkDma out.dma ..." << std::endl;
		return 0;
	}

	try
	{
		std::vector<DMA> dmas;
		for (int i = 2; i < argc; i++)
		{
			std::ifstream stream(argv[i], std::ios::binary);
			if (!stream.is_open())
				throw RuntimeError(std::string(argv[i]) + "failed to open");
			
			uint32_t nmshs = Read32(stream) / 4;
			for (uint32_t j = 0; j < nmshs; j++)
			{
				stream.seekg(j * 4);
				stream.seekg(Read32(stream));

				std::vector<DMA> dmasi;
				uint32_t ndmas = Read32(stream);

				for (uint32_t k = 0; k < ndmas; k++)
				{
					DMA dma;
					uint32_t poff = Read32(stream);
					dma.size = Read32(stream);
					dma.bcr = Read32(stream);
					dma.compress = Read32(stream);
					dma.xy = Read32(stream);
					dma.wh = Read32(stream);
					dmasi.push_back(std::move(dma));
				}
				
				for (auto &k : dmasi)
				{
					k.data.reset(new uint8_t[k.size]);
					stream.read((char*)k.data.get(), k.size);
					dmas.push_back(std::move(k));
				}
			}
		}

		auto end = dmas.end();
		for (auto it = dmas.begin(); it != end; it++)
			end = std::remove(it + 1, end, *it);

		dmas.erase(end, dmas.end());

		std::ofstream stream(argv[1], std::ios::binary);
		if (!stream.is_open())
			throw RuntimeError(std::string(argv[1]) + "failed to open");
		
		Write32(stream, dmas.size());
		uint32_t poff = 4 + (4 * 6) * dmas.size();
		for (auto &i : dmas)
		{
			Write32(stream, poff);
			poff += i.size;
			Write32(stream, i.size);
			Write32(stream, i.bcr);
			Write32(stream, i.compress);
			Write32(stream, i.xy);
			Write32(stream, i.wh);
		}
		for (auto &i : dmas)
		{
			stream.write((char*)i.data.get(), i.size);
		}
	}
	catch (std::exception &e)
	{
		std::cerr << e.what();
		return 1;
	}

	return 0;
}
