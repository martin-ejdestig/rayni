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

#include "lib/system/main_loop.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <optional>
#include <unordered_map>
#include <utility>

#include "lib/function/result.h"
#include "lib/log.h"
#include "lib/math/hash.h"

// TODO: Remove once https://bugs.llvm.org/show_bug.cgi?id=44325 is fixed.
// Spurious warning when e.g. >= is rewritten to <=>, which happens with chrono. *sigh*
#if defined __clang__
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

namespace Rayni
{
	// Data for monitoring file descriptors.
	//
	// A recursive mutex is used to allow for modification of data in a callback.
	//
	// Stored in a std::shared_pointer in MainLoop. FDMonitor:s reference the data with a weak
	// pointer to allow for it to be destroyed together with the MainLoop even if there are
	// started FDMonitor instances that have not been destroyed. FDMonitors will be stopped when
	// the MainLoop is destroyed.
	//
	// NOTE: Since dispatch() waits for more than 1 event another callback may remove/stop
	// monitoring an fd already in events filled in by Epoll::wait(). Must therefore make sure
	// fd is still in map when iterating over events and that callback has not been cleared
	// before invoking it.
	//
	// If a system error occurs and the MainLoop still exists, log and exit the main loop. If
	// main_loop_ is cleared, it means the MainLoop has been destroyed or is in the process of
	// being destroyed. Just ignore the error.
	class MainLoop::FDData
	{
	public:
		explicit FDData(MainLoop *main_loop) : main_loop_(main_loop)
		{
		}

		void add(int fd, FDFlags flags, std::function<void(FDFlags flags)> &&callback)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex_);

			if (auto r = epoll_.add(fd, flags); !r)
				if (main_loop_)
					main_loop_->log_error_and_exit(r.error());

			map_[fd] = Data{std::move(callback)};
		}

		void modify(int fd, FDFlags flags, std::function<void(FDFlags flags)> &&callback)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex_);

			if (auto r = epoll_.modify(fd, flags); !r)
				if (main_loop_)
					main_loop_->log_error_and_exit(r.error());

			map_[fd] = Data{std::move(callback)};
		}

		void remove(int fd)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex_);

			auto i = map_.find(fd);
			if (i == map_.cend())
				return;

			map_.erase(i);

			if (auto r = epoll_.remove(fd); !r)
				if (main_loop_)
					main_loop_->log_error_and_exit(r.error());
		}

		void dispatch()
		{
			std::array<Epoll::Event, 4> events;
			Epoll::EventCount num_events = 0;

			if (auto r = epoll_.wait(events); r) {
				num_events = *r;
			} else {
				std::lock_guard<std::recursive_mutex> lock(mutex_);
				if (main_loop_)
					main_loop_->log_error_and_exit(r.error());
				return;
			}

			std::lock_guard<std::recursive_mutex> lock(mutex_);

			for (Epoll::EventCount i = 0; i < num_events; i++) {
				auto it = map_.find(events[i].fd());

				if (it != map_.cend() && it->second.callback)
					it->second.callback(events[i].flags());
			}
		}

		Epoll &epoll()
		{
			return epoll_;
		}

		void main_loop_destroyed()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex_);
			main_loop_ = nullptr;
		}

	private:
		struct Data
		{
			std::function<void(FDFlags)> callback;
		};

		Epoll epoll_;

		std::recursive_mutex mutex_;
		std::unordered_map<int, Data> map_;

		MainLoop *main_loop_;
	};

	// Data for all timers. Can be accessed from multiple threads.
	//
	// A recursive mutex is used since timers can be added/removed while dispatching. Also note
	// that a map is used instead of an unordered_map since it can ivalidate iterators if timers
	// are added/removed while dispatching.
	//
	// Stored in a std::shared_pointer in MainLoop. Timers reference the data with a weak
	// pointer to allow for it to be destroyed together with the MainLoop even if there are
	// started Timer instances that have not been destroyed. Timers can not be dispatched when
	// MainLoop no longer exists.
	//
	// If a system error occurs and the MainLoop still exists, log and exit the main loop. If
	// main_loop_ is cleared, it means the MainLoop has been destroyed or is in the process of
	// being destroyed. Just ignore the error.
	class MainLoop::TimerData
	{
	public:
		explicit TimerData(MainLoop *main_loop) : main_loop_(main_loop)
		{
		}

		// TODO: [[nodiscard]] when https://bugs.llvm.org/show_bug.cgi?id=38401 is fixed.
		TimerId set(const Timer *timer,
		            TimerId id,
		            clock::time_point expiration,
		            std::chrono::nanoseconds interval,
		            std::function<void()> &&callback)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex_);

			if (id == TIMER_ID_EMPTY)
				id = generate_id(timer);

			map_[id] = Data{expiration, interval, std::move(callback)};

			if (auto r = changed_event_fd_.write(1); !r)
				if (main_loop_)
					main_loop_->log_error_and_exit(r.error());

			return id;
		}

		void remove(TimerId id)
		{
			std::lock_guard<std::recursive_mutex> lock(mutex_);

			auto i = map_.find(id);
			if (i == map_.cend())
				return;

			map_.erase(i);

			if (auto r = changed_event_fd_.write(1); !r)
				if (main_loop_)
					main_loop_->log_error_and_exit(r.error());
		}

		std::optional<clock::time_point> earliest_expiration()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex_);
			clock::time_point expiration = clock::time_point::max();

			for (const auto &[key, data] : map_)
				if (active(data))
					expiration = std::min(expiration, data.expiration);

			if (expiration == clock::time_point::max())
				return std::nullopt;

			return expiration;
		}

		void dispatch()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex_);
			clock::time_point now = clock::now();
			bool dispatch_needed = true; // Repeat timers may expire again and callback can restart timer.

			while (dispatch_needed) {
				dispatch_needed = false;

				for (auto &[key, data] : map_) {
					if (expired(data, now)) {
						if (data.interval.count() > 0)
							data.expiration += data.interval;
						else
							data.expiration = CLOCK_EPOCH;

						data.callback();

						dispatch_needed |= expired(data, now);
					}
				}
			}
		}

		EventFD &changed_event_fd()
		{
			return changed_event_fd_;
		}

		void main_loop_destroyed()
		{
			std::lock_guard<std::recursive_mutex> lock(mutex_);
			main_loop_ = nullptr;
		}

	private:
		struct Data
		{
			clock::time_point expiration;
			std::chrono::nanoseconds interval{0};
			std::function<void()> callback;
		};

		static bool active(const Data &data)
		{
			// NOLINTNEXTLINE(modernize-use-nullptr) TODO: https://bugs.llvm.org/show_bug.cgi?id=46235
			return data.expiration > CLOCK_EPOCH;
		}

		static bool expired(const Data &data, clock::time_point now)
		{
			// NOLINTNEXTLINE(modernize-use-nullptr) TODO: https://bugs.llvm.org/show_bug.cgi?id=46235
			return active(data) && data.expiration <= now;
		}

		TimerId generate_id(const Timer *timer) const
		{
			TimerId id = hash_combine_for(timer, map_.size());

			while (id == TIMER_ID_EMPTY || map_.find(id) != map_.cend())
				id = hash_combine_for(timer, id);

			return id;
		}

		EventFD changed_event_fd_;

		std::recursive_mutex mutex_;
		std::map<TimerId, Data> map_;
		MainLoop *main_loop_;
	};

	MainLoop::MainLoop() : timer_data_(std::make_shared<TimerData>(this)), fd_data_(std::make_shared<FDData>(this))
	{
		// Should maybe do early return on errors. But it does not really matter. Likelyhood
		// of error is small and the worst that happens is that > 1 error is logged.
		if (auto r = Epoll::create(); r)
			epoll_ = std::move(*r);
		else
			log_error_and_exit(r.error());

		if (auto r = EventFD::create(); r) {
			exit_event_fd_ = std::move(*r);

			if (auto re = epoll_.add(exit_event_fd_.fd(), Epoll::Flag::IN); !re)
				log_error_and_exit(re.error());
		} else {
			log_error_and_exit(r.error());
		}

		if (auto r = EventFD::create(); r) {
			run_in_event_fd_ = std::move(*r);

			if (auto re = epoll_.add(run_in_event_fd_.fd(), Epoll::Flag::IN); !re)
				log_error_and_exit(re.error());
		} else {
			log_error_and_exit(r.error());
		}

		if (auto r = TimerFD::create(); r) {
			timer_fd_ = std::move(*r);

			if (auto re = epoll_.add(timer_fd_.fd(), Epoll::Flag::IN); !re)
				log_error_and_exit(re.error());
		} else {
			log_error_and_exit(r.error());
		}

		if (auto r = EventFD::create(); r) {
			timer_data_->changed_event_fd() = std::move(*r);

			if (auto re = epoll_.add(timer_data_->changed_event_fd().fd(), Epoll::Flag::IN); !re)
				log_error_and_exit(re.error());
		} else {
			log_error_and_exit(r.error());
		}

		if (auto r = Epoll::create(); r) {
			fd_data_->epoll() = std::move(*r);

			if (auto re = epoll_.add(fd_data_->epoll().fd(), Epoll::Flag::IN); !re)
				log_error_and_exit(re.error());
		} else {
			log_error_and_exit(r.error());
		}
	}

	MainLoop::~MainLoop()
	{
		timer_data_->main_loop_destroyed();
		fd_data_->main_loop_destroyed();
	}

	int MainLoop::loop()
	{
		while (!exited()) {
			bool call_dispatch = wait();

			if (call_dispatch)
				dispatch();
		}

		return exit_code_;
	}

	void MainLoop::exit(int exit_code)
	{
		exit_code_ = exit_code;
		exited_ = true;

		if (auto r = exit_event_fd_.write(1); !r)
			log_error("MainLoop: %s", r.error().message().c_str());
	}

	void MainLoop::log_error_and_exit(const Error &error)
	{
		log_error("MainLoop: %s", error.message().c_str());

		if (!exited_) {
			exit_code_ = EXIT_FAILURE;
			exited_ = true;

			// log_error_and_exit() is internal and may be called before initialization has finished.
			if (exit_event_fd_.fd() != -1)
				if (auto r = exit_event_fd_.write(1); !r)
					log_error("MainLoop: %s", r.error().message().c_str());
		}
	}

	bool MainLoop::wait(std::chrono::milliseconds timeout)
	{
		if (exited())
			return false;

		if (auto r = epoll_.wait(events_, timeout); r)
			events_occurred_ = *r;
		else
			log_error_and_exit(r.error());

		return events_occurred_ > 0;
	}

	void MainLoop::dispatch()
	{
		auto events_left_to_process = std::exchange(events_occurred_, 0);

		for (const auto &event : events_) {
			if (events_left_to_process-- == 0)
				break;

			if (event.fd() == exit_event_fd_.fd()) {
				if (auto r = exit_event_fd_.read(); !r) {
					assert(exited_); // Should be exiting, no need for log_error_and_exit().
					log_error("MainLoop: %s", r.error().message().c_str());
				}
				break;
			}

			if (event.fd() == run_in_event_fd_.fd()) {
				if (auto r = run_in_event_fd_.read(); !r) {
					log_error_and_exit(r.error());
					return;
				}
				run_in_functions_.dispatch();
			} else if (event.fd() == timer_fd_.fd()) {
				if (auto r = timer_fd_.read(); !r) {
					log_error_and_exit(r.error());
					return;
				}
				timer_data_->dispatch();
				set_timer_fd_from_timer_data();
			} else if (event.fd() == timer_data_->changed_event_fd().fd()) {
				if (auto r = timer_data_->changed_event_fd().read(); !r) {
					log_error_and_exit(r.error());
					return;
				}
				set_timer_fd_from_timer_data();
			} else if (event.fd() == fd_data_->epoll().fd()) {
				fd_data_->dispatch();
			}
		}
	}

	void MainLoop::run_in(std::function<void()> &&function)
	{
		run_in_functions_.add(std::move(function));

		if (auto r = run_in_event_fd_.write(1); !r)
			log_error_and_exit(r.error());
	}

	void MainLoop::set_timer_fd_from_timer_data()
	{
		std::optional<clock::time_point> expiration = timer_data_->earliest_expiration();

		if (expiration.has_value()) {
			if (auto r = timer_fd_.set(expiration.value()); !r)
				log_error_and_exit(r.error());
		} else {
			if (auto r = timer_fd_.disarm(); !r)
				log_error_and_exit(r.error());
		}
	}

	void MainLoop::RunInFunctions::add(std::function<void()> &&function)
	{
		std::lock_guard<std::mutex> lock(mutex_);

		functions_.emplace_back(std::move(function));
	}

	void MainLoop::RunInFunctions::dispatch()
	{
		std::vector<std::function<void()>> functions_to_dispatch;
		{
			std::lock_guard<std::mutex> lock(mutex_);
			functions_to_dispatch = std::move(functions_);
			functions_ = std::vector<std::function<void()>>();
		}

		for (auto &function : functions_to_dispatch)
			function();
	}

	void MainLoop::FDMonitor::start(MainLoop &main_loop,
	                                int fd,
	                                FDFlags flags,
	                                std::function<void(FDFlags flags)> &&callback)
	{
		auto data = fd_data_.lock();
		if (data != main_loop.fd_data_) {
			if (data)
				data->remove(std::exchange(fd_, -1));

			fd_data_ = main_loop.fd_data_;
			data = main_loop.fd_data_;
		}

		if (fd_ == fd) {
			data->modify(fd, flags, std::move(callback));
			return;
		}

		if (fd_ != -1)
			data->remove(std::exchange(fd_, -1));

		data->add(fd, flags, std::move(callback));
		fd_ = fd;
	}

	void MainLoop::FDMonitor::stop()
	{
		auto data = fd_data_.lock();
		if (!data)
			return;

		fd_data_.reset();
		data->remove(std::exchange(fd_, -1));
	}

	void MainLoop::Timer::start(MainLoop &main_loop,
	                            clock::time_point expiration,
	                            std::chrono::nanoseconds interval,
	                            std::function<void()> &&callback)
	{
		auto data = timer_data_.lock();
		if (data != main_loop.timer_data_) {
			if (data)
				remove();

			timer_data_ = main_loop.timer_data_;
			data = main_loop.timer_data_;
		}

		id_ = data->set(this, id_, expiration, interval, std::move(callback));
	}

	void MainLoop::Timer::stop()
	{
		auto data = timer_data_.lock();
		if (!data)
			return;

		id_ = data->set(this, id_, CLOCK_EPOCH, std::chrono::nanoseconds(0), std::function<void()>());
	}

	void MainLoop::Timer::remove()
	{
		auto data = timer_data_.lock();
		if (!data)
			return;

		timer_data_.reset();
		data->remove(std::exchange(id_, TIMER_ID_EMPTY));
	}
}

#if defined __clang__
#	pragma clang diagnostic pop
#endif
