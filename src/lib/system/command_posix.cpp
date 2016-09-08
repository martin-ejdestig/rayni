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

#include <poll.h>
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

		bool duplicate_write_fd_to(int other_fd) const
		{
			while (dup2(fds[1], other_fd) == -1)
				if (errno != EINTR)
					return false;
			return true;
		}

		template <typename Buffer>
		ssize_t read(Buffer &buffer) const
		{
			ssize_t bytes_read;

			while (true)
			{
				bytes_read = ::read(fds[0], buffer.data(), buffer.size());

				if (bytes_read >= 0)
					break;
				if (errno != EINTR)
					break;
			}

			return bytes_read;
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
	static bool read_pipes(Pipe &stdout_pipe, Pipe &stderr_pipe, Command::Result &result)
	{
		std::array<char, 1024> buffer;
		std::array<pollfd, 2> poll_fds;

		poll_fds[0].fd = stdout_pipe.read_fd();
		poll_fds[0].events = POLLIN | POLLHUP;
		poll_fds[1].fd = stderr_pipe.read_fd();
		poll_fds[1].events = POLLIN | POLLHUP;

		while (poll_fds[0].fd >= 0 || poll_fds[1].fd >= 0)
		{
			while (poll(poll_fds.data(), poll_fds.size(), -1) == -1)
				if (errno != EINTR)
					return false;

			if (poll_fds[0].revents & (POLLIN | POLLHUP))
			{
				ssize_t bytes_read = stdout_pipe.read(buffer);

				if (bytes_read > 0)
					result.stdout.append(buffer.data(),
					                     static_cast<std::string::size_type>(bytes_read));
				else if (bytes_read == 0)
					poll_fds[0].fd = -1;
				else
					return false;
			}

			if (poll_fds[1].revents & (POLLIN | POLLHUP))
			{
				ssize_t bytes_read = stderr_pipe.read(buffer);

				if (bytes_read > 0)
					result.stderr.append(buffer.data(),
					                     static_cast<std::string::size_type>(bytes_read));
				else if (bytes_read == 0)
					poll_fds[1].fd = -1;
				else
					return false;
			}
		}

		return true;
	}

	std::experimental::optional<Command::Result> Command::run() const
	{
		Pipe stdout_pipe, stderr_pipe;
		if (!stdout_pipe.open() || !stderr_pipe.open())
			return std::experimental::nullopt;

		pid_t pid = fork();
		if (pid == -1)
			return std::experimental::nullopt;

		if (pid == 0)
		{
			if (!stdout_pipe.duplicate_write_fd_to(STDOUT_FILENO) ||
			    !stderr_pipe.duplicate_write_fd_to(STDERR_FILENO))
				child_exit_failure();

			stdout_pipe.close_fds();
			stderr_pipe.close_fds();

			child_exec(args);
		}

		stdout_pipe.close_write_fd();
		stderr_pipe.close_write_fd();

		Result result;
		bool read_success = read_pipes(stdout_pipe, stderr_pipe, result);
		int status;

		stdout_pipe.close_read_fd();
		stderr_pipe.close_read_fd();

		while (waitpid(pid, &status, 0) == -1)
			if (errno != EINTR)
				return std::experimental::nullopt;

		if (!WIFEXITED(status))
			return std::experimental::nullopt;

		result.exit_code = WEXITSTATUS(status);

		if (result.exit_code == CHILD_SETUP_EXIT_FAILURE || !read_success)
			return std::experimental::nullopt;

		return std::experimental::make_optional(result);
	}
}
