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

#include "lib/shapes/triangle_mesh_data.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "lib/containers/variant.h"
#include "lib/io/binary_reader.h"
#include "lib/math/vector3.h"
#include "lib/string/base85.h"

namespace Rayni
{
	namespace
	{
		std::vector<std::uint8_t> decode_base85(const Variant &v)
		{
			auto decoded = base85_decode(v.as_string());

			if (!decoded.has_value())
				throw Variant::Exception(v, "invalid base85 encoded data");

			return decoded.value();
		}

		template <typename V, typename I, std::size_t INTS_PER_VALUE>
		std::vector<V> decode_fixed_point_values(const Variant &v, unsigned int denominator)
		{
			std::vector<std::uint8_t> data = decode_base85(v);
			std::size_t number_of_values = data.size() / (sizeof(I) * INTS_PER_VALUE);
			BinaryReader reader;
			std::vector<V> values;

			reader.set_data(std::move(data));

			for (std::size_t i = 0; i < number_of_values; i++)
			{
				std::array<real_t, INTS_PER_VALUE> reals;

				for (auto &real : reals)
					real = real_t(reader.read_big_endian<I>()) / real_t(denominator);

				values.emplace_back(reals);
			}

			return values;
		}

		template <typename V, typename I, std::size_t INTS_PER_VALUE>
		std::vector<V> decode_integer_values(const Variant &v)
		{
			std::vector<std::uint8_t> data = decode_base85(v);
			std::size_t number_of_values = data.size() / (sizeof(I) * INTS_PER_VALUE);
			BinaryReader reader;
			std::vector<V> values;

			reader.set_data(std::move(data));

			for (std::size_t i = 0; i < number_of_values; i++)
			{
				std::array<I, INTS_PER_VALUE> integers;

				for (auto &integer : integers)
					integer = reader.read_big_endian<I>();

				values.emplace_back(integers);
			}

			return values;
		}

		std::vector<Vector3> decode_points(const Variant &v)
		{
			std::vector<Vector3> points = decode_fixed_point_values<Vector3, std::int32_t, 3>(v, 0x10000);

			if (points.size() < 3 || points.size() - 1 > TriangleMeshData::MAX_INDEX)
				throw Variant::Exception(v, "number of points out of range");

			return points;
		}

		std::vector<TriangleMeshData::Indices> decode_indices(const Variant &v, std::size_t num_points)
		{
			std::vector<TriangleMeshData::Indices> indices;

			if (num_points <= 0xff)
				indices = decode_integer_values<TriangleMeshData::Indices, std::uint8_t, 3>(v);
			else if (num_points <= 0xffff)
				indices = decode_integer_values<TriangleMeshData::Indices, std::uint16_t, 3>(v);
			else
				indices = decode_integer_values<TriangleMeshData::Indices, std::uint32_t, 3>(v);

			if (indices.empty())
				throw Variant::Exception(v, "no indices specified");

			for (const auto &idx : indices)
				if (idx.index1 >= num_points || idx.index2 >= num_points || idx.index3 >= num_points)
					throw Variant::Exception(v, "found index >= point count");

			return indices;
		}

		std::vector<Vector3> decode_normals(const Variant &v, std::size_t num_points)
		{
			std::vector<Vector3> normals = decode_fixed_point_values<Vector3, std::int16_t, 3>(v, 0x7fff);

			if (normals.size() != num_points)
				throw Variant::Exception(v, "normal count does not match point count");

			return normals;
		}

		std::vector<TriangleMeshData::UV> decode_uvs(const Variant &v, std::size_t num_points)
		{
			std::vector<TriangleMeshData::UV> uvs =
			        decode_fixed_point_values<TriangleMeshData::UV, std::int16_t, 2>(v, 0x7fff);

			if (uvs.size() != num_points)
				throw Variant::Exception(v, "uv count does not match point count");

			return uvs;
		}
	}

	TriangleMeshData TriangleMeshData::from_variant(const Variant &v)
	{
		TriangleMeshData data;

		data.points = decode_points(v.get("points"));
		data.indices = decode_indices(v.get("indices"), data.points.size());

		if (v.has("normals"))
		{
			const Variant &vn = v.get("normals");

			if (vn.as_string() == "calculate")
				calculate_normals(data);
			else
				data.normals = decode_normals(vn, data.points.size());
		}

		if (v.has("uvs"))
			data.uvs = decode_uvs(v.get("uvs"), data.points.size());

		return data;
	}

	void TriangleMeshData::calculate_normals(TriangleMeshData &data)
	{
		data.normals = std::vector<Vector3>(data.points.size());

		for (const Indices &is : data.indices)
		{
			const Vector3 &point1 = data.points[is.index1];
			const Vector3 &point2 = data.points[is.index2];
			const Vector3 &point3 = data.points[is.index3];
			Vector3 normal = (point2 - point1).cross(point3 - point1); // Do not normalize, see below.

			data.normals[is.index1] += normal;
			data.normals[is.index2] += normal;
			data.normals[is.index3] += normal;
		}

		// Cross product length is relative to triangle area (area of parallelogram of
		// edges). Only normalizing after each triangle normal has been added for a vertex
		// makes it so normals of triangles with a larger area contribute more.
		for (Vector3 &n : data.normals)
			n = n.normalize();
	}
}
