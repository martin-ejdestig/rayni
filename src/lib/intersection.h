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

#ifndef RAYNI_LIB_INTERSECTION_H
#define RAYNI_LIB_INTERSECTION_H

#include "lib/math/math.h"
#include "lib/math/vector3.h"

namespace Rayni
{
	class Intersectable;
	struct Material;

	struct Intersection
	{
		real_t t = REAL_INFINITY;
		Vector3 point;
		Vector3 point_error; // See e.g. error_bound_gamma() and ray_with_offset_origin() for references.
		Vector3 normal;
		Vector3 incident;
		const Intersectable *intersectable = nullptr;
		const Material *material = nullptr;
		real_t u = 0;
		real_t v = 0;
	};
}

#endif // RAYNI_LIB_INTERSECTION_H
