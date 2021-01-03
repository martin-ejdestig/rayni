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

#include "lib/math/transform.h"

#include <vector>

#include "lib/containers/variant.h"
#include "lib/math/math.h"
#include "lib/math/matrix4x4.h"
#include "lib/math/quaternion.h"
#include "lib/math/vector3.h"

namespace Rayni
{
	Result<Transform> Transform::from_variant(const Variant &v)
	{
		if (v.is_string())
			return from_variant_string(v);
		if (v.is_map())
			return from_variant_map(v);
		if (v.is_vector())
			return from_variant_vector(v);

		return Error(v.path(), "transform must be a string, map or vector");
	}

	Result<Transform> Transform::from_variant_string(const Variant &v)
	{
		const std::string &str = v.as_string();

		if (str == "identity")
			return Transform::identity();

		return Error(v.path(), "invalid transform \"" + str + "\"");
	}

	Result<Transform> Transform::from_variant_map(const Variant &v)
	{
		const auto &map = v.as_map();

		if (map.size() != 1)
			return Error(v.path(), "transform map must contain a single key value pair");

		const auto &[type, args] = *map.cbegin();

		if (type == "translate") {
			auto translation = args.to<Vector3>();
			if (!translation)
				return translation.error();
			return Transform::translate(*translation);
		}

		if (type == "scale") {
			if (args.is_vector()) {
				auto scale = args.to<Vector3>();
				if (!scale)
					return scale.error();
				return Transform::scale(*scale);
			}

			auto scale = args.to<real_t>();
			if (!scale)
				return scale.error();
			return Transform::scale(*scale);
		}

		if (type == "rotate_x") {
			auto x = args.to<real_t>();
			if (!x)
				return x.error();
			return Transform::rotate_x(radians_from_degrees(*x));
		}

		if (type == "rotate_y") {
			auto y = args.to<real_t>();
			if (!y)
				return y.error();
			return Transform::rotate_y(radians_from_degrees(*y));
		}

		if (type == "rotate_z") {
			auto z = args.to<real_t>();
			if (!z)
				return z.error();
			return Transform::rotate_z(radians_from_degrees(*z));
		}

		if (type == "rotate") {
			if (args.is_map()) {
				auto angle = args.get<real_t>("angle");
				if (!angle)
					return angle.error();
				auto axis = args.get<Vector3>("axis");
				if (!axis)
					return axis.error();
				return Transform::rotate(radians_from_degrees(*angle), *axis);
			}

			if (args.is_vector()) {
				auto rotation = args.to<Quaternion>();
				if (!rotation)
					return rotation.error();
				return Transform::rotate(*rotation);
			}

			return Error(args.path(), "expected map (with angle and axis) or vector (quaternion)");
		}

		if (type == "look_at") {
			auto translation = args.get<Vector3>("translation");
			if (!translation)
				return translation.error();
			auto center = args.get<Vector3>("center");
			if (!center)
				return center.error();
			auto up = args.get<Vector3>("up");
			if (!up)
				return up.error();
			return Transform::look_at(*translation, *center, *up);
		}

		return Error(v.path(), "unknown transform type \"" + type + "\"");
	}

	Result<Transform> Transform::from_variant_vector(const Variant &v)
	{
		const auto &vector = v.as_vector();

		if (vector.size() < 2)
			return Error(v.path(), "transform vector must contain at least 2 elements");

		Transform t = Transform::identity();

		for (std::size_t i = 0; i < vector.size(); i++) {
			auto ti = Transform::from_variant(vector[i]);
			if (!ti)
				return ti.error();

			if (i == 0)
				t = *ti;
			else
				t = Transform::combine(t, *ti);
		}

		return t;
	}
}
