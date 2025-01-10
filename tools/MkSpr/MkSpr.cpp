/*
	[ MkSpr ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- MkSpr.cpp -
	Sprite generator
*/

#include <FunkinAlgo.h>
#include <tinyxml2.h>

// Common functions
static void OpenDocument(tinyxml2::XMLDocument &doc, std::string name)
{
	if (doc.LoadFile(name.c_str()) != tinyxml2::XML_SUCCESS)
	{
		if (doc.ErrorID() == tinyxml2::XML_ERROR_FILE_NOT_FOUND ||
			doc.ErrorID() == tinyxml2::XML_ERROR_FILE_COULD_NOT_BE_OPENED ||
			doc.ErrorID() == tinyxml2::XML_ERROR_FILE_READ_ERROR)
			throw RuntimeError(std::string("Failed to open") + name);
		else
			throw RuntimeError(std::string(doc.ErrorName()) + " on line " + std::to_string(doc.ErrorLineNum()));
	}
}

static std::string GetDirectory(std::string name)
{
	size_t cut = name.find_last_of("/\\");
	if (cut != std::string::npos)
		return name.substr(0, cut + 1);
	return "";
}

static void Write32(std::ofstream &stream, uint32_t x)
{
	stream.put(x >> 0);
	stream.put(x >> 8);
	stream.put(x >> 16);
	stream.put(x >> 24);
}

// Sprite sheet xml
struct SubTexture
{
	int x, y, width, height;
	int frameX, frameY, frameWidth, frameHeight;

	bool operator==(SubTexture _x) const
	{ return x == _x.x && y == _x.y; }
};

class SpriteSheetXml
{
	public:
		// Document
		tinyxml2::XMLDocument doc;

		// Image
		Image image;

		// Sprite sheet
		std::map<std::string, SubTexture> subtextures;

	public:
		// Sprite sheet xml functions
		SpriteSheetXml(std::string name)
		{
			// Open document
			OpenDocument(doc, name);

			// Get sheet element
			tinyxml2::XMLElement *doc_sheet = doc.FirstChildElement("TextureAtlas");
			if (doc_sheet == nullptr)
				throw RuntimeError("Cannot find TextureAtlas element");
			
			// Read sheet information
			const char *image_name = doc_sheet->Attribute("imagePath");
			if (image_name == nullptr)
				throw RuntimeError("Cannot find imagePath attribute");
			
			// Decode image
			image.Decode(GetDirectory(name) + image_name);

			// Read subtextures
			for (
				tinyxml2::XMLElement *doc_subtexture = doc_sheet->FirstChildElement("SubTexture");
				doc_subtexture != nullptr;
				doc_subtexture = doc_subtexture->NextSiblingElement("SubTexture")
			)
			{
				// Read subtexture attributes
				const char *subtexture_name = doc_subtexture->Attribute("name");
				if (subtexture_name == nullptr)
					throw RuntimeError("Subtexture has no name");

				SubTexture subtexture;
				subtexture.x = doc_subtexture->IntAttribute("x");
				subtexture.y = doc_subtexture->IntAttribute("y");
				subtexture.width = doc_subtexture->IntAttribute("width");
				subtexture.height = doc_subtexture->IntAttribute("height");
				subtexture.frameX = doc_subtexture->IntAttribute("frameX", 0);
				subtexture.frameY = doc_subtexture->IntAttribute("frameY", 0);
				subtexture.frameWidth = doc_subtexture->IntAttribute("frameWidth", subtexture.width);
				subtexture.frameHeight = doc_subtexture->IntAttribute("frameHeight", subtexture.height);

				subtextures.emplace(std::make_pair(std::string(subtexture_name), subtexture));
			}
		}
};

// Character process xml
class CharacterXml
{
	public:
		// Document
		tinyxml2::XMLDocument doc;

	public:
		// Character xml functions
		CharacterXml(std::string name, const char *chr_name, const char *msh_name, const char *dma_name)
		{
			// Open document
			OpenDocument(doc, name);

			// Get character element
			tinyxml2::XMLElement *doc_chr = doc.FirstChildElement("spr");
			if (doc_chr == nullptr)
				throw RuntimeError("Cannot find spr element");

			// Read character information
			const char *sheet_name = doc_chr->Attribute("sheet");
			if (sheet_name == nullptr)
				throw RuntimeError("Cannot find sheet attribute");

			bool compress = doc_chr->IntAttribute("compress", 0) != 0;
			bool highbpp = doc_chr->IntAttribute("highbpp", 0) != 0;
			bool dither = doc_chr->IntAttribute("dither", 0) != 0;
			int semi = doc_chr->IntAttribute("semi", -1);
			float scale = doc_chr->FloatAttribute("scale", 1.0f);
			bool singleclut = doc_chr->IntAttribute("singleclut", 0) != 0;
			
			// Open sprite sheet
			SpriteSheetXml sheet(GetDirectory(name) + sheet_name);

			// Get fixed palette
			RGBA *fixed = nullptr;
			if (singleclut)
			{
				Quant quant;
				quant.Generate(sheet.image, nullptr, 0, 0, sheet.image.w, sheet.image.h, highbpp, dither);
				fixed = quant.palette;
			}

			// Convert frames to meshes
			std::vector<Sprites> sprites;
			std::unordered_map<std::string, unsigned> mesh_iv;
			
			std::vector<Anim> anims;

			unsigned mesh_i = 0;
			for (
				tinyxml2::XMLElement *doc_frame = doc_chr->FirstChildElement("frame");
				doc_frame != nullptr;
				doc_frame = doc_frame->NextSiblingElement("frame")
			)
			{
				const char *source_name = doc_frame->Attribute("source");
				if (source_name == nullptr)
					throw RuntimeError("Cannot find source attribute for frame");

				if (source_name[0] == '\0')
				{
					Sprites sprite;
					sprites.push_back(std::move(sprite));
					mesh_iv.emplace(std::make_pair(std::string(source_name), mesh_i));
					mesh_i++;
					continue;
				}
				
				auto subtex_find = sheet.subtextures.find(std::string(source_name));
				if (subtex_find == sheet.subtextures.end())
					throw RuntimeError(std::string(source_name) + "Subtexture not found");
				SubTexture subtex = subtex_find->second;

				bool flip = doc_frame->IntAttribute("flip", 0) != 0;
				int ax = doc_frame->IntAttribute("ax", subtex.frameWidth / 2);
				int ay = doc_frame->IntAttribute("ay", subtex.frameHeight / 2);
				int tx = doc_frame->IntAttribute("tx", 0);
				int ty = doc_frame->IntAttribute("ty", 0);
				int cx = doc_frame->IntAttribute("cx", 0);
				int cy = doc_frame->IntAttribute("cy", 0);
				
				ax += subtex.frameX;
				ay += subtex.frameY;

				Algo algo;
				algo.Generate(sheet.image, semi >= 0, subtex.x, subtex.y, subtex.x + subtex.width, subtex.y + subtex.height, ax, ay, scale);
				if (flip)
					algo.Flip();

				ax = algo.anchor_x;
				ay = algo.anchor_y;

				Quant quant;
				quant.Generate(algo.image, fixed, 0, 0, algo.image.w, algo.image.h, highbpp, dither);
				
				Sprites sprite;
				sprite.Compile(quant, compress, highbpp, semi, ax, ay, tx, ty, cx, cy);
				sprites.push_back(std::move(sprite));
				mesh_iv.emplace(std::make_pair(std::string(source_name), mesh_i));
				mesh_i++;
			}

			// Read animations
			for (
				tinyxml2::XMLElement *doc_anim = doc_chr->FirstChildElement("anim");
				doc_anim != nullptr;
				doc_anim = doc_anim->NextSiblingElement("anim")
			)
			{
				// Read animation
				Anim anim;
				for (
					tinyxml2::XMLElement *anim_element = doc_anim->FirstChildElement();
					anim_element != nullptr;
					anim_element = anim_element->NextSiblingElement()
				)
				{
					std::string element_name(anim_element->Name());
					if (element_name == "frame")
					{
						const char *source_name = anim_element->Attribute("source");
						if (source_name == nullptr)
							throw RuntimeError("Cannot find source attribute for anim frame");
						auto frame_source_find = mesh_iv.find(std::string(source_name));
						if (frame_source_find == mesh_iv.end())
							throw RuntimeError("Failed to find source for animation " + std::string(source_name));
						anim.Frame(frame_source_find->second, anim_element->IntAttribute("length"));
					}
					else if (element_name == "back")
					{
						anim.Back(anim_element->IntAttribute("length"));
					}
					else if (element_name == "end")
					{
						anim.End();
					}
					else
					{
						throw RuntimeError("Bad animation element " + element_name);
					}
				}
				anims.push_back(std::move(anim));
			}

			// Hack for alignment
			if (anims.size() && (Anim::Size(anims) & 3))
				anims[0].End();

			// Write out sprite
			if (chr_name != nullptr)
			{
				// Open .spr file
				std::ofstream stream(chr_name, std::ios::binary);
				if (!stream)
					throw RuntimeError(std::string("Failed to open") + chr_name);

				// Point to msh data
				Write32(stream, 4 + Anim::Size(anims));

				// Write anim data
				Anim::Out(anims, stream);

				// Write msh pointers
				uint32_t poff = (4 * 2) * sprites.size();
				for (auto &i : sprites)
				{
					Write32(stream, poff); poff += i.Size();
					Write32(stream, poff); poff += DMA::Size(i.dmas);
				}

				// Write msh and dma data
				for (auto &i : sprites)
				{
					i.Out(stream);
					DMA::Out(i.dmas, stream);
				}
			}
			else
			{
				{
					// Open .spr file
					std::ofstream stream(msh_name, std::ios::binary);
					if (!stream)
						throw RuntimeError(std::string("Failed to open") + msh_name);

					// Point to msh pointers
					Write32(stream, 4 + Anim::Size(anims));

					// Write anim data
					Anim::Out(anims, stream);

					// Write msh pointers
					uint32_t poff = (4 * 2) * sprites.size();
					for (auto &i : sprites)
					{
						Write32(stream, poff); poff += i.Size();
						Write32(stream, 0);
					}

					// Write msh data
					for (auto &i : sprites)
						i.Out(stream);
				}
				{
					// Open .dma file
					std::ofstream stream(dma_name, std::ios::binary);
					if (!stream)
						throw RuntimeError(std::string("Failed to open") + msh_name);

					// Write dma pointers
					uint32_t poff = 4 * sprites.size();
					for (auto &i : sprites)
					{
						Write32(stream, poff); poff += DMA::Size(i.dmas);
					}

					// Write dma data
					for (auto &i : sprites)
						DMA::Out(i.dmas, stream);
				}
			}
		}

		~CharacterXml()
		{

		}
};

// Entry point
int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		std::cout << "usage: MkChr spr.xml [spr.spr | spr.spr,chr.dma]" << std::endl;
		return 0;
	}
	try
	{
		if (argc == 3)
			CharacterXml chr_xml(argv[1], argv[2], nullptr, nullptr);
		else
			CharacterXml chr_xml(argv[1], nullptr, argv[2], argv[3]);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}
