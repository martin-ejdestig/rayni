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

#ifndef RAYNI_LIB_MATH_DECOMPOSED_MATRIX4X4_H
#define RAYNI_LIB_MATH_DECOMPOSED_MATRIX4X4_H

#include "lib/math/lerp.h"
#include "lib/math/math.h"
#include "lib/math/matrix3x3.h"
#include "lib/math/matrix4x4.h"
#include "lib/math/polar_decomposition.h"
#include "lib/math/quaternion.h"
#include "lib/math/vector3.h"

namespace Rayni
{
	class DecomposedMatrix4x4
	{
	public:
		DecomposedMatrix4x4() = default;

		explicit DecomposedMatrix4x4(const Matrix4x4 &matrix)
		{
			PolarDecomposition<Matrix3x3> pd(matrix.upper3x3());

			rotation = pd.rotation.rotation();
			scale = pd.scale;
			translation = matrix.translation();
		}

		Matrix4x4 compose() const
		{
			return Matrix4x4::translate(translation) * Matrix4x4::rotate(rotation) * Matrix4x4(scale);
		}

		DecomposedMatrix4x4 interpolate(real_t t, const DecomposedMatrix4x4 &to) const
		{
			DecomposedMatrix4x4 d;

			d.rotation = slerp(t, rotation, to.rotation);
			d.scale = lerp(t, scale, to.scale);
			d.translation = lerp(t, translation, to.translation);

			return d;
		}

	private:
		Quaternion rotation;
		Matrix3x3 scale;
		Vector3 translation;
	};
}

#endif // RAYNI_LIB_MATH_DECOMPOSED_MATRIX4X4_H
