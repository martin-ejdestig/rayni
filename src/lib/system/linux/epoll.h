/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2014-2017 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_SYSTEM_LINUX_EPOLL_H
#define RAYNI_LIB_SYSTEM_LINUX_EPOLL_H

#include <sys/epoll.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <system_error>
#include <type_traits>
#include <utility>

#include "lib/math/bitmask.h"

namespace Rayni
{
	class Epoll
	{
	public:
		enum class Flag : std::uint32_t;

		using EventCount = unsigned int;
		using Mask = Bitmask<Flag>;

		class Event : private epoll_event
		{
		public:
			friend Epoll;

			Event()
			{
				events = 0;
				data.u64 = 0;
			}

			bool is_set(Mask mask) const
			{
				return Mask(events).is_set(mask);
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

		Epoll() : epoll_fd(epoll_create1(EPOLL_CLOEXEC))
		{
			if (epoll_fd == -1)
				throw std::system_error(errno, std::system_category(), "epoll_create1() failed");
		}

		Epoll(const Epoll &other) = delete;

		Epoll(Epoll &&other) noexcept : epoll_fd(std::exchange(other.epoll_fd, -1))
		{
		}

		~Epoll()
		{
			close();
		}

		Epoll &operator=(const Epoll &other) = delete;

		Epoll &operator=(Epoll &&other) noexcept
		{
			close();
			epoll_fd = std::exchange(other.epoll_fd, -1);
			return *this;
		}

		int fd() const
		{
			return epoll_fd;
		}

		void add(int fd, Mask mask)
		{
			Event event;
			event.events = mask.value();
			event.data.fd = fd;

			ctl(EPOLL_CTL_ADD, fd, &event);
		}

		void add(int fd, Mask mask, void *ptr)
		{
			Event event;
			event.events = mask.value();
			event.data.ptr = ptr;

			ctl(EPOLL_CTL_ADD, fd, &event);
		}

		void modify(int fd, Mask mask)
		{
			Event event;
			event.events = mask.value();
			event.data.fd = fd;

			ctl(EPOLL_CTL_MOD, fd, &event);
		}

		void modify(int fd, Mask mask, void *ptr)
		{
			Event event;
			event.events = mask.value();
			event.data.ptr = ptr;

			ctl(EPOLL_CTL_MOD, fd, &event);
		}

		void remove(int fd) const
		{
			ctl(EPOLL_CTL_DEL, fd, nullptr);
		}

		template <typename Events>
		EventCount wait(Events &events, std::chrono::milliseconds timeout) const
		{
			return wait(events.data(), events.size(), timeout);
		}

		template <typename Events>
		EventCount wait(Events &events) const
		{
			return wait(events, std::chrono::milliseconds(-1));
		}

	private:
		static_assert(sizeof(Event) == sizeof(epoll_event), "Accidentally added member to Event?");

		void ctl(int op, int fd, Event *event) const
		{
			if (epoll_ctl(epoll_fd, op, fd, event) == -1)
				throw std::system_error(errno, std::system_category(), "epoll_ctl() failed");
		}

		EventCount wait(Event *events, std::size_t max_events, std::chrono::milliseconds timeout) const
		{
			using MsRep = std::chrono::milliseconds::rep;

			static constexpr int NUM_EVENTS_MAX_INT = std::numeric_limits<int>::max();
			static constexpr int TIMEOUT_MAX_INT = std::numeric_limits<int>::max();

			int max_events_int = int(std::min(max_events, std::size_t(NUM_EVENTS_MAX_INT)));
			int timeout_int = int(std::min(std::max(MsRep(-1), timeout.count()), MsRep(TIMEOUT_MAX_INT)));
			int ret = 0;

			while (true) // Loops if EINTR. Will cause timeout "drift" but leave as is for now.
			{
				ret = epoll_wait(epoll_fd, events, max_events_int, timeout_int);

				if (ret >= 0)
					break;
				if (errno != EINTR)
					throw std::system_error(errno, std::system_category(), "epoll_wait() failed");
			}

			return static_cast<EventCount>(ret);
		}

		void close()
		{
			if (epoll_fd == -1)
				return;

			::close(epoll_fd);
			epoll_fd = -1;
		}

		int epoll_fd = -1;
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

	RAYNI_BITMASK_GLOBAL_OPERATORS(Epoll::Mask)
}

#endif // RAYNI_LIB_SYSTEM_LINUX_EPOLL_H
