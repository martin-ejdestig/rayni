/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2014-2016 Martin Ejdestig <marejde@gmail.com>
 *
 * Rayni is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rayni is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rayni. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RAYNI_LIB_CONTAINERS_LISTENER_LIST_H
#define RAYNI_LIB_CONTAINERS_LISTENER_LIST_H

#include <algorithm>
#include <type_traits>
#include <vector>

namespace Rayni
{
	template <typename Listener>
	class ListenerList
	{
	public:
		class ListenerBase
		{
			friend class ListenerList<Listener>;

		protected:
			virtual ~ListenerBase()
			{
				if (list)
					// TODO: Unsafe cast (but will work for most common use case).
					list->remove(*static_cast<Listener *>(this));
			}

		private:
			ListenerList<Listener> *list = nullptr;
		};

		ListenerList()
		{
			static_assert(std::is_base_of<ListenerBase, Listener>::value,
			              "Listener must inherit from ListenerBase");
		}

		~ListenerList()
		{
			for (auto listener : listeners)
				listener->list = nullptr;
		}

		void add(Listener &listener)
		{
			if (listener.list)
				listener.list->remove(listener);

			listeners.emplace_back(&listener);
			listener.list = this;
		}

		void remove(Listener &listener)
		{
			auto iterator = std::find(listeners.cbegin(), listeners.cend(), &listener);
			if (iterator != listeners.cend())
			{
				listeners.erase(iterator);
				listener.list = nullptr;
			}
		}

		template <typename Method, typename... Args>
		void notify(Method method, const Args &... args)
		{
			for (auto listener : listeners)
				(listener->*method)(args...);
		}

	private:
		std::vector<Listener *> listeners;
	};
}

#endif // RAYNI_LIB_CONTAINERS_LISTENER_LIST_H
