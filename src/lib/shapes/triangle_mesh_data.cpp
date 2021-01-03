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
		template <typename V, typename I, std::size_t INTS_PER_VALUE>
		Result<std::vector<V>> decode_fixed_point_values(const Variant &v,
		                                                 const std::string &key,
		                                                 unsigned int denominator)
		{
			const Variant *vstr = v.get(key);
			if (!vstr || !vstr->is_string())
				return Error(v.path(), "missing " + key + " with string value");

			auto data = base85_decode(vstr->as_string());
			if (!data)
				return Error(v.path(), "invalid base85 encoded string in " + key);

			std::size_t number_of_values = data->size() / (sizeof(I) * INTS_PER_VALUE);
			BinaryReader reader;
			std::vector<V> values;

			reader.set_data(std::move(*data));

			for (std::size_t i = 0; i < number_of_values; i++) {
				std::array<real_t, INTS_PER_VALUE> reals;

				for (auto &real : reals)
					real = real_t(reader.read_big_endian<I>()) / real_t(denominator);

				values.emplace_back(reals);
			}

			return values;
		}

		template <typename V, typename I, std::size_t INTS_PER_VALUE>
		Result<std::vector<V>> decode_integer_values(const Variant &v, const std::string &key)
		{
			const Variant *vstr = v.get(key);
			if (!vstr || !vstr->is_string())
				return Error(v.path(), "missing " + key + " with string value");

			auto data = base85_decode(vstr->as_string());
			if (!data)
				return Error(v.path(), "invalid base85 encoded string in " + key);

			std::size_t number_of_values = data->size() / (sizeof(I) * INTS_PER_VALUE);
			BinaryReader reader;
			std::vector<V> values;

			reader.set_data(std::move(*data));

			for (std::size_t i = 0; i < number_of_values; i++) {
				std::array<I, INTS_PER_VALUE> integers;

				for (auto &integer : integers)
					integer = reader.read_big_endian<I>();

				values.emplace_back(integers);
			}

			return values;
		}

		Result<std::vector<Vector3>> decode_points(const Variant &v)
		{
			auto points = decode_fixed_point_values<Vector3, std::int32_t, 3>(v, "points", 0x10000);
			if (!points)
				return points.error();

			if (points->size() < 3 || points->size() - 1 > TriangleMeshData::MAX_INDEX)
				return Error(v.path(), "number of points out of range");

			return points;
		}

		Result<std::vector<TriangleMeshData::Indices>> decode_indices(const Variant &v, std::size_t num_points)
		{
			using Indices = TriangleMeshData::Indices;
			std::vector<Indices> indices;

			if (num_points <= 0xff) {
				auto is = decode_integer_values<Indices, std::uint8_t, 3>(v, "indices");
				if (!is)
					return is.error();
				indices = std::move(*is);
			} else if (num_points <= 0xffff) {
				auto is = decode_integer_values<Indices, std::uint16_t, 3>(v, "indices");
				if (!is)
					return is.error();
				indices = std::move(*is);
			} else {
				auto is = decode_integer_values<Indices, std::uint32_t, 3>(v, "indices");
				if (!is)
					return is.error();
				indices = std::move(*is);
			}

			if (indices.empty())
				return Error(v.path(), "no indices specified");

			for (const auto &idx : indices)
				if (idx.index1 >= num_points || idx.index2 >= num_points || idx.index3 >= num_points)
					return Error(v.path(), "found index >= point count");

			return indices;
		}

		Result<std::vector<Vector3>> decode_normals(const Variant &v, std::size_t num_points)
		{
			auto normals = decode_fixed_point_values<Vector3, std::int16_t, 3>(v, "normals", 0x7fff);
			if (!normals)
				return normals.error();

			if (normals->size() != num_points)
				return Error(v.path(), "normal count does not match point count");

			return normals;
		}

		Result<std::vector<TriangleMeshData::UV>> decode_uvs(const Variant &v, std::size_t num_points)
		{
			auto uvs = decode_fixed_point_values<TriangleMeshData::UV, std::int16_t, 2>(v, "uvs", 0x7fff);

			if (uvs->size() != num_points)
				return Error(v.path(), "uv count does not match point count");

			return uvs;
		}
	}

	Result<TriangleMeshData> TriangleMeshData::from_variant(const Variant &v)
	{
		TriangleMeshData data;

		auto points = decode_points(v);
		if (!points)
			return points.error();
		data.points = std::move(*points);

		auto indices = decode_indices(v, data.points.size());
		if (!indices)
			return indices.error();
		data.indices = std::move(*indices);

		if (const Variant *vn = v.get("normals"); vn) {
			if (vn->is_string() && vn->as_string() == "calculate") {
				calculate_normals(data);
			} else {
				auto normals = decode_normals(v, data.points.size());
				if (!normals)
					return normals.error();
				data.normals = std::move(*normals);
			}
		}

		if (v.has("uvs")) {
			auto uvs = decode_uvs(v, data.points.size());
			if (!uvs)
				return uvs.error();
			data.uvs = std::move(*uvs);
		}

		return data;
	}

	void TriangleMeshData::calculate_normals(TriangleMeshData &data)
	{
		data.normals = std::vector<Vector3>(data.points.size());

		for (const Indices &is : data.indices) {
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
