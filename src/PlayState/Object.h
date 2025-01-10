/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025

	- Object.h -
	Object processing
*/

#pragma once

#include <CKSDK/Util/Fixed.h>

#include "Boot/Timer.h"

#include <utility>
#include <type_traits>

namespace PlayState
{
	// Object types
	using ObjectFixed = CKSDK::Fixed::Fixed<int32_t, 16>;

	enum class ObjectProcessResult
	{
		Delete = 0,
		Continue = 1
	};
	
	// Object class
	class ObjectList;

	class Object
	{
		private:
			// Friend classes
			friend class ObjectList;

			// Linked list
			Object *prev, *next;

		public:
			// Object class
			virtual ~Object() = default;

			virtual ObjectProcessResult Process(Timer::FixedTime dt) = 0;
	};

	// Object list class
	class ObjectList
	{
		private:
			// Object list head
			Object *head = nullptr;

		public:
			// Object list class
			ObjectList();
			~ObjectList();

			template <typename T, typename... Args>
			T *New(Args&&... args)
			{
				// Create object
				T *obj = new T(std::forward<Args>(args)...);

				// Link object
				obj->prev = nullptr;
				obj->next = head;

				if (head != nullptr)
					head->prev = obj;
				head = obj;

				// Return object pointer
				return obj;
			}

			void Process(Timer::FixedTime dt);
	};
}
