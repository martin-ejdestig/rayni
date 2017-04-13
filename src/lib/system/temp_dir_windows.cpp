/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2016-2017 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/system/temp_dir.h"

namespace Rayni
{
	std::experimental::filesystem::path temp_dir_create_unique()
	{
		// TODO: Implement. Or will there be something in the final version of
		//       std::filesystem in C++17? See TODO in temp_dir_posix.cpp for why available
		//       functionality in draft is not enough.
		static_assert(false, "not implemented");
		return std::experimental::filesystem::path();
	}
}
