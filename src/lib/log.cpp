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

#include "lib/log.h"

#ifdef __unix__
#	include <unistd.h>
#endif

#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <utility>

#include "lib/string/split.h"

// Simple log functions. Not meant to be a generic "log framework". Do NOT complicate things
// unnecessarily. E.g. only reason why it is possible to set a callback is to allow for CLI
// application to clear progress line, output log message and then reprint the progress line
// and for silencing logs in tests (although this could be done with a boolean flag, callback
// is there so just use it in this case as well). Colorization is also only possible to
// override for testing.

namespace Rayni
{
	namespace
	{
		enum class Level
		{
			INFO,
			WARNING,
			ERROR
		};

		struct State
		{
			State();

			struct
			{
				std::shared_mutex mutex;
				LogCallback callback;
			} shared_callback;

			std::atomic<bool> colorize{false};
		};

		constexpr char COLOR_GREEN[] = "\033[1;32m";
		constexpr char COLOR_RED[] = "\033[1;31m";
		constexpr char COLOR_YELLOW[] = "\033[1;33m";
		constexpr char COLOR_RESET[] = "\033[0m";

		void default_callback(const std::string &message)
		{
			std::printf("%s\n", message.c_str());
		}

		bool default_output_supports_color()
		{
#ifdef __unix__
			return isatty(fileno(stdout)) == 1;
#else
			return false;
#endif
		}

		State::State() : colorize(default_output_supports_color())
		{
			shared_callback.callback = default_callback;
		}

		State &state()
		{
			static State state;
			return state;
		}

		std::string format_string(const char *format, std::va_list args)
		{
			char buffer[1024];
			std::va_list args_copy;

			va_copy(args_copy, args);
			int size = std::vsnprintf(buffer, sizeof(buffer), format, args_copy);
			va_end(args_copy);

			if (size <= 0)
				return "";

			if (unsigned(size) < sizeof(buffer))
				return std::string(buffer, unsigned(size));

			std::string str(unsigned(size), '\0');
			va_copy(args_copy, args);
			std::vsnprintf(str.data(), str.size() + 1, format, args);
			va_end(args_copy);

			return str;
		}

		std::string level_to_color_start(Level level)
		{
			switch (level)
			{
			case Level::INFO:
				return COLOR_GREEN;
			case Level::WARNING:
				return COLOR_YELLOW;
			case Level::ERROR:
				return COLOR_RED;
			}
			return "";
		}

		std::string level_to_prefix(Level level)
		{
			switch (level)
			{
			case Level::INFO:
				return "INFO";
			case Level::WARNING:
				return "WARNING";
			case Level::ERROR:
				return "ERROR";
			}
			return "";
		}

		std::string prepend_prefix(Level level, std::string_view message)
		{
			const std::string prefix = level_to_prefix(level);
			const bool colorize = state().colorize.load(std::memory_order_relaxed);
			bool first_line = true;
			std::string ret;

			if (colorize)
				ret += level_to_color_start(level);
			ret += prefix;
			if (colorize)
				ret += COLOR_RESET;
			ret += ": ";

			while (!message.empty())
			{
				if (first_line)
					first_line = false;
				else
					ret += "\n" + std::string(prefix.size() + 2, ' ');

				auto [line, remaining] = string_split_to_array<std::string_view, 2>(message, '\n');
				ret += line;
				message = remaining;
			}

			return ret;
		}

		void invoke_callback(const std::string &message)
		{
			std::shared_lock lock(state().shared_callback.mutex);
			state().shared_callback.callback(message);
		}
	}

	LogConfig log_set_config(LogConfig &&config)
	{
		LogConfig old;

		if (config.callback)
		{
			std::unique_lock lock(state().shared_callback.mutex);

			old.callback = std::move(state().shared_callback.callback);
			state().shared_callback.callback = std::move(config.callback);
		}

		if (config.colorize)
			old.colorize = state().colorize.exchange(*config.colorize);

		return old;
	}

	void log_info(const char *format, ...)
	{
		std::va_list args;
		va_start(args, format);
		std::string message = format_string(format, args);
		va_end(args);

		invoke_callback(prepend_prefix(Level::INFO, message));
	}

	void log_warning(const char *format, ...)
	{
		std::va_list args;
		va_start(args, format);
		std::string message = format_string(format, args);
		va_end(args);

		invoke_callback(prepend_prefix(Level::WARNING, message));
	}

	void log_error(const char *format, ...)
	{
		std::va_list args;
		va_start(args, format);
		std::string message = format_string(format, args);
		va_end(args);

		invoke_callback(prepend_prefix(Level::ERROR, message));
	}
}
