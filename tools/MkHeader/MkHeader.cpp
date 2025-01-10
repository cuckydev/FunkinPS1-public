#include <iostream>
#include <string>
#include <fstream>
#include <memory>

int main(int argc, char *argv[])
{
	// Get arguments
	if (argc < 3)
	{
		std::cout << "usage: MkHeader out.h in" << std::endl;
		return 0;
	}

	std::string path_out(argv[1]);
	std::string path_in(argv[2]);

	// Read file
	std::unique_ptr<char[]> data;
	size_t data_size;
	{
		std::ifstream stream(path_in, std::ios::binary | std::ios::ate);
		if (!stream)
		{
			std::cerr << "Failed to open " << path_in << std::endl;
			return 1;
		}

		data_size = stream.tellg();
		data.reset(new char[data_size]);
		stream.seekg(0);
		stream.read(data.get(), data_size);
	}

	// Write header
	{
		std::ofstream stream(path_out);
		if (!stream)
		{
			std::cerr << "Failed to open " << path_out << std::endl;
			return 1;
		}

		char *lolp = data.get();
		for (size_t i = data_size; i != 0; i--)
			stream << (int)((unsigned char)*lolp++) << ",";
	}
	return 0;
}