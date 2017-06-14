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

#ifndef RAYNI_LIB_SYSTEM_MAIN_LOOP_H
#define RAYNI_LIB_SYSTEM_MAIN_LOOP_H

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <experimental/optional>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "lib/system/linux/epoll.h"
#include "lib/system/linux/event_fd.h"
#include "lib/system/linux/timer_fd.h"

namespace Rayni
{
	// Simple main loop implementation with "run in" and timer functionality.
	//
	// Not optimized and currently no way to prioritize "events".
	//
	// Most applications just need to create a MainLoop, start timers and other threads etc. and
	// then invoke loop().
	//
	// If integration with another main loop (e.g. in a UI toolkit) is needed fd() can be used
	// to get a file descriptor that should be polled for in/read events (with e.g. poll()).
	// When the file descriptor signals that there is something to be read, wait() should be
	// called (timeout may be 0ms). If wait() returns true there is something to dispatch and
	// dispatch() should be called. If there is no need to poll a file descriptor this part may
	// be left out and the app can just do "if (main_loop.wait(0ms)) main_loop.dispatch();" in
	// a simple loop that also drives the rest of the toolkit.
	//
	// A main loop is exited with exit() that takes an optional exit code returned by loop()
	// (commonly used as exit code of program returned from main()).
	//
	// run_in() can be used to invoke a function in the main loop context. May be called from
	// any thread and functions are invoked in FIFO order.
	//
	// Timers are created by creating a Timer instance and then starting the timer with
	// Timer::start() or Timer::start_repeat() that takes a MainLoop as argument. This can be
	// done from any thread. The callback passed when starting a timer is invoked in the thread
	// the main loop is running in when the timer expires. Timer objects themselves need to be
	// protected by e.g. a mutex if used from different threads. Timer objects may outlive the
	// MainLoop they are running in, their callback is just never invoked if the MainLoop has
	// been destroyed.
	//
	// TODO: Make Timer itself thread safe?
	// TODO: Add priority queue for timers?
	// TODO: When porting to other OS, keep public interface. (Especially the fd()/poll part. Do
	//       not want to implement interfaces etc. to integrate with toolkits.)
	class MainLoop
	{
	public:
		class Timer;

		using clock = std::chrono::steady_clock;

		MainLoop();

		int loop();

		void exit(int exit_code);
		void exit()
		{
			exit(EXIT_SUCCESS);
		}

		int exit_code() const
		{
			return exit_code_;
		}

		bool exited() const
		{
			return exited_;
		}

		bool wait(std::chrono::milliseconds timeout);
		bool wait()
		{
			return wait(std::chrono::milliseconds(-1));
		}

		void dispatch();

		void run_in(std::function<void()> &&function);

		int fd() const
		{
			return epoll.fd();
		}

	private:
		class RunInFunctions
		{
		public:
			void add(std::function<void()> &&function);
			void dispatch();

		private:
			std::mutex mutex;
			std::vector<std::function<void()>> functions;
		};

		class TimerData;
		using TimerId = std::size_t;

		static constexpr clock::time_point CLOCK_EPOCH = clock::time_point();
		static constexpr TimerId TIMER_ID_EMPTY = 0;

		void set_timer_fd_from_timer_data() const;

		Epoll epoll;
		std::array<Epoll::Event, 4> events;
		Epoll::EventCount events_occurred = 0;

		std::atomic<int> exit_code_{EXIT_SUCCESS};
		std::atomic<bool> exited_{false};
		EventFD exit_event_fd;

		EventFD run_in_event_fd;
		RunInFunctions run_in_functions;

		TimerFD timer_fd;
		std::shared_ptr<TimerData> timer_data;
	};

	class MainLoop::Timer
	{
	public:
		Timer() = default;

		~Timer()
		{
			remove();
		}

		Timer(const Timer &other) = delete;

		Timer(Timer &&other) noexcept
		        : timer_data(std::move(other.timer_data)), id(std::exchange(other.id, TIMER_ID_EMPTY))
		{
		}

		Timer &operator=(const Timer &other) = delete;

		Timer &operator=(Timer &&other) noexcept
		{
			remove();

			timer_data = std::move(other.timer_data);
			id = std::exchange(other.id, TIMER_ID_EMPTY);

			return *this;
		}

		template <typename Duration>
		void start(MainLoop &main_loop, Duration timeout, std::function<void()> &&callback)
		{
			set(main_loop, clock::now() + timeout, std::chrono::nanoseconds(0), std::move(callback));
		}

		void start(MainLoop &main_loop, clock::time_point expiration, std::function<void()> &&callback)
		{
			set(main_loop, expiration, std::chrono::nanoseconds(0), std::move(callback));
		}

		template <typename Duration>
		void start_repeat(MainLoop &main_loop, Duration interval, std::function<void()> &&callback)
		{
			set(main_loop, clock::now() + interval, interval, std::move(callback));
		}

		template <typename Duration>
		void start_repeat(MainLoop &main_loop,
		                  clock::time_point first_expiration,
		                  Duration interval,
		                  std::function<void()> &&callback)
		{
			set(main_loop,
			    first_expiration,
			    std::chrono::duration_cast<std::chrono::nanoseconds>(interval),
			    std::move(callback));
		}

		void stop()
		{
			set(clock::time_point(), std::chrono::nanoseconds(0), std::function<void()>());
		}

	private:
		void set(MainLoop &main_loop,
		         clock::time_point expiration,
		         std::chrono::nanoseconds interval,
		         std::function<void()> &&callback);

		void set(clock::time_point expiration,
		         std::chrono::nanoseconds interval,
		         std::function<void()> &&callback);

		void set_timer_data(const std::shared_ptr<TimerData> &new_timer_data);

		void remove();

		std::weak_ptr<TimerData> timer_data;
		TimerId id = TIMER_ID_EMPTY;
	};
}

#endif // RAYNI_LIB_SYSTEM_MAIN_LOOP_H
