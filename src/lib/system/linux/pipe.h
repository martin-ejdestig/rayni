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

#ifndef RAYNI_LIB_SYSTEM_LINUX_PIPE_H
#define RAYNI_LIB_SYSTEM_LINUX_PIPE_H

#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <cstdlib>
#include <system_error>
#include <utility>

namespace Rayni
{
	class Pipe
	{
	public:
		Pipe() : Pipe(0)
		{
		}

		explicit Pipe(int flags)
		{
			if (pipe2(fds.data(), flags) != 0)
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

		template <typename Buffer>
		std::size_t read(Buffer &buffer) const
		{
			ssize_t bytes_read;

			while (true)
			{
				bytes_read = ::read(read_fd(), buffer.data(), buffer.size());

				if (bytes_read >= 0)
					break;
				if (errno != EINTR)
					throw std::system_error(errno, std::system_category(), "pipe read() error");
			}

			return static_cast<std::size_t>(bytes_read);
		}

		std::size_t read_append_to_string(std::string &str)
		{
			std::array<char, 1024> buffer;
			std::size_t bytes_read = read(buffer);

			if (bytes_read > 0)
				str.append(buffer.data(), bytes_read);

			return bytes_read;
		}

	private:
		static void safe_close(int &fd)
		{
			if (fd == -1)
				return;

			close(fd);
			fd = -1;
		}

		std::array<int, 2> fds = {-1, -1};
	};
}

#endif // RAYNI_LIB_SYSTEM_LINUX_PIPE_H
