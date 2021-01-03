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

#ifndef RAYNI_LIB_MATH_MATRIX3X3_H
#define RAYNI_LIB_MATH_MATRIX3X3_H

#include <algorithm>
#include <cassert>
#include <cmath>

#include "lib/math/math.h"
#include "lib/math/matrix_inverse.h"
#include "lib/math/quaternion.h"
#include "lib/math/vector3.h"

namespace Rayni
{
	class Matrix3x3
	{
	public:
		static constexpr unsigned int SIZE = Vector3::SIZE;

		Matrix3x3() = default;

		Matrix3x3(const Vector3 &row0, const Vector3 &row1, const Vector3 &row2) : rows_{row0, row1, row2}
		{
		}

		static Matrix3x3 scale(real_t x, real_t y, real_t z)
		{
			return {{x, 0, 0}, {0, y, 0}, {0, 0, z}};
		}

		static Matrix3x3 scale(real_t s)
		{
			return scale(s, s, s);
		}

		real_t &operator()(unsigned int row_index, unsigned int column_index)
		{
			assert(row_index < SIZE && column_index < SIZE);
			return rows_[row_index][column_index];
		}

		real_t operator()(unsigned int row_index, unsigned int column_index) const
		{
			assert(row_index < SIZE && column_index < SIZE);
			return rows_[row_index][column_index];
		}

		Matrix3x3 operator+(const Matrix3x3 &right) const
		{
			return {rows_[0] + right.rows_[0], rows_[1] + right.rows_[1], rows_[2] + right.rows_[2]};
		}

		Matrix3x3 operator-(const Matrix3x3 &right) const
		{
			return {rows_[0] - right.rows_[0], rows_[1] - right.rows_[1], rows_[2] - right.rows_[2]};
		}

		Matrix3x3 operator*(const Matrix3x3 &right) const
		{
			Matrix3x3 result;

			for (unsigned int i = 0; i < SIZE; i++)
				for (unsigned int j = 0; j < SIZE; j++)
					result.rows_[i][j] = rows_[i][0] * right.rows_[0][j] +
					                     rows_[i][1] * right.rows_[1][j] +
					                     rows_[i][2] * right.rows_[2][j];

			return result;
		}

		Matrix3x3 operator*(real_t s) const
		{
			return {rows_[0] * s, rows_[1] * s, rows_[2] * s};
		}

		friend Matrix3x3 operator*(real_t s, const Matrix3x3 &m)
		{
			return {s * m.rows_[0], s * m.rows_[1], s * m.rows_[2]};
		}

		Vector3 &row(unsigned int row_index)
		{
			assert(row_index < SIZE);

			return rows_[row_index];
		}

		void swap_rows(unsigned int row1_index, unsigned int row2_index)
		{
			assert(row1_index < SIZE && row2_index < SIZE && row1_index != row2_index);

			std::swap(rows_[row1_index], rows_[row2_index]);
		}

		void swap_columns(unsigned int column1_index, unsigned int column2_index)
		{
			assert(column1_index < SIZE && column2_index < SIZE && column1_index != column2_index);

			for (auto &row : rows_)
				std::swap(row[column1_index], row[column2_index]);
		}

		Matrix3x3 inverse() const
		{
			return MatrixInverse::find(*this);
		}

		Matrix3x3 transpose() const
		{
			return {{rows_[0][0], rows_[1][0], rows_[2][0]},
			        {rows_[0][1], rows_[1][1], rows_[2][1]},
			        {rows_[0][2], rows_[1][2], rows_[2][2]}};
		}

		real_t trace() const
		{
			return rows_[0][0] + rows_[1][1] + rows_[2][2];
		}

		unsigned int max_diagonal_position() const
		{
			unsigned int pos = 0;

			if (rows_[1][1] > rows_[0][0])
				pos = 1;

			if (rows_[2][2] > rows_[pos][pos])
				pos = 2;

			return pos;
		}

		real_t max_absolute_row_sum_norm() const
		{
			real_t norm = 0;

			for (const auto &row : rows_)
				norm = std::max(norm, std::abs(row.x()) + std::abs(row.y()) + std::abs(row.z()));

			return norm;
		}

		Quaternion rotation() const
		{
			real_t xyz[3];
			real_t w;
			real_t t = trace();

			if (t > 0)
			{
				real_t s = std::sqrt(t + 1);

				w = s * real_t(0.5);
				s = real_t(0.5) / s;
				xyz[0] = (rows_[2][1] - rows_[1][2]) * s;
				xyz[1] = (rows_[0][2] - rows_[2][0]) * s;
				xyz[2] = (rows_[1][0] - rows_[0][1]) * s;
			}
			else
			{
				unsigned int i = max_diagonal_position();
				unsigned int j = (i + 1) % 3;
				unsigned int k = (j + 1) % 3;
				real_t s = std::sqrt(rows_[i][i] - rows_[j][j] - rows_[k][k] + 1);

				xyz[i] = s * real_t(0.5);
				if (s != 0)
					s = real_t(0.5) / s;
				w = (rows_[k][j] - rows_[j][k]) * s;
				xyz[j] = (rows_[j][i] + rows_[i][j]) * s;
				xyz[k] = (rows_[k][i] + rows_[i][k]) * s;
			}

			return {xyz[0], xyz[1], xyz[2], w};
		}

		bool preserves_orientation_of_basis() const
		{
			real_t determinant = rows_[0].dot(rows_[1].cross(rows_[2]));
			return determinant > 0;
		}

	private:
		Vector3 rows_[SIZE];
	};
}

#endif // RAYNI_LIB_MATH_MATRIX3X3_H
