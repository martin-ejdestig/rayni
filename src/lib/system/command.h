/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2016 Martin Ejdestig <marejde@gmail.com>
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

#ifndef _RAYNI_LIB_SYSTEM_COMMAND_H_
#define _RAYNI_LIB_SYSTEM_COMMAND_H_

#include <experimental/optional>
#include <string>

namespace Rayni
{
	class Command
	{
	public:
		struct Result;

		explicit Command(const std::string &command_string) : command_string(command_string)
		{
		}

		std::experimental::optional<Result> run() const;

	private:
		class PcloseDeleter;

		const std::string command_string;
	};

	struct Command::Result
	{
		std::string stdout;
		int exit_code = 0;
	};
}

#endif // _RAYNI_LIB_SYSTEM_COMMAND_H_
