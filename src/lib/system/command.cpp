/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2017 Martin Ejdestig <marejde@gmail.com>
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

#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstdlib>
#include <utility>

#include "lib/system/linux/pipe.h"

namespace
{
	constexpr int CHILD_SETUP_FAILURE_EXIT_CODE = 127;

	bool duplicate_fd(int fd, int other_fd)
	{
		while (dup2(fd, other_fd) == -1)
			if (errno != EINTR)
				return false;
		return true;
	}

	[[noreturn]] void child_exec(const std::vector<std::string> &args,
	                             Rayni::Pipe &stdout_pipe,
	                             Rayni::Pipe &stderr_pipe) noexcept
	{
		if (!duplicate_fd(stdout_pipe.write_fd(), STDOUT_FILENO) ||
		    !duplicate_fd(stderr_pipe.write_fd(), STDERR_FILENO))
			std::_Exit(CHILD_SETUP_FAILURE_EXIT_CODE);

		stdout_pipe.close_fds();
		stderr_pipe.close_fds();

		std::vector<const char *> argv;
		argv.reserve(args.size() + 1);
		for (const std::string &arg : args)
			argv.push_back(arg.data());
		argv.push_back(nullptr);

		execvp(argv[0], const_cast<char **>(&argv[0])); // NOLINT: cppcoreguidelines-pro-type-const-cast
		std::_Exit(CHILD_SETUP_FAILURE_EXIT_CODE);
	}

	bool read_pipes(Rayni::Pipe &stdout_pipe,
	                Rayni::Pipe &stderr_pipe,
	                std::string &stdout_str,
	                std::string &stderr_str)
	{
		static constexpr short int PIPE_READ_EVENTS = POLLIN | POLLHUP;

		std::array<pollfd, 2> poll_fds;
		poll_fds[0].fd = stdout_pipe.read_fd();
		poll_fds[0].events = PIPE_READ_EVENTS;
		poll_fds[1].fd = stderr_pipe.read_fd();
		poll_fds[1].events = PIPE_READ_EVENTS;

		while (poll_fds[0].fd >= 0 || poll_fds[1].fd >= 0)
		{
			while (poll(poll_fds.data(), poll_fds.size(), -1) == -1)
				if (errno != EINTR)
					return false;

			try
			{
				if ((poll_fds[0].revents & PIPE_READ_EVENTS) != 0)
					if (stdout_pipe.read_append_to_string(stdout_str) == 0)
						poll_fds[0].fd = -1;

				if ((poll_fds[1].revents & PIPE_READ_EVENTS) != 0)
					if (stderr_pipe.read_append_to_string(stderr_str) == 0)
						poll_fds[1].fd = -1;
			}
			catch (const std::exception &)
			{
				return false;
			}
		}

		return true;
	}

	class ChildProcess
	{
	public:
		explicit ChildProcess(pid_t pid) : child_pid(pid)
		{
		}

		ChildProcess(const ChildProcess &other) = delete;
		ChildProcess(ChildProcess &&other) = delete;

		~ChildProcess()
		{
			if (child_pid != -1)
				wait();
		}

		ChildProcess &operator=(const ChildProcess &other) = delete;
		ChildProcess &operator=(ChildProcess &&other) = delete;

		bool wait()
		{
			pid_t pid = std::exchange(child_pid, -1);

			while (waitpid(pid, &status, 0) == -1)
				if (errno != EINTR)
					return false;

			return WIFEXITED(status) && WEXITSTATUS(status) != CHILD_SETUP_FAILURE_EXIT_CODE;
		}

		pid_t pid()
		{
			return child_pid;
		}

		int exit_code() const
		{
			return WEXITSTATUS(status);
		}

	private:
		pid_t child_pid = -1;
		int status = 0;
	};
}

namespace Rayni
{
	std::experimental::optional<Command::Result> Command::run() const
	{
		Pipe stdout_pipe(O_CLOEXEC);
		Pipe stderr_pipe(O_CLOEXEC);

		ChildProcess child_process(fork());
		if (child_process.pid() == -1)
			return std::experimental::nullopt;

		if (child_process.pid() == 0)
			child_exec(args, stdout_pipe, stderr_pipe);

		stdout_pipe.close_write_fd();
		stderr_pipe.close_write_fd();

		Result result;
		bool read_success = read_pipes(stdout_pipe, stderr_pipe, result.stdout, result.stderr);

		stdout_pipe.close_read_fd();
		stderr_pipe.close_read_fd();

		if (!child_process.wait())
			return std::experimental::nullopt;

		result.exit_code = child_process.exit_code();

		if (!read_success)
			return std::experimental::nullopt;

		return result;
	}
}
