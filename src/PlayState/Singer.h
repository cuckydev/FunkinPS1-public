/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025

	- Singer.h -
	Play state singer
*/

#pragma once

#include "PlayState/PlayState.h"

namespace PlayState
{
	// Singer class
	class Singer
	{
		private:
			// Parent PlayState
			PlayState &play_state;

			// Character
			Character character;

			// Animation indices
			uint32_t anim_idle;
			uint32_t anim_sing;
			uint32_t anim_alt_sing;
			uint32_t anim_miss;

			// Sing state
			Timer::FixedTime sing_end = Timer::FixedTime::Min();

		public:
			// Constructor
			Singer(PlayState &parent, void *chr, uint32_t idle, uint32_t sing, uint32_t alt_sing, uint32_t miss);

			// Singer functions
			void Tick(Timer::FixedTime dt);
			void Draw(int32_t x, int32_t y, size_t ot);

			void Dance();

			void SingAnimation(uint32_t i);
			void Sing(Note *note);
			void Miss(uint32_t key);
	};
}