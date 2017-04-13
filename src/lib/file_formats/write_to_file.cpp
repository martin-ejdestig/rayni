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

#include "lib/file_formats/write_to_file.h"

#include <algorithm>
#include <fstream>
#include <iterator>

#include "lib/file_formats/file_format_exception.h"

namespace Rayni
{
	void write_to_file(const std::string &path, const std::vector<std::uint8_t> &data)
	{
		std::ofstream file(path, std::ios_base::binary);

		std::copy(data.begin(), data.end(), std::ostream_iterator<std::uint8_t>(file));

		if (!file.good())
			throw FileFormatException(path, "failed to write to file");
	}
}
