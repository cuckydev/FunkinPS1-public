#pragma once

#include "Menu.h"

namespace Menu
{
	// Opening substate
	class OpeningSubstate : public Substate
	{
		private:
			// Opening texts
			const char *opening_text_a;
			const char *opening_text_b;

			// Bold font
			const void *bold_spr = MMP::Search(perm_mmp.get(), "Bold.spr"_h);

		public:
			OpeningSubstate();
			~OpeningSubstate() override;

			Substate *Loop(Timer::FixedTime dt) override;
	};
}
