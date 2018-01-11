/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2018 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_MATH_ANIMATED_TRANSFORM_H
#define RAYNI_LIB_MATH_ANIMATED_TRANSFORM_H

#include "lib/containers/variant.h"
#include "lib/math/decomposed_matrix4x4.h"
#include "lib/math/math.h"
#include "lib/math/transform.h"

namespace Rayni
{
	class AnimatedTransform
	{
	public:
		AnimatedTransform(real_t start_time,
		                  const Transform &start_transform,
		                  real_t end_time,
		                  const Transform &end_transform);

		explicit AnimatedTransform(const Variant &v);

		Transform interpolate(real_t time) const;

		AABB motion_bounds(const AABB &aabb) const;

	private:
		real_t start_time_ = 0;
		DecomposedMatrix4x4 start_;

		real_t end_time_ = 0;
		DecomposedMatrix4x4 end_;
	};
}

#endif // RAYNI_LIB_MATH_ANIMATED_TRANSFORM_H
