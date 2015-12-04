/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2015 Martin Ejdestig <marejde@gmail.com>
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

#include <sys/wait.h>

#include <array>
#include <cstdio>
#include <experimental/optional>
#include <memory>
#include <string>

#include "lib/system/command.h"

// TODO: Non POSIX compliant systems. Prepared to do minor things (i.e. for Windows add
//       "std::FILE *popen() { return _popen(); }" in an anonymous namespace etc.) but if it is too
//       much work, forget about it.

namespace Rayni
{
	class Command::PcloseDeleter
	{
	public:
		void operator()(std::FILE *file)
		{
			return_value = pclose(file);
		}

		bool pclose_failed() const
		{
			return return_value == -1;
		}

		bool child_ran_and_exited_normally() const
		{
			if (!WIFEXITED(return_value))
				return false;

			// See pclose() (on Linux) for magic,,, buggy POSIX spec. magic. Cannot
			// determine if it was shell or command that was not found.
			static const int SHELL_OR_COMMAND_NOT_FOUND_EXIT_CODE = 127;

			return WEXITSTATUS(return_value) != SHELL_OR_COMMAND_NOT_FOUND_EXIT_CODE;
		}

		int exit_code() const
		{
			return WEXITSTATUS(return_value);
		}

	private:
		int return_value = 0;
	};

	std::experimental::optional<Command::Result> Command::run() const
	{
		std::unique_ptr<std::FILE, PcloseDeleter> file(popen(command_string.c_str(), "r"));
		if (!file)
			return std::experimental::nullopt;

		Result result;
		std::array<char, 1024> buffer;

		while (std::fgets(buffer.data(), buffer.size(), file.get()))
			result.stdout += buffer.data();

		if (!std::feof(file.get()))
			return std::experimental::nullopt;

		file.reset();
		auto deleter = file.get_deleter();

		if (deleter.pclose_failed() || !deleter.child_ran_and_exited_normally())
			return std::experimental::nullopt;

		result.exit_code = deleter.exit_code();

		return std::experimental::make_optional(result);
	}
}
