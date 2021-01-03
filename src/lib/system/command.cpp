// This file is part of Rayni.
//
// Copyright (C) 2013-2021 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/system/command.h"

#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstdlib>
#include <system_error>
#include <utility>

#include "lib/function/result.h"
#include "lib/system/linux/pipe.h"

namespace Rayni
{
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

		[[noreturn]] void child_exec(std::vector<std::string> &&args_in,
		                             Pipe &stdout_pipe,
		                             Pipe &stderr_pipe) noexcept
		{
			if (!duplicate_fd(stdout_pipe.write_fd(), STDOUT_FILENO) ||
			    !duplicate_fd(stderr_pipe.write_fd(), STDERR_FILENO))
				std::_Exit(CHILD_SETUP_FAILURE_EXIT_CODE);

			stdout_pipe.close_fds();
			stderr_pipe.close_fds();

			std::vector<std::string> args = std::move(args_in);
			std::vector<char *> argv;
			argv.reserve(args.size() + 1);
			for (std::string &arg : args)
				argv.push_back(arg.data());
			argv.push_back(nullptr);

			execvp(args[0].data(), &argv[0]);
			std::_Exit(CHILD_SETUP_FAILURE_EXIT_CODE);
		}

		Result<void> read_pipes(Pipe &stdout_pipe,
		                        Pipe &stderr_pipe,
		                        std::string &stdout_str,
		                        std::string &stderr_str)
		{
			static constexpr short int PIPE_READ_EVENTS = POLLIN | POLLHUP;

			std::array<pollfd, 2> poll_fds;
			poll_fds[0].fd = stdout_pipe.read_fd();
			poll_fds[0].events = PIPE_READ_EVENTS;
			poll_fds[1].fd = stderr_pipe.read_fd();
			poll_fds[1].events = PIPE_READ_EVENTS;

			while (poll_fds[0].fd >= 0 || poll_fds[1].fd >= 0) {
				while (poll(poll_fds.data(), poll_fds.size(), -1) == -1) {
					if (errno != EINTR)
						return Error("poll() for child pipes failed",
						             std::error_code(errno, std::system_category()));
				}

				try {
					if ((poll_fds[0].revents & PIPE_READ_EVENTS) != 0)
						if (stdout_pipe.read_append_to_string(stdout_str) == 0)
							poll_fds[0].fd = -1;

					if ((poll_fds[1].revents & PIPE_READ_EVENTS) != 0)
						if (stderr_pipe.read_append_to_string(stderr_str) == 0)
							poll_fds[1].fd = -1;
				} catch (const std::exception &e) {
					return Error("failed to read child pipes: " + std::string(e.what()));
				}
			}

			return {};
		}

		Result<int> child_wait(pid_t child_pid)
		{
			int status = 0;

			while (waitpid(child_pid, &status, 0) == -1) {
				if (errno != EINTR)
					return Error("waitpid() for pid " + std::to_string(child_pid) + " failed",
					             std::error_code(errno, std::system_category()));
			}

			if (!WIFEXITED(status))
				return Error("pid " + std::to_string(child_pid) + " exited abnormally");

			if (WEXITSTATUS(status) == CHILD_SETUP_FAILURE_EXIT_CODE)
				return Error("pid " + std::to_string(child_pid) + " setup failure");

			return WEXITSTATUS(status);
		}
	}

	Result<CommandOutput> command_run(std::vector<std::string> &&args)
	{
		Pipe stdout_pipe(O_CLOEXEC);
		Pipe stderr_pipe(O_CLOEXEC);

		pid_t child_pid = fork();
		if (child_pid == -1)
			return Error("fork() failed", std::error_code(errno, std::system_category()));

		if (child_pid == 0)
			child_exec(std::move(args), stdout_pipe, stderr_pipe);

		stdout_pipe.close_write_fd();
		stderr_pipe.close_write_fd();

		CommandOutput output;
		Result<void> pipe_read_result = read_pipes(stdout_pipe, stderr_pipe, output.stdout, output.stderr);

		stdout_pipe.close_read_fd();
		stderr_pipe.close_read_fd();

		Result<int> wait_result = child_wait(child_pid);
		if (!wait_result)
			return wait_result.error();

		if (!pipe_read_result)
			return pipe_read_result.error();

		output.exit_code = *wait_result;

		return output;
	}
}
