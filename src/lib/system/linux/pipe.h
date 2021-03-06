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

#ifndef RAYNI_LIB_SYSTEM_LINUX_PIPE_H
#define RAYNI_LIB_SYSTEM_LINUX_PIPE_H

#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <system_error>

#include "lib/function/result.h"
#include "lib/system/unique_fd.h"

namespace Rayni
{
	class Pipe
	{
	public:
		static Result<Pipe> create(int flags = 0)
		{
			std::array<int, 2> fds = {-1, -1};

			if (pipe2(fds.data(), flags) != 0)
				return Error("pipe2() failed", std::error_code(errno, std::system_category()));

			Pipe pipe;

			pipe.read_fd_ = UniqueFD(fds[0]);
			pipe.write_fd_ = UniqueFD(fds[1]);

			return pipe;
		}

		void close_fds()
		{
			close_read_fd();
			close_write_fd();
		}

		void close_read_fd()
		{
			read_fd_.close();
		}

		void close_write_fd()
		{
			write_fd_.close();
		}

		int read_fd() const
		{
			return read_fd_.get();
		}

		int write_fd() const
		{
			return write_fd_.get();
		}

		template <typename Buffer>
		Result<std::size_t> read(Buffer &buffer) const
		{
			ssize_t bytes_read;

			while (true) {
				bytes_read = ::read(read_fd(), buffer.data(), buffer.size());

				if (bytes_read >= 0)
					break;
				if (errno != EINTR)
					return Error("pipe read() error",
					             std::error_code(errno, std::system_category()));
			}

			return static_cast<std::size_t>(bytes_read);
		}

		Result<std::size_t> read_append_to_string(std::string &str) const
		{
			std::array<char, PIPE_BUF> buffer;
			Result<std::size_t> bytes_read = read(buffer);

			if (bytes_read && *bytes_read > 0)
				str.append(buffer.data(), *bytes_read);

			return bytes_read;
		}

		template <typename Buffer>
		Result<std::size_t> write(const Buffer &buffer) const
		{
			ssize_t bytes_written;

			while (true) {
				bytes_written = ::write(write_fd(), buffer.data(), buffer.size());

				if (bytes_written >= 0)
					break;
				if (errno != EINTR)
					return Error("pipe write() error",
					             std::error_code(errno, std::system_category()));
			}

			return static_cast<std::size_t>(bytes_written);
		}

	private:
		UniqueFD read_fd_;
		UniqueFD write_fd_;
	};
}

#endif // RAYNI_LIB_SYSTEM_LINUX_PIPE_H
