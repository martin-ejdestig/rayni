/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2014-2018 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/system/linux/event_fd.h"

#include <gtest/gtest.h>
#include <poll.h>

#include <array>
#include <cerrno>
#include <system_error>

namespace Rayni
{
	TEST(EventFD, FD)
	{
		EventFD event_fd;

		EXPECT_NE(-1, event_fd.fd());
	}

	TEST(EventFD, ReadWrite)
	{
		EventFD event_fd;

		event_fd.write(1);
		EXPECT_EQ(1, event_fd.read());

		event_fd.write(1);
		event_fd.write(2);
		EXPECT_EQ(3, event_fd.read());

		event_fd.write(0x88c3306c799fc4b5U);
		event_fd.write(0x7629581c5fb0628eU);
		EXPECT_EQ(0xfeec8888d9502743, event_fd.read());
	}

	TEST(EventFD, ReadWriteAfterMoveThrows)
	{
		EventFD event_fd1;
		EventFD event_fd2(std::move(event_fd1));

		EXPECT_THROW(event_fd1.write(1), std::system_error); // NOLINT(bugprone-use-after-move) Tests move.
		EXPECT_THROW(event_fd1.read(), std::system_error);
	}

	TEST(EventFD, MaxValue)
	{
		// TODO: EXPECT_EQ => odr-use. C++17 will add inline variables and (constexpr variables will be
		//       implicitly inlined). Should be possible to remove MAX and use EventFD::MAX_VALUE directly then.
		static constexpr auto MAX = EventFD::MAX_VALUE;

		EventFD event_fd;

		event_fd.write(EventFD::MAX_VALUE);
		EXPECT_EQ(MAX, event_fd.read());

		event_fd.write(EventFD::MAX_VALUE - 1);
		event_fd.write(1);
		EXPECT_EQ(MAX, event_fd.read());
	}

	TEST(EventFD, Poll)
	{
		EventFD event_fd;

		std::array<pollfd, 1> poll_fds;
		poll_fds[0].fd = event_fd.fd();
		poll_fds[0].events = POLLIN | POLLOUT;

		auto poll_and_check = [&](auto expected_events) {
			while (poll(poll_fds.data(), poll_fds.size(), -1) == -1)
				if (errno != EINTR)
					throw std::system_error(errno, std::system_category(), "poll() failed");

			return (poll_fds[0].revents & expected_events) == expected_events;
		};

		EXPECT_TRUE(poll_and_check(POLLOUT));

		event_fd.write(1);
		EXPECT_TRUE(poll_and_check(POLLIN | POLLOUT));

		event_fd.read();
		EXPECT_TRUE(poll_and_check(POLLOUT));

		event_fd.write(EventFD::MAX_VALUE);
		EXPECT_TRUE(poll_and_check(POLLIN));

		event_fd.read();
		EXPECT_TRUE(poll_and_check(POLLOUT));
	}
}
