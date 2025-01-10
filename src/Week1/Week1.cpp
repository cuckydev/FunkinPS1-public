/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- Week1.cpp -
	Week1 scene
*/

#include "Boot/Funkin.h"

#include <CKSDK/TTY.h>
#include <CKSDK/GPU.h>
#include <CKSDK/SPI.h>
#include <CKSDK/TTY.h>

#include "Boot/MMP.h"
#include "Boot/Loader.h"
#include "Boot/Character.h"
#include "Boot/Wipe.h"
#include "Boot/Timer.h"

#include "Boot/Profiler.h"

#include "PlayState/PlayState.h"
#include "PlayState/Singer.h"
#include "PlayState/Combo.h"
#include "PlayState/Camera.h"

// Week1 globals
static CDP::CDP week1_cdp;
static std::unique_ptr<char[]> perm_mmp;

namespace PlayState
{
	// Week1 charts
	static Chart chart_bopeebo[3] = {
		#include <guns-new.h>
	};
	
	// Week1 play state
	class Week1 : public PlayState
	{
		protected:
			// Singers
			Singer bf = Singer(*this, MMP::Search(perm_mmp.get(), "Bf.chr"_h), 0, 1, 0, 5);
			Singer dad = Singer(*this, MMP::Search(perm_mmp.get(), "Dad.chr"_h), 0, 1, 0, 0);

			Character gf = Character(MMP::Search(perm_mmp.get(), "Gf.chr"_h));

			// Camera
			Camera camera;

			// Stage assets
			const void *icon_bf_msh = MMP::Search(perm_mmp.get(), "IconBf.chr"_h);
			const void *icon_bf_old_msh = MMP::Search(perm_mmp.get(), "IconBfOld.chr"_h);

			const void *combo_msh = MMP::Search(perm_mmp.get(), "Combo.chr"_h);

			const void *stage_front_msh = MMP::Search(perm_mmp.get(), "StageFront.chr"_h);
			const void *stage_back_msh = MMP::Search(perm_mmp.get(), "StageBack.chr"_h);
			const void *stage_curtains_msh = MMP::Search(perm_mmp.get(), "StageCurtains.chr"_h);

		public:
			// Play state functions
			Week1()
			{
				// Setup characters
				bf.Dance();
				dad.Dance();
				gf.SetAnimation(0);

				// Get assets
				note_msh = MMP::Search(perm_mmp.get(), "Note.chr"_h);

				notesplash_msh = MMP::Search(perm_mmp.get(), "NoteSplash.chr"_h);
				score_spr = MMP::Search(perm_mmp.get(), "Score.spr"_h);

				icon_player_msh = icon_bf_msh;
				icon_opponent_msh = MMP::Search(perm_mmp.get(), "IconDad.chr"_h);
			}

			~Week1()
			{

			}

			void BeatHit() override
			{
				// Base beat hit
				PlayState::BeatHit();

				// Bop
				if ((beat & 1) == 0)
				{
					bf.Dance();
					dad.Dance();
				}
				gf.SetAnimation(beat & 1);

				// Do the "HEY!"
				if (beat < (32 * 4) && (beat & 7) == 7)
					bf.SingAnimation(9);

				// Set camera target position
				if (sectionp->type & SectionMustHit)
					camera.SetTarget(40, 15, 256);
				else
					camera.SetTarget(-40, -20, 256);

				// Bump camera
				if ((beat & 3) == 0)
					camera.Bump(12);
			}

			void NoteHit(Note *notep) override
			{
				// Base note hit
				PlayState::NoteHit(notep);

				// Set character animation
				if (notep->type & NoteOpponent)
					dad.Sing(notep);
				else
					bf.Sing(notep);
			}

			void NoteMiss(uint32_t key) override
			{
				// Base note miss
				PlayState::NoteMiss(key);

				// Set boyfriend animation
				bf.Miss(key);
			}

			void ShowCombo(Judgement judgement) override
			{
				// Base show combo
				PlayState::ShowCombo(judgement);

				// Show combo
				object_list.New<Combo>(combo_msh, combo, judgement);
			}

			void Process(Timer::FixedTime dt) override
			{
				// Base process
				PlayState::Process(dt);

				// Health icon easter egg
				if (CKSDK::SPI::g_pad[0].press & CKSDK::SPI::Select)
					icon_player_msh = (icon_player_msh == icon_bf_msh) ? icon_bf_old_msh : icon_bf_msh;

				// Update camera
				camera.Process(dt);

				int32_t cx = camera.GetX();
				int32_t cy = camera.GetY();
				int32_t cz = camera.GetZ();

				// Initialize matrix
				CKSDK::GPU::Matrix mat = CKSDK::GPU::Matrix::Identity();
				gte_SetRotMatrix(&mat);
				gte_ldtz(cz);

				// Draw black background
				CKSDK::GPU::FillPrim<> &rect = CKSDK::GPU::AllocPacket<CKSDK::GPU::FillPrim<>>(OT::Background);
				rect.c = CKSDK::GPU::Color(0x40, 0x30, 0x40);
				rect.xy = CKSDK::GPU::ScreenCoord(0, 0);
				rect.wh = CKSDK::GPU::ScreenDim(g_width, g_height);

				// Draw stage
				Character::Draw(0 - cx, 64 - cy, OT::Background - 1, stage_front_msh, 0, Color::White());

				{
					int32_t cx = camera.GetX(0.9);
					int32_t cy = camera.GetY(0.9);
					Character::Draw(-150 - cx, -80 - cy, OT::Background - 1, stage_back_msh, 0, Color::White());
					Character::Draw(0 - cx, -110 - cy, OT::Background - 1, stage_back_msh, 1, Color::White());
				}

				{
					int32_t cx = camera.GetX(1.5);
					int32_t cy = camera.GetY(1.5);
					Character::Draw(-250 - cx, -150 - cy, OT::Focus - 1, stage_curtains_msh, 0, Color::White());
					Character::Draw(220 - cx, -150 - cy, OT::Focus - 1, stage_curtains_msh, 1, Color::White());
				}

				// Draw boyfriend character
				bf.Tick(dt);
				bf.Draw(57 - cx, 88 - cy, OT::Focus);

				// Draw dad character
				dad.Tick(dt);
				dad.Draw(-124 - cx, 92 - cy, OT::Focus);

				// Draw girlfriend character
				gf.Tick(dt);
				gf.Draw(0 - cx, 80 - cy, OT::Focus + 1);
			}
	};
}

namespace Week1
{
	// Week1 entry point
	extern "C" void Entry()
	{
		// Read week1.cdp
		week1_cdp.Read(g_data_cdp.Search("week1.cdp"_h));

		// Read temporary data
		{
			CKSDK::CD::File file_temp_mmp = week1_cdp.Search("temp.mmp"_h);
			std::unique_ptr<char[]> temp_mmp(new char[file_temp_mmp.Size()]);
			
			CKSDK::CD::ReadSectors(nullptr, temp_mmp.get(), file_temp_mmp, CKSDK::CD::Mode::Speed);
			CKSDK::CD::ReadSync();

			void *msh_dma = MMP::Search(temp_mmp.get(), "perm.dma"_h);
			Character::DMA(msh_dma);
		}
		
		// Read permanent data
		CKSDK::CD::File file_perm_mmp = week1_cdp.Search("perm.mmp"_h);
		perm_mmp.reset(new char[file_perm_mmp.Size()]);

		CKSDK::CD::ReadSectors(nullptr, perm_mmp.get(), file_perm_mmp, CKSDK::CD::Mode::Speed);
		CKSDK::CD::ReadSync();

		// Init play state
		std::unique_ptr<PlayState::Week1> play_state(new PlayState::Week1());
		play_state->Start(&PlayState::chart_bopeebo[(int)PlayState::Difficulty::Hard], 5);

		// Wipe in
		Wipe::In();

		// Start timer
		Timer::Start();

		// State loop
		while (1)
		{
			// Start frame
			Timer::FixedTime dt = Timer::Update();
			Profiler::StartFrame();

			// Pad state
			CKSDK::SPI::PollPads();
			if (CKSDK::SPI::g_pad[0].press & CKSDK::SPI::PadButton::Start)
				Wipe::Out();

			// Draw wipe
			if (Wipe::Draw())
			{
				g_scene_dll = "Menu.dll"_h;
				break;
			}
				
			// Process play state
			play_state->Process(dt);

			// End frame
			Profiler::EndFrame();
			CKSDK::GPU::Flip();
		}
	}
}
