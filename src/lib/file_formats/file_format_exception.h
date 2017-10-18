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

#ifndef RAYNI_LIB_FILE_FORMATS_FILE_FORMAT_EXCEPTION_H
#define RAYNI_LIB_FILE_FORMATS_FILE_FORMAT_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace Rayni
{
	class FileFormatException : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error;

		FileFormatException(const std::string &prefix, const std::string &str) :
		        std::runtime_error(prefix.empty() ? str : prefix + ": " + str)
		{
		}
	};
}

#endif // RAYNI_LIB_FILE_FORMATS_FILE_FORMAT_EXCEPTION_H
