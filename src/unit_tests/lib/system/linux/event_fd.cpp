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

#include <system_error>

#include "lib/system/linux/event_fd.h"

namespace Rayni
{
	TEST(EventFDTest, FD)
	{
		EventFD event_fd;

		EXPECT_NE(-1, event_fd.fd());
	}

	TEST(EventFDTest, MoveConstructor)
	{
		EventFD event_fd1;
		int fd = event_fd1.fd();

		EventFD event_fd2(std::move(event_fd1));

		EXPECT_EQ(-1, event_fd1.fd()); // NOLINT: misc-use-after-move (tests move)
		EXPECT_EQ(fd, event_fd2.fd());
	}

	TEST(EventFDTest, MoveAssignment)
	{
		EventFD event_fd1;
		int fd = event_fd1.fd();

		EventFD event_fd2;
		event_fd2 = std::move(event_fd1);

		EXPECT_EQ(-1, event_fd1.fd()); // NOLINT: misc-use-after-move (tests move)
		EXPECT_EQ(fd, event_fd2.fd());
	}

	TEST(EventFDTest, ReadWrite)
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

	TEST(EventFDTest, ReadWriteAfterMoveThrows)
	{
		EventFD event_fd1;
		EventFD event_fd2(std::move(event_fd1));

		EXPECT_THROW(event_fd1.write(1), std::system_error); // NOLINT: misc-use-after-move (tests move)
		EXPECT_THROW(event_fd1.read(), std::system_error);
	}

	TEST(EventFDTest, MaxValue)
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
}
