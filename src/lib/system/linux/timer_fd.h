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

#ifndef RAYNI_LIB_SYSTEM_LINUX_TIMER_FD_H
#define RAYNI_LIB_SYSTEM_LINUX_TIMER_FD_H

#include <sys/timerfd.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <ctime>
#include <system_error>
#include <utility>

#include "lib/function/result.h"
#include "lib/system/unique_fd.h"

namespace Rayni
{
	// Only allows for CLOCK_MONOTONIC at the moment and assumes it is what std::chrono::steady_clock uses.
	class TimerFD
	{
	public:
		using clock = std::chrono::steady_clock;

		struct Value
		{
			std::chrono::nanoseconds initial_expiration;
			std::chrono::nanoseconds interval;
		};

		static Result<TimerFD> create()
		{
			TimerFD t;

			t.timer_fd_ = UniqueFD(timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC));
			if (t.timer_fd_.get() == -1)
				return Error("timerfd_create() failed", std::error_code(errno, std::system_category()));

			return t;
		}

		int fd() const
		{
			return timer_fd_.get();
		}

		Result<void> set(std::chrono::nanoseconds expiration) const
		{
			struct itimerspec timer_spec = {};

			timer_spec.it_value = time_spec_from_ns(expiration);

			return settime(0, timer_spec);
		}

		Result<void> set(std::chrono::nanoseconds initial_expiration, std::chrono::nanoseconds interval) const
		{
			struct itimerspec timer_spec = {};

			timer_spec.it_value = time_spec_from_ns(initial_expiration);
			timer_spec.it_interval = time_spec_from_ns(interval);

			return settime(0, timer_spec);
		}

		Result<void> set(clock::time_point expiration) const
		{
			struct itimerspec timer_spec = {};

			timer_spec.it_value = time_spec_from_time_point(expiration);

			return settime(TFD_TIMER_ABSTIME, timer_spec);
		}

		Result<void> set(clock::time_point initial_expiration, std::chrono::nanoseconds interval) const
		{
			struct itimerspec timer_spec = {};

			timer_spec.it_value = time_spec_from_time_point(initial_expiration);
			timer_spec.it_interval = time_spec_from_ns(interval);

			return settime(TFD_TIMER_ABSTIME, timer_spec);
		}

		Result<Value> get() const
		{
			struct itimerspec timer_spec = {};

			if (timerfd_gettime(timer_fd_.get(), &timer_spec) == -1)
				return Error("timerfd_gettime() failed",
				             std::error_code(errno, std::system_category()));

			return Value{time_spec_to_ns(timer_spec.it_value), time_spec_to_ns(timer_spec.it_interval)};
		}

		Result<void> disarm() const
		{
			struct itimerspec timer_spec = {};

			return settime(0, timer_spec);
		}

		Result<std::uint64_t> read() const
		{
			std::uint64_t value = 0;

			while (::read(timer_fd_.get(), &value, sizeof value) != static_cast<ssize_t>(sizeof value))
				if (errno != EINTR)
					return Error("read() from timerfd failed",
					             std::error_code(errno, std::system_category()));

			return value;
		}

	private:
		Result<void> settime(int flags, struct itimerspec &timer_spec) const
		{
			if (timerfd_settime(timer_fd_.get(), flags, &timer_spec, nullptr) == -1)
				return Error("timerfd_settime() failed",
				             std::error_code(errno, std::system_category()));
			return {};
		}

		static constexpr struct timespec time_spec_from_ns(std::chrono::nanoseconds ns)
		{
			struct timespec time_spec = {};

			time_spec.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(ns).count();
			time_spec.tv_nsec = (ns % std::chrono::seconds(1)).count();

			return time_spec;
		}

		static constexpr struct timespec time_spec_from_time_point(clock::time_point time_point)
		{
			auto duration = time_point.time_since_epoch();
			auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

			return time_spec_from_ns(ns);
		}

		static constexpr std::chrono::nanoseconds time_spec_to_ns(const struct timespec &time_spec)
		{
			return std::chrono::seconds(time_spec.tv_sec) + std::chrono::nanoseconds(time_spec.tv_nsec);
		}

		UniqueFD timer_fd_;
	};
}

#endif // RAYNI_LIB_SYSTEM_LINUX_TIMER_FD_H
