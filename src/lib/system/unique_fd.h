// This file is part of Rayni.
//
// Copyright (C) 2013-2020 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_SYSTEM_UNIQUE_FD_H
#define RAYNI_LIB_SYSTEM_UNIQUE_FD_H

#include <unistd.h>

#include <utility>

namespace Rayni
{
	class UniqueFD
	{
	public:
		UniqueFD() = default;

		explicit UniqueFD(int fd) : fd_(fd)
		{
		}

		UniqueFD(const UniqueFD &other) = delete;

		UniqueFD(UniqueFD &&other) noexcept : fd_(std::exchange(other.fd_, -1))
		{
		}

		~UniqueFD()
		{
			close();
		}

		UniqueFD &operator=(const UniqueFD &other) = delete;

		UniqueFD &operator=(UniqueFD &&other) noexcept
		{
			close();
			fd_ = std::exchange(other.fd_, -1);
			return *this;
		}

		int get() const
		{
			return fd_;
		}

		void close() noexcept
		{
			if (fd_ == -1)
				return;

			::close(fd_);
			fd_ = -1;
		}

	private:
		int fd_ = -1;
	};
}

#endif // RAYNI_LIB_SYSTEM_UNIQUE_FD_H
