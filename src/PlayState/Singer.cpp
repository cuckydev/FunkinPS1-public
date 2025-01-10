/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025

	- Singer.cpp -
	Play state singer
*/

#include "PlayState/Singer.h"

namespace PlayState
{
	// Singer class
	// Constructor
	Singer::Singer(PlayState &parent, void *chr, uint32_t idle, uint32_t sing, uint32_t alt_sing, uint32_t miss) : play_state(parent), character(chr), anim_idle(idle), anim_sing(sing), anim_alt_sing(alt_sing), anim_miss(miss)
	{
		// Initialize character
		character.SetAnimation(anim_idle);
	}

	// Singer functions
	void Singer::Tick(Timer::FixedTime dt)
	{
		// Tick animation
		character.Tick(dt);

		// Play idle animation if we're in the sing animation
		if (character.GetAnimation() != anim_idle && play_state.time >= sing_end && character.GetAnimationEnded())
			character.SetAnimation(anim_idle);
	}

	void Singer::Draw(int32_t x, int32_t y, size_t ot)
	{
		// Draw character
		character.Draw(x, y, ot);
	}

	void Singer::Dance()
	{
		// Play idle animation if we're already in the idle animation
		if (character.GetAnimation() == anim_idle && character.GetAnimationEnded())
			character.SetAnimation(anim_idle);
	}

	void Singer::SingAnimation(uint32_t i)
	{
		// Play animation
		character.SetAnimation(i);

		// Set sing end
		sing_end = play_state.time + play_state.sectionp->length / 4;
	}

	void Singer::Sing(Note *note)
	{
		// Play sing animation
		uint32_t key = note->type & NoteDirection;
		if (note->type & NoteAltAnim)
			character.SetAnimation(anim_alt_sing + key);
		else
			character.SetAnimation(anim_sing + key);

		// Set sing end
		sing_end = note->time + note->length + play_state.sectionp->length / 4;
	}

	void Singer::Miss(uint32_t key)
	{
		// Play miss animation
		character.SetAnimation(anim_miss + key);
		sing_end = play_state.time + play_state.sectionp->length / 4;
	}
}