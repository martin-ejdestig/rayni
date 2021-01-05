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

#include "lib/system/linux/event_fd.h"

#include <gtest/gtest.h>
#include <poll.h>

#include <array>
#include <cerrno>

namespace Rayni
{
	TEST(EventFD, FD)
	{
		EventFD event_fd = EventFD::create().value_or({});

		EXPECT_NE(-1, event_fd.fd());
	}

	TEST(EventFD, ReadWrite)
	{
		EventFD event_fd = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd.fd());

		ASSERT_TRUE(event_fd.write(1));
		EXPECT_EQ(1, event_fd.read().value_or(0));

		ASSERT_TRUE(event_fd.write(1));
		ASSERT_TRUE(event_fd.write(2));
		EXPECT_EQ(3, event_fd.read().value_or(0));

		ASSERT_TRUE(event_fd.write(0x88c3306c799fc4b5U));
		ASSERT_TRUE(event_fd.write(0x7629581c5fb0628eU));
		EXPECT_EQ(0xfeec8888d9502743, event_fd.read().value_or(0));
	}

	TEST(EventFD, MaxValue)
	{
		EventFD event_fd = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd.fd());

		ASSERT_TRUE(event_fd.write(EventFD::MAX_VALUE));
		EXPECT_EQ(EventFD::MAX_VALUE, event_fd.read().value_or(0));

		ASSERT_TRUE(event_fd.write(EventFD::MAX_VALUE - 1));
		ASSERT_TRUE(event_fd.write(1));
		EXPECT_EQ(EventFD::MAX_VALUE, event_fd.read().value_or(0));
	}

	TEST(EventFD, Poll)
	{
		EventFD event_fd = EventFD::create().value_or({});
		ASSERT_NE(-1, event_fd.fd());

		std::array<pollfd, 1> poll_fds;
		poll_fds[0].fd = event_fd.fd();
		poll_fds[0].events = POLLIN | POLLOUT;

		auto poll_and_check = [&](auto expected_events) {
			while (poll(poll_fds.data(), poll_fds.size(), -1) == -1)
				if (errno != EINTR)
					return false;

			return (poll_fds[0].revents & expected_events) == expected_events;
		};

		EXPECT_TRUE(poll_and_check(POLLOUT));

		ASSERT_TRUE(event_fd.write(1));
		EXPECT_TRUE(poll_and_check(POLLIN | POLLOUT));

		ASSERT_TRUE(event_fd.read());
		EXPECT_TRUE(poll_and_check(POLLOUT));

		ASSERT_TRUE(event_fd.write(EventFD::MAX_VALUE));
		EXPECT_TRUE(poll_and_check(POLLIN));

		ASSERT_TRUE(event_fd.read());
		EXPECT_TRUE(poll_and_check(POLLOUT));
	}
}
