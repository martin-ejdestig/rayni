// This file is part of Rayni.
//
// Copyright (C) 2017-2021 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_MATH_BITMASK_H
#define RAYNI_LIB_MATH_BITMASK_H

#include <type_traits>

namespace Rayni
{
	template <typename E>
	class Bitmask
	{
	public:
		using Enum = E;
		using Value = std::underlying_type_t<Enum>;

		Bitmask() = default;

		explicit Bitmask(Value value) : value_(value)
		{
		}

		// NOLINTNEXTLINE(google-explicit-constructor) Want implicit conversion for less verbosity.
		constexpr Bitmask(Enum e) : Bitmask(static_cast<Value>(e))
		{
		}

		constexpr inline Bitmask operator&(Bitmask b) const
		{
			return Bitmask(value_ & b.value_);
		}

		constexpr inline Bitmask operator|(Bitmask b) const
		{
			return Bitmask(value_ | b.value_);
		}

		constexpr inline Bitmask operator^(Bitmask b) const
		{
			return Bitmask(value_ ^ b.value_);
		}

		constexpr inline Bitmask operator~() const
		{
			return Bitmask(~value_);
		}

		constexpr inline Bitmask &operator&=(Bitmask b)
		{
			return *this = *this & b;
		}

		constexpr inline Bitmask &operator|=(Bitmask b)
		{
			return *this = *this | b;
		}

		constexpr inline Bitmask &operator^=(Bitmask b)
		{
			return *this = *this ^ b;
		}

		constexpr inline bool operator==(Bitmask b) const
		{
			return value_ == b.value_;
		}

		constexpr inline bool operator!=(Bitmask b) const
		{
			return value_ != b.value_;
		}

		constexpr Value value() const
		{
			return value_;
		}

		constexpr bool is_set(Bitmask b) const
		{
			return (*this & b) == b;
		}

		constexpr bool empty() const
		{
			return value_ == 0;
		}

	private:
		Value value_ = 0;
	};

// TODO: Find better alternative to macro. All SFINAE alternatives I can
//       come up with at the moment are more verbose and harder to read.
#define RAYNI_BITMASK_GLOBAL_OPERATORS(Bitmask)                                                                        \
	static constexpr inline Bitmask operator&(Bitmask::Enum e, Bitmask b) /* NOLINT(bugprone-macro-parentheses) */ \
	{                                                                                                              \
		return Bitmask(e) & b;                                                                                 \
	}                                                                                                              \
                                                                                                                       \
	static constexpr inline Bitmask operator|(Bitmask::Enum e, Bitmask b) /* NOLINT(bugprone-macro-parentheses) */ \
	{                                                                                                              \
		return Bitmask(e) | b;                                                                                 \
	}                                                                                                              \
                                                                                                                       \
	static constexpr inline Bitmask operator^(Bitmask::Enum e, Bitmask b) /* NOLINT(bugprone-macro-parentheses) */ \
	{                                                                                                              \
		return Bitmask(e) ^ b;                                                                                 \
	}                                                                                                              \
                                                                                                                       \
	static constexpr inline Bitmask operator~(Bitmask::Enum e) /* NOLINT(bugprone-macro-parentheses) */            \
	{                                                                                                              \
		return ~Bitmask(e);                                                                                    \
	}                                                                                                              \
                                                                                                                       \
	static constexpr inline bool operator==(Bitmask::Enum e, Bitmask b)                                            \
	{                                                                                                              \
		return Bitmask(e) == b;                                                                                \
	}                                                                                                              \
                                                                                                                       \
	static constexpr inline bool operator!=(Bitmask::Enum e, Bitmask b)                                            \
	{                                                                                                              \
		return Bitmask(e) != b;                                                                                \
	}
}

#endif // RAYNI_LIB_MATH_BITMASK_H
