// This file is part of Rayni.
//
// Copyright (C) 2013-2019 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_MATH_MATRIX4X4_H
#define RAYNI_LIB_MATH_MATRIX4X4_H

#include <cassert>
#include <cmath>
#include <utility>

#include "lib/math/math.h"
#include "lib/math/matrix3x3.h"
#include "lib/math/matrix_inverse.h"
#include "lib/math/quaternion.h"
#include "lib/math/vector3.h"
#include "lib/math/vector4.h"

namespace Rayni
{
	class Matrix4x4
	{
	public:
		static constexpr unsigned int SIZE = Vector4::SIZE;

		Matrix4x4() = default;

		Matrix4x4(const Vector4 &row0, const Vector4 &row1, const Vector4 &row2, const Vector4 &row3) :
		        rows_{row0, row1, row2, row3}
		{
		}

		explicit Matrix4x4(const Matrix3x3 &m) :
		        Matrix4x4({m(0, 0), m(0, 1), m(0, 2), 0},
		                  {m(1, 0), m(1, 1), m(1, 2), 0},
		                  {m(2, 0), m(2, 1), m(2, 2), 0},
		                  {0, 0, 0, 1})
		{
		}

		static Matrix4x4 identity()
		{
			return {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
		}

		static Matrix4x4 translate(real_t x, real_t y, real_t z)
		{
			return {{1, 0, 0, x}, {0, 1, 0, y}, {0, 0, 1, z}, {0, 0, 0, 1}};
		}

		static Matrix4x4 translate(const Vector3 &v)
		{
			return translate(v.x(), v.y(), v.z());
		}

		static Matrix4x4 scale(real_t x, real_t y, real_t z)
		{
			return {{x, 0, 0, 0}, {0, y, 0, 0}, {0, 0, z, 0}, {0, 0, 0, 1}};
		}

		static Matrix4x4 scale(const Vector3 &v)
		{
			return scale(v.x(), v.y(), v.z());
		}

		static Matrix4x4 scale(real_t s)
		{
			return scale(s, s, s);
		}

		static Matrix4x4 rotate_x(real_t radians)
		{
			real_t sin = std::sin(radians);
			real_t cos = std::cos(radians);
			return {{1, 0, 0, 0}, {0, cos, -sin, 0}, {0, sin, cos, 0}, {0, 0, 0, 1}};
		}

		static Matrix4x4 rotate_y(real_t radians)
		{
			real_t sin = std::sin(radians);
			real_t cos = std::cos(radians);
			return {{cos, 0, sin, 0}, {0, 1, 0, 0}, {-sin, 0, cos, 0}, {0, 0, 0, 1}};
		}

		static Matrix4x4 rotate_z(real_t radians)
		{
			real_t sin = std::sin(radians);
			real_t cos = std::cos(radians);
			return {{cos, -sin, 0, 0}, {sin, cos, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};
		}

		static Matrix4x4 rotate(real_t radians, const Vector3 &axis)
		{
			Vector3 a = axis.normalize();
			real_t s = std::sin(radians);
			real_t c = std::cos(radians);
			real_t t = 1 - c;

			return {{a.x() * a.x() * t + c,
			         a.x() * a.y() * t - a.z() * s,
			         a.x() * a.z() * t + a.y() * s,
			         0},
			        {a.x() * a.y() * t + a.z() * s,
			         a.y() * a.y() * t + c,
			         a.y() * a.z() * t - a.x() * s,
			         0},
			        {a.x() * a.z() * t - a.y() * s,
			         a.y() * a.z() * t + a.x() * s,
			         a.z() * a.z() * t + c,
			         0},
			        {0, 0, 0, 1}};
		}

		static Matrix4x4 rotate(const Quaternion &q)
		{
			real_t xx = q.x() * q.x();
			real_t yy = q.y() * q.y();
			real_t zz = q.z() * q.z();
			real_t xy = q.x() * q.y();
			real_t xz = q.x() * q.z();
			real_t yz = q.y() * q.z();
			real_t xw = q.x() * q.w();
			real_t yw = q.y() * q.w();
			real_t zw = q.z() * q.w();

			return {{1 - 2 * (yy + zz), 2 * (xy - zw), 2 * (xz + yw), 0},
			        {2 * (xy + zw), 1 - 2 * (xx + zz), 2 * (yz - xw), 0},
			        {2 * (xz - yw), 2 * (yz + xw), 1 - 2 * (xx + yy), 0},
			        {0, 0, 0, 1}};
		}

		static Matrix4x4 look_at(const Vector3 &translation, const Vector3 &center, const Vector3 &up)
		{
			Vector3 z_axis = (center - translation).normalize();
			Vector3 x_axis = up.normalize().cross(z_axis).normalize();
			Vector3 y_axis = z_axis.cross(x_axis);

			return {{x_axis.x(), y_axis.x(), z_axis.x(), translation.x()},
			        {x_axis.y(), y_axis.y(), z_axis.y(), translation.y()},
			        {x_axis.z(), y_axis.z(), z_axis.z(), translation.z()},
			        {0, 0, 0, 1}};
		}

		static Matrix4x4 from_axes(const Vector3 &x_axis, const Vector3 &y_axis, const Vector3 &z_axis)
		{
			return {{x_axis.x(), y_axis.x(), z_axis.x(), 0},
			        {x_axis.y(), y_axis.y(), z_axis.y(), 0},
			        {x_axis.z(), y_axis.z(), z_axis.z(), 0},
			        {0, 0, 0, 1}};
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

		Matrix4x4 operator*(const Matrix4x4 &right) const
		{
			Matrix4x4 result;

			for (unsigned int i = 0; i < SIZE; i++)
				for (unsigned int j = 0; j < SIZE; j++)
					result.rows_[i][j] =
					        rows_[i][0] * right.rows_[0][j] + rows_[i][1] * right.rows_[1][j] +
					        rows_[i][2] * right.rows_[2][j] + rows_[i][3] * right.rows_[3][j];

			return result;
		}

		Matrix4x4 inverse() const
		{
			return MatrixInverse::find(*this);
		}

		Matrix4x4 transpose() const
		{
			return {{rows_[0][0], rows_[1][0], rows_[2][0], rows_[3][0]},
			        {rows_[0][1], rows_[1][1], rows_[2][1], rows_[3][1]},
			        {rows_[0][2], rows_[1][2], rows_[2][2], rows_[3][2]},
			        {rows_[0][3], rows_[1][3], rows_[2][3], rows_[3][3]}};
		}

		Vector4 &row(unsigned int row_index)
		{
			assert(row_index < SIZE);

			return rows_[row_index];
		}

		const Vector4 &row(unsigned int row_index) const
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

		Vector3 x_axis() const
		{
			return {rows_[0][0], rows_[1][0], rows_[2][0]};
		}

		Vector3 y_axis() const
		{
			return {rows_[0][1], rows_[1][1], rows_[2][1]};
		}

		Vector3 z_axis() const
		{
			return {rows_[0][2], rows_[1][2], rows_[2][2]};
		}

		Vector3 translation() const
		{
			return {rows_[0][3], rows_[1][3], rows_[2][3]};
		}

		Quaternion rotation() const
		{
			return upper3x3().rotation();
		}

		Matrix3x3 upper3x3() const
		{
			return {{rows_[0].x(), rows_[0].y(), rows_[0].z()},
			        {rows_[1].x(), rows_[1].y(), rows_[1].z()},
			        {rows_[2].x(), rows_[2].y(), rows_[2].z()}};
		}

	private:
		Vector4 rows_[SIZE];
	};
}

#endif // RAYNI_LIB_MATH_MATRIX4X4_H
