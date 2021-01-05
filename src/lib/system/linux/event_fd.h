// This file is part of Rayni.
//
// Copyright (C) 2014-2021 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_SYSTEM_LINUX_EVENT_FD_H
#define RAYNI_LIB_SYSTEM_LINUX_EVENT_FD_H

#include <sys/eventfd.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <system_error>

#include "lib/function/result.h"
#include "lib/system/unique_fd.h"

namespace Rayni
{
	class EventFD
	{
	public:
		// write()/add will block if value will exceed MAX_VALUE (and EFD_NONBLOCK is not set).
		static constexpr std::uint64_t MAX_VALUE = 0xfffffffffffffffe;

		static Result<EventFD> create()
		{
			EventFD e;

			e.event_fd_ = UniqueFD(eventfd(0, EFD_CLOEXEC));
			if (e.event_fd_.get() == -1)
				return Error("eventfd() failed", std::error_code(errno, std::system_category()));

			return e;
		}

		int fd() const
		{
			return event_fd_.get();
		}

		Result<std::uint64_t> read() const
		{
			std::uint64_t value = 0;

			while (::read(event_fd_.get(), &value, sizeof value) != static_cast<ssize_t>(sizeof value))
				if (errno != EINTR)
					return Error("read() from eventfd failed",
					             std::error_code(errno, std::system_category()));

			return value;
		}

		Result<void> write(std::uint64_t value) const
		{
			while (::write(event_fd_.get(), &value, sizeof value) != static_cast<ssize_t>(sizeof value))
				if (errno != EINTR)
					return Error("write() to eventfd failed",
					             std::error_code(errno, std::system_category()));
			return {};
		}

	private:
		UniqueFD event_fd_;
	};
}

#endif // RAYNI_LIB_SYSTEM_LINUX_EVENT_FD_H
