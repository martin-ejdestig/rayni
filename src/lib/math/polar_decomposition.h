/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2019 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_MATH_POLAR_DECOMPOSITION_H
#define RAYNI_LIB_MATH_POLAR_DECOMPOSITION_H

#include "lib/math/math.h"

namespace Rayni
{
	/**
	 * Polar decomposition of matrix. Repeatedly calculates (where x^T = x transposed,
	 * x^-1 = inverse of x):
	 *
	 * M = (M + (M^T)^-1) * 0.5
	 *
	 * until convergence. If M is a pure rotation matrix, averaging it with the inverse of its
	 * transpose will leave it unchanged since M^-1 = M^T .
	 *
	 * See Higham, Nicholas J. (1986). Computing the polar decomposition - with Applications.
	 */
	template <typename Matrix>
	struct PolarDecomposition
	{
		explicit PolarDecomposition(const Matrix &matrix)
		{
			static constexpr unsigned int MAX_STEPS = 100;
			rotation = matrix;

			for (unsigned int i = 0; i < MAX_STEPS; i++)
			{
				Matrix rotation_next = (rotation + rotation.transpose().inverse()) * real_t(0.5);
				real_t norm_of_diff = (rotation - rotation_next).max_absolute_row_sum_norm();

				if (norm_of_diff <= real_t(0.0001))
					break;

				rotation = rotation_next;
			}

			if (!rotation.preserves_orientation_of_basis())
				rotation = rotation * Matrix::scale(-1);

			scale = rotation.inverse() * matrix;
		}

		Matrix rotation;
		Matrix scale;
	};
}

#endif // RAYNI_LIB_MATH_POLAR_DECOMPOSITION_H
