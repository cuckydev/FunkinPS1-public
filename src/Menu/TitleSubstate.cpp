#include "TitleSubstate.h"

namespace Menu
{
	// Logo bop table
	static constexpr int16_t logo_bop_z[] = {
		0x100000 / 270,
		0x100000 / 240,
		0x100000 / 245,
		0x100000 / 248,
		0x100000 / 251,
		0x100000 / 253,
		0x100000 / 254,
		0x100000 / 255,
		0x100000 / 256,
	};

	// Title substate
	TitleSubstate::TitleSubstate()
	{
		// Initialize
		gftitle.SetAnimation(0);
		gftitle_dance = 0;

		// Show flash
		flash = 0x7F;
	}

	TitleSubstate::~TitleSubstate()
	{

	}

	Substate *TitleSubstate::Loop(Timer::FixedTime dt)
	{
		// Set matrix
		CKSDK::GPU::Matrix mat = CKSDK::GPU::Matrix::Identity();
		gte_SetRotMatrix(&mat);
		gte_ldtz(256);

		// Draw black background
		CKSDK::GPU::FillPrim<> &rect = CKSDK::GPU::AllocPacket<CKSDK::GPU::FillPrim<>>(OT::Background);
		rect.c = CKSDK::GPU::Color(0, 0, 0);
		rect.xy = CKSDK::GPU::ScreenCoord(0, 0);
		rect.wh = CKSDK::GPU::ScreenDim(g_width, g_height);

		// Start press
		if (CKSDK::SPI::g_pad[0].press & (CKSDK::SPI::Start | CKSDK::SPI::Cross))
		{
			flash = 0xFE;
			wipe_time = 60;
			// g_scene_dll = "Week1.dll"_h;
			// return nullptr;
		}

		if (wipe_time != 0 && --wipe_time == 0)
			Wipe::Out();

		// Wipe
		if (Wipe::Draw())
		{
			g_scene_dll = "Week1.dll"_h;
			return nullptr;
		}

		// Song beat
		if (song_beat != (gftitle_dance >> 1))
		{
			// Girlfriend dance
			gftitle_dance = (song_beat << 1) | ((gftitle_dance & 1) ^ 1);
			gftitle.SetAnimation(gftitle_dance & 1);

			// Bop logo
			logo_bop = 0;
		}

		// Girlfriend dance
		gftitle.Tick(dt);
		gftitle.Draw(68, 90, OT::Focus);

		// Draw logo
		logo_bop += dt * 24;

		int logo_i = logo_bop.Floor();
		int16_t logo_scale;
		if (logo_i < std::size(logo_bop_z))
			logo_scale = logo_bop_z[logo_i];
		else
			logo_scale = 0x1000;

		mat.m[0][0] = logo_scale;
		mat.m[1][1] = logo_scale;
		gte_SetRotMatrix(&mat);

		Character::Draw(-80, -40, OT::Focus - 1, logo_chr, 0, Color::White());

		// Draw flash
		if ((flash & 0x7F) > 0)
		{
			// Decrease flash
			flash = (flash & 0x80) | ((flash & 0x7F) - (1 + (flash >> 7)));

			// Draw primitive
			struct FlashPrim
			{
				CKSDK::GPU::DrawModePrim mode = CKSDK::GPU::DrawModePrim(0, CKSDK::GPU::SemiMode::SemiMode_Add, 0, 0, 1, 0);
				CKSDK::GPU::FillPrim<> rect;
			} &flash_prim = CKSDK::GPU::AllocPacket<FlashPrim>(OT::UI - 3);

			flash_prim.rect.SetSemi(true);
			flash_prim.rect.c = CKSDK::GPU::Color(flash << 1, flash << 1, flash << 1);
			flash_prim.rect.xy = CKSDK::GPU::ScreenCoord(0, 0);
			flash_prim.rect.wh = CKSDK::GPU::ScreenDim(g_width, g_height);
		}

		return this;
	}
}
