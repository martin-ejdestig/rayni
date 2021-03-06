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

#ifndef RAYNI_LIB_SYSTEM_LINUX_EPOLL_H
#define RAYNI_LIB_SYSTEM_LINUX_EPOLL_H

#include <sys/epoll.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <system_error>
#include <type_traits>

#include "lib/function/result.h"
#include "lib/math/bitmask.h"
#include "lib/system/unique_fd.h"

namespace Rayni
{
	class Epoll
	{
	public:
		enum class Flag : std::uint32_t;

		using EventCount = unsigned int;
		using Flags = Bitmask<Flag>;

		class Event : private epoll_event
		{
		public:
			friend Epoll;

			Event()
			{
				events = 0;
				data.u64 = 0;
			}

			bool is_set(Flags flags_to_check) const
			{
				return flags().is_set(flags_to_check);
			}

			Flags flags() const
			{
				return Flags(events);
			}

			int fd() const
			{
				return data.fd;
			}

			void *ptr() const
			{
				return data.ptr;
			}
		};

		static Result<Epoll> create()
		{
			Epoll e;

			e.epoll_fd_ = UniqueFD(epoll_create1(EPOLL_CLOEXEC));
			if (e.epoll_fd_.get() == -1)
				return Error("epoll_create1() failed", std::error_code(errno, std::system_category()));

			return e;
		}

		int fd() const
		{
			return epoll_fd_.get();
		}

		Result<void> add(int fd, Flags flags)
		{
			Event event;
			event.events = flags.value();
			event.data.fd = fd;

			return ctl(EPOLL_CTL_ADD, fd, &event);
		}

		Result<void> add(int fd, Flags flags, void *ptr)
		{
			Event event;
			event.events = flags.value();
			event.data.ptr = ptr;

			return ctl(EPOLL_CTL_ADD, fd, &event);
		}

		Result<void> modify(int fd, Flags flags)
		{
			Event event;
			event.events = flags.value();
			event.data.fd = fd;

			return ctl(EPOLL_CTL_MOD, fd, &event);
		}

		Result<void> modify(int fd, Flags flags, void *ptr)
		{
			Event event;
			event.events = flags.value();
			event.data.ptr = ptr;

			return ctl(EPOLL_CTL_MOD, fd, &event);
		}

		Result<void> remove(int fd) const
		{
			return ctl(EPOLL_CTL_DEL, fd, nullptr);
		}

		template <typename Events>
		Result<EventCount> wait(Events &events, std::chrono::milliseconds timeout) const
		{
			return wait(events.data(), events.size(), timeout);
		}

		template <typename Events>
		Result<EventCount> wait(Events &events) const
		{
			return wait(events, std::chrono::milliseconds(-1));
		}

	private:
		static_assert(sizeof(Event) == sizeof(epoll_event), "Accidentally added member to Event?");

		Result<void> ctl(int op, int fd, Event *event) const
		{
			if (epoll_ctl(epoll_fd_.get(), op, fd, event) == -1)
				return Error("epoll_ctl() failed", std::error_code(errno, std::system_category()));
			return {};
		}

		Result<EventCount> wait(Event *events, std::size_t max_events, std::chrono::milliseconds timeout) const
		{
			using MsRep = std::chrono::milliseconds::rep;

			static constexpr int NUM_EVENTS_MAX_INT = std::numeric_limits<int>::max();
			static constexpr int TIMEOUT_MAX_INT = std::numeric_limits<int>::max();

			int max_events_int = int(std::min(max_events, std::size_t(NUM_EVENTS_MAX_INT)));
			int timeout_int = int(std::clamp(timeout.count(), MsRep(-1), MsRep(TIMEOUT_MAX_INT)));
			int ret = 0;

			while (true) // Loops if EINTR. Will cause timeout "drift" but leave as is for now.
			{
				ret = epoll_wait(epoll_fd_.get(), events, max_events_int, timeout_int);

				if (ret >= 0)
					break;
				if (errno != EINTR)
					return Error("epoll_wait() failed",
					             std::error_code(errno, std::system_category()));
			}

			return static_cast<EventCount>(ret);
		}

		UniqueFD epoll_fd_;
	};

	enum class Epoll::Flag : std::uint32_t
	{
		IN = EPOLLIN,
		PRI = EPOLLPRI,
		OUT = EPOLLOUT,
		RDNORM = EPOLLRDNORM,
		RDBAND = EPOLLRDBAND,
		WRNORM = EPOLLWRNORM,
		WRBAND = EPOLLWRBAND,
		MSG = EPOLLMSG,
		ERR = EPOLLERR,
		HUP = EPOLLHUP,
		RDHUP = EPOLLRDHUP,
		EXCLUSIVE = EPOLLEXCLUSIVE,
		WAKEUP = EPOLLWAKEUP,
		ONESHOT = EPOLLONESHOT,
		ET = EPOLLET // TODO: Provide EDGE_TRIGGERED alias? ET (and EPOLLET) are bad names.
	};

	RAYNI_BITMASK_GLOBAL_OPERATORS(Epoll::Flags)
}

#endif // RAYNI_LIB_SYSTEM_LINUX_EPOLL_H
