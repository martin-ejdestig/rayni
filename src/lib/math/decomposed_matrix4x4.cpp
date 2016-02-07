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

#include "lib/math/decomposed_matrix4x4.h"

namespace Rayni
{
	DecomposedMatrix4x4::DecomposedMatrix4x4(const Matrix4x4 &matrix)
	{
		auto pd = matrix.polar_decomposition();

		rotation = pd.rotation.get_rotation();

		scale_x = pd.scale.get_x_axis();
		scale_y = pd.scale.get_y_axis();
		scale_z = pd.scale.get_z_axis();

		translation = matrix.get_translation();
	}
}
