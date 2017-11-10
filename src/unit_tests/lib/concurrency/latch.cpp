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

#include <atomic>
#include <thread>

#include "lib/concurrency/latch.h"

namespace Rayni
{
	TEST(Latch, CountDownAndWait)
	{
		constexpr unsigned int NUM_THREADS = 16;
		constexpr unsigned int ITERATIONS_PER_THREAD = 100;
		Latch latch(NUM_THREADS * ITERATIONS_PER_THREAD);
		std::atomic<int> counter{0};
		std::vector<std::thread> threads;

		for (unsigned int i = 0; i < NUM_THREADS; i++)
			threads.emplace_back([&] {
				for (unsigned int i = 0; i < ITERATIONS_PER_THREAD; i++)
				{
					counter++;
					latch.count_down();
				}
			});

		latch.wait();
		EXPECT_EQ(NUM_THREADS * ITERATIONS_PER_THREAD, counter);

		for (auto &thread : threads)
			thread.join();
	}
}
