// This file is part of Rayni.
//
// Copyright (C) 2015-2020 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/time/stopwatch.h"

#include <gtest/gtest.h>

#include <chrono>

namespace Rayni
{
	TEST(Stopwatch, Started)
	{
		Stopwatch stopwatch;

		EXPECT_FALSE(stopwatch.started());
		stopwatch.start();
		EXPECT_TRUE(stopwatch.started());
		stopwatch.stop();
		EXPECT_FALSE(stopwatch.started());
	}

	TEST(Stopwatch, GetDuration)
	{
		Stopwatch stopwatch;

		auto duration0 = stopwatch.duration();
		stopwatch.start();
		auto duration1 = stopwatch.duration();
		auto duration2 = stopwatch.duration();
		stopwatch.stop();
		auto duration3 = stopwatch.duration();
		auto duration4 = stopwatch.duration();
		EXPECT_EQ(duration0.count(), 0);
		EXPECT_GE(duration1.count(), 0);
		EXPECT_GE(duration2.count(), duration1.count());
		EXPECT_GE(duration3.count(), duration2.count());
		EXPECT_EQ(duration4.count(), duration3.count());

		Stopwatch::clock::time_point time_point1;
		Stopwatch::clock::time_point time_point2 = time_point1 + std::chrono::seconds(10);
		stopwatch.start(time_point1);
		stopwatch.stop(time_point2);
		EXPECT_EQ((time_point2 - time_point1).count(), stopwatch.duration().count());
	}
}
