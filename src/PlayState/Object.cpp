/*
	[ Funkin ]
	Copyright Regan "CKDEV" Green 2023-2025

	- Object.cpp -
	Object processing
*/

#include "PlayState/Object.h"

namespace PlayState
{
	// Object list class
	ObjectList::ObjectList()
	{

	}

	ObjectList::~ObjectList()
	{
		// Delete all objects
		for (Object *obj = head; obj != nullptr;)
		{
			Object *next = obj->next;
			delete obj;
			obj = next;
		}
	}

	void ObjectList::Process(Timer::FixedTime dt)
	{
		// Process all objects
		for (Object *obj = head; obj != nullptr;)
		{
			Object *next = obj->next;
			if (obj->Process(dt) == ObjectProcessResult::Delete)
			{
				// Unlink object
				if (obj->prev != nullptr)
					obj->prev->next = obj->next;
				if (obj->next != nullptr)
					obj->next->prev = obj->prev;
				if (head == obj)
					head = obj->next;

				// Delete object
				delete obj;
			}
			obj = next;
		}
	}
}
