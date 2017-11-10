/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2017 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/concurrency/thread_pool.h"

#include <exception>
#include <iterator>
#include <thread>
#include <utility>

namespace Rayni
{
	ThreadPool::ThreadPool() : ThreadPool(default_size())
	{
	}

	ThreadPool::ThreadPool(unsigned int size)
	{
		if (size < 1)
			throw std::invalid_argument("Number of threads in thread pool must be at least 1.");

		for (unsigned int i = 0; i < size; i++)
			threads_.emplace_back(&ThreadPool::work, this);
	}

	ThreadPool::~ThreadPool()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		stop_ = true;
		work_condition_.notify_all();
		lock.unlock();

		for (std::thread &t : threads_)
			t.join();
	}

	unsigned int ThreadPool::default_size()
	{
		unsigned int size = std::thread::hardware_concurrency();

		if (size == 0)
			std::runtime_error("Unable to determine number of default threads to use in thread pool");

		return size;
	}

	void ThreadPool::add_task(std::function<void()> &&task)
	{
		std::unique_lock<std::mutex> lock(mutex_);

		tasks_.emplace_back(std::move(task));

		work_condition_.notify_one();
	}

	void ThreadPool::add_tasks(std::vector<std::function<void()>> &&tasks)
	{
		std::unique_lock<std::mutex> lock(mutex_);

		tasks_.insert(tasks_.end(),
		              std::make_move_iterator(tasks.begin()),
		              std::make_move_iterator(tasks.end()));

		work_condition_.notify_all();
	}

	void ThreadPool::wait()
	{
		std::unique_lock<std::mutex> lock(mutex_);

		waiting_++;

		while (!tasks_.empty() || processing_task_ > 0)
			wait_condition_.wait(lock);

		waiting_--;
	}

	void ThreadPool::work()
	{
		std::unique_lock<std::mutex> lock(mutex_);

		while (true)
		{
			while (!stop_ && tasks_.empty())
				work_condition_.wait(lock);

			if (stop_)
				break;

			std::function<void()> task = std::move(tasks_.front());
			tasks_.pop_front();

			processing_task_++;
			lock.unlock();

			task();

			lock.lock();
			processing_task_--;

			if (waiting_ > 0 && tasks_.empty() && processing_task_ == 0)
				wait_condition_.notify_all();
		}
	}
}
