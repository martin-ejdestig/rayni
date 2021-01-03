// This file is part of Rayni.
//
// Copyright (C) 2016-2021 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_FUNCTION_SCOPE_EXIT_H
#define RAYNI_LIB_FUNCTION_SCOPE_EXIT_H

#include <utility>

namespace Rayni
{
	// TODO: Will p0052r6 make it into C++20 so this naive implementation can be removed?
	//       See http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0052r6.pdf .
	template <typename F>
	class ScopeExit
	{
	public:
		explicit ScopeExit(F &&f) : function_(std::move(f))
		{
		}

		ScopeExit(ScopeExit &) = delete;
		ScopeExit(ScopeExit &&) noexcept = default;

		~ScopeExit()
		{
			function_();
		}

		ScopeExit &operator=(ScopeExit &) = delete;
		ScopeExit &operator=(ScopeExit &&) noexcept = default;

	private:
		F function_;
	};

	template <typename F>
	ScopeExit<F> scope_exit(F &&f)
	{
		return ScopeExit<F>(std::forward<F>(f));
	}
}

#endif // RAYNI_LIB_FUNCTION_SCOPE_EXIT_H
