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

#include <algorithm>

#include "lib/math/hash.h"
#include "lib/system/main_loop.h"

namespace Rayni
{
	MainLoop::MainLoop()
	{
		epoll.add(exit_event_fd.fd(), Epoll::Flag::IN);
		epoll.add(run_in_event_fd.fd(), Epoll::Flag::IN);
		epoll.add(timer_fd.fd(), Epoll::Flag::IN);
		epoll.add(timer_data->changed_event_fd.fd(), Epoll::Flag::IN);
	}

	int MainLoop::loop()
	{
		while (!exited())
		{
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
		exit_event_fd.write(1);
	}

	bool MainLoop::wait(std::chrono::milliseconds timeout)
	{
		if (exited())
			return false;

		events_occurred = epoll.wait(events, timeout);

		return events_occurred > 0;
	}

	void MainLoop::dispatch()
	{
		auto events_left_to_process = std::exchange(events_occurred, 0);

		for (const auto &event : events)
		{
			if (events_left_to_process-- == 0)
				break;

			if (event.fd() == exit_event_fd.fd())
			{
				exit_event_fd.read();
				break;
			}

			if (event.fd() == run_in_event_fd.fd())
			{
				run_in_event_fd.read();
				run_in_functions.dispatch();
			}
			else if (event.fd() == timer_fd.fd())
			{
				timer_fd.read();
				timer_data->dispatch();
				set_timer_fd_from_timer_data();
			}
			else if (event.fd() == timer_data->changed_event_fd.fd())
			{
				timer_data->changed_event_fd.read();
				set_timer_fd_from_timer_data();
			}
		}
	}

	void MainLoop::run_in(std::function<void()> &&function)
	{
		run_in_functions.add(std::move(function));
		run_in_event_fd.write(1);
	}

	void MainLoop::set_timer_fd_from_timer_data() const
	{
		std::experimental::optional<clock::time_point> expiration = timer_data->earliest_expiration();

		if (expiration)
			timer_fd.set(*expiration);
		else
			timer_fd.disarm();
	}

	void MainLoop::RunInFunctions::add(std::function<void()> &&function)
	{
		std::unique_lock<std::mutex> lock(mutex);

		functions.emplace_back(std::move(function));
	}

	void MainLoop::RunInFunctions::dispatch()
	{
		std::vector<std::function<void()>> functions_to_dispatch;
		{
			std::unique_lock<std::mutex> lock(mutex);
			functions_to_dispatch = std::move(functions);
		}

		for (auto &function : functions_to_dispatch)
			function();
	}

	MainLoop::TimerData::Id MainLoop::TimerData::set(const Timer *timer,
	                                                 Id id,
	                                                 clock::time_point expiration,
	                                                 std::chrono::nanoseconds interval,
	                                                 std::function<void()> &&callback)
	{
		std::unique_lock<std::recursive_mutex> lock(mutex);

		if (id == ID_EMPTY)
			id = generate_id(timer);

		map[id] = Data{expiration, interval, std::move(callback)};

		changed_event_fd.write(1);

		return id;
	}

	MainLoop::TimerData::Id MainLoop::TimerData::generate_id(const Timer *timer) const
	{
		Id id = hash_combine_for(timer, map.size());

		while (id == ID_EMPTY || map.find(id) != map.cend())
			id = hash_combine_for(timer, id);

		return id;
	}

	void MainLoop::TimerData::remove(Id id)
	{
		std::unique_lock<std::recursive_mutex> lock(mutex);

		auto i = map.find(id);
		if (i == map.cend())
			return;

		map.erase(i);

		changed_event_fd.write(1);
	}

	std::experimental::optional<MainLoop::clock::time_point> MainLoop::TimerData::earliest_expiration()
	{
		std::unique_lock<std::recursive_mutex> lock(mutex);
		clock::time_point expiration = clock::time_point::max();

		for (const auto &key_value : map)
		{
			const Data &data = key_value.second;

			if (data.active())
				expiration = std::min(expiration, data.expiration);
		}

		if (expiration == clock::time_point::max())
			return std::experimental::nullopt;

		return expiration;
	}

	void MainLoop::TimerData::dispatch()
	{
		std::unique_lock<std::recursive_mutex> lock(mutex);
		clock::time_point now = clock::now();
		bool dispatch_needed = true; // Repeat timers may expire again and callback can also restart timer.

		while (dispatch_needed)
		{
			dispatch_needed = false;

			for (auto &key_value : map)
			{
				Data &data = key_value.second;

				if (data.expired(now))
				{
					data.dispatch();
					dispatch_needed |= data.expired(now);
				}
			}
		}
	}

	void MainLoop::Timer::set(MainLoop &main_loop,
	                          clock::time_point expiration,
	                          std::chrono::nanoseconds interval,
	                          std::function<void()> &&callback)
	{
		set_timer_data(main_loop.timer_data);
		set(expiration, interval, std::move(callback));
	}

	void MainLoop::Timer::set(clock::time_point expiration,
	                          std::chrono::nanoseconds interval,
	                          std::function<void()> &&callback)
	{
		auto data = timer_data.lock();
		if (!data)
			return;

		id = data->set(this, id, expiration, interval, std::move(callback));
	}

	void MainLoop::Timer::set_timer_data(const std::shared_ptr<TimerData> &new_timer_data)
	{
		auto data = timer_data.lock();

		if (data == new_timer_data)
			return;

		if (data)
			remove();

		timer_data = new_timer_data;
	}

	void MainLoop::Timer::remove()
	{
		auto data = timer_data.lock();
		if (!data)
			return;

		data->remove(id);

		timer_data.reset();
		id = TimerData::ID_EMPTY;
	}
}