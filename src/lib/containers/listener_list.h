// This file is part of Rayni.
//
// Copyright (C) 2014-2021 Martin Ejdestig <marejde@gmail.com>
//
// Rayni is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Rayni is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Rayni. If not, see <http://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RAYNI_LIB_CONTAINERS_LISTENER_LIST_H
#define RAYNI_LIB_CONTAINERS_LISTENER_LIST_H

#include <algorithm>
#include <type_traits>
#include <utility>
#include <vector>

namespace Rayni
{
	template <typename Listener>
	class ListenerList
	{
	public:
		class ListenerBase
		{
		public:
			ListenerBase(const ListenerBase &other) = delete;
			ListenerBase(ListenerBase &&other) = delete;

			ListenerBase &operator=(const ListenerBase &other) = delete; // TODO: Make copyable... ?

			ListenerBase &operator=(ListenerBase &&other) noexcept
			{
				auto other_list = other.list_;

				if (other_list)
				{
					other_list->remove(other.self_);
					other_list->add(self_);
				}
				else if (list_)
				{
					list_->remove(self_);
				}

				return *this;
			}

		protected:
			explicit ListenerBase(Listener &self) : self_(self)
			{
			}

			virtual ~ListenerBase()
			{
				if (list_)
					list_->remove(self_);
			}

		private:
			friend class ListenerList<Listener>;

			Listener &self_; // Needed to avoid unsafe downcast. TODO: Alternatives?
			ListenerList<Listener> *list_ = nullptr;
		};

		ListenerList()
		{
			static_assert(std::is_base_of_v<ListenerBase, Listener>);
		}

		ListenerList(const ListenerList &other) = delete;

		ListenerList(ListenerList &&other) noexcept : listeners_(std::move(other.listeners_))
		{
			update_listeners();
		}

		~ListenerList()
		{
			for (auto *listener : listeners_)
				listener->list_ = nullptr;
		}

		ListenerList &operator=(const ListenerList &other) = delete;

		ListenerList &operator=(ListenerList &&other) noexcept
		{
			listeners_ = std::move(other.listeners_);
			update_listeners();
			return *this;
		}

		void add(Listener &listener)
		{
			if (listener.list_ == this)
				return;

			if (listener.list_)
				listener.list_->remove(listener);

			listeners_.emplace_back(&listener);
			listener.list_ = this;
		}

		void remove(Listener &listener)
		{
			auto iterator = std::find(listeners_.cbegin(), listeners_.cend(), &listener);
			if (iterator != listeners_.cend())
			{
				listeners_.erase(iterator);
				listener.list_ = nullptr;
			}
		}

		template <typename Method, typename... Args>
		void notify(Method method, const Args &...args)
		{
			for (auto *listener : listeners_)
				(listener->*method)(args...);
		}

	private:
		void update_listeners()
		{
			for (auto *listener : listeners_)
				listener->list_ = this;
		}

		std::vector<Listener *> listeners_;
	};
}

#endif // RAYNI_LIB_CONTAINERS_LISTENER_LIST_H
