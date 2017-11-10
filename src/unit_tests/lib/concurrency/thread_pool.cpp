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
#include <exception>
#include <functional>
#include <utility>
#include <vector>

#include "lib/concurrency/barrier.h"
#include "lib/concurrency/thread_pool.h"

namespace
{
	constexpr unsigned int SUM_TERM_COUNT = 100;
	constexpr unsigned int SUM = SUM_TERM_COUNT * (SUM_TERM_COUNT - 1) / 2;
}

namespace Rayni
{
	TEST(ThreadPool, DefaultSizeGreaterThanZero)
	{
		EXPECT_GT(ThreadPool::default_size(), 0);
	}

	TEST(ThreadPool, ZeroSizeNotAllowed)
	{
		EXPECT_THROW(ThreadPool(0), std::invalid_argument);
	}

	TEST(ThreadPool, AddTaskAndWait)
	{
		ThreadPool thread_pool;
		std::atomic<unsigned int> counter{0};

		for (unsigned int i = 0; i < SUM_TERM_COUNT; i++)
			thread_pool.add_task([&counter, i] { counter += i; });

		thread_pool.wait();

		EXPECT_EQ(SUM, counter);
	}

	TEST(ThreadPool, AddTasksAndWait)
	{
		ThreadPool thread_pool;
		std::atomic<unsigned int> counter{0};
		std::vector<std::function<void()>> tasks;

		for (unsigned int i = 0; i < SUM_TERM_COUNT; i++)
			tasks.emplace_back([&counter, i] { counter += i; });

		thread_pool.add_tasks(std::move(tasks));

		thread_pool.wait();

		EXPECT_EQ(SUM, counter);
	}

	TEST(ThreadPool, CustomNumberOfThreads)
	{
		// Do not want to add a size()/thread_count() method to ThreadPool so use a barrier
		// and atomics to test that correct number of threads are actually created.
		constexpr unsigned int NUM_THREADS = 20;
		ThreadPool thread_pool(NUM_THREADS);
		Barrier barrier(NUM_THREADS + 1);
		std::atomic<unsigned int> counter1{0}, counter2{0};

		for (unsigned int i = 0; i < NUM_THREADS; i++)
			thread_pool.add_task([&] {
				counter1++;
				barrier.arrive_and_wait(); // counter1 has been increased.
				barrier.arrive_and_wait(); // Wait for counter2 to be compared against 0.
			});

		for (unsigned int i = 0; i < NUM_THREADS * 2; i++)
			thread_pool.add_task([&] { counter2++; });

		barrier.arrive_and_wait(); // Wait for counter1 to be increased.

		EXPECT_EQ(NUM_THREADS, counter1);
		EXPECT_EQ(0, counter2);

		barrier.arrive_and_wait(); // counter2 has been compared against 0.

		thread_pool.wait();

		EXPECT_EQ(NUM_THREADS, counter1);
		EXPECT_EQ(NUM_THREADS * 2, counter2);
	}
}
