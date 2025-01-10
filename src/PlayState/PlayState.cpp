/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025

	- PlayState.h -
	Play state
*/

#include "PlayState.h"

#include "Boot/CDP.h"
#include "Boot/MMP.h"
#include "Boot/Random.h"
#include "Boot/DATracker.h"

namespace PlayState
{
	// PlayState helper functions
	void PlayState::AddScore(int32_t add_score)
	{
		// Add score
		score += add_score;

		// Get absolute score
		uint32_t abs_score = (score < 0) ? -score : score;

		// Write out score numbers
		uint8_t *strp = score_str + sizeof(score_str) - 1;

		if (abs_score == 0)
		{
			// There should always be at least one zero
			*strp-- = 0;
		}
		else
		{
			// Divide score by 10 until we reach zero
			while (1)
			{
				if (abs_score == 0)
					break;
				*strp-- = abs_score % 10;
				abs_score /= 10;
			}
		}

		// Write out negative sign
		if (score < 0)
			*strp-- = 10;

		// Calculate score string width
		score_str_w = ((score_str + sizeof(score_str)) - strp) * 7;

		// Clear remaining string
		while (strp >= score_str)
			*strp-- = 0x80;
	}

	// Virtual implementations
	void PlayState::StepHit()
	{

	}

	void PlayState::BeatHit()
	{
		// Bump health
		health_bumper.Bump(0x400);
	}

	void PlayState::KeyPress(uint32_t key)
	{
		// Get strum
		Strum &strum = strums[key];

		// Check if we hit a note
		bool can_ghost_tap = true;

		for (Note *note = notep; note != notee; note++)
		{
			// Check if note is outside of judge window
			if (note->time - c_judge_window > time)
				break;
			if (note->time + c_judge_window < time)
				continue;

			// Check note status
			if ((note->type & NoteStatus) != NoteStatusNone)
				continue;

			// If this is another key we can press, we can't ghost tap
			if ((note->type & NoteOpponent) == 0)
				can_ghost_tap = false;

			// Now check if this is the key we've pressed
			if ((note->type & NoteKey) != key)
				continue;

			// Hit note
			NoteHit(note);
			return;
		}

		// Animate strum (Press)
		strum.strum.SetAnimation(8 + key);

		// If we're pressing the wrong key while another key is in range, miss
		if (!can_ghost_tap)
		{
			NoteMiss(key);
			return;
		}
	}

	void PlayState::KeyHold(uint32_t key, Timer::FixedTime dt)
	{
		// Get strum
		Strum &strum = strums[key];

		// Use up hold health
		HealthFixed health_to_use = dt * c_hold_health;
		if (health_to_use > strum.hold_health_remaining)
			health_to_use = strum.hold_health_remaining;

		strum.hold_health_remaining -= health_to_use;
		health += health_to_use;
	}

	void PlayState::KeyRelease(uint32_t key)
	{
		// Get strum
		Strum &strum = strums[key];

		// Clear hold health
		strum.hold_health_remaining = 0;

		// Animate strum (Static)
		strum.strum.SetAnimation(0 + key);

		// Check if a note is being held
		for (Note *note = notep; note != notee; note++)
		{
			// Check if note is outside of judge window
			if (note->time - c_judge_window > time)
				break;

			// Check note status and direction
			if ((note->type & NoteStatus) != NoteStatusHolding)
				continue;
			if ((note->type & NoteKey) != key)
				continue;

			// Check if we're close enough to hit the hold
			if (note->time + note->length - c_judge_window > time)
			{
				// Miss note
				note->SetStatus(NoteStatusMiss);
				NoteMiss(key);
			}
			else
			{
				// Hit note
				note->SetStatus(NoteStatusHit);
			}
		}
	}

	void PlayState::NoteHit(Note *note)
	{
		// Get strum
		uint32_t key = note->type & NoteKey;
		Strum &strum = strums[key];

		// Hit note
		if (note->length != 0)
		{
			// Flag as holding
			note->SetStatus(NoteStatusHolding);
		}
		else
		{
			// Flag as hit
			note->SetStatus(NoteStatusHit);
		}

		if (!(note->type & NoteOpponent))
		{
			// Increment combo
			combo++;

			// Judge our time offset
			Timer::FixedTime delta = time - note->time;
			if (delta < 0)
				delta = -delta;

			Judgement judgement;
			if (delta < c_judge_sick)
				judgement = Judgement::Sick;
			else if (delta < c_judge_good)
				judgement = Judgement::Good;
			else if (delta < c_judge_bad)
				judgement = Judgement::Bad;
			else
				judgement = Judgement::Shit;

			// Add to score
			AddScore(c_judge_score[(int)judgement]);

			// Add to health
			if (note->length == 0)
				health += c_judge_health[(int)judgement];
			else
				strum.hold_health_remaining = note->length * c_hold_health;

			// Animate strum (Confirm)
			strum.strum.SetAnimation(4 + (key & NoteDirection));

			// Splash
			if (judgement == Judgement::Sick)
			{
				uint32_t splash_random = Random::Next(1) * 4;
				strum.splash.SetAnimation(splash_random + (key & NoteDirection));
			}

			// Show combo
			ShowCombo(judgement);
		}
	}

	void PlayState::NoteMiss(uint32_t key)
	{
		// Reduce score and health
		AddScore(c_miss_score);
		health += c_miss_health;

		// Kill combo
		if (combo != 0)
		{
			combo = 0;
			ShowCombo(Judgement::Shit);
		}
	}

	void PlayState::ShowCombo(Judgement judgement)
	{

	}

	// Play state internal processes
	void PlayState::ProcessTime(Timer::FixedTime dt)
	{
		// Track time
		if (song_started)
		{
			// Follow digital audio time
			time = DATracker::Tick(dt);
		}
		else
		{
			// Scroll time forward
			time += dt;

			// Check if song should be started
			if (time >= 0)
			{
				// Start song
				time = 0;

				song_started = true;

				// Start digital audio track
				DATracker::PlayTrack(song_track, nullptr);
			}
		}

		// Update step timer
		while (time >= step_time)
		{
			if (step_time_c != 0)
			{
				// Hit step
				if (step_time_c == 16)
					SetStep((sectionp - chart->section) * 16); // Set to section start
				else
					SetStep(step + 1); // Increment step

				// Increment timer
				step_time_c--;
				step_time += step_length;
			}
			else
			{
				// Check if we are at the next section
				if ((sectionp + 1) == sectione)
				{
					// If this is the last section, just keep scrolling past the end of the song
					// Hit step
					SetStep(step + 1);

					// Increment timer
					step_time += step_length;
					break;
				}
				if (time < sectionp[1].time)
					break;

				// Set section pointer
				SetSection(sectionp + 1);
			}
		}
	}

	void PlayState::ProcessKeys(Timer::FixedTime dt)
	{
		// Check key hits
		if (CKSDK::SPI::g_pad[0].press & c_key_left)
			KeyPress(NoteLeft);
		if (CKSDK::SPI::g_pad[0].press & c_key_down)
			KeyPress(NoteDown);
		if (CKSDK::SPI::g_pad[0].press & c_key_up)
			KeyPress(NoteUp);
		if (CKSDK::SPI::g_pad[0].press & c_key_right)
			KeyPress(NoteRight);

		// Check key holds
		if (CKSDK::SPI::g_pad[0].held & c_key_left)
			KeyHold(NoteLeft, dt);
		if (CKSDK::SPI::g_pad[0].held & c_key_down)
			KeyHold(NoteDown, dt);
		if (CKSDK::SPI::g_pad[0].held & c_key_up)
			KeyHold(NoteUp, dt);
		if (CKSDK::SPI::g_pad[0].held & c_key_right)
			KeyHold(NoteRight, dt);

		// Check key releases
		if (CKSDK::SPI::g_pad[0].release & c_key_left)
			KeyRelease(NoteLeft);
		if (CKSDK::SPI::g_pad[0].release & c_key_down)
			KeyRelease(NoteDown);
		if (CKSDK::SPI::g_pad[0].release & c_key_up)
			KeyRelease(NoteUp);
		if (CKSDK::SPI::g_pad[0].release & c_key_right)
			KeyRelease(NoteRight);
	}

	void PlayState::ProcessNotes(Timer::FixedTime dt)
	{
		// Process notes
		for (Note *note = notep; note != notee; note++)
		{
			// Check if note is ahead of time
			if (note->time > time)
				break;

			// Check note status
			if ((note->type & NoteStatus) == NoteStatusHolding)
			{
				// Check if note has been fully held
				if (note->time + note->length < time)
					note->SetStatus(NoteStatusHit);
				continue;
			}
			if ((note->type & NoteStatus) != NoteStatusNone)
				continue;

			if (note->type & NoteOpponent)
			{
				// Hit note
				NoteHit(note);
			}
			else
			{
				// Check if note is outside of judge window
				if (note->time + c_judge_window < time)
				{
					// Miss note
					note->SetStatus(NoteStatusMiss);
					NoteMiss(note->type & NoteKey);
				}
			}
		}
	}

	void PlayState::DrawNotes(Timer::FixedTime dt)
	{
		// Initialize matrix
		CKSDK::GPU::Matrix mat = CKSDK::GPU::Matrix::Identity();
		gte_SetRotMatrix(&mat);
		gte_ldtz(256);

		// Draw strums
		for (auto &strum : strums)
		{
			// Draw strum
			strum.strum.Tick(dt);
			strum.strum.Draw(c_note_x[(&strum - strums)], c_note_y, OT::UI);

			// Draw splash
			if (!strum.splash.GetAnimationEnded())
			{
				strum.splash.Tick(dt);
				strum.splash.Draw(c_note_x[(&strum - strums)], c_note_y, OT::UI - 2);
			}
		}

		// Draw notes
		for (Note *note = notep; note != notee; note++)
		{
			// Check note status
			if ((note->type & NoteStatus) == NoteStatusHit)
			{
				// Cull note
				if (notep == note)
					notep++;
				continue;
			}

			// Get note type
			uint32_t key = note->type & NoteKey;

			NoteFrames &note_frame = note_frames[key & NoteDirection];
			int32_t x = c_note_x[key];

			Color color;
			if ((note->type & NoteStatus) == NoteStatusMiss)
				color = Color::RGB(0x80, 0x80, 0x80);
			else
				color = Color::RGB(0xFF, 0xFF, 0xFF);

			if (note->length == 0)
			{
				// Get note position
				int32_t y = c_note_y + (int32_t)((note->time - time) * chart->scroll);

				// Check if note has gone off screen and should be culled
				if (y < -c_note_cull)
				{
					// Note must be missed to be culled
					if ((note->type & NoteStatus) == NoteStatusMiss)
					{
						// Cull note
						if (notep == note)
							notep++;
						continue;
					}
				}

				// Check if note should be drawn
				if ((note->type & NoteStatus) == NoteStatusHit)
					continue;
				if (y > c_note_cull)
					break;

				// Draw note
				Character::Draw(x, y, OT::UI - 1, note_frame.note_msh, color);
			}
			else
			{
				// Get note positions
				int32_t start_y = c_note_y + (int32_t)((note->time - time) * chart->scroll);
				int32_t end_y = c_note_y + (int32_t)((note->time + note->length - time) * chart->scroll);

				// Check if note has gone off screen and should be culled
				if (end_y < -c_note_cull)
				{
					// Note must be missed to be culled
					if ((note->type & NoteStatus) == NoteStatusMiss)
					{
						// Cull note
						if (notep == note)
							notep++;
						continue;
					}
				}

				// Check if note should be drawn
				if ((note->type & NoteStatus) == NoteStatusHit)
					continue;
				if (start_y > c_note_cull)
					break;

				// Clip note positions
				if (start_y < -c_note_cull)
					start_y = -c_note_cull;
				if (end_y > c_note_cull)
					end_y = c_note_cull;

				// Draw hold note
				if ((note->type & NoteStatus) != NoteStatusHolding)
				{
					// Draw note mesh
					Character::Draw(x, start_y, OT::UI - 1, note_frame.note_msh, color);
				}
				else
				{
					// Clip hold
					start_y = c_note_y;
				}

				// Draw hold
				note_frame.hold_poly.v[2].y = (end_y - start_y);
				note_frame.hold_poly.v[3].y = (end_y - start_y);

				Character::Draw(x, start_y, OT::UI - 1, &note_frame.hold_msh, color);

				// Draw hold end
				Character::Draw(x, end_y, OT::UI - 1, note_frame.hold_end_msh, color);
			}
		}
	}

	void PlayState::DrawHealth(Timer::FixedTime dt)
	{
		// Initialize matrix
		int16_t health_scale = 0x1000 + health_bumper.GetBump();
		health_bumper.Process(dt);

		CKSDK::GPU::Matrix mat = CKSDK::GPU::Matrix::Identity();
		mat.m[0][0] = health_scale;
		mat.m[1][1] = health_scale;
		// mat.m[2][2] = health_scale;
		gte_SetRotMatrix(&mat);

		gte_ldtz(256);

		// Clamp health
		if (health < 0.0)
			health = 0.0;
		if (health > 2.0)
			health = 2.0;

		// Draw icons
		uint32_t player_dead = 0, opponent_dead = 0;
		if (health < 0.4)
			player_dead = 1;
		if (health > 1.6)
			opponent_dead = 1;

		int32_t x = (health - 1.0) * -c_health_w;
		Character::Draw(x, c_health_y, OT::UI - 3, icon_player_msh, player_dead, Color::White());
		Character::Draw(x, c_health_y, OT::UI - 3, icon_opponent_msh, opponent_dead, Color::White());

		// Draw bar
		struct BarPacket
		{
			CKSDK::GPU::FillPrim<> back;
			CKSDK::GPU::FillPrim<> opponent;
			CKSDK::GPU::FillPrim<> player;
		};

		int32_t player_w = health * c_health_w;
		int32_t opponent_w = (c_health_w * 2) - player_w;

		BarPacket &bar = CKSDK::GPU::AllocPacket<BarPacket>(OT::UI - 3);
		bar.back.c = CKSDK::GPU::Color(0, 0, 0);
		bar.back.xy = CKSDK::GPU::ScreenCoord(g_width / 2 - c_health_w - 2, g_height / 2 + c_health_y - c_health_h - 1);
		bar.back.wh = CKSDK::GPU::ScreenDim(c_health_w * 2 + 4, c_health_h * 2 + 2);

		bar.opponent.c = CKSDK::GPU::Color(250, 26, 4);
		bar.opponent.xy = CKSDK::GPU::ScreenCoord(g_width / 2 - c_health_w, g_height / 2 + c_health_y - c_health_h);
		bar.opponent.wh = CKSDK::GPU::ScreenDim(opponent_w, c_health_h * 2);

		bar.player.c = CKSDK::GPU::Color(94, 224, 50);
		bar.player.xy = CKSDK::GPU::ScreenCoord(g_width / 2 - c_health_w + opponent_w, g_height / 2 + c_health_y - c_health_h);
		bar.player.wh = CKSDK::GPU::ScreenDim(player_w, c_health_h * 2);
	}

	void PlayState::DrawScore(Timer::FixedTime dt)
	{
		// Get score position;
		int32_t x = c_score_x;
		Sprite::Draw(x, c_score_y, OT::UI - 4, score_spr, 11, Color::White()); // Score:

		x += 32 - (sizeof(score_str) * 7) + score_str_w;
		for (auto &i : score_str)
		{
			if ((i & 0x80) == 0)
				Sprite::Draw(x, c_score_y, OT::UI - 4, score_spr, i, Color::White());
			x += 7;
		}
	}

	// Play state functions
	void PlayState::Start(Chart *_chart, uint32_t track)
	{
		// Set chart pointer
		chart = _chart;

		// Initialize time state
		time = -4;
		step = 0;
		beat = 0;

		// Set section pointer
		SetSection(chart->section);
		sectione = chart->section + chart->sections;

		// Set note pointer
		notep = chart->note;
		notee = chart->note + chart->notes;

		for (Note *p = notep; p != notee; p++)
			p->SetStatus(NoteStatusNone);

		// Initialize score state
		score = 0;
		combo = 0;
		health = 1.0;

		AddScore(0);

		// Initialize strums
		for (auto &strum : strums)
		{
			// Animate strum (Static)
			strum.strum = note_msh;
			strum.strum.SetAnimation((&strum - strums) & NoteDirection);

			// Hide splash
			strum.splash = notesplash_msh;
			strum.splash.SetAnimationEnded();
		}

		// Initialize note frames
		for (auto &note_frame : note_frames)
		{
			// Get meshes
			uint32_t i = 12 + (&note_frame - note_frames);
			note_frame.note_msh = Character::Animation::GetMeshAt(note_msh, i, 0);
			note_frame.hold_end_msh = Character::Animation::GetMeshAt(note_msh, i, 2);

			// Setup hold mesh
			const void *hold_msh = Character::Animation::GetMeshAt(note_msh, i, 1);

			note_frame.hold_msh = *((const Character::MeshHeader*)hold_msh);
			note_frame.hold_poly = *((const Character::MeshPoly*)((uintptr_t)hold_msh + sizeof(Character::MeshHeader)));

			note_frame.hold_poly.s.uv2.s.v -= 2; // Cut off bottom 2 pixels of UVs
			note_frame.hold_poly.s.uv3.s.v -= 2;

			note_frame.hold_poly.v[0].y = 0; // Align top to 0
			note_frame.hold_poly.v[1].y = 0;
		}

		// Initialize song state
		song_track = track;
		song_started = false;

		// Get track location
		{
			uint8_t param[1] = { (uint8_t)track };
			CKSDK::CD::Issue(CKSDK::CD::Command::GetTD, +[](CKSDK::CD::IRQStatus status, const CKSDK::CD::Result &result)
				{
					// Seek to track location
					CKSDK::CD::Loc loc;
					loc.bcd.minute = result[1];
					loc.bcd.second = result[2];
					loc.bcd.sector = 0;

					CKSDK::CD::Issue(CKSDK::CD::Command::SetLoc, nullptr, nullptr, nullptr, loc.param, sizeof(loc.param));
					CKSDK::CD::Issue(CKSDK::CD::Command::SeekP, nullptr, nullptr, nullptr, nullptr, 0);

					// Reset CD mode
					uint8_t param[1] = { 0 };
					CKSDK::CD::Issue(CKSDK::CD::Command::SetMode, nullptr, nullptr, nullptr, param, sizeof(param));
				},
				nullptr, nullptr, param, sizeof(param));
		}
	}

	void PlayState::Process(Timer::FixedTime dt)
	{
		// Process game
		ProcessTime(dt);
		ProcessKeys(dt);
		ProcessNotes(dt);

		// Draw game
		DrawNotes(dt);
		DrawHealth(dt);
		DrawScore(dt);

		// Process objects
		object_list.Process(dt);
	}
}
