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

#ifndef RAYNI_LIB_MATH_TRANSFORM_H
#define RAYNI_LIB_MATH_TRANSFORM_H

#include <vector>

#include "lib/containers/variant.h"
#include "lib/function/result.h"
#include "lib/math/aabb.h"
#include "lib/math/math.h"
#include "lib/math/matrix4x4.h"
#include "lib/math/quaternion.h"
#include "lib/math/ray.h"
#include "lib/math/vector3.h"

namespace Rayni
{
	class Transform
	{
	public:
		Transform(const Matrix4x4 &matrix, const Matrix4x4 &inverse_matrix) :
		        matrix_(matrix),
		        inverse_matrix_(inverse_matrix)
		{
		}

		static Result<Transform> from_variant(const Variant &v);

		static Transform identity()
		{
			return {Matrix4x4::identity(), Matrix4x4::identity()};
		}

		static Transform translate(real_t x, real_t y, real_t z)
		{
			return {Matrix4x4::translate(x, y, z), Matrix4x4::translate(-x, -y, -z)};
		}

		static Transform translate(const Vector3 &v)
		{
			return translate(v.x(), v.y(), v.z());
		}

		static Transform scale(real_t x, real_t y, real_t z)
		{
			return {Matrix4x4::scale(x, y, z), Matrix4x4::scale(1 / x, 1 / y, 1 / z)};
		}

		static Transform scale(const Vector3 &v)
		{
			return scale(v.x(), v.y(), v.z());
		}

		static Transform scale(real_t s)
		{
			return scale(s, s, s);
		}

		static Transform rotate_x(real_t radians)
		{
			Matrix4x4 m = Matrix4x4::rotate_x(radians);
			return {m, m.transpose()};
		}

		static Transform rotate_y(real_t radians)
		{
			Matrix4x4 m = Matrix4x4::rotate_y(radians);
			return {m, m.transpose()};
		}

		static Transform rotate_z(real_t radians)
		{
			Matrix4x4 m = Matrix4x4::rotate_z(radians);
			return {m, m.transpose()};
		}

		static Transform rotate(real_t radians, const Vector3 &axis)
		{
			Matrix4x4 m = Matrix4x4::rotate(radians, axis);
			return {m, m.transpose()};
		}

		static Transform rotate(const Quaternion &q)
		{
			Matrix4x4 m = Matrix4x4::rotate(q);
			return {m, m.transpose()};
		}

		static Transform look_at(const Vector3 &translation, const Vector3 &center, const Vector3 &up)
		{
			Matrix4x4 m = Matrix4x4::look_at(translation, center, up);
			return {m.inverse(), m};
		}

		static Transform combine(const Transform &t1, const Transform &t2)
		{
			return {t1.matrix_ * t2.matrix_, t2.inverse_matrix_ * t1.inverse_matrix_};
		}

		Transform inverse() const
		{
			return {inverse_matrix_, matrix_};
		}

		const Matrix4x4 &matrix() const
		{
			return matrix_;
		}

		// Transform a point represented by a Vector3.
		//
		// When using a Vector3 to represent a homogeneous point, the weight is implicitly
		// set to 1.
		//
		// Assumes that the matrix is affine. If it is not, the new x, y and z should be
		// divided by the new weight. But since this assumption is mostly true, calculating
		// and dividing by w would result in extra needless calculations.
		// TODO: Mention perspective transform method in the section above.
		Vector3 transform_point(const Vector3 &v) const
		{
			Vector4 p(v.x(), v.y(), v.z(), 1);
			real_t x = matrix_.row(0).dot(p);
			real_t y = matrix_.row(1).dot(p);
			real_t z = matrix_.row(2).dot(p);
			return {x, y, z};
		}

		std::vector<Vector3> &transform_points(std::vector<Vector3> &points) const
		{
			for (Vector3 &p : points)
				p = transform_point(p);
			return points;
		}

		// Calculate error bounds for when transform is appliced to point with error bounds
		// point_error.
		//
		// See Pharr, M, Jakob, W, and Humphreys, G 2016. Physically Based Rendering. 3rd ed.
		// Chapter 3.9.4, Bounding Intersection Point Error, p. 227-229.
		Vector3 transform_point_error(const Vector3 &point, const Vector3 &point_error) const
		{
			Vector3 r0 = matrix_.upper3x3().row(0).abs();
			Vector3 r1 = matrix_.upper3x3().row(1).abs();
			Vector3 r2 = matrix_.upper3x3().row(2).abs();
			Vector3 t = matrix_.translation().abs();
			real_t g3 = error_bound_gamma(3);
			real_t g31 = g3 + real_t(1);
			real_t x_error = g31 * r0.dot(point_error) + g3 * (r0.dot(point) + t.x());
			real_t y_error = g31 * r1.dot(point_error) + g3 * (r1.dot(point) + t.y());
			real_t z_error = g31 * r2.dot(point_error) + g3 * (r2.dot(point) + t.z());
			return {x_error, y_error, z_error};
		}

		// Transform a direction/vector represented by a Vector3.
		//
		// When using a Vector3 to represent a homogeneous vector, the weight is implicitly
		// set to 0.
		Vector3 transform_direction(const Vector3 &v) const
		{
			Vector4 d(v.x(), v.y(), v.z(), 0);
			real_t x = matrix_.row(0).dot(d);
			real_t y = matrix_.row(1).dot(d);
			real_t z = matrix_.row(2).dot(d);
			return {x, y, z};
		}

		// Transform a normal represented by a Vector3.
		//
		// When using a Vector3 to represent a homogeneous vector, the weight is implicitly
		// set to 0. A normal should be transformed with the transpose of the inverse of the
		// transformation matrix.
		//
		// n   = normal          n' = transformed normal   S = normal transformation matrix
		// t   = tangent         t' = transformed tangent  M = transformation matrix
		// x^T = x transposed  x^-1 = inverse of x
		//
		//       0 = (n')^T * t' = (S * n)^T * M * t = n^T * S^T * M * t  =>
		// S^T * M = I                                                    =>
		//       S = (M^-1)^T
		Vector3 transform_normal(const Vector3 &v) const
		{
			Vector4 n(v.x(), v.y(), v.z(), 0);
			Matrix4x4 t = inverse_matrix_.transpose();
			real_t x = t.row(0).dot(n);
			real_t y = t.row(1).dot(n);
			real_t z = t.row(2).dot(n);
			return Vector3(x, y, z).normalize();
		}

		std::vector<Vector3> &transform_normals(std::vector<Vector3> &normals) const
		{
			for (Vector3 &n : normals)
				n = transform_normal(n);
			return normals;
		}

		// Transform an axis-aligned bounding box.
		//
		// And create a new axis-aligned bounding box that encompasses all the transformed
		// points.
		//
		// Takes advantage of the symmetry in the AABB to reduce the number of calculations.
		// See Arvo, J. (1995). Transforming Axis-Aligned Bounding Boxes. In A. S. Glassner,
		// Graphics Gems (pp. 548-550).
		//
		// This version, unlike the version in the book, does not use any loop or if
		// statements. This allows for the compiler to generate more efficient code. (E.g.
		// on X86 no branch instructions are needed to implement the component wise minimum
		// and maximum of a vector.)
		//
		// M       = transformation matrix        M|x     = column x of transformation matrix
		// min(v)  = component minimum of vector  max(v)  = component maximum of vector
		// c       = center                       r       = vector from center to +++ corner
		//
		// |c.x +/- r.x|
		// |c.y +/- r.y| = c +/- r = component +/- of vector used to represent all corners
		// |c.z +/- r.z|
		// |     1     |
		//
		// minimum = c - r = min(c +/- r) (here min() applies to all corners)
		//
		// transformed minimum = min(M * (c +/- r)) =
		// min(M|1 * (c.x +/- r.x) + M|2 * (c.y +/- r.y) + M|3 * (c.z +/- r.z) + M|4) =
		// min(M|1 * (c.x +/- r.x)) + min(M|2 * (c.y +/- r.y)) + min(M|3 * (c.z +/- r.z)) + M|4
		//
		// The same applies for maximum with min() replaced with max().
		AABB transform_aabb(const AABB &aabb) const
		{
			Vector3 x1 = matrix_.x_axis() * aabb.minimum().x();
			Vector3 x2 = matrix_.x_axis() * aabb.maximum().x();
			Vector3 y1 = matrix_.y_axis() * aabb.minimum().y();
			Vector3 y2 = matrix_.y_axis() * aabb.maximum().y();
			Vector3 z1 = matrix_.z_axis() * aabb.minimum().z();
			Vector3 z2 = matrix_.z_axis() * aabb.maximum().z();
			Vector3 min = Vector3::min(x1, x2) + Vector3::min(y1, y2) + Vector3::min(z1, z2) +
			              matrix_.translation();
			Vector3 max = Vector3::max(x1, x2) + Vector3::max(y1, y2) + Vector3::max(z1, z2) +
			              matrix_.translation();
			return {min, max};
		}

		Ray transform_ray(const Ray &r) const
		{
			Vector3 o = transform_point(r.origin);
			Vector3 d = transform_direction(r.direction);
			return {o, d, r.time};
		}

	private:
		static Result<Transform> from_variant_string(const Variant &v);
		static Result<Transform> from_variant_map(const Variant &v);
		static Result<Transform> from_variant_vector(const Variant &v);

		Matrix4x4 matrix_;
		Matrix4x4 inverse_matrix_;
	};
}

#endif // RAYNI_LIB_MATH_TRANSFORM_H
