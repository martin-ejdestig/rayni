// This file is part of Rayni.
//
// Copyright (C) 2013-2021 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_LOG_H
#define RAYNI_LIB_LOG_H

#include <functional>
#include <optional>
#include <string>

#ifdef __GNUC__
#	define RAYNI_LOG_PRINTF_ATTRIBUTE(f, a) __attribute__((format(printf, f, a)))
#else
#	define RAYNI_LOG_PRINTF_ATTRIBUTE(f, a)
#endif

namespace Rayni
{
	using LogCallback = std::function<void(const std::string &message)>;

	// Default values mean "do not set" to allow for using designated initializers to only set
	// what is wanted. E.g.: LogConfig old_config = log_set_config({.colorize = false});
	struct LogConfig
	{
		LogCallback callback = LogCallback();
		std::optional<bool> colorize = {};
	};

	LogConfig log_set_config(LogConfig &&config);

	void log_info(const char *format, ...) RAYNI_LOG_PRINTF_ATTRIBUTE(1, 2);
	void log_warning(const char *format, ...) RAYNI_LOG_PRINTF_ATTRIBUTE(1, 2);
	void log_error(const char *format, ...) RAYNI_LOG_PRINTF_ATTRIBUTE(1, 2);
}

#endif // RAYNI_LIB_LOG_H
