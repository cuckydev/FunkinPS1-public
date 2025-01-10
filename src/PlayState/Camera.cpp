/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025

	- Camera.cpp -
	Camera code
*/

#include "PlayState/Camera.h"

namespace PlayState
{
	// Camera functions
	void Camera::Process(Timer::FixedTime dt)
	{
		// Move camera towards target
		cx += (tx - cx) * td;
		cy += (ty - cy) * td;
		cz += (tz - cz) * td;

		// Reduce bump
		bumper.Process(dt);
	}
}
