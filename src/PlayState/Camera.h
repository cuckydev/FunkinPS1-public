/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025

	- Camera.h -
	Camera code
*/

#pragma once

#include "PlayState/Object.h"

namespace PlayState
{
	// Bumper class
	class Bumper
	{
		private:
			// Bump
			ObjectFixed bump = 0;
			int timer = 0;

		public:
			// Bumper functions
			void Bump(ObjectFixed bump_add)
			{
				bump = bump_add;
				timer = 30;
			}

			inline ObjectFixed GetBump() { return bump; }

			void Process(Timer::FixedTime dt)
			{
				// Reduce bump
				if (timer > 0)
				{
					bump *= 0.91;
					timer--;
				}
				else
				{
					bump = 0;
				}
			}
	};

	// Camera class
	class Camera
	{
		private:
			// Internal camera position and target
			ObjectFixed cx = 0;
			ObjectFixed cy = 0;
			ObjectFixed cz = 256;

			ObjectFixed tx = 0;
			ObjectFixed ty = 0;
			ObjectFixed tz = 256;

			Bumper bumper;

			ObjectFixed td = 0; // Divisor

		public:
			// Camera functions
			Camera() = default;
			Camera(ObjectFixed x, ObjectFixed y, ObjectFixed z) : cx(x), cy(y), tx(x), ty(y) {}

			void SetTarget(ObjectFixed target_x, ObjectFixed target_y, ObjectFixed target_z, ObjectFixed divisor = 1.0 / 24.0)
			{
				tx = target_x;
				ty = target_y;
				tz = target_z;
				td = divisor;
			}
			void Bump(ObjectFixed bump_add)
			{
				bumper.Bump(bump_add);
			}

			inline int32_t GetX(ObjectFixed scroll = 1)
			{
				if (scroll == 1)
					return cx;
				else
					return cx * scroll;
			}
			inline int32_t GetY(ObjectFixed scroll = 1)
			{
				if (scroll == 1)
					return cy;
				else
					return cy * scroll;
			}
			inline int32_t GetZ(ObjectFixed scroll = 1)
			{
				if (scroll == 1)
					return (cz - bumper.GetBump());
				else
					return (cz - bumper.GetBump()) * scroll;
			}

			void Process(Timer::FixedTime dt);
	};
}
