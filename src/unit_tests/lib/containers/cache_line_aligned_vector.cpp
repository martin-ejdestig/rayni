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

#include "lib/containers/cache_line_aligned_vector.h"

#include <gtest/gtest.h>

#include <cstdint>

#include "config.h"

namespace Rayni
{
	TEST(CacheLineAlignedVector, IsAligned)
	{
		struct alignas(RAYNI_L1_CACHE_LINE_SIZE) Foo
		{
			int bar;
		};

		struct alignas(RAYNI_L1_CACHE_LINE_SIZE * 2) FooBig
		{
			int bar;
		};

		struct alignas(RAYNI_L1_CACHE_LINE_SIZE / 2) FooSmall
		{
			int bar;
		};

		CacheLineAlignedVector<Foo> vectors[100];
		CacheLineAlignedVector<FooSmall> vectors_big[100];
		CacheLineAlignedVector<FooSmall> vectors_small[100];

		for (unsigned int i = 0; i < 100; i++) {
			vectors[i].resize(i + 1);
			vectors_big[i].resize(i + 1);
			vectors_small[i].resize(i + 1);

			ASSERT_EQ(0U, std::uintptr_t(vectors[i].data()) % RAYNI_L1_CACHE_LINE_SIZE);
			ASSERT_EQ(0U, std::uintptr_t(vectors_big[i].data()) % RAYNI_L1_CACHE_LINE_SIZE);
			ASSERT_EQ(0U, std::uintptr_t(vectors_small[i].data()) % RAYNI_L1_CACHE_LINE_SIZE);
		}
	}
}
