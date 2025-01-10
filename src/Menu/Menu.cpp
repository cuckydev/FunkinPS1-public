/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- Menu.cpp -
	Menu scene
*/

#include "Menu.h"

#include "OpeningSubstate.h"

namespace Menu
{
	// Menu globals
	CDP::CDP menu_cdp;
	std::unique_ptr<char[]> perm_mmp;

	Timer::FixedTime song_time;
	uint32_t song_beat;

	// Menu entry point
	extern "C" void Entry()
	{
		// Read menu.cdp
		menu_cdp.Read(g_data_cdp.Search("menu.cdp"_h));

		// Read temporary data
		{
			CKSDK::CD::File file_temp_mmp = menu_cdp.Search("temp.mmp"_h);
			std::unique_ptr<char[]> temp_mmp(new char[file_temp_mmp.Size()]);

			CKSDK::CD::ReadSectors(nullptr, temp_mmp.get(), file_temp_mmp, CKSDK::CD::Mode::Speed);
			CKSDK::CD::ReadSync();

			void *msh_dma = MMP::Search(temp_mmp.get(), "perm.dma"_h);
			Character::DMA(msh_dma);
		}
		
		// Read permanent data
		CKSDK::CD::File file_perm_mmp = menu_cdp.Search("perm.mmp"_h);
		perm_mmp.reset(new char[file_perm_mmp.Size()]);

		CKSDK::CD::ReadSectors(nullptr, perm_mmp.get(), file_perm_mmp, CKSDK::CD::Mode::Speed);
		CKSDK::CD::ReadSync();

		// Get logo mesh
		/*
		const void *logo_msh = MMP::Search(perm_mmp.get(), "Logo.chr"_h);
		Timer::FixedTime logo_bop = 0;

		const void *main_menu_spr = MMP::Search(perm_mmp.get(), "MainMenu.spr"_h);
		const void *menu_desat_spr = MMP::Search(perm_mmp.get(), "MenuDesat.spr"_h);

		Sprite menu_story_mode(main_menu_spr);
		Sprite menu_freeplay(main_menu_spr);
		Sprite menu_options(main_menu_spr);

		menu_story_mode.SetAnimation(1);
		menu_freeplay.SetAnimation(2);
		menu_options.SetAnimation(4);
		*/

		// Wipe in
		Wipe::In();

		// Start timer
		Timer::Start();

		// Start playing the menu theme
		DATracker::PlayTrack(2, DATracker::Callback_Loop);

		// Start substate
		std::unique_ptr<Substate> substate(new OpeningSubstate());

		while (substate != nullptr)
		{
			// Start frame
			Timer::FixedTime dt = Timer::Update();
			Profiler::StartFrame();

			// Track song time
			song_time = DATracker::Tick(dt);
			song_beat = (uint32_t)(song_time * 102 / 60);

			// Pad state
			CKSDK::SPI::PollPads();

			// Process substate
			Substate *new_substate = substate->Loop(dt);
			if (new_substate != substate.get())
				substate.reset(new_substate);

			/*
			// Draw black background
			CKSDK::GPU::FillPrim<> &rect = CKSDK::GPU::AllocPacket<CKSDK::GPU::FillPrim<>>(OT::Background);
			rect.c = CKSDK::GPU::Color(0, 0, 0);
			rect.xy = CKSDK::GPU::ScreenCoord(0, 0);
			rect.wh = CKSDK::GPU::ScreenDim(g_width, g_height);

			// Test mat
			CKSDK::GPU::Matrix mat = CKSDK::GPU::Matrix::Identity();
			gte_SetRotMatrix(&mat);
			gte_ldtz(256);

			// BOP!
			bop += 1;
			if (bop >= 37)
			{
				alterbop ^= 1;
				gftitle.SetAnimation(alterbop);
				logo_bop = 0;
				bop = 0;
			}

			// Draw gf
			gftitle.Tick(dt);
			gftitle.Draw(68, 90, OT::Focus);

			// Draw logo
			logo_bop += dt * 24;

			uint32_t logo_i = logo_bop.Floor();
			int32_t logo_z;
			if (logo_i < COUNTOF(logo_bop_z))
				logo_z = logo_bop_z[logo_i];
			else
				logo_z = 256;

			gte_ldtz(logo_z);

			Character::Draw(-80, -40, logo_msh, 0, OT::Focus - 1);

			*/

			/*
			// SPRITER
			Sprite::Draw(0, -8, menu_desat_spr, 0, OT::Background);

			menu_story_mode.Tick(dt);
			menu_story_mode.Draw(g_width / 2, g_height / 2 - 50, OT::Focus);

			menu_freeplay.Tick(dt);
			menu_freeplay.Draw(g_width / 2, g_height / 2, OT::Focus);

			menu_options.Tick(dt);
			menu_options.Draw(g_width / 2, g_height / 2 + 50, OT::Focus);
			*/

			/*
			// Draw black background
			CKSDK::GPU::FillPrim<> &rect = CKSDK::GPU::AllocPacket<CKSDK::GPU::FillPrim<>>(OT::Background);
			rect.c = CKSDK::GPU::Color(0, 0, 0);
			rect.xy = CKSDK::GPU::ScreenCoord(0, 0);
			rect.wh = CKSDK::GPU::ScreenDim(g_width, g_height);

			WriteBoldTest(intro_texta, (g_width - BoldWidthTest(intro_texta)) / 2, g_height / 2 - 24);
			WriteBoldTest(intro_textb, (g_width - BoldWidthTest(intro_textb)) / 2, g_height / 2 + 8);
			*/
			
			// End frame
			Profiler::EndFrame();
			CKSDK::GPU::Flip();
		}
	}
}
