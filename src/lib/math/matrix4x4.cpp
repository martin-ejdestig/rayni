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

#include "lib/math/matrix4x4.h"

#include <algorithm>
#include <cassert>
#include <cmath>

#include "lib/math/math.h"

namespace Rayni
{
	struct Matrix4x4::PivotPosition
	{
		unsigned int row;
		unsigned int column;
	};

	/**
	 * In-place inverse of (non-singular) matrix.
	 *
	 * Calling this method with a singular matrix is considered a programming error.
	 *
	 * Uses Gauss-Jordan elimination with partial (row) pivoting to increase numerical
	 * stability.
	 */
	void Matrix4x4::in_place_inverse()
	{
		PivotPosition pivot_positions[4] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
		unsigned int pivot_used[4] = {0, 0, 0, 0};

		for (auto &pivot_position : pivot_positions)
		{
			real_t max = 0;

			for (unsigned int row = 0; row < 4; row++)
			{
				if (pivot_used[row] == 1)
					continue;

				for (unsigned int column = 0; column < 4; column++)
				{
					if (pivot_used[column] == 0 && std::abs(rows[row][column]) >= max)
					{
						max = std::abs(rows[row][column]);
						pivot_position = {row, column};
					}
				}
			}

			if (pivot_position.row != pivot_position.column)
				swap_rows(pivot_position.row, pivot_position.column);

			unsigned int pos = pivot_position.column;
			assert(max > 0 && pivot_used[pos] == 0); // Singular matrix?
			pivot_used[pos]++;

			real_t pivot_inv = 1 / rows[pos][pos];
			rows[pos][pos] = 1;
			rows[pos] *= pivot_inv;

			for (unsigned int row = 0; row < 4; row++)
			{
				if (row != pos)
				{
					real_t old_value = rows[row][pos];
					rows[row][pos] = 0;
					rows[row] += rows[pos] * -old_value;
				}
			}
		}

		for (int i = 3; i >= 0; i--)
			if (pivot_positions[i].row != pivot_positions[i].column)
				swap_columns(pivot_positions[i].row, pivot_positions[i].column);
	}

	Quaternion Matrix4x4::rotation() const
	{
		real_t xyz[3], w;
		real_t trace = upper3x3().trace();

		if (trace > 0)
		{
			real_t s = std::sqrt(trace + 1);

			w = s * real_t(0.5);
			s = real_t(0.5) / s;
			xyz[0] = (rows[2][1] - rows[1][2]) * s;
			xyz[1] = (rows[0][2] - rows[2][0]) * s;
			xyz[2] = (rows[1][0] - rows[0][1]) * s;
		}
		else
		{
			unsigned int i = upper3x3().max_diagonal_position();
			unsigned int j = (i + 1) % 3;
			unsigned int k = (j + 1) % 3;
			real_t s = std::sqrt(rows[i][i] - rows[j][j] - rows[k][k] + 1);

			xyz[i] = s * real_t(0.5);
			if (s != 0)
				s = real_t(0.5) / s;
			w = (rows[k][j] - rows[j][k]) * s;
			xyz[j] = (rows[j][i] + rows[i][j]) * s;
			xyz[k] = (rows[k][i] + rows[i][k]) * s;
		}

		return {xyz[0], xyz[1], xyz[2], w};
	}

	/**
	 * Polar decomposition. Only consider upper 3x3 part since it is only used on affine
	 * transformations and translation is never relevant when decomposing.
	 *
	 * Repeatedly calculates (where x^T = x transposed, x^-1 = inverse of x):
	 *
	 * M = (M + (M^T)^-1) * 0.5
	 *
	 * until convergence. If M is a pure rotation matrix, averaging it with the inverse of its
	 * transpose will leave it unchanged since M^-1 = M^T .
	 *
	 * See Higham, Nicholas J. (1986). Computing the polar decomposition - with Applications.
	 */
	Matrix4x4::PolarDecomposition Matrix4x4::polar_decomposition() const
	{
		static constexpr unsigned int MAX_STEPS = 100;

		Matrix4x4 rotation_start = *this;
		rotation_start.set_row(3, {0, 0, 0, 1});
		rotation_start.set_column(3, {0, 0, 0, 1});

		Matrix4x4 rotation = rotation_start;

		for (unsigned int i = 0; i < MAX_STEPS; i++)
		{
			Matrix4x4 rotation_next = (rotation + rotation.transpose().inverse()) * real_t(0.5);
			real_t norm_of_diff = 0;

			for (unsigned int row = 0; row < 3; row++)
				norm_of_diff = std::max(norm_of_diff,
				                        std::abs(rotation(row, 0) - rotation_next(row, 0)) +
				                                std::abs(rotation(row, 1) - rotation_next(row, 1)) +
				                                std::abs(rotation(row, 2) - rotation_next(row, 2)));

			if (norm_of_diff <= real_t(0.0001))
				break;

			rotation = rotation_next;
		}

		if (!rotation.preserves_orientation_of_basis())
			rotation = rotation * scale(-1);

		Matrix4x4 scale = rotation.inverse() * rotation_start;

		return {rotation, scale};
	}
}
