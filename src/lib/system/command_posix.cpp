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

#include "lib/system/command.h"

#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdlib>

namespace
{
	constexpr int CHILD_SETUP_EXIT_FAILURE = 127;

	class Pipe
	{
	public:
		~Pipe()
		{
			close_fds();
		}

		bool open()
		{
			return pipe(fds.data()) == 0;
		}

		void close_fds()
		{
			close_read_fd();
			close_write_fd();
		}

		void close_read_fd()
		{
			safe_close(fds[0]);
		}

		void close_write_fd()
		{
			safe_close(fds[1]);
		}

		int read_fd() const
		{
			return fds[0];
		}

		int write_fd() const
		{
			return fds[1];
		}

	private:
		static void safe_close(int &fd)
		{
			if (fd != -1)
			{
				close(fd);
				fd = -1;
			}
		}

		std::array<int, 2> fds = {-1, -1};
	};

	bool dup2_handle_eintr(int fd1, int fd2)
	{
		while (dup2(fd1, fd2) == -1)
			if (errno != EINTR)
				return false;
		return true;
	}

	bool waitpid_handle_eintr(pid_t pid, int *status)
	{
		while (waitpid(pid, status, 0) == -1)
			if (errno != EINTR)
				return false;
		return true;
	}

	template <typename Buffer>
	ssize_t read_handle_eintr(int fd, Buffer &buffer)
	{
		ssize_t bytes_read;

		while (true)
		{
			bytes_read = read(fd, buffer.data(), buffer.size());

			if (bytes_read >= 0)
				break;
			if (errno != EINTR)
				break;
		}

		return bytes_read;
	}

	[[noreturn]] void child_exit_failure()
	{
		std::_Exit(CHILD_SETUP_EXIT_FAILURE);
	}

	[[noreturn]] void child_exec(const std::vector<std::string> &args)
	{
		std::vector<char *> argv(args.size() + 1, nullptr);
		std::transform(args.begin(), args.end(), argv.begin(), [](auto &s) {
			return const_cast<char *>(s.data());
		});

		execvp(argv[0], &argv[0]);
		child_exit_failure();
	}
}

namespace Rayni
{
	std::experimental::optional<Command::Result> Command::run() const
	{
		Pipe stdout_pipe;
		if (!stdout_pipe.open())
			return std::experimental::nullopt;

		pid_t pid = fork();
		if (pid == -1)
			return std::experimental::nullopt;

		if (pid == 0)
		{
			if (!dup2_handle_eintr(stdout_pipe.write_fd(), STDOUT_FILENO))
				child_exit_failure();
			stdout_pipe.close_fds();

			child_exec(args);
		}

		stdout_pipe.close_write_fd();

		Result result;
		ssize_t bytes_read = 0;
		std::array<char, 1024> buffer;

		while (true)
		{
			bytes_read = read_handle_eintr(stdout_pipe.read_fd(), buffer);
			if (bytes_read <= 0)
				break;

			result.stdout.append(buffer.data(), static_cast<std::string::size_type>(bytes_read));
		}

		int status;
		if (!waitpid_handle_eintr(pid, &status))
			return std::experimental::nullopt;

		if (bytes_read < 0 || !WIFEXITED(status) || WEXITSTATUS(status) == CHILD_SETUP_EXIT_FAILURE)
			return std::experimental::nullopt;

		result.exit_code = WEXITSTATUS(status);

		return std::experimental::make_optional(result);
	}
}
