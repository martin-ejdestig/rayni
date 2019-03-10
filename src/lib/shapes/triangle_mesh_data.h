// This file is part of Rayni.
//
// Copyright (C) 2013-2019 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_SHAPES_TRIANGLE_MESH_DATA_H
#define RAYNI_LIB_SHAPES_TRIANGLE_MESH_DATA_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include "lib/containers/variant.h"
#include "lib/math/hash.h"
#include "lib/math/math.h"
#include "lib/math/vector3.h"

namespace Rayni
{
	struct TriangleMeshData
	{
		struct Indices;
		struct UV;

		using Index = unsigned int;

		static constexpr Index MAX_INDEX = std::numeric_limits<Index>::max();

		static TriangleMeshData from_variant(const Variant &v);

		static void calculate_normals(TriangleMeshData &data);

		std::vector<Vector3> points;
		std::vector<Vector3> normals;
		std::vector<UV> uvs;
		std::vector<Indices> indices;
	};

	struct TriangleMeshData::Indices
	{
		Indices(Index i1, Index i2, Index i3) : index1(i1), index2(i2), index3(i3)
		{
		}

		explicit Indices(const std::array<Index, 3> &is) : Indices(is[0], is[1], is[2])
		{
		}

		explicit Indices(const std::array<std::uint16_t, 3> &is) : Indices(is[0], is[1], is[2])
		{
		}

		explicit Indices(const std::array<std::uint8_t, 3> &is) : Indices(is[0], is[1], is[2])
		{
		}

		Index index1;
		Index index2;
		Index index3;
	};

	struct TriangleMeshData::UV
	{
		UV() = default;

		UV(real_t u_in, real_t v_in) : u(u_in), v(v_in)
		{
		}

		explicit UV(const std::array<real_t, 2> &uv) : UV(uv[0], uv[1])
		{
		}

		std::size_t hash() const
		{
			return hash_combine_for(u, v);
		}

		static inline int compare(const UV &uv1, const UV &uv2)
		{
			if (uv1.u < uv2.u)
				return -1;
			if (uv1.u > uv2.u)
				return 1;
			if (uv1.v < uv2.v)
				return -1;
			if (uv1.v > uv2.v)
				return 1;
			return 0;
		}

		real_t u = 0;
		real_t v = 0;
	};
}

#endif // RAYNI_LIB_SHAPES_TRIANGLE_MESH_DATA_H
