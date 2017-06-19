/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2014-2017 Martin Ejdestig <marejde@gmail.com>
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

#include <array>
#include <system_error>

#include "lib/system/linux/epoll.h"
#include "lib/system/linux/event_fd.h"

namespace Rayni
{
	TEST(Epoll, FD)
	{
		Epoll epoll;

		EXPECT_NE(-1, epoll.fd());
	}

	TEST(Epoll, MoveConstructor)
	{
		Epoll epoll1;
		int fd = epoll1.fd();

		Epoll epoll2(std::move(epoll1));

		EXPECT_EQ(-1, epoll1.fd()); // NOLINT: misc-use-after-move (tests move)
		EXPECT_EQ(fd, epoll2.fd());
	}

	TEST(Epoll, MoveAssignment)
	{
		Epoll epoll1;
		int fd = epoll1.fd();

		Epoll epoll2;
		epoll2 = std::move(epoll1);

		EXPECT_EQ(-1, epoll1.fd()); // NOLINT: misc-use-after-move (tests move)
		EXPECT_EQ(fd, epoll2.fd());
	}

	TEST(Epoll, AddAndWaitSingle)
	{
		Epoll epoll;
		std::array<Epoll::Event, 1> events;
		EventFD event_fd;

		epoll.add(event_fd.fd(), Epoll::Flag::IN);

		EXPECT_EQ(0, epoll.wait(events, std::chrono::milliseconds(0)));

		event_fd.write(1);

		EXPECT_EQ(1, epoll.wait(events));
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));

		EXPECT_EQ(1, epoll.wait(events, std::chrono::milliseconds(0)));
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));

		event_fd.read();

		EXPECT_EQ(0, epoll.wait(events, std::chrono::milliseconds(0)));
	}

	TEST(Epoll, AddAndWaitMultiple)
	{
		Epoll epoll;
		std::array<Epoll::Event, 2> events;
		EventFD event_fd1, event_fd2;

		epoll.add(event_fd1.fd(), Epoll::Flag::IN);
		epoll.add(event_fd2.fd(), Epoll::Flag::IN | Epoll::Flag::OUT);

		EXPECT_EQ(1, epoll.wait(events));
		EXPECT_EQ(event_fd2.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::OUT));

		event_fd2.write(1);

		EXPECT_EQ(1, epoll.wait(events));
		EXPECT_EQ(event_fd2.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::OUT));

		event_fd1.write(1);

		EXPECT_EQ(2, epoll.wait(events));

		for (const auto &event : events)
		{
			ASSERT_TRUE(event.fd() == event_fd1.fd() || event.fd() == event_fd2.fd());

			if (event.fd() == event_fd1.fd())
			{
				EXPECT_TRUE(event.is_set(Epoll::Flag::IN));
				EXPECT_FALSE(event.is_set(Epoll::Flag::OUT)); // event_fd1 not added with OUT.
			}
			else if (event.fd() == event_fd2.fd())
			{
				EXPECT_TRUE(event.is_set(Epoll::Flag::IN));
				EXPECT_TRUE(event.is_set(Epoll::Flag::OUT));
			}
		}

		event_fd1.read();
		event_fd2.read();

		event_fd2.write(EventFD::MAX_VALUE); // Not possible to write more without blocking. OUT not set.

		EXPECT_EQ(1, epoll.wait(events, std::chrono::milliseconds(0)));
		EXPECT_EQ(event_fd2.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));
		EXPECT_FALSE(events[0].is_set(Epoll::Flag::OUT));
	}

	TEST(Epoll, AddPointer)
	{
		Epoll epoll;
		std::array<Epoll::Event, 1> events;
		EventFD event_fd1, event_fd2;

		epoll.add(event_fd1.fd(), Epoll::Flag::OUT, &event_fd1);
		epoll.add(event_fd2.fd(), Epoll::Flag::IN, &event_fd2);

		EXPECT_EQ(1, epoll.wait(events));
		EXPECT_EQ(events[0].ptr(), &event_fd1);
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::OUT));

		event_fd1.write(EventFD::MAX_VALUE); // Not possible to write more without blocking. OUT not set.
		event_fd2.write(1);

		EXPECT_EQ(1, epoll.wait(events));
		EXPECT_EQ(events[0].ptr(), &event_fd2);
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));
	}

	TEST(Epoll, AddInvalidFileDescriptor)
	{
		Epoll epoll;

		EXPECT_THROW(epoll.add(1147483648, Epoll::Flag::IN), std::system_error);
		EXPECT_THROW(epoll.add(1147483648, Epoll::Flag::IN, nullptr), std::system_error);
	}

	TEST(Epoll, Modify)
	{
		Epoll epoll;
		std::array<Epoll::Event, 1> events;
		EventFD event_fd;

		epoll.add(event_fd.fd(), Epoll::Flag::OUT);

		EXPECT_EQ(1, epoll.wait(events));
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::OUT));

		epoll.modify(event_fd.fd(), Epoll::Flag::IN);
		event_fd.write(1);

		EXPECT_EQ(1, epoll.wait(events));
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));
		EXPECT_FALSE(events[0].is_set(Epoll::Flag::OUT));

		epoll.modify(event_fd.fd(), Epoll::Flag::IN | Epoll::Flag::OUT);

		EXPECT_EQ(1, epoll.wait(events));
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN | Epoll::Flag::OUT));
	}

	TEST(Epoll, ModifyPointer)
	{
		Epoll epoll;
		std::array<Epoll::Event, 1> events;
		EventFD event_fd;

		epoll.add(event_fd.fd(), Epoll::Flag::OUT, &event_fd);

		EXPECT_EQ(1, epoll.wait(events));
		EXPECT_EQ(&event_fd, events[0].ptr());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::OUT));

		epoll.modify(event_fd.fd(), Epoll::Flag::IN, &event_fd + 1);
		event_fd.write(1);

		EXPECT_EQ(1, epoll.wait(events));
		EXPECT_EQ(&event_fd + 1, events[0].ptr());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));
		EXPECT_FALSE(events[0].is_set(Epoll::Flag::OUT));

		epoll.modify(event_fd.fd(), Epoll::Flag::IN | Epoll::Flag::OUT, &event_fd);

		EXPECT_EQ(1, epoll.wait(events));
		EXPECT_EQ(&event_fd, events[0].ptr());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN | Epoll::Flag::OUT));
	}

	TEST(Epoll, ModifyNotAdded)
	{
		Epoll epoll;
		EventFD event_fd;

		EXPECT_THROW(epoll.modify(event_fd.fd(), Epoll::Flag::IN), std::system_error);
	}

	TEST(Epoll, Remove)
	{
		Epoll epoll;
		std::array<Epoll::Event, 2> events;
		EventFD event_fd1, event_fd2;

		epoll.add(event_fd1.fd(), Epoll::Flag::OUT);
		epoll.add(event_fd2.fd(), Epoll::Flag::OUT);

		epoll.remove(event_fd2.fd());

		EXPECT_EQ(1, epoll.wait(events));
		EXPECT_EQ(event_fd1.fd(), events[0].fd());
	}

	TEST(Epoll, RemoveNotAdded)
	{
		Epoll epoll;
		EventFD event_fd;

		EXPECT_THROW(epoll.remove(event_fd.fd()), std::system_error);
	}

	TEST(Epoll, UseAfterMoveThrows)
	{
		Epoll epoll1;
		Epoll epoll2(std::move(epoll1));
		std::array<Epoll::Event, 1> events;
		EventFD event_fd;

		EXPECT_THROW(epoll1.add(event_fd.fd(), Epoll::Flag::IN), // NOLINT: misc-use-after-move (tests move)
		             std::system_error);
		EXPECT_THROW(epoll1.add(event_fd.fd(), Epoll::Flag::IN, &event_fd), std::system_error);
		EXPECT_THROW(epoll1.modify(event_fd.fd(), Epoll::Flag::IN), std::system_error);
		EXPECT_THROW(epoll1.modify(event_fd.fd(), Epoll::Flag::IN, &event_fd), std::system_error);
		EXPECT_THROW(epoll1.remove(event_fd.fd()), std::system_error);
		EXPECT_THROW(epoll1.wait(events), std::system_error);
	}

	TEST(Epoll, Nest)
	{
		Epoll epoll1, epoll2;
		std::array<Epoll::Event, 1> events;
		EventFD event_fd;

		epoll1.add(epoll2.fd(), Epoll::Flag::IN);
		epoll2.add(event_fd.fd(), Epoll::Flag::IN);

		EXPECT_EQ(0, epoll1.wait(events, std::chrono::milliseconds(0)));

		event_fd.write(1);

		EXPECT_EQ(1, epoll1.wait(events));
		EXPECT_EQ(epoll2.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));

		EXPECT_EQ(1, epoll2.wait(events));
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));

		event_fd.read();

		EXPECT_EQ(0, epoll1.wait(events, std::chrono::milliseconds(0)));
		EXPECT_EQ(0, epoll2.wait(events, std::chrono::milliseconds(0)));
	}

	TEST(Epoll, WaitZeroEvents)
	{
		Epoll epoll;
		std::array<Epoll::Event, 0> events;

		EXPECT_THROW(epoll.wait(events, std::chrono::milliseconds(0)), std::system_error);
	}

	TEST(Epoll, FlagsSetToZeroDisablesPollingOfFD)
	{
		Epoll epoll;
		std::array<Epoll::Event, 1> events;
		EventFD event_fd;

		epoll.add(event_fd.fd(), Epoll::Flags());

		EXPECT_EQ(0, epoll.wait(events, std::chrono::milliseconds(0)));

		event_fd.write(1);

		EXPECT_EQ(0, epoll.wait(events, std::chrono::milliseconds(0)));

		epoll.modify(event_fd.fd(), Epoll::Flag::IN);

		EXPECT_EQ(1, epoll.wait(events));
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));

		epoll.modify(event_fd.fd(), Epoll::Flags());

		EXPECT_EQ(0, epoll.wait(events, std::chrono::milliseconds(0)));

		epoll.remove(event_fd.fd());
		epoll.add(event_fd.fd(), Epoll::Flag::IN);

		EXPECT_EQ(1, epoll.wait(events));
		EXPECT_EQ(event_fd.fd(), events[0].fd());
		EXPECT_TRUE(events[0].is_set(Epoll::Flag::IN));

		epoll.modify(event_fd.fd(), Epoll::Flags());

		EXPECT_EQ(0, epoll.wait(events, std::chrono::milliseconds(0)));
	}
}
