/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015-2018 Martin Ejdestig <marejde@gmail.com>
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

#include <functional>

#include "lib/math/hash.h"

namespace Rayni
{
	TEST(Hash, CombineFor)
	{
		float x(1), y(2), z(3), w(4);
		std::size_t expected_hash =
		        hash_combine(std::hash<float>()(x),
		                     hash_combine(std::hash<float>()(y),
		                                  hash_combine(std::hash<float>()(z), std::hash<float>()(w))));

		EXPECT_EQ(expected_hash, hash_combine_for(x, y, z, w));
	}
}
