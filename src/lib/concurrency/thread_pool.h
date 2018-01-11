/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2018 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_CONCURRENCY_THREAD_POOL_H
#define RAYNI_LIB_CONCURRENCY_THREAD_POOL_H

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace Rayni
{
	class ThreadPool
	{
	public:
		ThreadPool();
		explicit ThreadPool(unsigned int size);

		~ThreadPool();

		ThreadPool(const ThreadPool &other) = delete;
		ThreadPool(ThreadPool &&other) = delete;
		ThreadPool &operator=(const ThreadPool &other) = delete;
		ThreadPool &operator=(ThreadPool &&other) = delete;

		static unsigned int default_size();

		void add_task(std::function<void()> &&task);
		void add_tasks(std::vector<std::function<void()>> &&tasks);

		void wait();

	private:
		void work();

		std::vector<std::thread> threads_;

		std::mutex mutex_;
		std::condition_variable work_condition_;
		std::condition_variable wait_condition_;

		std::deque<std::function<void()>> tasks_;

		unsigned int threads_working_ = 0;
		unsigned int threads_waiting_ = 0;
		bool stop_ = false;
	};
}

#endif // RAYNI_LIB_CONCURRENCY_THREAD_POOL_H
