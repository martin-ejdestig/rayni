// This file is part of Rayni.
//
// Copyright (C) 2016-2021 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/io/file.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iterator>

#include "lib/function/result.h"
#include "lib/system/memory_mapped_file.h"

namespace Rayni
{
	Result<std::vector<std::uint8_t>> file_read(const std::string &path)
	{
		MemoryMappedFile file;
		if (auto r = file.map(path); !r)
			return r.error();

		std::vector<std::uint8_t> buffer;
		buffer.resize(file.size());

		if (file.size() > 0)
			std::memcpy(buffer.data(), file.data(), buffer.size());

		return buffer;
	}

	Result<void> file_write(const std::string &path, const std::vector<std::uint8_t> &data)
	{
		std::ofstream file(path, std::ios_base::binary);

		std::copy(data.begin(), data.end(), std::ostream_iterator<std::uint8_t>(file));

		if (!file.good())
			return Error(path + ": failed to write to file");

		return {};
	}
}
