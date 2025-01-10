#pragma once

#include "Menu.h"

namespace Menu
{
	// Title substate
	class TitleSubstate : public Substate
	{
		private:
			// Girlfriend character
			Character gftitle = Character(MMP::Search(perm_mmp.get(), "GfTitle.chr"_h));
			uint32_t gftitle_dance = 0;

			// Flash state
			uint8_t flash = 0;
			uint8_t wipe_time = 0;

			// Title logo
			const void *logo_chr = MMP::Search(perm_mmp.get(), "Logo.chr"_h);
			Timer::FixedTime logo_bop = 0;

		public:
			TitleSubstate();
			~TitleSubstate() override;

			Substate *Loop(Timer::FixedTime dt) override;
	};
}
