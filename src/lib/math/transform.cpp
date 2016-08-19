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

#include "lib/math/transform.h"

#include <vector>

#include "lib/containers/variant.h"
#include "lib/math/math.h"
#include "lib/math/matrix4x4.h"
#include "lib/math/quaternion.h"
#include "lib/math/vector3.h"

namespace Rayni
{
	Transform Transform::from_variant(const Variant &v)
	{
		if (v.is_string())
			return from_variant_string(v);
		if (v.is_map())
			return from_variant_map(v);
		if (v.is_vector())
			return from_variant_vector(v);

		throw Variant::Exception(v, "transform must be a string, map or vector");
	}

	Transform Transform::from_variant_string(const Variant &v)
	{
		const std::string &str = v.as_string();

		if (str == "identity")
			return Transform::identity();

		throw Variant::Exception(v, "invalid transform \"" + str + "\"");
	}

	Transform Transform::from_variant_map(const Variant &v)
	{
		auto &map = v.as_map();

		if (map.size() != 1)
			throw Variant::Exception(v, "transform map must contain a single key value pair");

		const std::string &type = map.cbegin()->first;
		const Variant &args = map.cbegin()->second;

		if (type == "translate")
			return Transform::translate(args.to<Vector3>());

		if (type == "scale")
			return args.is_vector() ? Transform::scale(args.to<Vector3>()) :
			                          Transform::scale(args.to<real_t>());

		if (type == "rotate_x")
			return Transform::rotate_x(radians_from_degrees(args.to<real_t>()));

		if (type == "rotate_y")
			return Transform::rotate_y(radians_from_degrees(args.to<real_t>()));

		if (type == "rotate_z")
			return Transform::rotate_z(radians_from_degrees(args.to<real_t>()));

		if (type == "rotate")
		{
			if (args.is_map())
				return Transform::rotate(radians_from_degrees(args.get<real_t>("angle")),
				                         args.get<Vector3>("axis"));
			if (args.is_vector())
				return Transform::rotate(Quaternion(args));

			throw Variant::Exception(args, "expected map (with angle and axis) or vector (quaternion)");
		}

		if (type == "look_at")
			return Transform::look_at(args.get<Vector3>("translation"),
			                          args.get<Vector3>("center"),
			                          args.get<Vector3>("up"));

		throw Variant::Exception(v, "unknown transform type \"" + type + "\"");
	}

	Transform Transform::from_variant_vector(const Variant &v)
	{
		auto num_transforms = v.as_vector().size();

		if (num_transforms < 2)
			throw Variant::Exception(v, "transform vector must contain at least 2 elements");

		Transform t = v.get<Transform>(0);

		for (std::size_t i = 1; i < num_transforms; i++)
			t = Transform::combine(t, v.get<Transform>(i));

		return t;
	}
}
