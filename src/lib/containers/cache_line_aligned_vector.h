// This file is part of Rayni.
//
// Copyright (C) 2014-2021 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_CONTAINERS_CACHE_LINE_ALIGNED_VECTOR_H
#define RAYNI_LIB_CONTAINERS_CACHE_LINE_ALIGNED_VECTOR_H

#if __cplusplus < 201703L && (defined _WIN32 || defined _WIN64)
#	include <malloc.h>
#endif

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <vector>

#include "config.h"

namespace Rayni
{
	// Note: Since C++17 alignas() automatically aligns start address to be aligned as
	// well. This is not enough if there is more than 1 element per cache line.

	template <class T>
	struct CacheLineAlignedAllocator
	{
		using value_type = T;

		T *allocate(std::size_t n)
		{
			assert(n <= SIZE_MAX / sizeof(T));

			void *ptr;
#if __cplusplus >= 201703L
			ptr = std::aligned_alloc(RAYNI_L1_CACHE_LINE_SIZE, n * sizeof(T));
#elif _POSIX_C_SOURCE >= 200112L
			if (posix_memalign(&ptr, RAYNI_L1_CACHE_LINE_SIZE, n * sizeof(T)) != 0)
				ptr = nullptr;
#elif defined _WIN32 || defined _WIN64
			ptr = _aligned_malloc(n * sizeof(T), RAYNI_L1_CACHE_LINE_SIZE); // TODO: Not tested.
#else
#	error Aligned alloc not implemented
#endif
			return static_cast<T *>(ptr);
		}

		void deallocate(T *ptr, std::size_t /*n*/)
		{
#if __cplusplus >= 201703L
			std::free(ptr);
#elif _POSIX_C_SOURCE >= 200112L
			std::free(ptr);
#elif defined _WIN32 || defined _WIN64
			_aligned_free(ptr);
#else
#	error Aligned free not implemented
#endif
		}
	};

	template <typename T>
	using CacheLineAlignedVector = std::vector<T, CacheLineAlignedAllocator<T>>;
}

#endif // RAYNI_LIB_CONTAINERS_CACHE_LINE_ALIGNED_VECTOR_H
