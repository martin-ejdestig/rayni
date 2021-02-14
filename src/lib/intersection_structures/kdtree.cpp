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

#include "lib/intersection_structures/kdtree.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "config.h"
#include "lib/concurrency/barrier.h"
#include "lib/containers/cache_line_aligned_vector.h"
#include "lib/intersection.h"
#include "lib/intersection_structures/kdtree.h"
#include "lib/log.h"
#include "lib/math/aabb.h"
#include "lib/math/hash.h"
#include "lib/math/math.h"
#include "lib/stopwatch.h"
#include "lib/string/duration_format.h"

// For algorithm used to build KdTree, see:
//
// "On building fast kd-Trees for Ray Tracing, and doing that in O(N log N)"
// http://www.cgg.cvut.cz/members/havran/ARTICLES/ingo06rtKdtree.pdf
//
// An effort has been made to reduce memory allocations/reallocations when
// building and to be cache friendly when intersecting and building. There are
// some properties of the build algorithm that makes further improvements hard
// though. See TODO below.

// TODO: Possible to use less memory (and be more cache friendly) with current
//       build algorithm?
//
// E.g. not good to have separate std::vector:s in all BuildNode:s. Also
// separate allocations for events and indices. If memory is not freed for old
// input to split_build_input() memory usage increases quite a lot. Not sure how
// to avoid this and also no allocate separate small allocations that can be
// freed in each recursion step.

// TODO: Better use of threads when building.
//
// Some problems with current implementation:
//
// - Finding split position for initial node (the most expensive one) only
//   utilizes one thread.
//
// - ThreadPool::threads_available() has to be used to avoid indefinite blocking
//   when all threads are used to create far right side nodes. Next right side
//   node task will be queued to pool but no thread will ever run it. Can also
//   currently "starve" out creation of nodes higher up in tree with lower nodes
//   which means threads will be idle more than necessary.
//
// Probably have to switch build algorithm completely to address these concerns.

// TODO: "Main" thread is currently used to do work as well when building.
//
// Which means #logical_cores * 2 + 1 can be building a sub-tree. Easy to fix.
// But on CPU I have been testing on (an old one) and current implementation,
// this actually helps with performance. Should reevaluate this on a never CPU
// with more cores and faster memory etc.

namespace Rayni
{
	namespace
	{
		constexpr real_t TRAVERSAL_COST = 0.3;
		constexpr real_t INTERSECTION_COST = 1.0;
		constexpr real_t EMPTY_BONUS = 0.8;
		constexpr unsigned int ABSOLUTE_MAX_DEPTH = 64;

		constexpr unsigned int THREAD_MIN_INTERSECTABLES = 10000;

		// Max (start+end) * #axes = 2 * 3 = 6 events/intersectable. Planar events
		// (1 event/axis) should be fairly uncommon so not that much memory is wasted.
		constexpr unsigned int MAX_EVENTS_PER_INTERSECTABLE = 6;

		class Node
		{
		public:
			Node(std::uint8_t split_axis, real_t split_position)
			{
				value_and_axis_ = split_axis & AXIS_MASK;
				real_or_uint32_.split_position = split_position;
			}

			Node(std::uint32_t index_count, std::uint32_t index_offset)
			{
				assert(index_count <= MAX_INDEX_COUNT && index_offset <= MAX_INDEX_OFFSET);
				value_and_axis_ = 3;
				value_and_axis_ |= std::uint32_t(index_count) << AXIS_BITS;
				real_or_uint32_.index_offset = std::uint32_t(index_offset);
			}

			bool is_leaf() const
			{
				return (value_and_axis_ & AXIS_MASK) > 2;
			}

			std::uint8_t split_axis() const
			{
				assert(!is_leaf());
				return value_and_axis_ & AXIS_MASK;
			}

			real_t split_position() const
			{
				assert(!is_leaf());
				return real_or_uint32_.split_position;
			}

			std::uint32_t index_count() const
			{
				assert(is_leaf());
				return value_and_axis_ >> AXIS_BITS;
			}

			std::uint32_t index_offset() const
			{
				assert(is_leaf());
				return real_or_uint32_.index_offset;
			}

			const Node *left() const
			{
				assert(!is_leaf());
				return this + 1;
			}

			const Node *right() const
			{
				assert(!is_leaf());
				std::uint32_t right_offset = (value_and_axis_ >> AXIS_BITS);
				return this + right_offset;
			}

			void set_right_offset(std::uint32_t right_offset)
			{
				assert(right_offset <= MAX_RIGHT_OFFSET);
				value_and_axis_ |= std::uint32_t(right_offset) << AXIS_BITS;
			}

		private:
			static constexpr std::uint32_t MAX_RIGHT_OFFSET = 0x3fffffff;
			static constexpr std::uint32_t MAX_INDEX_COUNT = 0x3fffffff;
			static constexpr std::uint32_t MAX_INDEX_OFFSET = 0xffffffff;
			static constexpr std::uint32_t AXIS_BITS = 2;
			static constexpr std::uint32_t AXIS_MASK = 0x03;

			std::uint32_t value_and_axis_;

			union
			{
				real_t split_position;
				std::uint32_t index_offset;
			} real_or_uint32_;
		};

		static_assert(sizeof(Node) == 4 + sizeof(real_t));

		struct IntersectionStackElement
		{
			const Node *node;
			real_t t_min;
			real_t t_max;
		};

		class KdTree : public Intersectable
		{
		public:
			KdTree(std::vector<const Intersectable *> &&intersectables,
			       std::vector<std::uint32_t> &&indices,
			       std::vector<Node> &&nodes,
			       const AABB &aabb) :
			        intersectables_(std::move(intersectables)),
			        indices_(std::move(indices)),
			        nodes_(std::move(nodes)),
			        aabb_(aabb)
			{
			}

			AABB aabb() const override
			{
				return aabb_;
			}

			bool intersect(const Ray &ray) const override
			{
				return intersect(ray, nullptr);
			}

			bool intersect(const Ray &ray, Intersection &intersection) const override
			{
				return intersect(ray, &intersection);
			}

		private:
			bool intersect(const Ray &ray, Intersection *intersection) const
			{
				real_t t_min;
				real_t t_max;

				if (!aabb_.intersects(ray, t_min, t_max))
					return false;

				IntersectionStackElement stack[ABSOLUTE_MAX_DEPTH];
				unsigned int stack_pos = 0;

				for (const Node *node = &nodes_[0]; node;) {
					if (!node->is_leaf()) {
						real_t o = ray.origin[node->split_axis()];
						real_t d = ray.direction[node->split_axis()];
						real_t t = (node->split_position() - o) / d;
						const Node *node_near;
						const Node *node_far;

						if (o < node->split_position() ||
						    (o == node->split_position() && d <= 0)) {
							node_near = node->left();
							node_far = node->right();
						} else {
							node_near = node->right();
							node_far = node->left();
						}

						if (t > t_max || t <= 0) {
							node = node_near;
						} else if (t < t_min) {
							node = node_far;
						} else {
							stack[stack_pos++] = {node_far, t, t_max};
							node = node_near;
							t_max = t;
						}
					} else {
						if (intersection) {
							if (intersect(ray, *intersection, *node))
								return true;
						} else {
							if (intersect(ray, *node))
								return true;
						}

						if (stack_pos > 0) {
							stack_pos--;
							node = stack[stack_pos].node;
							t_min = stack[stack_pos].t_min;
							t_max = stack[stack_pos].t_max;
						} else {
							node = nullptr;
						}
					}
				}

				return false;
			}

			bool intersect(const Ray &ray, const Node &node) const
			{
				std::uint32_t count = node.index_count();
				std::uint32_t offset = node.index_offset();

				if (count == 1)
					return intersectables_[offset]->intersect(ray);

				for (std::uint32_t i = offset; i < offset + count; i++)
					if (intersectables_[indices_[i]]->intersect(ray))
						return true;

				return false;
			}

			bool intersect(const Ray &ray, Intersection &intersection, const Node &node) const
			{
				std::uint32_t count = node.index_count();
				std::uint32_t offset = node.index_offset();

				if (count == 1)
					return intersectables_[offset]->intersect(ray, intersection);

				bool hit = false;

				for (unsigned int i = offset; i < offset + count; i++)
					if (intersectables_[indices_[i]]->intersect(ray, intersection))
						hit = true;

				return hit;
			}

			const std::vector<const Intersectable *> intersectables_;
			const std::vector<std::uint32_t> indices_;
			const std::vector<Node> nodes_;
			const AABB aabb_;
		};

		struct BuildEvent
		{
			enum class Type : std::uint8_t
			{
				END,
				PLANAR,
				START
			};

			BuildEvent(Type t, std::uint8_t a, real_t p, std::uint32_t i) :
			        type(t),
			        axis(a),
			        position(p),
			        index(i)
			{
			}

			bool operator<(const BuildEvent &r) const
			{
				if (position < r.position)
					return true;
				if (position > r.position)
					return false;
				if (axis < r.axis)
					return true;
				if (axis > r.axis)
					return false;
				return type < r.type;
			}

			Type type;
			std::uint8_t axis;
			real_t position;
			std::uint32_t index;
		};

		struct BuildInput
		{
			std::vector<std::uint32_t> indices;
			std::vector<BuildEvent> events;
			AABB aabb;
		};

		struct BuildSplit
		{
			BuildInput left;
			BuildInput right;
		};

		struct Plane
		{
			enum class Side : std::uint8_t
			{
				LEFT,
				RIGHT
			};

			Plane() = default;

			Plane(std::uint8_t a, real_t p) : axis(a), position(p)
			{
			}

			Side side_if_in_plane = Side::LEFT;
			std::uint8_t axis = 0;
			real_t position = 0;
		};

		enum class SideOfPlane : std::uint8_t
		{
			BOTH,
			LEFT_ONLY,
			RIGHT_ONLY
		};

		struct BuildNode
		{
			// = default can not be used due to union. Sometimes I wonder why I bother
			// with clang-tidy. Or well... C++ is a crappy language, so I know why.
			// But sometimes I want to strangle it. A crappy linter for a crappy
			// language, makes sense.
			BuildNode(){}; // NOLINT(modernize-use-equals-default)
			BuildNode(const BuildNode &) = delete;
			// Have no idea why move constructor can not be deleted. GCC spews out some
			// garbage with "result type must be constructible from value type of input
			// range" in it if it is. Should never move construct BuildNodes...
			BuildNode(BuildNode && /*other*/) noexcept // = delete; TODO: Why does this not work. *Sigh*.
			{
				assert(false);
			}

			~BuildNode()
			{
				if (split_axis >= 3)
					leaf.indices.~vector();
			}

			BuildNode &operator=(const BuildNode &) = delete;
			BuildNode &operator=(BuildNode &&) = delete;

			std::uint8_t split_axis = 0;

			union
			{
				struct
				{
					real_t position;
					const BuildNode *left;
					const BuildNode *right;
				} split;

				struct
				{
					std::vector<std::uint32_t> indices;
				} leaf;
			};
		};

		struct BuildNodeBlock
		{
			static constexpr unsigned int SIZE = 4096;
			unsigned int used = 0;
			BuildNode nodes[SIZE];
			std::unique_ptr<BuildNodeBlock> next;
		};

		struct alignas(RAYNI_L1_CACHE_LINE_SIZE) BuildThreadState
		{
			std::vector<SideOfPlane> sides_of_plane;

			BuildNodeBlock build_node_block_root;
			BuildNodeBlock *build_node_block_current = &build_node_block_root;
		};

		struct BuildContext
		{
			BuildContext(std::vector<const Intersectable *> &&is, const Cancellable &c, ThreadPool &tp) :
			        intersectables(std::move(is)),
			        cancellable(c),
			        thread_pool(tp)
			{
			}

			std::vector<const Intersectable *> intersectables;
			const Cancellable &cancellable;
			ThreadPool &thread_pool;

			static thread_local BuildThreadState *thread_state;
			CacheLineAlignedVector<BuildThreadState> thread_states;
		};

		thread_local BuildThreadState *BuildContext::thread_state = nullptr;

		void prepare_build_context(BuildContext &context)
		{
			unsigned int threads = context.thread_pool.threads_available();
			auto count = context.intersectables.size();
			Barrier barrier(threads);

			context.thread_states.resize(threads + 1);

			for (unsigned int t = 0; t < threads; t++) {
				context.thread_pool.add_task([&, count, t] {
					context.thread_state = &context.thread_states[t + 1];
					context.thread_state->sides_of_plane.resize(count);

					barrier.arrive_and_wait();
				});
			}

			context.thread_state = &context.thread_states[0];
			context.thread_state->sides_of_plane.resize(count);

			context.thread_pool.wait();
		}

		BuildNode *next_build_node(const BuildContext &context)
		{
			BuildNodeBlock *block = context.thread_state->build_node_block_current;

			if (block->used == BuildNodeBlock::SIZE) {
				block->next = std::make_unique<BuildNodeBlock>();
				block = block->next.get();
				context.thread_state->build_node_block_current = block;
			}

			return &block->nodes[block->used++];
		}

		std::uint32_t build_nodes_used(const BuildContext &context, std::uint32_t &indices_count)
		{
			std::uint32_t nodes_used = 0;
			indices_count = 0;

			for (const auto &state : context.thread_states) {
				const BuildNodeBlock *node_block = &state.build_node_block_root;

				while (node_block) {
					nodes_used += node_block->used;

					for (unsigned int i = 0; i < node_block->used; i++) {
						const BuildNode &node = node_block->nodes[i];

						if (node.split_axis >= 3 && node.leaf.indices.size() > 1)
							indices_count += node.leaf.indices.size();
					}

					node_block = node_block->next.get();
				}
			}

			return nodes_used;
		}

		real_t split_cost(real_t probability_left,
		                  real_t probability_right,
		                  std::uint32_t n_left,
		                  std::uint32_t n_right)
		{
			real_t cost = TRAVERSAL_COST +
			              INTERSECTION_COST * (probability_left * n_left + probability_right * n_right);

			if (n_left == 0 || n_right == 0)
				cost *= EMPTY_BONUS;

			return cost;
		}

		real_t surface_area_heuristic(Plane &plane,
		                              const AABB &aabb,
		                              real_t aabb_inv_sa,
		                              std::uint32_t n_left,
		                              std::uint32_t n_right,
		                              std::uint32_t n_plane)
		{
			auto aabb_split = aabb.split(plane.axis, plane.position);

			real_t probability_left = aabb_split.left.surface_area() * aabb_inv_sa;
			real_t probability_right = aabb_split.right.surface_area() * aabb_inv_sa;

			real_t cost_left = split_cost(probability_left, probability_right, n_left + n_plane, n_right);
			real_t cost_right = split_cost(probability_left, probability_right, n_left, n_right + n_plane);

			if (cost_left < cost_right)
				plane.side_if_in_plane = Plane::Side::LEFT;
			else
				plane.side_if_in_plane = Plane::Side::RIGHT;

			return std::min(cost_left, cost_right);
		}

		real_t find_plane(const BuildInput &input,
		                  Plane &plane_best,
		                  std::uint32_t &n_left_best,
		                  std::uint32_t &n_plane_best,
		                  std::uint32_t &n_right_best)
		{
			real_t cost_best = REAL_INFINITY;
			real_t aabb_inv_sa = 1 / input.aabb.surface_area();
			std::uint32_t n = input.indices.size();
			std::uint32_t n_left[3] = {0, 0, 0};
			std::uint32_t n_plane[3] = {0, 0, 0};
			std::uint32_t n_right[3] = {n, n, n};

			auto event = input.events.cbegin();

			auto num_events_in_plane = [&](const Plane &plane, BuildEvent::Type type) {
				std::uint32_t num = 0;

				for (; event != input.events.cend(); num++, event++)
					if (event->axis != plane.axis || event->position != plane.position ||
					    event->type != type)
						break;

				return num;
			};

			while (event != input.events.cend()) {
				Plane plane(event->axis, event->position);
				std::uint32_t p_end = num_events_in_plane(plane, BuildEvent::Type::END);
				std::uint32_t p_planar = num_events_in_plane(plane, BuildEvent::Type::PLANAR);
				std::uint32_t p_start = num_events_in_plane(plane, BuildEvent::Type::START);

				n_plane[plane.axis] = p_planar;
				n_right[plane.axis] -= p_planar + p_end;

				real_t cost = surface_area_heuristic(plane,
				                                     input.aabb,
				                                     aabb_inv_sa,
				                                     n_left[plane.axis],
				                                     n_right[plane.axis],
				                                     n_plane[plane.axis]);
				if (cost < cost_best) {
					cost_best = cost;
					plane_best = plane;
					n_left_best = n_left[plane.axis];
					n_plane_best = n_plane[plane.axis];
					n_right_best = n_right[plane.axis];
				}

				n_left[plane.axis] += p_start + p_planar;
				n_plane[plane.axis] = 0;
			}

			return cost_best;
		}

		const std::vector<SideOfPlane> &classify_intersectables(const BuildContext &context,
		                                                        const BuildInput &input,
		                                                        const Plane &plane)
		{
			auto &sides_of_plane = context.thread_state->sides_of_plane;

			for (std::uint32_t i : input.indices)
				sides_of_plane[i] = SideOfPlane::BOTH;

			for (const BuildEvent &e : input.events) {
				if (e.type == BuildEvent::Type::END && e.axis == plane.axis &&
				    e.position <= plane.position) {
					sides_of_plane[e.index] = SideOfPlane::LEFT_ONLY;
				} else if (e.type == BuildEvent::Type::START && e.axis == plane.axis &&
				           e.position >= plane.position) {
					sides_of_plane[e.index] = SideOfPlane::RIGHT_ONLY;
				} else if (e.type == BuildEvent::Type::PLANAR && e.axis == plane.axis) {
					if (e.position < plane.position ||
					    (e.position == plane.position &&
					     plane.side_if_in_plane == Plane::Side::LEFT)) {
						sides_of_plane[e.index] = SideOfPlane::LEFT_ONLY;
					} else if (e.position > plane.position ||
					           (e.position == plane.position &&
					            plane.side_if_in_plane == Plane::Side::RIGHT)) {
						sides_of_plane[e.index] = SideOfPlane::RIGHT_ONLY;
					}
				}
			}

			return sides_of_plane;
		}

		void generate_build_events(std::uint32_t index, const AABB &aabb, std::vector<BuildEvent> &events)
		{
			for (unsigned int axis = 0; axis < 3; axis++) {
				real_t min = aabb.minimum()[axis];
				real_t max = aabb.maximum()[axis];

				if (aabb.is_planar(axis)) {
					events.emplace_back(BuildEvent::Type::PLANAR, axis, min, index);
				} else {
					events.emplace_back(BuildEvent::Type::START, axis, min, index);
					events.emplace_back(BuildEvent::Type::END, axis, max, index);
				}
			}
		}

		BuildSplit split_build_input(const BuildContext &context,
		                             BuildInput &&input_arg,
		                             const Plane &plane,
		                             std::uint32_t n_left,
		                             std::uint32_t n_plane,
		                             std::uint32_t n_right)
		{
			// Move to stack of old input is important. Memory usage can get pretty steep if it
			// is not freed before recursion in create_build_node().
			const BuildInput input = std::move(input_arg);
			BuildSplit split;

			auto num_indices_left = n_left + (plane.side_if_in_plane == Plane::Side::LEFT ? n_plane : 0);
			auto num_indices_right = n_right + (plane.side_if_in_plane == Plane::Side::RIGHT ? n_plane : 0);
			auto max_num_events_left = std::uint64_t(num_indices_left) * MAX_EVENTS_PER_INTERSECTABLE;
			auto max_num_events_right = std::uint64_t(num_indices_right) * MAX_EVENTS_PER_INTERSECTABLE;
			split.left.indices.reserve(num_indices_left);
			split.left.events.reserve(max_num_events_left);
			split.right.indices.reserve(num_indices_right);
			split.right.events.reserve(max_num_events_right);

			AABB::Split aabb_split = input.aabb.split(plane.axis, plane.position);
			split.left.aabb = aabb_split.left;
			split.right.aabb = aabb_split.right;

			const auto &sides_of_plane = classify_intersectables(context, input, plane);

			for (const BuildEvent &e : input.events) {
				if (sides_of_plane[e.index] == SideOfPlane::LEFT_ONLY)
					split.left.events.push_back(e);
				else if (sides_of_plane[e.index] == SideOfPlane::RIGHT_ONLY)
					split.right.events.push_back(e);
			}

			auto left_events_sorted = std::distance(split.left.events.begin(), split.left.events.end());
			auto right_events_sorted = std::distance(split.right.events.begin(), split.right.events.end());

			for (std::uint32_t i : input.indices) {
				if (sides_of_plane[i] == SideOfPlane::BOTH) {
					AABB aabb = context.intersectables[i]->aabb();

					split.left.indices.push_back(i);
					generate_build_events(i, aabb.intersection(split.left.aabb), split.left.events);

					split.right.indices.push_back(i);
					generate_build_events(i,
					                      aabb.intersection(split.right.aabb),
					                      split.right.events);
				} else if (sides_of_plane[i] == SideOfPlane::LEFT_ONLY) {
					split.left.indices.push_back(i);
				} else if (sides_of_plane[i] == SideOfPlane::RIGHT_ONLY) {
					split.right.indices.push_back(i);
				}
			}

			auto left_events_middle = split.left.events.begin() + left_events_sorted;
			std::sort(left_events_middle, split.left.events.end());
			std::inplace_merge(split.left.events.begin(), left_events_middle, split.left.events.end());

			auto right_events_middle = split.right.events.begin() + right_events_sorted;
			std::sort(right_events_middle, split.right.events.end());
			std::inplace_merge(split.right.events.begin(), right_events_middle, split.right.events.end());

			assert(split.left.indices.size() == num_indices_left); // No reallocation should have occurred.
			assert(split.right.indices.size() == num_indices_right);
			assert(split.left.events.size() <= max_num_events_left);
			assert(split.right.events.size() <= max_num_events_right);

			return split;
		}

		const BuildNode *create_build_node(const BuildContext &context,
		                                   unsigned int max_depth,
		                                   BuildInput &&input)
		{
			BuildNode *node = next_build_node(context);
			std::uint32_t count = input.indices.size();
			Plane plane;
			std::uint32_t n_left = 0;
			std::uint32_t n_plane = 0;
			std::uint32_t n_right = 0;
			bool create_leaf;

			if (max_depth == 0 || count <= 1 || context.cancellable.cancelled()) {
				create_leaf = true;
			} else {
				real_t split_cost = find_plane(input, plane, n_left, n_plane, n_right);
				create_leaf = split_cost >= INTERSECTION_COST * count;
			}

			if (create_leaf) {
				node->split_axis = 3;
				new (&node->leaf.indices) std::vector(std::move(input.indices));
				return node;
			}

			BuildSplit split =
			        split_build_input(context, std::move(input), plane, n_left, n_plane, n_right);

			node->split_axis = plane.axis;
			node->split.position = plane.position;

			if (count > THREAD_MIN_INTERSECTABLES && context.thread_pool.threads_available() > 0) {
				auto future = context.thread_pool.async(
				        [&context, max_depth, right = std::move(split.right)]() mutable {
					        return create_build_node(context, max_depth - 1, std::move(right));
				        });
				node->split.left = create_build_node(context, max_depth - 1, std::move(split.left));
				node->split.right = future.get();
			} else {
				node->split.left = create_build_node(context, max_depth - 1, std::move(split.left));
				node->split.right = create_build_node(context, max_depth - 1, std::move(split.right));
			}

			return node;
		}

		BuildInput initial_build_input(const BuildContext &context)
		{
			assert(context.intersectables.size() <= std::numeric_limits<std::uint32_t>::max());

			BuildInput input;
			input.indices.resize(context.intersectables.size());
			input.events.reserve(context.intersectables.size() * MAX_EVENTS_PER_INTERSECTABLE);

			for (std::uint32_t index = 0; index < std::uint32_t(context.intersectables.size()); index++) {
				AABB aabb = context.intersectables[index]->aabb();

				input.indices[index] = index;
				generate_build_events(index, aabb, input.events);
				input.aabb.merge(aabb);
			}

			assert(input.events.size() <= context.intersectables.size() * MAX_EVENTS_PER_INTERSECTABLE);

			std::sort(input.events.begin(), input.events.end());

			return input;
		}

		unsigned int max_depth_limit(std::uint32_t num_intersectables)
		{
			static constexpr real_t MAX_DEPTH_K1 = 1.3;
			static constexpr real_t MAX_DEPTH_K2 = 8.0;
			auto depth = static_cast<unsigned int>(MAX_DEPTH_K1 * std::log2(real_t(num_intersectables)) +
			                                       MAX_DEPTH_K2 + real_t(0.5));

			return std::min(depth, ABSOLUTE_MAX_DEPTH);
		}

		void build_node_to_nodes(std::vector<std::uint32_t> &indices,
		                         std::vector<Node> &nodes,
		                         const BuildNode *build_node)
		{
			if (build_node->split_axis < 3) {
				std::uint32_t pos = nodes.size();
				nodes.emplace_back(build_node->split_axis, build_node->split.position);

				build_node_to_nodes(indices, nodes, build_node->split.left);

				nodes[pos].set_right_offset(nodes.size() - pos);

				build_node_to_nodes(indices, nodes, build_node->split.right);
			} else {
				if (build_node->leaf.indices.size() == 1) {
					nodes.emplace_back(1, build_node->leaf.indices[0]);
				} else {
					nodes.emplace_back(std::uint32_t(build_node->leaf.indices.size()),
					                   std::uint32_t(indices.size()));
					for (std::uint32_t i : build_node->leaf.indices)
						indices.emplace_back(i);
				}
			}
		}

		void log_build_info(const Stopwatch &stopwatch,
		                    const std::vector<const Intersectable *> &intersectables,
		                    const std::vector<Node> &nodes,
		                    const std::vector<std::uint32_t> &indices,
		                    const AABB &aabb)
		{
			struct PositionInfo
			{
				const Node *node;
				unsigned int depth;
			};

			struct SavedInfo
			{
				Stopwatch::clock::duration total_time = {};
				unsigned int total_count = 0;
			};

			unsigned int min_depth = std::numeric_limits<unsigned int>::max();
			unsigned int max_depth = 0;
			unsigned int leafs = 0;
			unsigned int leaf_index_count[10] = {};
			unsigned int max_index_count = 0;
			PositionInfo current = {&nodes[0], 0};
			PositionInfo stack[ABSOLUTE_MAX_DEPTH];
			unsigned int stack_pos = 0;

			while (current.node) {
				if (current.node->is_leaf()) {
					min_depth = std::min(min_depth, current.depth);
					max_depth = std::max(max_depth, current.depth);
					leafs++;

					auto index_count = current.node->index_count();
					leaf_index_count[std::min(index_count, 9U)]++;
					max_index_count = std::max(max_index_count, current.node->index_count());

					current = stack_pos > 0 ? stack[--stack_pos] : PositionInfo{nullptr, 0};
				} else {
					current.depth++;
					assert(stack_pos < ABSOLUTE_MAX_DEPTH);
					stack[stack_pos++] = {current.node->right(), current.depth};
					current.node = current.node->left();
				}
			}

			static std::unordered_map<std::size_t, SavedInfo> saved_info;
			std::size_t saved_hash = hash_combine_for(intersectables.size(),
			                                          aabb.minimum().x(),
			                                          aabb.minimum().y(),
			                                          aabb.minimum().z(),
			                                          aabb.maximum().x(),
			                                          aabb.maximum().y(),
			                                          aabb.maximum().z());
			SavedInfo &si = saved_info[saved_hash];
			si.total_time += stopwatch.duration();
			si.total_count++;

			double intersectables_mb = double(intersectables.size() * sizeof(void *)) / (1024 * 1024);
			double nodes_mb = double(nodes.size() * sizeof(Node)) / (1024 * 1024);
			double indices_mb = double(indices.size() * sizeof(std::uint32_t)) / (1024 * 1024);

			log_info("KdTree build information:\n"
			         "  Time to build        : %s\n"
			         "  Average time to build: %s (builds: %u)\n"
			         "  Intersectables       : %zu (%.2fMb)\n"
			         "  Nodes                : %zu (%.2fMb)\n"
			         "  Indices              : %zu (%.2fMb)\n"
			         "  Memory usage         : %.2fMb\n"
			         "  Min depth            : %u\n"
			         "  Max depth (limit)    : %u (%u)\n"
			         "  Leafs (ceil log2)    : %u (%d)\n"
			         "  Leafs with  0 indices: %u\n"
			         "  Leafs with  1 indices: %u\n"
			         "  Leafs with  2 indices: %u\n"
			         "  Leafs with  3 indices: %u\n"
			         "  Leafs with  4 indices: %u\n"
			         "  Leafs with  5 indices: %u\n"
			         "  Leafs with  6 indices: %u\n"
			         "  Leafs with  7 indices: %u\n"
			         "  Leafs with  8 indices: %u\n"
			         "  Leafs with >8 indices: %u\n"
			         "  Max indices in leaf  : %u\n"
			         "  AABB minimum         : (%f, %f, %f)\n"
			         "  AABB maximum         : (%f, %f, %f)",
			         stopwatch.string().c_str(),
			         duration_format(si.total_time / si.total_count, {.seconds_precision = 3}).c_str(),
			         si.total_count,
			         intersectables.size(),
			         intersectables_mb,
			         nodes.size(),
			         nodes_mb,
			         indices.size(),
			         indices_mb,
			         intersectables_mb + nodes_mb + indices_mb,
			         min_depth,
			         max_depth,
			         max_depth_limit(intersectables.size()),
			         leafs,
			         int(std::ceil(std::log2(leafs))),
			         leaf_index_count[0],
			         leaf_index_count[1],
			         leaf_index_count[2],
			         leaf_index_count[3],
			         leaf_index_count[4],
			         leaf_index_count[5],
			         leaf_index_count[6],
			         leaf_index_count[7],
			         leaf_index_count[8],
			         leaf_index_count[9],
			         max_index_count,
			         double(aabb.minimum().x()),
			         double(aabb.minimum().y()),
			         double(aabb.minimum().z()),
			         double(aabb.maximum().x()),
			         double(aabb.maximum().y()),
			         double(aabb.maximum().z()));
		}
	}

	std::unique_ptr<Intersectable> kdtree_build(std::vector<const Intersectable *> &&intersectables,
	                                            const Cancellable &cancellable,
	                                            ThreadPool &thread_pool)
	{
		auto stopwatch = Stopwatch().start();

		BuildContext context(std::move(intersectables), cancellable, thread_pool);
		prepare_build_context(context);

		BuildInput input = initial_build_input(context);
		AABB aabb = input.aabb;
		unsigned int max_depth = max_depth_limit(context.intersectables.size());
		const BuildNode *root = create_build_node(context, max_depth, std::move(input));

		std::uint32_t indices_count;
		std::uint32_t node_count = build_nodes_used(context, indices_count);
		std::vector<std::uint32_t> indices;
		std::vector<Node> nodes;
		indices.reserve(indices_count);
		nodes.reserve(node_count);

		build_node_to_nodes(indices, nodes, root);

		assert(indices_count == indices.size()); // No reallocation should have occurred.
		assert(node_count == nodes.size());

		stopwatch.stop();

		if (!cancellable.cancelled())
			log_build_info(stopwatch, context.intersectables, nodes, indices, aabb);

		return std::make_unique<KdTree>(std::move(context.intersectables),
		                                std::move(indices),
		                                std::move(nodes),
		                                aabb);
	}
}
