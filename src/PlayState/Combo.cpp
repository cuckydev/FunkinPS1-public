/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025

	- Combo.cpp -
	Combo display
*/

#include "PlayState/Combo.h"

namespace PlayState
{
	// Piece functions
	void Combo::Piece::SetMesh(const void *msh)
	{
		// Set mesh
		const Character::MeshHeader *msh_header = (const Character::MeshHeader*)msh;
		const Character::MeshPoly *msh_poly = (const Character::MeshPoly*)(msh_header + 1);
		src_poly = msh_poly;

		header = *msh_header;
		poly = *msh_poly;
	}

	void Combo::Piece::Process(CKSDK::GPU::Matrix &mat, Timer::FixedTime dt, uint32_t trim)
	{
		// Check trim
		if (CheckTrim(trim))
			return;

		// Fall
		x += xsp * dt;
		y += ysp * dt;
		ysp += dt * 180;

		// Setup mesh
		poly.s.uv2.s.v = src_poly->s.uv2.s.v - trim;
		poly.s.uv3.s.v = src_poly->s.uv3.s.v - trim;

		poly.v[2].y = src_poly->v[2].y - trim;
		poly.v[3].y = src_poly->v[3].y - trim;

		// Draw mesh
		Character::Draw(x, y, OT::UI, &header, Color::White());
	}

	bool Combo::Piece::CheckTrim(uint32_t trim)
	{
		// Check if mesh is set
		if (src_poly == nullptr)
			return true;

		// Check if trim is greater than poly height
		if (trim >= src_poly->v[2].y)
			return true;

		return false;
	}

	// Combo class
	Combo::Combo(const void *combo_msh, uint32_t combo, Judgement judgement) : msh(combo_msh)
	{
		// Set judgement piece
		piece_judgement.SetMesh(Character::GetMesh(combo_msh, 10 + (int)judgement));
		piece_judgement.x = 0;
		piece_judgement.y = -32;

		// Set 3 combo digit pieces
		if (combo == 0 || combo >= 10)
		{
			static constexpr uint32_t digits[3] = { 100, 10, 1 };
			for (int i = 0; i < 3; i++)
			{
				piece_combo[i].SetMesh(Character::GetMesh(combo_msh, (combo / digits[i]) % 10));
				piece_combo[i].x = -32 + i * 13;
				piece_combo[i].y = 0;
			}
		}
	}

	ObjectProcessResult Combo::Process(Timer::FixedTime dt)
	{
		// Decrement time and calculate trim
		time += dt;

		uint32_t trim = 0;
		if (time > 0)
			trim = time * 80;

		// Initialize matrix
		CKSDK::GPU::Matrix mat = CKSDK::GPU::Matrix::Identity();
		gte_SetRotMatrix(&mat);
		gte_ldtz(256);

		// Process pieces
		piece_judgement.Process(mat, dt, trim);
		piece_combo[0].Process(mat, dt, trim);
		piece_combo[1].Process(mat, dt, trim);
		piece_combo[2].Process(mat, dt, trim);

		// Check if all pieces are trimmed
		if (piece_judgement.CheckTrim(trim) && piece_combo[0].CheckTrim(trim) && piece_combo[1].CheckTrim(trim) && piece_combo[2].CheckTrim(trim))
			return ObjectProcessResult::Delete;

		return ObjectProcessResult::Continue;
	}
}
