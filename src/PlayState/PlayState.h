/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- PlayState.h -
	Play state
*/

#pragma once

#include "PlayState/Object.h"
#include "PlayState/Camera.h"

#include <CKSDK/CKSDK.h>
#include <CKSDK/GPU.h>
#include <CKSDK/SPI.h>

#include "Boot/Funkin.h"
#include "Boot/Character.h"
#include "Boot/Timer.h"

namespace PlayState
{
	// Play state types
	enum class Difficulty
	{
		Hard = 0,
		Easy = 1,
		Normal = 2
	};

	enum class Judgement
	{
		Sick,
		Good,
		Bad,
		Shit
	};

	enum SectionType : uint32_t
	{
		SectionMustHit = (1UL << 0)
	};

	struct Section
	{
		Timer::FixedTime time, length;
		uint32_t type;
	};

	enum NoteType : uint32_t
	{
		// Direction
		NoteLeft = 0,
		NoteDown = 1,
		NoteUp = 2,
		NoteRight = 3,
		NoteDirection = NoteLeft | NoteDown | NoteUp | NoteRight,
		NoteDirections = NoteDirection + 1,

		// Note opponent flag
		NoteOpponent = (1UL << 2),

		// Note key (Direction and Opponent)
		NoteKey = NoteDirection | NoteOpponent,
		NoteKeys = NoteKey + 1,

		// Flags
		NoteAltAnim = (1UL << 3),

		// Status
		NoteStatusNone = (0UL << 30),
		NoteStatusHolding = (1UL << 30),
		NoteStatusMiss = (2UL << 30),
		NoteStatusHit = (3UL << 30),
		NoteStatus = NoteStatusHolding | NoteStatusMiss | NoteStatusHit
	};

	struct Note
	{
		Timer::FixedTime time, length;
		uint32_t type;

		void SetStatus(uint32_t status)
		{
			type = (type & ~NoteStatus) | status;
		}
	};

	struct Chart
	{
		Timer::FixedTime scroll;
		uint32_t sections; Section *section;
		uint32_t notes; Note *note;
	};

	typedef ObjectFixed HealthFixed;

	template<typename U, typename = typename std::enable_if_t<std::is_arithmetic_v<U>, U>>
	static constexpr HealthFixed HealthPercent(U percent)
	{
		return HealthFixed((1.0 / 100.0 * 2.0) * percent);
	}

	// Play state constants
	static constexpr uint16_t c_key_left = CKSDK::SPI::Left | CKSDK::SPI::Square;
	static constexpr uint16_t c_key_down = CKSDK::SPI::Down | CKSDK::SPI::Cross;
	static constexpr uint16_t c_key_up = CKSDK::SPI::Up | CKSDK::SPI::Triangle;
	static constexpr uint16_t c_key_right = CKSDK::SPI::Right | CKSDK::SPI::Circle;

	static constexpr Timer::FixedTime c_judge_window = 10.0 / 60.0; // 166ms
	static constexpr Timer::FixedTime c_judge_bad = c_judge_window * 0.8; // 133ms
	static constexpr Timer::FixedTime c_judge_good = c_judge_window * 0.55; // 91ms
	static constexpr Timer::FixedTime c_judge_sick = c_judge_window * 0.2; // 33ms

	static constexpr int32_t c_judge_score[4] = {
		350, // Sick
		200,
		100,
		50
	};
	static constexpr int32_t c_miss_score = -10;

	static constexpr HealthFixed c_judge_health[4] = {
		HealthPercent(1.65 * 1.0), // Sick
		HealthPercent(1.65 * 0.78),
		HealthPercent(1.65 * 0.2),
		HealthPercent(1.65 * 0.0)
	};
	static constexpr HealthFixed c_miss_health = HealthPercent(-3.5);

	static constexpr HealthFixed c_hold_health = HealthPercent(7.5);
	
	static constexpr int32_t c_note_x[NoteKeys] = {
		// Player
		24 + 0 * 36,
		24 + 1 * 36,
		24 + 2 * 36,
		24 + 3 * 36,
		// Opponent
		-(24 + 3 * 36) + 0 * 36,
		-(24 + 3 * 36) + 1 * 36,
		-(24 + 3 * 36) + 2 * 36,
		-(24 + 3 * 36) + 3 * 36
	};
	static constexpr int32_t c_note_y = -84;

	static constexpr int32_t c_note_cull = 140;

	static constexpr int32_t c_health_y = g_height / 2 - 39;
	static constexpr int32_t c_health_w = 112;
	static constexpr int32_t c_health_h = 2;

	static constexpr int32_t c_score_x = g_width / 2;
	static constexpr int32_t c_score_y = g_height - 30;

	// Play state classes
	class Singer;

	class PlayState
	{
		protected:
			// Friend classes
			friend class Singer;

			// Assets
			const void *score_spr = nullptr;

			const void *note_msh = nullptr;
			const void *notesplash_msh = nullptr;

			const void *icon_player_msh = nullptr;
			const void *icon_opponent_msh = nullptr;

			// Time state
			Timer::FixedTime time = 0;

			uint32_t step_time_c = 0;
			Timer::FixedTime step_time = 0, step_length = 0;

			int32_t step = 0, beat = 0;

			// Song state
			uint32_t song_track = 0;
			bool song_started = false;

			// Chart state
			Chart *chart = nullptr;
			Section *sectionp = nullptr, *sectione = nullptr;
			Note *notep = nullptr, *notee = nullptr;

			// Score state
			int32_t score = 0;
			uint32_t combo = 0;

			HealthFixed health = 1.0;
			Bumper health_bumper;

			uint8_t score_str[8]; // 7 digits for number + negative sign
			int32_t score_str_w;

			// Note frames
			struct NoteFrames
			{
				// Note frames
				const void *note_msh;
				const void *hold_end_msh;

				// Note hold mesh
				Character::MeshHeader hold_msh;
				Character::MeshPoly hold_poly;
			} note_frames[NoteDirections];

			// Strums
			struct Strum
			{
				Character strum, splash;
				HealthFixed hold_health_remaining;
			} strums[NoteKeys];

			// Object list
			ObjectList object_list;

			// Helper functions
			void SetStep(uint32_t set_step)
			{
				// Set step
				step = set_step;
				StepHit();

				beat = set_step / 4;
				if ((set_step & 3) == 0)
					BeatHit();
			}

			void SetSection(Section *section)
			{
				// Set section pointer
				sectionp = section;

				// Setup step timer
				step_time_c = 16;
				step_time = section->time;
				step_length = section->length / 16;
			}

			void AddScore(int32_t add_score);
		
			// Virtual implementations
			virtual void StepHit();
			virtual void BeatHit();

			virtual void KeyPress(uint32_t key);
			virtual void KeyHold(uint32_t key, Timer::FixedTime dt);
			virtual void KeyRelease(uint32_t key);

			virtual void NoteHit(Note *note);
			virtual void NoteMiss(uint32_t key);

			virtual void ShowCombo(Judgement judgement);

			// Play state internal processes
			void ProcessTime(Timer::FixedTime dt);
			void ProcessKeys(Timer::FixedTime dt);
			void ProcessNotes(Timer::FixedTime dt);

			void DrawNotes(Timer::FixedTime dt);
			void DrawHealth(Timer::FixedTime dt);
			void DrawScore(Timer::FixedTime dt);

		public:
			// Play state functions
			void Start(Chart *_chart, uint32_t track);

			virtual void Process(Timer::FixedTime dt);
	};
}
