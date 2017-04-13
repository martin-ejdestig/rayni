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

#ifndef RAYNI_LIB_STRING_BASE85_H
#define RAYNI_LIB_STRING_BASE85_H

#include <cstdint>
#include <experimental/optional>
#include <string>
#include <vector>

namespace Rayni
{
	std::experimental::optional<std::vector<std::uint8_t>> base85_decode(const std::string &str);
}

#endif // RAYNI_LIB_STRING_BASE85_H
