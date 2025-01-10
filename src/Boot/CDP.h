/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- CDP.h -
	CD package class
*/

#pragma once

#include <CKSDK/CKSDK.h>

#include <CKSDK/CD.h>

#include "Boot/Hash.h"

namespace CDP
{
	// CD package class
	class CDP
	{
		private:
			union
			{
				// Files
				struct
				{
					uint32_t files;
					struct
					{
						Hash::Hash hash;
						union
						{
							CKSDK::CD::Loc loc;
							uint32_t lba;
						} loc;
						uint32_t size;
					} file[2044 / (4 * 3)];
				} header;
				uint32_t b[2048 / 4];
			} s;
			
		public:
			// Archive functions
			void Read(const CKSDK::CD::File &file);
			CKSDK::CD::File Search(Hash::Hash hash);
	};
}
