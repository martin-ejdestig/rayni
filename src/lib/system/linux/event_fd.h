/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2014-2018 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_SYSTEM_LINUX_EVENT_FD_H
#define RAYNI_LIB_SYSTEM_LINUX_EVENT_FD_H

#include <sys/eventfd.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <system_error>

#include "lib/system/unique_fd.h"

namespace Rayni
{
	class EventFD
	{
	public:
		// write()/add will block if value will exceed MAX_VALUE (and EFD_NONBLOCK is not set).
		static constexpr std::uint64_t MAX_VALUE = 0xfffffffffffffffe;

		EventFD() : event_fd_(eventfd(0, EFD_CLOEXEC))
		{
			if (event_fd_.get() == -1)
				throw std::system_error(errno, std::system_category(), "eventfd() failed");
		}

		int fd() const
		{
			return event_fd_.get();
		}

		std::uint64_t read() const
		{
			std::uint64_t value = 0;

			while (::read(event_fd_.get(), &value, sizeof value) != static_cast<ssize_t>(sizeof value))
				if (errno != EINTR)
					throw std::system_error(errno,
					                        std::system_category(),
					                        "read() from eventfd failed");

			return value;
		}

		void write(std::uint64_t value) const
		{
			while (::write(event_fd_.get(), &value, sizeof value) != static_cast<ssize_t>(sizeof value))
				if (errno != EINTR)
					throw std::system_error(errno,
					                        std::system_category(),
					                        "write() to eventfd failed");
		}

	private:
		UniqueFD event_fd_;
	};
}

#endif // RAYNI_LIB_SYSTEM_LINUX_EVENT_FD_H
