// This file is part of Rayni.
//
// Copyright (C) 2014-2020 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_CONCURRENCY_BARRIER_H
#define RAYNI_LIB_CONCURRENCY_BARRIER_H

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <mutex>

namespace Rayni
{
	class Barrier
	{
	public:
		explicit Barrier(unsigned int num_threads) : num_threads_(num_threads)
		{
			assert(num_threads > 0);
		}

		Barrier(const Barrier &other) = delete;
		Barrier(Barrier &&other) = delete;
		Barrier &operator=(const Barrier &other) = delete;
		Barrier &operator=(Barrier &&other) = delete;

		void arrive_and_wait()
		{
			std::unique_lock<std::mutex> lock(mutex_);

			arrived_++;

			if (arrived_ == num_threads_)
			{
				arrived_ = 0;
				generation_++;
				condition_.notify_all();
			}
			else
			{
				unsigned int current_generation = generation_;

				while (current_generation == generation_)
					condition_.wait(lock);
			}
		}

	private:
		std::mutex mutex_;
		std::condition_variable condition_;
		unsigned int num_threads_;
		unsigned int arrived_ = 0;
		unsigned int generation_ = 0;
	};
}

#endif // RAYNI_LIB_CONCURRENCY_BARRIER_H
