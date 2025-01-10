#include "OpeningSubstate.h"

#include "Boot/Font.h"

#include "TitleSubstate.h"

namespace Menu
{
	// Menu text
	static const char *opening_text[] = {
		// The commented out ones don't fit on the screen
		"shoutouts to tom fulp\0lmao"_bold,
		"ludum dare\0extraordinaire"_bold,
		"cyberzone\0coming soon"_bold,
		"love to thriftman\0swag"_bold,
		"ultimate rhythm gaming\0probably"_bold,
		"dope ass game\0playstation magazine"_bold,
		"in loving memory of\0henryeyes"_bold,
		"dancin\0forever"_bold,
		"funkin\0forever"_bold,
		"ritz dx\0rest in peace lol"_bold,
		"rate five\0pls no blam"_bold,
		"rhythm gaming\0ultimate"_bold,
		"game of the year\0forever"_bold,
		"you already know\0we really out here"_bold,
		"rise and grind\0love to luis"_bold,
		"like parappa\0but cooler"_bold,
		"album of the year\0chuckie finster"_bold,
		"free gitaroo man\0with love to wandaboy"_bold,
		// "better than geometry dash\0fight me robtop"_bold,
		"kiddbrute for president\0vote now"_bold,
		"play dead estate\0on newgrounds"_bold,
		// "this is a god damn prototype\0we workin on it okay"_bold,
		"women are real\0this is official"_bold,
		// "too over exposed\0newgrounds cant handle us"_bold,
		"Hatsune Miku\0biggest inspiration"_bold,
		"too many people\0my head hurts"_bold,
		"newgrounds\0forever"_bold,
		"refined taste in music\0if i say so myself"_bold,
		"his name isnt keith\0dumb eggy lol"_bold,
		"his name isnt evan\0silly tiktok"_bold,
		"stream chuckie finster\0on spotify"_bold,
		"never forget to\0pray to god"_bold,
		"dont play rust\0we only funkin"_bold,
		"good bye\0my penis"_bold,
		"dababy\0biggest inspiration"_bold,
		"fashionably late\0but here it is"_bold,
		"yooooooooooo\0yooooooooo"_bold,
		"pico funny\0pico funny"_bold,
		"updates each friday\0on time every time"_bold,
		"shoutouts to mason\0for da homies"_bold,
		// "bonk\0get in the discord call"_bold
	};

	// Opening substate
	OpeningSubstate::OpeningSubstate()
	{
		// Pick random intro string
		opening_text_a = opening_text[Random::Next(0U, std::size(opening_text) - 1)];
		opening_text_b = opening_text_a;
		for (; *opening_text_b != '\xFF'; opening_text_b++);
		opening_text_b++;
	}

	OpeningSubstate::~OpeningSubstate()
	{

	}

	Substate *OpeningSubstate::Loop(Timer::FixedTime dt)
	{
		// If start is pressed, skip to title
		if (CKSDK::SPI::g_pad[0].press & (CKSDK::SPI::Start | CKSDK::SPI::Cross))
			return new TitleSubstate();

		// Wipe
		Wipe::Draw();

		// Draw black background
		CKSDK::GPU::FillPrim<> &rect = CKSDK::GPU::AllocPacket<CKSDK::GPU::FillPrim<>>(OT::Background);
		rect.c = CKSDK::GPU::Color(0, 0, 0);
		rect.xy = CKSDK::GPU::ScreenCoord(0, 0);
		rect.wh = CKSDK::GPU::ScreenDim(g_width, g_height);

		if (song_time == 0)
			return this;

		// Draw text
		uint32_t song_page = song_beat / 4;
		uint32_t song_subpage = song_beat % 4;

		if (song_page >= 4)
			return new TitleSubstate();
		if (song_subpage == 0)
			return this;

		switch (song_page)
		{
			case 0:
				// Write credits
				Font::BoldPrintCenter(bold_spr, "ninjamuffin"_bold, g_width / 2, g_height / 2 - 50 + 18 * 0);
				Font::BoldPrintCenter(bold_spr, "phantomarcade"_bold, g_width / 2, g_height / 2 - 50 + 18 * 1);
				Font::BoldPrintCenter(bold_spr, "kawaisprite"_bold, g_width / 2, g_height / 2 - 50 + 18 * 2);
				Font::BoldPrintCenter(bold_spr, "evilsker"_bold, g_width / 2, g_height / 2 - 50 + 18 * 3);
				if (song_subpage == 3)
					Font::BoldPrintCenter(bold_spr, "presents"_bold, g_width / 2, g_height / 2 + 30);
				break;
			case 1:
				// Write newgrounds
				Font::BoldPrintCenter(bold_spr, "in association"_bold, g_width / 2, g_height / 2 - 80);
				Font::BoldPrintCenter(bold_spr, "with"_bold, g_width / 2, g_height / 2 - 60);
				if (song_subpage == 3)
					Font::BoldPrintCenter(bold_spr, "newgrounds"_bold, g_width / 2, g_height / 2 + 60);
				break;
			case 2:
				// Write opening texts
				Font::BoldPrintCenter(bold_spr, opening_text_a, g_width / 2, g_height / 2 - 24);
				if (song_subpage == 3)
					Font::BoldPrintCenter(bold_spr, opening_text_b, g_width / 2, g_height / 2 + 8);
				break;
			case 3:
				// Write title
				if (song_subpage >= 1)
					Font::BoldPrintCenter(bold_spr, "friday"_bold, g_width / 2, g_height / 2 - 40 + 24 * 0);
				if (song_subpage >= 2)
					Font::BoldPrintCenter(bold_spr, "night"_bold, g_width / 2, g_height / 2 - 40 + 24 * 1);
				if (song_subpage >= 3)
					Font::BoldPrintCenter(bold_spr, "funkin"_bold, g_width / 2, g_height / 2 - 40 + 24 * 2);
				break;
		}

		return this;
	}
}
