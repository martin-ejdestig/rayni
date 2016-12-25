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

#include "lib/math/math.h"

namespace Rayni
{
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
			real_t norm_of_diff = (rotation - rotation_next).upper3x3().max_absolute_row_sum_norm();

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
