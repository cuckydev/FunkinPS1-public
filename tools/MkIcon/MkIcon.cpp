/*
	[ MkIcon ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- MkIcon.cpp -
	Memory card icon generator
*/

#include <FunkinAlgo.h>

// Icons process
class Icons
{
	private:
		// Image
		Image image;
		int icons = 0;
		std::unique_ptr<Quant[]> icon;

	public:
		// Icon functions
		Icons(std::string name)
		{
			// Decode image
			image.Decode(name);
			if (image.w != 16 || (image.h & 0xF) != 0)
				throw RuntimeError(std::string("Bad resolution ") + name);
			
			// Quantize icons
			icons = image.h >> 4;
			icon.reset(new Quant[icons]);
			for (int i = 0; i < icons; i++)
				icon[i].Generate(image, nullptr, 0, (i) << 4, 16, (i + 1) << 4, false, false);
		}
		
		~Icons()
		{

		}

		void Out(std::ostream &stream)
		{
			for (int i = 0; i < icons; i++)
			{
				stream << "{" << std::endl;
				stream << "{ ";
				for (int j = 0; j < 16; j++)
					stream << std::hex << "0x" << (int)icon[i].palette[j].ToPS1() << ", ";
				stream << "}," << std::endl;
				stream << "{ ";
				for (int j = 0; j < (8 * 16); j++)
					stream << std::hex << "0x" << (int)((icon[i].image[(j << 1)] << 0) | (icon[i].image[(j << 1) + 1]) << 4) << ", ";
				stream << "}," << std::endl;
				stream << "}," << std::endl;
			}
		}
};

// Entry point
int main(int argc, char *argv[])
{
	// Get arguments
	if (argc < 3)
	{
		std::cout << "usage: MkIcon icons.png out.h" << std::endl;
		return 0;
	}

	std::string arg_out_h = argv[2];

	// Process icons
	try
	{
		Icons icons(argv[1]);
		
		std::ofstream out_stream(arg_out_h);
		if (!out_stream.is_open())
			throw RuntimeError(std::string("Failed to open ") + arg_out_h);
		
		icons.Out(out_stream);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}
