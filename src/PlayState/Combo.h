/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025

	- Combo.h -
	Combo display
*/

#pragma once

#include "PlayState/PlayState.h"

#include "Boot/Random.h"

namespace PlayState
{
	// Combo class
	class Combo : public Object
	{
		private:
			// Combo piece
			class Piece
			{
				public:
					// Combo piece mesh
					const Character::MeshPoly *src_poly = nullptr;
					Character::MeshHeader header;
					Character::MeshPoly poly;

					// Combo piece state
					ObjectFixed x, y;
					ObjectFixed xsp = Random::NextFixed<ObjectFixed>(-2, 2);
					ObjectFixed ysp = Random::NextFixed<ObjectFixed>(-66, -48);

					// Piece functions
					void SetMesh(const void *msh);
					void Process(CKSDK::GPU::Matrix &mat, Timer::FixedTime dt, uint32_t trim);
					bool CheckTrim(uint32_t trim);
			};

			// Combo state
			const void *msh;

			Timer::FixedTime time = -0.4;

			Piece piece_judgement;
			Piece piece_combo[3];

		public:
			// Combo class
			Combo(const void *combo_msh, uint32_t combo, Judgement judgement);

			ObjectProcessResult Process(Timer::FixedTime dt) override;
	};
}
