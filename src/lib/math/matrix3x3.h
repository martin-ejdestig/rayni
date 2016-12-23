/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2016 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_MATH_MATRIX3X3_H
#define RAYNI_LIB_MATH_MATRIX3X3_H

#include <cassert>

#include "lib/math/math.h"
#include "lib/math/vector3.h"

namespace Rayni
{
	class Matrix3x3 // NOLINT Remove when https://llvm.org/bugs/show_bug.cgi?id=30965 is fixed.
	{
	public:
		Matrix3x3()
		{
		}

		Matrix3x3(const Vector3 &row0, const Vector3 &row1, const Vector3 &row2) : rows{row0, row1, row2}
		{
		}

		real_t &operator()(unsigned int row_index, unsigned int column_index)
		{
			assert(row_index < 3 && column_index < 3);
			return rows[row_index][column_index];
		}

		real_t operator()(unsigned int row_index, unsigned int column_index) const
		{
			assert(row_index < 3 && column_index < 3);
			return rows[row_index][column_index];
		}

		real_t trace() const
		{
			return rows[0][0] + rows[1][1] + rows[2][2];
		}

		unsigned int max_diagonal_position() const
		{
			unsigned int pos = 0;

			if (rows[1][1] > rows[0][0])
				pos = 1;

			if (rows[2][2] > rows[pos][pos])
				pos = 2;

			return pos;
		}

	private:
		Vector3 rows[3];
	};
}

#endif // RAYNI_LIB_MATH_MATRIX3X3_H
