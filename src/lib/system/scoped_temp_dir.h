/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2016 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_SYSTEM_SCOPED_TEMP_DIR_H
#define RAYNI_LIB_SYSTEM_SCOPED_TEMP_DIR_H

#include <experimental/filesystem>

namespace Rayni
{
	class ScopedTempDir
	{
	public:
		ScopedTempDir();
		~ScopedTempDir();

		const std::experimental::filesystem::path &path() const
		{
			return path_;
		}

	private:
		const std::experimental::filesystem::path path_;
	};
}

#endif // RAYNI_LIB_SYSTEM_SCOPED_TEMP_DIR_H
