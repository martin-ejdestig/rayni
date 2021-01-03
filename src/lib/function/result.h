// This file is part of Rayni.
//
// Copyright (C) 2018-2021 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_FUNCTION_RESULT_H
#define RAYNI_LIB_FUNCTION_RESULT_H

#include <cassert>
#include <string>
#include <system_error>
#include <utility>

// TODO: Other compilers than GCC. __GNUC__ is defined for Clang but it is just ignored. At least it
//       looks like it when looking at the generated assembly.
#ifdef __GNUC__
#	define RAYNI_RESULT_COLD_ATTRIBUTE __attribute__((cold))
#else
#	warning Missing compiler support for RAYNI_RESULT_COLD_ATTRIBUTE
#	define RAYNI_RESULT_COLD_ATTRIBUTE
#endif

// TODO: clang-format can not handle [[nodiscard]]. Remove clang-format off/on once
//       https://bugs.llvm.org/show_bug.cgi?id=38401 is fixed. Until then, temporarily remove
//       [[nodiscard]], format and add it back again when changing this file.
// clang-format off

namespace Rayni
{
	// Result allows for returning a value (or void) or an error.
	//
	// Since Result can be used quite a lot an effort has been made to keep the size overhead
	// of Result as small as possible. Goal is to have a maximum size overhead of sizeof(void *).
	// To accomplish this Error is stored as a pointer. This reduces cache locality but only in
	// the case of an Error and most common case by far should be to return success.
	//
	// TODO: Perform more performance measurements to see if reasoning about size is justified.
	//       Code generated for returning error will be larger with separate allocation which in
	//       turn puts higher preassure on instruction cache.

	class Error
	{
	public:
		explicit Error(std::string &&message) RAYNI_RESULT_COLD_ATTRIBUTE : message_(std::move(message))
		{
		}

		Error(const std::string &message, std::error_code error_code) RAYNI_RESULT_COLD_ATTRIBUTE :
		        message_(message + ": " + error_code.message())
		{
		}

		Error(const std::string &prefix, const std::string &message) RAYNI_RESULT_COLD_ATTRIBUTE :
		        message_(prefix.empty() ? message : prefix + ": " + message)
		{
		}

		const std::string &message() const RAYNI_RESULT_COLD_ATTRIBUTE
		{
			return message_;
		}

	private:
		std::string message_;
	};

	template <typename T>
	class [[nodiscard]] Result final
	{
	public:
		// NOLINTNEXTLINE(google-explicit-constructor) Want implicit conversion for less verbosity.
		Result(T &&value)
		{
			new (&value_or_error_.value) T(std::move(value));
		}

		// NOLINTNEXTLINE(google-explicit-constructor) Want implicit conversion for less verbosity.
		Result(Error &&error) RAYNI_RESULT_COLD_ATTRIBUTE : is_error_(true)
		{
			value_or_error_.error = new Error(std::move(error));
		}

		// NOLINTNEXTLINE(google-explicit-constructor) Want implicit conversion for less verbosity.
		Result(const Error &error) RAYNI_RESULT_COLD_ATTRIBUTE : is_error_(true)
		{
			value_or_error_.error = new Error(error);
		}

		Result(Result &&other) noexcept
		{
			set_from(std::move(other));
		}

		Result(const Result &) = delete;

		~Result()
		{
			reset();
		}

		Result &operator=(const Result &) = delete;

		Result &operator=(Result &&other) noexcept
		{
			reset();
			set_from(std::move(other));
			return *this;
		}

		explicit operator bool() const
		{
			if (is_error_) [[unlikely]]
				return false; // NOLINT(readability-simplify-boolean-expr) Want [[unlikely]].

			return true;
		}

		bool is_error() const
		{
			if (is_error_) [[unlikely]]
				return true; // NOLINT(readability-simplify-boolean-expr) Want [[unlikely]].

			return false;
		}

		const Error &error() const RAYNI_RESULT_COLD_ATTRIBUTE
		{
			assert(is_error_ && value_or_error_.error);
			return *value_or_error_.error;
		}

		T &operator*() &
		{
			assert(!is_error_);
			return value_or_error_.value;
		}

		const T &operator*() const &
		{
			assert(!is_error_);
			return value_or_error_.value;
		}

		T &&operator*() &&
		{
			assert(!is_error_);
			return std::move(value_or_error_.value);
		}

		const T &&operator*() const &&
		{
			assert(!is_error_);
			return std::move(value_or_error_.value);
		}

		T *operator->()
		{
			assert(!is_error_);
			return &value_or_error_.value;
		}

		const T *operator->() const
		{
			assert(!is_error_);
			return &value_or_error_.value;
		}

		T &value() &
		{
			assert(!is_error_);
			return value_or_error_.value;
		}

		const T &value() const &
		{
			assert(!is_error_);
			return value_or_error_.value;
		}

		T &&value() &&
		{
			assert(!is_error_);
			return std::move(value_or_error_.value);
		}

		const T &&value() const &&
		{
			assert(!is_error_);
			return std::move(value_or_error_.value);
		}

		T value_or(T &&default_value) const &
		{
			if (is_error_) [[unlikely]]
				return std::forward<T>(default_value);

			return value_or_error_.value;
		}

		T value_or(T &&default_value) &&
		{
			if (is_error_) [[unlikely]]
				return std::forward<T>(default_value);

			return std::move(value_or_error_.value);
		}

	private:
		void reset() noexcept
		{
			if (is_error_) [[unlikely]] {
				delete value_or_error_.error;
				value_or_error_.error = nullptr;
			} else {
				if constexpr (std::is_destructible_v<T>)
					value_or_error_.value.~T();
			}
		}

		void set_from(Result &&other) noexcept
		{
			if (other.is_error_) [[unlikely]] {
				value_or_error_.error = std::exchange(other.value_or_error_.error, nullptr);
				is_error_ = true;
			} else {
				new (&value_or_error_.value) T(std::move(other.value_or_error_.value));
				is_error_ = false;
			}
		}

		bool is_error_ = false;

		union ValueOrError
		{
			ValueOrError() {} // NOLINT(modernize-use-equals-default)
			~ValueOrError() {} // NOLINT(modernize-use-equals-default)
			T value;
			const Error *error;
		} value_or_error_;
	};

	template <>
	class [[nodiscard]] Result<void> final
	{
	public:
		Result() = default;

		// NOLINTNEXTLINE(google-explicit-constructor) Want implicit conversion for less verbosity.
		Result(Error &&error) RAYNI_RESULT_COLD_ATTRIBUTE : error_(new Error(std::move(error)))
		{
		}

		// NOLINTNEXTLINE(google-explicit-constructor) Want implicit conversion for less verbosity.
		Result(const Error &error) RAYNI_RESULT_COLD_ATTRIBUTE : error_(new Error(error))
		{
		}

		Result(const Result &) = delete;

		Result(Result &&other) noexcept : error_(std::exchange(other.error_, nullptr))
		{
		}

		~Result()
		{
			delete error_;
		}

		Result &operator=(const Result &) noexcept = delete;

		Result &operator=(Result &&other) noexcept
		{
			delete error_;
			error_ = std::exchange(other.error_, nullptr);
			return *this;
		}

		explicit operator bool() const
		{
			if (error_) [[unlikely]]
				return false; // NOLINT(readability-simplify-boolean-expr) Want [[unlikely]].

			return true;
		}

		bool is_error() const
		{
			if (error_) [[unlikely]]
				return true; // NOLINT(readability-simplify-boolean-expr) Want [[unlikely]].

			return false;
		}

		const Error &error() const RAYNI_RESULT_COLD_ATTRIBUTE
		{
			assert(error_ != nullptr);
			return *error_;
		}

	private:
		const Error *error_ = nullptr;
	};

	// See resoning about size at top of this file.
	static_assert(sizeof(Result<void>) <= sizeof(void *));
	static_assert(sizeof(Result<char>) <= sizeof(void *) * 2);
	static_assert(sizeof(Result<int>) <= sizeof(void *) * 2);
}

// TODO: Remove once clang-format bug is fixed. See above.
// clang-format on

#undef RAYNI_RESULT_COLD_ATTRIBUTE

#endif // RAYNI_LIB_FUNCTION_RESULT_H
