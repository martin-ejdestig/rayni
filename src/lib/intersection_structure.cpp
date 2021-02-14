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

#include "lib/intersection_structure.h"

#include <string>
#include <utility>

#include "lib/containers/variant.h"
#include "lib/intersection_structures/bvh.h"
#include "lib/intersection_structures/kdtree.h"

namespace Rayni
{
	Result<IntersectionStructureType> intersection_structure_type_from_variant(const Variant &v)
	{
		if (v.is_string()) {
			const std::string &str = v.as_string();

			if (str == "bvh")
				return IntersectionStructureType::BVH;
			if (str == "kdtree")
				return IntersectionStructureType::KDTREE;
			if (str == "default")
				return IntersectionStructureType::DEFAULT;
		}

		return Error(v.path(), "unknown intersection structure type");
	}

	std::unique_ptr<Intersectable> intersection_structure_build(IntersectionStructureType type,
	                                                            std::vector<const Intersectable *> &&intersectables,
	                                                            const Cancellable &cancellable,
	                                                            ThreadPool &thread_pool)
	{
		std::unique_ptr<Intersectable> intersection_structure;

		switch (type) {
		case IntersectionStructureType::BVH:
			intersection_structure = bvh_build(std::move(intersectables), cancellable, thread_pool);
			break;
		case IntersectionStructureType::KDTREE:
			intersection_structure = kdtree_build(std::move(intersectables), cancellable, thread_pool);
			break;
		}

		return intersection_structure;
	}
}
