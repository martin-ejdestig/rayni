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

#include "lib/system/linux/epoll.h"

#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <thread>

#include "lib/system/linux/event_fd.h"

namespace Rayni
{
	TEST(Epoll, FD)
	{
		Epoll epoll = Epoll::create().value_or({});

		EXPECT_NE(-1, epoll.fd());
	}

	TEST(Epoll, AddAndWaitSingle)
	{
		Epoll epoll = Epoll::create().value_or({});
		ASSERT_NE(-1, epoll.fd());

		EventFD event_fd = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd.fd());

		std::array<Epoll::Event, 1> events;

		ASSERT_TRUE(epoll.add(event_fd.fd(), Epoll::Flag::IN));

		EXPECT_EQ(0, epoll.wait(events, std::chrono::milliseconds(0)).value_or(1));

		ASSERT_TRUE(event_fd.write(1));

		EXPECT_EQ(1, epoll.wait(events).value_or(0));
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));

		EXPECT_EQ(1, epoll.wait(events, std::chrono::milliseconds(0)).value_or(0));
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));

		ASSERT_TRUE(event_fd.read());

		EXPECT_EQ(0, epoll.wait(events, std::chrono::milliseconds(0)).value_or(1));
	}

	TEST(Epoll, AddAndWaitMultiple)
	{
		Epoll epoll = Epoll::create().value_or({});
		ASSERT_NE(-1, epoll.fd());

		EventFD event_fd1 = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd1.fd());
		EventFD event_fd2 = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd2.fd());

		std::array<Epoll::Event, 2> events;

		ASSERT_TRUE(epoll.add(event_fd1.fd(), Epoll::Flag::IN));
		ASSERT_TRUE(epoll.add(event_fd2.fd(), Epoll::Flag::IN | Epoll::Flag::OUT));

		EXPECT_EQ(1, epoll.wait(events).value_or(0));
		EXPECT_EQ(event_fd2.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::OUT));

		ASSERT_TRUE(event_fd2.write(1));

		EXPECT_EQ(1, epoll.wait(events).value_or(0));
		EXPECT_EQ(event_fd2.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::OUT));

		ASSERT_TRUE(event_fd1.write(1));

		EXPECT_EQ(2, epoll.wait(events).value_or(0));

		for (const auto &event : events) {
			ASSERT_TRUE(event.fd() == event_fd1.fd() || event.fd() == event_fd2.fd());

			if (event.fd() == event_fd1.fd()) {
				EXPECT_TRUE(event.is_set(Epoll::Flag::IN));
				EXPECT_FALSE(event.is_set(Epoll::Flag::OUT)); // event_fd1 not added with OUT.
			} else if (event.fd() == event_fd2.fd()) {
				EXPECT_TRUE(event.is_set(Epoll::Flag::IN));
				EXPECT_TRUE(event.is_set(Epoll::Flag::OUT));
			}
		}

		ASSERT_TRUE(event_fd1.read());
		ASSERT_TRUE(event_fd2.read());

		ASSERT_TRUE(event_fd2.write(EventFD::MAX_VALUE)); // Can not write more without blocking. OUT not set.

		EXPECT_EQ(1, epoll.wait(events, std::chrono::milliseconds(0)).value_or(0));
		EXPECT_EQ(event_fd2.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));
		EXPECT_FALSE(events[0].is_set(Epoll::Flag::OUT));
	}

	TEST(Epoll, AddPointer)
	{
		Epoll epoll = Epoll::create().value_or({});
		ASSERT_NE(-1, epoll.fd());

		EventFD event_fd1 = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd1.fd());
		EventFD event_fd2 = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd2.fd());

		std::array<Epoll::Event, 1> events;

		ASSERT_TRUE(epoll.add(event_fd1.fd(), Epoll::Flag::OUT, &event_fd1));
		ASSERT_TRUE(epoll.add(event_fd2.fd(), Epoll::Flag::IN, &event_fd2));

		EXPECT_EQ(1, epoll.wait(events).value_or(0));
		EXPECT_EQ(events[0].ptr(), &event_fd1);
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::OUT));

		ASSERT_TRUE(event_fd1.write(EventFD::MAX_VALUE)); // Can not write more without blocking. OUT not set.
		ASSERT_TRUE(event_fd2.write(1));

		EXPECT_EQ(1, epoll.wait(events).value_or(0));
		EXPECT_EQ(events[0].ptr(), &event_fd2);
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));
	}

	TEST(Epoll, AddInvalidFileDescriptor)
	{
		Epoll epoll = Epoll::create().value_or({});
		ASSERT_NE(-1, epoll.fd());

		EXPECT_FALSE(epoll.add(1147483648, Epoll::Flag::IN));
		EXPECT_FALSE(epoll.add(1147483648, Epoll::Flag::IN, nullptr));
	}

	TEST(Epoll, Modify)
	{
		Epoll epoll = Epoll::create().value_or({});
		ASSERT_NE(-1, epoll.fd());

		EventFD event_fd = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd.fd());

		std::array<Epoll::Event, 1> events;

		ASSERT_TRUE(epoll.add(event_fd.fd(), Epoll::Flag::OUT));

		EXPECT_EQ(1, epoll.wait(events).value_or(0));
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::OUT));

		ASSERT_TRUE(epoll.modify(event_fd.fd(), Epoll::Flag::IN));
		ASSERT_TRUE(event_fd.write(1));

		EXPECT_EQ(1, epoll.wait(events).value_or(0));
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));
		EXPECT_FALSE(events[0].is_set(Epoll::Flag::OUT));

		ASSERT_TRUE(epoll.modify(event_fd.fd(), Epoll::Flag::IN | Epoll::Flag::OUT));

		EXPECT_EQ(1, epoll.wait(events).value_or(0));
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN | Epoll::Flag::OUT));
	}

	TEST(Epoll, ModifyPointer)
	{
		Epoll epoll = Epoll::create().value_or({});
		ASSERT_NE(-1, epoll.fd());

		EventFD event_fd = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd.fd());

		std::array<Epoll::Event, 1> events;

		ASSERT_TRUE(epoll.add(event_fd.fd(), Epoll::Flag::OUT, &event_fd));

		EXPECT_EQ(1, epoll.wait(events).value_or(0));
		EXPECT_EQ(&event_fd, events[0].ptr());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::OUT));

		ASSERT_TRUE(epoll.modify(event_fd.fd(), Epoll::Flag::IN, &event_fd + 1));
		ASSERT_TRUE(event_fd.write(1));

		EXPECT_EQ(1, epoll.wait(events).value_or(0));
		EXPECT_EQ(&event_fd + 1, events[0].ptr());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));
		EXPECT_FALSE(events[0].is_set(Epoll::Flag::OUT));

		ASSERT_TRUE(epoll.modify(event_fd.fd(), Epoll::Flag::IN | Epoll::Flag::OUT, &event_fd));

		EXPECT_EQ(1, epoll.wait(events).value_or(0));
		EXPECT_EQ(&event_fd, events[0].ptr());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN | Epoll::Flag::OUT));
	}

	TEST(Epoll, ModifyNotAdded)
	{
		Epoll epoll = Epoll::create().value_or({});
		ASSERT_NE(-1, epoll.fd());
		EventFD event_fd = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd.fd());

		EXPECT_FALSE(epoll.modify(event_fd.fd(), Epoll::Flag::IN));
	}

	TEST(Epoll, Remove)
	{
		Epoll epoll = Epoll::create().value_or({});
		ASSERT_NE(-1, epoll.fd());

		EventFD event_fd1 = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd1.fd());
		EventFD event_fd2 = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd2.fd());

		std::array<Epoll::Event, 2> events;

		ASSERT_TRUE(epoll.add(event_fd1.fd(), Epoll::Flag::OUT));
		ASSERT_TRUE(epoll.add(event_fd2.fd(), Epoll::Flag::OUT));

		ASSERT_TRUE(epoll.remove(event_fd2.fd()));

		EXPECT_EQ(1, epoll.wait(events).value_or(0));
		EXPECT_EQ(event_fd1.fd(), events[0].fd());
	}

	TEST(Epoll, RemoveNotAdded)
	{
		Epoll epoll = Epoll::create().value_or({});
		ASSERT_NE(-1, epoll.fd());

		EventFD event_fd = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd.fd());

		EXPECT_FALSE(epoll.remove(event_fd.fd()));
	}

	TEST(Epoll, Nest)
	{
		Epoll epoll1 = Epoll::create().value_or({});
		ASSERT_NE(-1, epoll1.fd());
		Epoll epoll2 = Epoll::create().value_or({});
		ASSERT_NE(-1, epoll2.fd());

		EventFD event_fd = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd.fd());

		std::array<Epoll::Event, 1> events;

		ASSERT_TRUE(epoll1.add(epoll2.fd(), Epoll::Flag::IN));
		ASSERT_TRUE(epoll2.add(event_fd.fd(), Epoll::Flag::IN));

		EXPECT_EQ(0, epoll1.wait(events, std::chrono::milliseconds(0)).value_or(1));

		ASSERT_TRUE(event_fd.write(1));

		EXPECT_EQ(1, epoll1.wait(events).value_or(0));
		EXPECT_EQ(epoll2.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));

		EXPECT_EQ(1, epoll2.wait(events).value_or(0));
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));

		ASSERT_TRUE(event_fd.read());

		EXPECT_EQ(0, epoll1.wait(events, std::chrono::milliseconds(0)).value_or(1));
		EXPECT_EQ(0, epoll2.wait(events, std::chrono::milliseconds(0)).value_or(1));
	}

	TEST(Epoll, WaitZeroEvents)
	{
		Epoll epoll = Epoll::create().value_or({});
		ASSERT_NE(-1, epoll.fd());

		std::array<Epoll::Event, 0> events;

		EXPECT_FALSE(epoll.wait(events, std::chrono::milliseconds(0)));
	}

	TEST(Epoll, FlagsSetToZeroDisablesPollingOfFD)
	{
		Epoll epoll = Epoll::create().value_or({});
		ASSERT_NE(-1, epoll.fd());

		EventFD event_fd = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd.fd());

		std::array<Epoll::Event, 1> events;

		ASSERT_TRUE(epoll.add(event_fd.fd(), Epoll::Flags()));

		EXPECT_EQ(0, epoll.wait(events, std::chrono::milliseconds(0)).value_or(1));

		ASSERT_TRUE(event_fd.write(1));

		EXPECT_EQ(0, epoll.wait(events, std::chrono::milliseconds(0)).value_or(1));

		ASSERT_TRUE(epoll.modify(event_fd.fd(), Epoll::Flag::IN));

		EXPECT_EQ(1, epoll.wait(events).value_or(0));
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));

		ASSERT_TRUE(epoll.modify(event_fd.fd(), Epoll::Flags()));

		EXPECT_EQ(0, epoll.wait(events, std::chrono::milliseconds(0)).value_or(1));

		ASSERT_TRUE(epoll.remove(event_fd.fd()));
		ASSERT_TRUE(epoll.add(event_fd.fd(), Epoll::Flag::IN));

		EXPECT_EQ(1, epoll.wait(events).value_or(0));
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));

		ASSERT_TRUE(epoll.modify(event_fd.fd(), Epoll::Flags()));

		EXPECT_EQ(0, epoll.wait(events, std::chrono::milliseconds(0)).value_or(1));
	}

	TEST(Epoll, ModifyInOtherThread)
	{
		Epoll epoll = Epoll::create().value_or({});
		ASSERT_NE(-1, epoll.fd());

		EventFD event_fd = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd.fd());

		std::array<Epoll::Event, 1> events;
		std::atomic<bool> thread_end_reached = false;

		std::thread thread([&] {
			std::this_thread::sleep_for(std::chrono::microseconds(100));
			if (!epoll.add(event_fd.fd(), Epoll::Flag::IN))
				return;
			if (!event_fd.write(1))
				return;
			thread_end_reached = true;
		});

		Epoll::EventCount event_count = epoll.wait(events, std::chrono::seconds(1)).value_or(0);
		ASSERT_TRUE(thread_end_reached);

		EXPECT_EQ(1, event_count);
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));

		thread.join();
	}
}
