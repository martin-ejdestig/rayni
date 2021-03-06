// This file is part of Rayni.
//
// Copyright (C) 2015-2021 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/containers/listener_list.h"

#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace Rayni
{
	namespace
	{
		class Listener : public ListenerList<Listener>::ListenerBase
		{
		public:
			Listener() : ListenerBase(*this)
			{
			}

			virtual void foo_happened() = 0;
			virtual void bar_occurred(int number, const std::string &string) = 0;
		};

		class BazListener : public Listener
		{
		public:
			void foo_happened() override
			{
				data_ += "foo";
			}

			void bar_occurred(int number, const std::string &string) override
			{
				data_ += "bar" + std::to_string(number) + string;
			}

			const std::string &data() const
			{
				return data_;
			}

		private:
			std::string data_;
		};
	}

	TEST(ListenerList, AddNotifyBasic)
	{
		ListenerList<Listener> listeners;
		BazListener listener1;
		BazListener listener2;

		listeners.add(listener1);

		listeners.notify(&Listener::foo_happened);
		EXPECT_EQ("foo", listener1.data());

		listeners.add(listener2);

		listeners.notify(&Listener::bar_occurred, 12, "ab");
		EXPECT_EQ("foobar12ab", listener1.data());
		EXPECT_EQ("bar12ab", listener2.data());

		listeners.notify(&Listener::foo_happened);
		EXPECT_EQ("foobar12abfoo", listener1.data());
		EXPECT_EQ("bar12abfoo", listener2.data());
	}

	TEST(ListenerList, AddAlreadyAdded)
	{
		ListenerList<Listener> listeners;
		BazListener listener;

		listeners.add(listener);
		listeners.add(listener);

		listeners.notify(&Listener::foo_happened);
		EXPECT_EQ("foo", listener.data());

		listeners.add(listener);

		listeners.notify(&Listener::bar_occurred, 12, "ab");
		EXPECT_EQ("foobar12ab", listener.data());
	}

	TEST(ListenerList, AddAlreadyAddedOtherList)
	{
		ListenerList<Listener> listeners1;
		ListenerList<Listener> listeners2;
		BazListener listener;

		listeners1.add(listener);
		listeners2.add(listener);

		listeners1.notify(&Listener::foo_happened);
		EXPECT_EQ("", listener.data());

		listeners2.notify(&Listener::foo_happened);
		EXPECT_EQ("foo", listener.data());
	}

	TEST(ListenerList, Remove)
	{
		ListenerList<Listener> listeners;
		BazListener listener1;
		BazListener listener2;
		BazListener listener3;

		listeners.add(listener1);
		listeners.add(listener2);
		listeners.add(listener3);
		listeners.notify(&Listener::foo_happened);
		EXPECT_EQ("foo", listener1.data());
		EXPECT_EQ("foo", listener2.data());
		EXPECT_EQ("foo", listener3.data());

		listeners.remove(listener2);
		listeners.notify(&Listener::foo_happened);
		EXPECT_EQ("foofoo", listener1.data());
		EXPECT_EQ("foo", listener2.data());
		EXPECT_EQ("foofoo", listener3.data());

		listeners.remove(listener3);
		listeners.notify(&Listener::foo_happened);
		EXPECT_EQ("foofoofoo", listener1.data());
		EXPECT_EQ("foo", listener2.data());
		EXPECT_EQ("foofoo", listener3.data());

		listeners.remove(listener1);
		listeners.notify(&Listener::foo_happened);
		EXPECT_EQ("foofoofoo", listener1.data());
		EXPECT_EQ("foo", listener2.data());
		EXPECT_EQ("foofoo", listener3.data());
	}

	TEST(ListenerList, RemoveNotAdded)
	{
		ListenerList<Listener> listeners;
		BazListener listener;

		listeners.remove(listener);
		listeners.notify(&Listener::foo_happened);
		EXPECT_EQ("", listener.data());
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

		EXPECT_EQ("foo", listener1.data());
		EXPECT_EQ("foo", listener2.data());
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

		EXPECT_EQ("foobar12ab", listener1.data());
	}

	TEST(ListenerList, MoveConstructor)
	{
		ListenerList<Listener> listeners1;
		BazListener listener1;
		BazListener listener2;

		listeners1.add(listener1);
		listeners1.add(listener2);

		ListenerList<Listener> listeners2(std::move(listeners1));

		// NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move) Tests move.
		listeners1.notify(&Listener::foo_happened);

		EXPECT_EQ("", listener1.data());
		EXPECT_EQ("", listener2.data());

		listeners2.notify(&Listener::foo_happened);

		EXPECT_EQ("foo", listener1.data());
		EXPECT_EQ("foo", listener2.data());
	}

	TEST(ListenerList, MoveAssignment)
	{
		ListenerList<Listener> listeners1;
		ListenerList<Listener> listeners2;
		BazListener listener1;
		BazListener listener2;

		listeners1.add(listener1);
		listeners1.add(listener2);

		listeners2 = std::move(listeners1);

		// NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move) Tests move.
		listeners1.notify(&Listener::foo_happened);

		EXPECT_EQ("", listener1.data());
		EXPECT_EQ("", listener2.data());

		listeners2.notify(&Listener::foo_happened);

		EXPECT_EQ("foo", listener1.data());
		EXPECT_EQ("foo", listener2.data());
	}

	TEST(ListenerList, ListenerMoveAssignment)
	{
		ListenerList<Listener> listeners;
		BazListener listener1;
		BazListener listener2;

		listeners.add(listener1);

		listener2 = std::move(listener1);

		listeners.notify(&Listener::foo_happened);

		// NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move) Tests move.
		EXPECT_EQ("", listener1.data());
		EXPECT_EQ("foo", listener2.data());
	}

	TEST(ListenerList, ListenerMoveAssignmentAlreadyAddedSameList)
	{
		ListenerList<Listener> listeners;
		BazListener listener1;
		BazListener listener2;

		listeners.add(listener1);
		listeners.add(listener2);

		listener2 = std::move(listener1);

		listeners.notify(&Listener::foo_happened);

		// NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move) Tests move.
		EXPECT_EQ("", listener1.data());
		EXPECT_EQ("foo", listener2.data());
	}

	TEST(ListenerList, ListenerMoveAssignmentAlreadyAddedOtherList)
	{
		ListenerList<Listener> listeners1;
		ListenerList<Listener> listeners2;
		BazListener listener1;
		BazListener listener2;

		listeners1.add(listener1);
		listeners2.add(listener2);

		listener2 = std::move(listener1);

		listeners1.notify(&Listener::foo_happened);
		listeners2.notify(&Listener::bar_occurred, 12, "ab");

		// NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move) Tests move.
		EXPECT_EQ("", listener1.data());
		EXPECT_EQ("foo", listener2.data());
	}

	TEST(ListenerList, ListenerMoveAssignmentRemovedWhenOtherNotAdded)
	{
		ListenerList<Listener> listeners;
		BazListener listener1;
		BazListener listener2;

		listeners.add(listener2);

		listener2 = std::move(listener1);

		listeners.notify(&Listener::foo_happened);

		// NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move) Tests move.
		EXPECT_EQ("", listener1.data());
		EXPECT_EQ("", listener2.data());
	}
}
