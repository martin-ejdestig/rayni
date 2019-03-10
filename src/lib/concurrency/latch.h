/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2014-2019 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_CONCURRENCY_LATCH_H
#define RAYNI_LIB_CONCURRENCY_LATCH_H

#include <cassert>
#include <condition_variable>
#include <mutex>

namespace Rayni
{
	class Latch
	{
	public:
		explicit Latch(unsigned int count) : count_(count)
		{
			assert(count_ > 0);
		}

		Latch(const Latch &other) = delete;
		Latch(Latch &&other) = delete;
		Latch &operator=(const Latch &other) = delete;
		Latch &operator=(Latch &&other) = delete;

		void count_down()
		{
			std::lock_guard<std::mutex> lock(mutex_);

			assert(count_ > 0);
			count_--;

			if (count_ == 0)
				condition_.notify_one();
		}

		void wait()
		{
			std::unique_lock<std::mutex> lock(mutex_);

			while (count_ != 0)
				condition_.wait(lock);
		}

	private:
		std::mutex mutex_;
		std::condition_variable condition_;
		unsigned int count_ = 0;
	};
}

#endif // RAYNI_LIB_CONCURRENCY_LATCH_H
