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

			rotation_ = pd.rotation.rotation();
			scale_ = pd.scale;
			translation_ = matrix.translation();
		}

		Matrix4x4 compose() const
		{
			return Matrix4x4::translate(translation_) * Matrix4x4::rotate(rotation_) * Matrix4x4(scale_);
		}

		DecomposedMatrix4x4 interpolate(real_t t, const DecomposedMatrix4x4 &to) const
		{
			DecomposedMatrix4x4 d;

			d.rotation_ = slerp(t, rotation_, to.rotation_);
			d.scale_ = lerp(t, scale_, to.scale_);
			d.translation_ = lerp(t, translation_, to.translation_);

			return d;
		}

	private:
		Quaternion rotation_;
		Matrix3x3 scale_;
		Vector3 translation_;
	};
}

#endif // RAYNI_LIB_MATH_DECOMPOSED_MATRIX4X4_H
