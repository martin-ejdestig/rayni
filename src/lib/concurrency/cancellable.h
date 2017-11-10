/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2014-2017 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_CONCURRENCY_CANCELLABLE_H
#define RAYNI_LIB_CONCURRENCY_CANCELLABLE_H

#include <atomic>

namespace Rayni
{
	class Cancellable
	{
	public:
		void cancel()
		{
			cancelled_.store(true, std::memory_order_relaxed);
		}

		void reset()
		{
			cancelled_.store(false, std::memory_order_relaxed);
		}

		bool cancelled() const
		{
			return cancelled_.load(std::memory_order_relaxed);
		}

	private:
		std::atomic<bool> cancelled_{false};
	};
}

#endif // RAYNI_LIB_CONCURRENCY_CANCELLABLE_H
