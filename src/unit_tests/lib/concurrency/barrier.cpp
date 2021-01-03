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

#include "lib/concurrency/barrier.h"

#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <thread>
#include <vector>

namespace Rayni
{
	TEST(Barrier, ArriveAndWait)
	{
		constexpr unsigned int NUM_THREADS = 16;
		Barrier barrier(NUM_THREADS + 1);
		std::atomic<int> counter{0};
		std::vector<std::thread> threads;

		for (unsigned int i = 0; i < NUM_THREADS; i++)
			threads.emplace_back([&] {
				counter++;
				barrier.arrive_and_wait();
			});

		barrier.arrive_and_wait();
		EXPECT_EQ(NUM_THREADS, counter);

		for (auto &thread : threads)
			thread.join();
	}

	TEST(Barrier, Arrive)
	{
		constexpr unsigned int NUM_THREADS = 16;
		Barrier barrier(NUM_THREADS + 1);
		std::atomic<int> counter{0};
		std::vector<std::thread> threads;

		for (unsigned int i = 0; i < NUM_THREADS; i++)
			threads.emplace_back([&] {
				counter++;
				barrier.arrive();
			});

		barrier.arrive_and_wait();
		EXPECT_EQ(NUM_THREADS, counter);

		for (auto &thread : threads)
			thread.join();
	}

	TEST(Barrier, ArriveAndWaitMultipleTimesWithSameBarrier)
	{
		constexpr unsigned int NUM_THREADS = 16;
		constexpr unsigned int ITERATIONS = 8;
		Barrier barrier(NUM_THREADS + 1);
		std::array<std::atomic<int>, ITERATIONS> counters;
		std::vector<std::thread> threads;

		for (auto &counter : counters)
			counter = 0;

		for (unsigned int i = 0; i < NUM_THREADS; i++)
			threads.emplace_back([&] {
				for (auto &counter : counters) {
					counter++;
					barrier.arrive_and_wait();
				}
			});

		for (auto &counter : counters) {
			barrier.arrive_and_wait();
			EXPECT_EQ(NUM_THREADS, counter);
		}

		for (auto &thread : threads)
			thread.join();
	}
}
