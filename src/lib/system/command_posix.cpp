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

#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstdlib>
#include <system_error>
#include <utility>

namespace
{
	constexpr int CHILD_SETUP_FAILURE_EXIT_CODE = 127;

	class Pipe
	{
	public:
		Pipe()
		{
			// Note: No O_NONBLOCK. Child should block if parent does not keep up with reading.
			if (pipe2(fds.data(), O_CLOEXEC) != 0)
				throw std::system_error(errno, std::system_category(), "pipe2() failed");
		}

		Pipe(const Pipe &other) = delete;

		Pipe(Pipe &&other) noexcept : fds(std::exchange(other.fds, {-1, -1}))
		{
		}

		~Pipe()
		{
			close_fds();
		}

		Pipe &operator=(const Pipe &other) = delete;

		Pipe &operator=(Pipe &&other) noexcept
		{
			close_fds();
			fds = std::exchange(other.fds, {-1, -1});
			return *this;
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

		bool duplicate_write_fd_to(int other_fd) const
		{
			while (dup2(write_fd(), other_fd) == -1)
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
				bytes_read = ::read(read_fd(), buffer.data(), buffer.size());

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

	[[noreturn]] void child_exec(const std::vector<std::string> &args,
	                             Pipe &stdout_pipe,
	                             Pipe &stderr_pipe) noexcept
	{
		if (!stdout_pipe.duplicate_write_fd_to(STDOUT_FILENO) ||
		    !stderr_pipe.duplicate_write_fd_to(STDERR_FILENO))
			std::_Exit(CHILD_SETUP_FAILURE_EXIT_CODE);

		stdout_pipe.close_fds();
		stderr_pipe.close_fds();

		std::vector<char *> argv;
		for (const std::string &arg : args)
			argv.push_back(const_cast<char *>(arg.data()));
		argv.push_back(nullptr);

		execvp(argv[0], &argv[0]);
		std::_Exit(CHILD_SETUP_FAILURE_EXIT_CODE);
	}

	bool read_pipes(Pipe &stdout_pipe, Pipe &stderr_pipe, std::string &stdout_str, std::string &stderr_str)
	{
		static constexpr short int PIPE_READ_EVENTS = POLLIN | POLLHUP;

		auto read_pipe_if_event_occurred = [](pollfd &poll_fd, Pipe &pipe, std::string &dest_str) {
			if ((poll_fd.revents & PIPE_READ_EVENTS) == 0)
				return true;

			std::array<char, 1024> buffer;
			ssize_t bytes_read = pipe.read(buffer);

			if (bytes_read > 0)
				dest_str.append(buffer.data(), static_cast<std::string::size_type>(bytes_read));
			else if (bytes_read == 0)
				poll_fd.fd = -1;
			else
				return false;

			return true;
		};

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

			if (!read_pipe_if_event_occurred(poll_fds[0], stdout_pipe, stdout_str) ||
			    !read_pipe_if_event_occurred(poll_fds[1], stderr_pipe, stderr_str))
				return false;
		}

		return true;
	}
}

namespace Rayni
{
	std::experimental::optional<Command::Result> Command::run() const
	{
		Pipe stdout_pipe, stderr_pipe;

		pid_t pid = fork();
		if (pid == -1)
			return std::experimental::nullopt;

		if (pid == 0)
			child_exec(args, stdout_pipe, stderr_pipe);

		stdout_pipe.close_write_fd();
		stderr_pipe.close_write_fd();

		Result result;
		bool read_success = read_pipes(stdout_pipe, stderr_pipe, result.stdout, result.stderr);

		stdout_pipe.close_read_fd();
		stderr_pipe.close_read_fd();

		int status;
		while (waitpid(pid, &status, 0) == -1)
			if (errno != EINTR)
				return std::experimental::nullopt;

		if (!WIFEXITED(status))
			return std::experimental::nullopt;

		result.exit_code = WEXITSTATUS(status);

		if (result.exit_code == CHILD_SETUP_FAILURE_EXIT_CODE || !read_success)
			return std::experimental::nullopt;

		return result;
	}
}
