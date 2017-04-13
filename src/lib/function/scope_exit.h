/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2016-2017 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_FUNCTION_SCOPE_EXIT_H
#define RAYNI_LIB_FUNCTION_SCOPE_EXIT_H

#include <utility>

namespace Rayni
{
	// TODO: Looks like N4189 (scope_exit) will not make it into C++17. Is it in the next
	//       version of the standard so this naive implementation can be removed?
	template <typename F>
	class ScopeExit
	{
	public:
		explicit ScopeExit(F &&f) : function(std::move(f))
		{
		}

		ScopeExit(ScopeExit &) = delete;
		ScopeExit(ScopeExit &&) = default;

		~ScopeExit()
		{
			function();
		}

		ScopeExit &operator=(ScopeExit &) = delete;
		ScopeExit &operator=(ScopeExit &&) = default;

	private:
		F function;
	};

	template <typename F>
	ScopeExit<F> scope_exit(F &&f)
	{
		return ScopeExit<F>(std::move(f));
	}
}

#endif // RAYNI_LIB_FUNCTION_SCOPE_EXIT_H
