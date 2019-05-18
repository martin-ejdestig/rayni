// This file is part of Rayni.
//
// Copyright (C) 2014-2019 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/system/linux/timer_fd.h"

#include <gtest/gtest.h>

#include <array>
#include <system_error>
#include <thread>

#include "lib/system/linux/epoll.h"

using namespace std::chrono_literals;

namespace Rayni
{
	TEST(TimerFD, FD)
	{
		TimerFD timer_fd;

		EXPECT_NE(-1, timer_fd.fd());
	}

	TEST(TimerFD, SetGetTime)
	{
		TimerFD timer_fd;
		TimerFD::Value value;

		timer_fd.set(10s);
		value = timer_fd.get();
		EXPECT_GT(10s, value.initial_expiration);
		EXPECT_EQ(0ns, value.interval);

		timer_fd.set(5s, 1s + 123ms);
		value = timer_fd.get();
		EXPECT_GT(5s, value.initial_expiration);
		EXPECT_EQ(1s + 123ms, value.interval);

		timer_fd.set(TimerFD::clock::now() + 10s);
		value = timer_fd.get();
		EXPECT_GT(10s, value.initial_expiration);
		EXPECT_EQ(0ns, value.interval);

		timer_fd.set(TimerFD::clock::now() + 5s, 1s + 987ms);
		value = timer_fd.get();
		EXPECT_GT(5s, value.initial_expiration);
		EXPECT_EQ(1s + 987ms, value.interval);
	}

	TEST(TimerFD, Disarm)
	{
		TimerFD timer_fd;
		TimerFD::Value value;

		value = timer_fd.get();
		EXPECT_EQ(0ns, value.initial_expiration);
		EXPECT_EQ(0ns, value.interval);

		timer_fd.set(10s, 1s);
		timer_fd.disarm();

		value = timer_fd.get();
		EXPECT_EQ(0ns, value.initial_expiration);
		EXPECT_EQ(0ns, value.interval);
	}

	TEST(TimerFD, PollAndRead)
	{
		TimerFD timer_fd;
		Epoll epoll;
		std::array<Epoll::Event, 1> events;

		epoll.add(timer_fd.fd(), Epoll::Flag::IN);

		EXPECT_EQ(0, epoll.wait(events, 0ms));

		timer_fd.set(1ns);

		EXPECT_EQ(1, epoll.wait(events, 1ms));
		EXPECT_EQ(1, timer_fd.read());
		EXPECT_EQ(0, epoll.wait(events, 0ms));
	}

	TEST(TimerFD, ReadInterval)
	{
		TimerFD timer_fd;

		timer_fd.set(1ns, 1ns);
		std::this_thread::sleep_for(2ns);
		EXPECT_LE(2, timer_fd.read());
	}

	TEST(TimerFD, UseAfterMoveThrows)
	{
		TimerFD timer_fd1;
		TimerFD timer_fd2(std::move(timer_fd1));

		// NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move) Tests move.
		EXPECT_THROW(timer_fd1.set(10s), std::system_error);
		EXPECT_THROW(timer_fd1.get(), std::system_error);
		EXPECT_THROW(timer_fd1.disarm(), std::system_error);
		EXPECT_THROW(timer_fd1.read(), std::system_error);
	}
}
