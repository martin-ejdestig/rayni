/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015-2017 Martin Ejdestig <marejde@gmail.com>
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

#include <gtest/gtest.h>

#include <string>

#include "lib/containers/listener_list.h"

namespace Rayni
{
	namespace
	{
		class Listener : public ListenerList<Listener>::ListenerBase
		{
		public:
			virtual void foo_happened() = 0;
			virtual void bar_occurred(int number, const std::string &string) = 0;
		};

		class BazListener : public Listener
		{
		public:
			void foo_happened() override
			{
				data += "foo";
			}

			void bar_occurred(int number, const std::string &string) override
			{
				data += "bar" + std::to_string(number) + string;
			}

			std::string data;
		};
	}

	TEST(ListenerList, AddRemoveNotify)
	{
		ListenerList<Listener> listeners;
		BazListener listener1;
		BazListener listener2;

		listeners.add(listener1);
		listeners.add(listener2);
		listeners.notify(&Listener::foo_happened);
		EXPECT_EQ("foo", listener1.data);
		EXPECT_EQ("foo", listener2.data);

		listeners.remove(listener1);
		listeners.notify(&Listener::foo_happened);
		EXPECT_EQ("foo", listener1.data);
		EXPECT_EQ("foofoo", listener2.data);

		listeners.add(listener1);
		listeners.add(listener2);
		listeners.add(listener1);
		listeners.add(listener2);
		listeners.notify(&Listener::bar_occurred, 12, "ab");
		EXPECT_EQ("foobar12ab", listener1.data);
		EXPECT_EQ("foofoobar12ab", listener2.data);

		listeners.remove(listener1);
		listeners.remove(listener1);
		listeners.remove(listener2);
		listeners.remove(listener2);
		listeners.notify(&Listener::foo_happened);
		listeners.notify(&Listener::bar_occurred, 34, "cd");
		EXPECT_EQ("foobar12ab", listener1.data);
		EXPECT_EQ("foofoobar12ab", listener2.data);
	}

	TEST(ListenerList, ListDestroyedBeforeListeners)
	{
		BazListener listener1;
		BazListener listener2;
		{
			ListenerList<Listener> listeners;
			listeners.add(listener1);
			listeners.add(listener2);
			listeners.notify(&Listener::foo_happened);
		}

		EXPECT_EQ("foo", listener1.data);
		EXPECT_EQ("foo", listener2.data);
	}

	TEST(ListenerList, ListenersDestroyedBeforeList)
	{
		ListenerList<Listener> listeners;
		BazListener listener1;
		listeners.add(listener1);
		{
			BazListener listener2;
			BazListener listener3;
			listeners.add(listener2);
			listeners.add(listener3);
			listeners.notify(&Listener::foo_happened);
		}
		listeners.notify(&Listener::bar_occurred, 12, "ab");

		EXPECT_EQ("foobar12ab", listener1.data);
	}
}
