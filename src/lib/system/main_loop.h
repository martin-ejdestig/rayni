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

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdlib>
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
	// Simple main loop implementation with dispatch in, fd monitoring and timer functionality.
	//
	// Not optimized and currently no way to prioritize events.
	//
	// Most applications just need to create a MainLoop, start timers and other threads etc. and
	// then invoke loop().
	//
	// Integration with other main loops (in e.g. a UI toolkit) can be done with fd(), wait()
	// and dispatch(). fd() returns a file descriptor that can be polled for in/read events
	// (with e.g. poll()). wait() should be called when there is something to read. wait()
	// returns true if there is something to dispatch in which case dispatch() should be called.
	// Polling is optional. It is possible to check if there is something to dispatch by calling
	// wait() with a timeout of 0ms.
	//
	// A main loop is exited with exit(). An optional exit code may be passed as an argument.
	// The exit code is returned by loop(). Default value of exit code is EXIT_SUCCESS. Makes it
	// possible to do: int main(...) { MainLoop main_loop; ...; return main_loop.loop(); }
	//
	// run_in() is used to invoke a function in the main loop's thread. run_in() can be called
	// from any thread. Functions are invoked in FIFO order.
	//
	// File descriptors can be monitored by creating a FDMonitor instance and passing the file
	// descriptor to monitor to FDMonitor::start().
	//
	// Timers are created by creating a timer instance and then starting it with Timer::start()
	// or Timer::start_repeat().
	//
	// Both FDMonitor and Timer instances can be created and started from any thread. Their
	// callbacks are invoked in the thread the main loop is running in. The objects themselves
	// are not thread safe and need to be protected by e.g. a mutex if used
	// (started/stopped/moved) from different threads. FDMonitor:s and Timer:s may outlive the
	// MainLoop they are running in. Their callback is just never invoked if the MainLoop has
	// been destroyed.
	//
	// TODO: Make FDMonitor and Timer objects thread safe? (Note: already OK to use them in
	//       other threads than the MainLoop thread, see above.) Not a common enough use case
	//       and other user data probably needs to be protected together with monitor/timer so
	//       might as well protect monitor/timer objects in same manner as well?
	// TODO: Add priority queue for timers? Timers that have expired earlier then others should
	//       have their callback invoked first. Right now they are dispatched in order determined
	//       by container that holds timer data.
	// TODO: When porting to other OS, keep public interface. (Especially the fd()/poll part. Do
	//       not want to implement interfaces etc. to integrate with toolkits.)
	// TODO: When porting to other OS, consider adding a src/lib/system/poll.h:Poll class.
	//       Should map more or less directly to Epoll on Linux (maybe even do
	//       "using Poll = Epoll;"... maybe not... but small inline functions that just
	//       delegates should be enough). Can move FDFlag and FDFlags to Poll::Flag/Flags.
	//       Currently type alias for Epoll::Flag/Flags, should probably map.
	// TODO: When porting to other OS, consider adding a src/lib/system/wakeup.h:Wakeup class.
	//       Thin inline wrapper around EventFD and read/write. Integrate with Poll? See GWakeup
	//       in GLib for inspiration (also has implementation for other OS:es).
	class MainLoop
	{
	public:
		class FDMonitor;
		class Timer;

		using clock = std::chrono::steady_clock;

		using FDFlag = Epoll::Flag;
		using FDFlags = Epoll::Flags;

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
			return epoll_.fd();
		}

	private:
		class RunInFunctions
		{
		public:
			void add(std::function<void()> &&function);
			void dispatch();

		private:
			std::mutex mutex_;
			std::vector<std::function<void()>> functions_;
		};

		class FDData;
		class TimerData;

		using TimerId = std::size_t;

		static constexpr clock::time_point CLOCK_EPOCH = clock::time_point();
		static constexpr TimerId TIMER_ID_EMPTY = 0;

		void set_timer_fd_from_timer_data() const;

		Epoll epoll_;
		std::array<Epoll::Event, 5> events_;
		Epoll::EventCount events_occurred_ = 0;

		std::atomic<int> exit_code_{EXIT_SUCCESS};
		std::atomic<bool> exited_{false};
		EventFD exit_event_fd_;

		EventFD run_in_event_fd_;
		RunInFunctions run_in_functions_;

		TimerFD timer_fd_;
		std::shared_ptr<TimerData> timer_data_;

		std::shared_ptr<FDData> fd_data_;
	};

	class MainLoop::FDMonitor
	{
	public:
		FDMonitor() = default;

		~FDMonitor()
		{
			stop();
		}

		FDMonitor(const FDMonitor &other) = delete;

		FDMonitor(FDMonitor &&other) noexcept :
		        fd_data_(std::move(other.fd_data_)),
		        fd_(std::exchange(other.fd_, -1))
		{
		}

		FDMonitor &operator=(const FDMonitor &other) = delete;

		FDMonitor &operator=(FDMonitor &&other) noexcept
		{
			stop();

			fd_data_ = std::move(other.fd_data_);
			fd_ = std::exchange(other.fd_, -1);

			return *this;
		}

		void start(MainLoop &main_loop, int fd, FDFlags flags, std::function<void(FDFlags flags)> &&callback);

		void stop();

	private:
		std::weak_ptr<FDData> fd_data_;
		int fd_ = -1;
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

		Timer(Timer &&other) noexcept :
		        timer_data_(std::move(other.timer_data_)),
		        id_(std::exchange(other.id_, TIMER_ID_EMPTY))
		{
		}

		Timer &operator=(const Timer &other) = delete;

		Timer &operator=(Timer &&other) noexcept
		{
			remove();

			timer_data_ = std::move(other.timer_data_);
			id_ = std::exchange(other.id_, TIMER_ID_EMPTY);

			return *this;
		}

		template <typename Duration>
		void start(MainLoop &main_loop, Duration timeout, std::function<void()> &&callback)
		{
			start(main_loop, clock::now() + timeout, std::chrono::nanoseconds(0), std::move(callback));
		}

		void start(MainLoop &main_loop, clock::time_point expiration, std::function<void()> &&callback)
		{
			start(main_loop, expiration, std::chrono::nanoseconds(0), std::move(callback));
		}

		template <typename Duration>
		void start_repeat(MainLoop &main_loop, Duration interval, std::function<void()> &&callback)
		{
			start(main_loop, clock::now() + interval, interval, std::move(callback));
		}

		template <typename Duration>
		void start_repeat(MainLoop &main_loop,
		                  clock::time_point first_expiration,
		                  Duration interval,
		                  std::function<void()> &&callback)
		{
			start(main_loop,
			      first_expiration,
			      std::chrono::duration_cast<std::chrono::nanoseconds>(interval),
			      std::move(callback));
		}

		void stop();

	private:
		void start(MainLoop &main_loop,
		           clock::time_point expiration,
		           std::chrono::nanoseconds interval,
		           std::function<void()> &&callback);

		void remove();

		std::weak_ptr<TimerData> timer_data_;
		TimerId id_ = TIMER_ID_EMPTY;
	};
}

#endif // RAYNI_LIB_SYSTEM_MAIN_LOOP_H
