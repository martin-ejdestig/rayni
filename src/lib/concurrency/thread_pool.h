// This file is part of Rayni.
//
// Copyright (C) 2013-2019 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_CONCURRENCY_THREAD_POOL_H
#define RAYNI_LIB_CONCURRENCY_THREAD_POOL_H

#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>
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

		// Like std::async() but always runs in a thread from pool.
		template <typename Function>
		auto async(Function &&function)
		{
			using Result = std::result_of_t<std::decay_t<Function>()>;
			// Have to wrap std::promise in std::shared_ptr to make it possible to
			// store it in a std::function (must be copyable).
			auto promise = std::make_shared<std::promise<Result>>();

			add_task([promise, function = std::forward<Function>(function)]() mutable {
				if constexpr (std::is_void_v<Result>)
				{
					function();
					promise->set_value();
				}
				else
				{
					promise->set_value(function());
				}
			});

			return promise->get_future();
		}

		bool thread_available() const
		{
			std::lock_guard<std::mutex> lock(mutex_);
			return threads_.size() - threads_working_ > 0;
		}

	private:
		void work();

		std::vector<std::thread> threads_;

		mutable std::mutex mutex_;
		std::condition_variable work_condition_;
		std::condition_variable wait_condition_;

		std::deque<std::function<void()>> tasks_;

		unsigned int threads_working_ = 0;
		unsigned int threads_waiting_ = 0;
		bool stop_ = false;
	};
}

#endif // RAYNI_LIB_CONCURRENCY_THREAD_POOL_H
