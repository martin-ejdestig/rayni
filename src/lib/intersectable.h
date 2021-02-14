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

#ifndef RAYNI_LIB_INTERSECTABLE_H
#define RAYNI_LIB_INTERSECTABLE_H

#include "lib/intersection.h"
#include "lib/math/aabb.h"
#include "lib/math/ray.h"

namespace Rayni
{
	class Intersectable
	{
	public:
		virtual ~Intersectable() = default;

		virtual AABB aabb() const = 0;

		virtual bool intersect(const Ray &ray) const = 0;
		virtual bool intersect(const Ray &ray, Intersection &intersection) const = 0;
	};
}

#endif // RAYNI_LIB_INTERSECTABLE_H
