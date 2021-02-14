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

#include "lib/intersection_structures/bvh.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "config.h"
#include "lib/concurrency/barrier.h"
#include "lib/containers/cache_line_aligned_vector.h"
#include "lib/intersection.h"
#include "lib/intersection_structures/bvh.h"
#include "lib/log.h"
#include "lib/math/aabb.h"
#include "lib/math/hash.h"
#include "lib/math/math.h"
#include "lib/math/ray.h"
#include "lib/math/vector3.h"
#include "lib/stopwatch.h"
#include "lib/string/duration_format.h"

// For algorithm used to build BVH, see:
//
// Wald, I., 2007, On fast Construction of SAH-based Bounding Volume Hierarchies
// http://www.sci.utah.edu/~wald/Publications/2007/ParallelBVHBuild/fastbuild.pdf
//
// An effort has been made to be cache friendly when building the BVH and when
// intersecting. Threads could be used better when building. See TODO below.

// TODO: Make it so same thread operates on same range of info data when
//       threading horizontally.
//
// And so that a thread used for horizontal threading can not be used to do
// vertical threading at the same time. Should reduce cache misses when
// threading horizontally and perhaps also help with TODO for
// RAYNI_BVH_MULTITHREAD_INTERSECTABLE_INFO_PARTITIONING (copying for
// partitioning will go faster).
//
// Add separate work queues for each thread to ThreadPool and make it possible
// to schedule a job to a thread with a specific index in the pool. Work in the
// thread specific queue should have higher priority than the global queue.
//
// To prevent vertical work to be interleaved with horizontal work, there needs
// to be a way to say "only look for work in thread specific queue for thread
// with indices X-Y" or something. Start by not caring about this and see how it
// goes (there should be no mixing at the top of the tree). Also wait with
// determining final solution here until more testing can be done on a CPU with
// more than 4 physical cores (which is what I have available at the moment).

// TODO: Remove RAYNI_BVH_MULTITHREAD_INTERSECTABLE_INFO_PARTITIONING.
//
// At the moment, threading partitioning of IntersectableInfo is slower than not
// doing so. At least on system I am testing on. IntersectableInfo is 40 bytes.
// Making IntersectableInfo smaller by removing pre-calculated centroid (28
// bytes instead of 40) leads to worse performance overall (calculation overhead
// and does not help enough with memory performance). Using and partitioning 4
// byte indices instead (constant IntersectableInfo memory area, and 1 extra
// level of indirection with Intersectable indices) also does not improve
// performance overall. Should retest all of this on a newer system (that does
// not have an >8 year old CPU etc). Then decide what to do and remove define.
#define RAYNI_BVH_MULTITHREAD_INTERSECTABLE_INFO_PARTITIONING 0

// TODO: "Main" thread is currently used to do work as well.
//
// This means that when threading vertically (not a problem with horizontal
// threading since "threads" variable and fact that last chunk is done on
// calling thread makes # threads working correct), there can be unnecessary
// context switching and inefficient use of cache.
//
// On CPU I have been testing on (a very old one) and with current
// implementation, there is not much of a performance degradation (sometimes it
// is even a bit faster). Should reevaluate this when testing on a never CPU
// with more cores and faster memory etc. Other TODO:s above also probably
// matters here.

namespace Rayni
{
	namespace
	{
		constexpr unsigned int ABSOLUTE_MAX_DEPTH = 64;

		constexpr unsigned int NUM_BUCKETS = 16;
		constexpr unsigned int MAX_LEAF_INTERSECTABLES = 4;

		constexpr unsigned int THREAD_HORIZONTALLY_MIN_THREADS = 2;
		constexpr unsigned int THREAD_MIN_INTERSECTABLES = 10000;

		class Node
		{
		public:
			Node(const AABB &aabb, std::uint32_t intersectable_offset, std::uint8_t intersectable_count) :
			        aabb_(aabb),
			        offset_(intersectable_offset),
			        intersectable_count_(intersectable_count)
			{
			}

			Node(const AABB &aabb, std::uint8_t axis) : aabb_(aabb), axis_(axis)
			{
			}

			bool is_leaf() const
			{
				return intersectable_count_ > 0;
			}

			const Node *left() const
			{
				assert(!is_leaf());
				return this + 1;
			}

			const Node *right() const
			{
				assert(!is_leaf());
				return this + offset_;
			}

			const AABB &aabb() const
			{
				return aabb_;
			}

			std::uint32_t intersectable_offset() const
			{
				assert(is_leaf());
				return offset_;
			}

			std::uint8_t intersectable_count() const
			{
				assert(is_leaf());
				return intersectable_count_;
			}

			std::uint8_t axis() const
			{
				assert(!is_leaf());
				return axis_;
			}

			void set_right_offset(std::uint32_t right_offset)
			{
				assert(!is_leaf());
				offset_ = right_offset;
			}

		private:
			AABB aabb_;
			std::uint32_t offset_ = 0;
			std::uint8_t intersectable_count_ = 0;
			std::uint8_t axis_ = 0;
		};

		static_assert(sizeof(Node) <= sizeof(AABB) + 8);

		class BVH : public Intersectable
		{
		public:
			BVH(std::vector<const Intersectable *> &&intersectables, std::vector<Node> &&nodes) :
			        intersectables_(std::move(intersectables)),
			        nodes_(std::move(nodes))
			{
			}

			AABB aabb() const override
			{
				return nodes_[0].aabb();
			}

			bool intersect(const Ray &ray) const override
			{
				Vector3 inv_dir(1 / ray.direction.x(), 1 / ray.direction.y(), 1 / ray.direction.z());
				const Node *node = &nodes_[0];
				const Node *stack[ABSOLUTE_MAX_DEPTH];
				unsigned int stack_pos = 0;

				while (node) {
					if (node->aabb().intersects(ray, inv_dir)) {
						if (node->is_leaf()) {
							for (unsigned int i = 0; i < node->intersectable_count(); i++) {
								std::uint32_t o = node->intersectable_offset() + i;
								if (intersectables_[o]->intersect(ray))
									return true;
							}
							node = stack_pos > 0 ? stack[--stack_pos] : nullptr;
						} else if (inv_dir[node->axis()] < 0) {
							stack[stack_pos++] = node->left();
							node = node->right();
						} else {
							stack[stack_pos++] = node->right();
							node = node->left();
						}
					} else {
						node = stack_pos > 0 ? stack[--stack_pos] : nullptr;
					}
				}

				return false;
			}

			bool intersect(const Ray &ray, Intersection &intersection) const override
			{
				Vector3 inv_dir(1 / ray.direction.x(), 1 / ray.direction.y(), 1 / ray.direction.z());
				const Node *node = &nodes_[0];
				const Node *stack[ABSOLUTE_MAX_DEPTH];
				unsigned int stack_pos = 0;
				bool hit = false;

				while (node) {
					if (node->aabb().intersects(ray, inv_dir, intersection.t)) {
						if (node->is_leaf()) {
							for (unsigned int i = 0; i < node->intersectable_count(); i++) {
								std::uint32_t o = node->intersectable_offset() + i;
								if (intersectables_[o]->intersect(ray, intersection))
									hit = true;
							}
							node = stack_pos > 0 ? stack[--stack_pos] : nullptr;
						} else if (inv_dir[node->axis()] < 0) {
							stack[stack_pos++] = node->left();
							node = node->right();
						} else {
							stack[stack_pos++] = node->right();
							node = node->left();
						}
					} else {
						node = stack_pos > 0 ? stack[--stack_pos] : nullptr;
					}
				}

				return hit;
			}

		private:
			const std::vector<const Intersectable *> intersectables_;
			const std::vector<Node> nodes_;
		};

		struct IntersectableInfo
		{
			std::uint32_t index = 0;
			AABB aabb{{0, 0, 0}, {0, 0, 0}};
			Vector3 centroid;
		};

		struct BuildNode
		{
			AABB aabb{{0, 0, 0}, {0, 0, 0}};
			std::uint8_t split_axis = 0;

			union
			{
				struct
				{
					const BuildNode *left;
					const BuildNode *right;
				} split;

				struct
				{
					std::uint32_t start;
					std::uint32_t end;
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

			std::vector<IntersectableInfo> infos;
#if RAYNI_BVH_MULTITHREAD_INTERSECTABLE_INFO_PARTITIONING
			std::vector<IntersectableInfo> infos_copy;
#endif
		};

		thread_local BuildThreadState *BuildContext::thread_state = nullptr;

		struct Bucket
		{
			std::uint32_t count = 0;
			AABB aabb;
		};

		struct BucketSplit
		{
			real_t cost;
			unsigned int bucket;
		};

		struct alignas(RAYNI_L1_CACHE_LINE_SIZE) HorizontalChunkState
		{
			AABB aabb;
			AABB centroids_aabb;
			Bucket buckets[NUM_BUCKETS];

#if RAYNI_BVH_MULTITHREAD_INTERSECTABLE_INFO_PARTITIONING
			std::uint32_t partition_left_start = 0;
			std::uint32_t partition_right_start = 0;
#endif
		};

		void prepare_build_context(BuildContext &context, unsigned int threads)
		{
			std::uint32_t count = context.intersectables.size();
			Barrier barrier(threads);

			context.thread_states.resize(threads + 1);
			context.thread_state = &context.thread_states[0];
			context.infos.resize(count);

			for (unsigned int t = 0; t < threads; t++) {
				context.thread_pool.add_task([&, count, t] {
					context.thread_state = &context.thread_states[t + 1];

					std::uint32_t start = (std::uint64_t(t) * count) / threads;
					std::uint32_t end = (std::uint64_t(t + 1) * count) / threads;

					for (std::uint32_t i = start; i < end; i++) {
						context.infos[i].index = i;
						context.infos[i].aabb = context.intersectables[i]->aabb();
						context.infos[i].centroid = context.infos[i].aabb.centroid();
					}

					barrier.arrive_and_wait();
				});
			}

#if RAYNI_BVH_MULTITHREAD_INTERSECTABLE_INFO_PARTITIONING
			context.infos_copy.resize(count);
#endif
			context.thread_pool.wait();
		}

		BuildNode *next_build_node(BuildContext &context)
		{
			BuildNodeBlock *block = context.thread_state->build_node_block_current;

			if (block->used == BuildNodeBlock::SIZE) {
				block->next = std::make_unique<BuildNodeBlock>();
				block = block->next.get();
				context.thread_state->build_node_block_current = block;
			}

			return &block->nodes[block->used++];
		}

		unsigned int build_nodes_used(const BuildContext &context)
		{
			unsigned int nodes_used = 0;

			for (const auto &state : context.thread_states) {
				const BuildNodeBlock *node_block = &state.build_node_block_root;
				while (node_block) {
					nodes_used += node_block->used;
					node_block = node_block->next.get();
				}
			}

			return nodes_used;
		}

		unsigned int bucket_index(const Vector3 &centroid, const AABB &centroids_aabb, std::uint8_t split_axis)
		{
			real_t pos = centroid[split_axis];
			real_t min = centroids_aabb.minimum()[split_axis];
			real_t max = centroids_aabb.maximum()[split_axis];
			auto b = unsigned(NUM_BUCKETS * (pos - min) / (max - min));
			if (b >= NUM_BUCKETS)
				b = NUM_BUCKETS - 1;
			return b;
		}

		BucketSplit bucket_split(const Bucket *buckets, const AABB &aabb)
		{
			real_t split_cost = REAL_INFINITY;
			unsigned int split_bucket = 0;

			for (unsigned int i = 0; i < NUM_BUCKETS - 1; i++) {
				AABB left_aabb;
				AABB right_aabb;
				std::uint32_t left_count = 0;
				std::uint32_t right_count = 0;

				for (unsigned int j = 0; j <= i; j++) {
					left_aabb.merge(buckets[j].aabb);
					left_count += buckets[j].count;
				}

				for (unsigned int j = i + 1; j < NUM_BUCKETS; j++) {
					right_aabb.merge(buckets[j].aabb);
					right_count += buckets[j].count;
				}

				real_t cost = 1 + (left_count * left_aabb.surface_area() +
				                   right_count * right_aabb.surface_area()) /
				                          aabb.surface_area();

				if (cost < split_cost) {
					split_cost = cost;
					split_bucket = i;
				}
			}

			return {split_cost, split_bucket};
		}

		const BuildNode *create_build_node(BuildContext &context,
		                                   std::uint32_t start,
		                                   std::uint32_t end,
		                                   unsigned int threads);

		const BuildNode *create_build_node_thread_horizontally(BuildContext &context,
		                                                       std::uint32_t start,
		                                                       std::uint32_t end,
		                                                       unsigned int threads)
		{
			assert(start < end);
			assert(threads >= 2);

			BuildNode *node = next_build_node(context);
			Barrier barrier(threads);
			std::uint32_t count = end - start;

			// TODO: This is bad. Would like same thread to operate on specific data range
			//       for each call to minimize cache misses. See TODO at top of this file.
			auto parallel_for = [&](auto &&func) {
				for (unsigned int t = 0; t < threads; t++) {
					std::uint32_t start_t = start + (std::uint64_t(t) * count) / threads;
					std::uint32_t end_t = start + (std::uint64_t(t + 1) * count) / threads;

					if (t < threads - 1) {
						context.thread_pool.add_task([&, t, start_t, end_t] {
							func(t, start_t, end_t);
							barrier.arrive();
						});
					} else {
						func(t, start_t, end_t);
						barrier.arrive_and_wait();
					}
				}
			};

			CacheLineAlignedVector<HorizontalChunkState> chunk_states(threads);

			parallel_for([&](unsigned int t, std::uint32_t start_t, std::uint32_t end_t) {
				AABB aabb;
				AABB centroids_aabb;

				for (std::uint32_t i = start_t; i < end_t; i++) {
					aabb.merge(context.infos[i].aabb);
					centroids_aabb.merge(context.infos[i].centroid);
				}

				chunk_states[t].aabb = aabb;
				chunk_states[t].centroids_aabb = centroids_aabb;
			});

			AABB aabb;
			AABB centroids_aabb;

			for (unsigned int t = 0; t < threads; t++) {
				aabb.merge(chunk_states[t].aabb);
				centroids_aabb.merge(chunk_states[t].centroids_aabb);
			}

			std::uint8_t split_axis = centroids_aabb.max_extent_axis();

			if (centroids_aabb.is_planar(split_axis) || context.cancellable.cancelled()) {
				node->aabb = aabb;
				node->split_axis = 3;
				node->leaf.start = start;
				node->leaf.end = end;
				return node;
			}

			parallel_for([&](unsigned int t, std::uint32_t start_t, std::uint32_t end_t) {
				Bucket *buckets = chunk_states[t].buckets;

				for (std::uint32_t i = start_t; i < end_t; i++) {
					auto b = bucket_index(context.infos[i].centroid, centroids_aabb, split_axis);
					buckets[b].count++;
					buckets[b].aabb.merge(context.infos[i].aabb);
				}
			});

			Bucket buckets[NUM_BUCKETS];

			for (unsigned int t = 0; t < threads; t++) {
				for (unsigned int i = 0; i < NUM_BUCKETS; i++) {
					buckets[i].count += chunk_states[t].buckets[i].count;
					buckets[i].aabb.merge(chunk_states[t].buckets[i].aabb);
				}
			}

			BucketSplit split = bucket_split(buckets, aabb);

#if RAYNI_BVH_MULTITHREAD_INTERSECTABLE_INFO_PARTITIONING
			std::uint32_t partition_start = start;

			for (unsigned int t = 0; t < threads; t++) {
				chunk_states[t].partition_left_start = partition_start;

				for (unsigned int i = 0; i <= split.bucket; i++)
					partition_start += chunk_states[t].buckets[i].count;
			}

			for (unsigned int t = 0; t < threads; t++) {
				chunk_states[t].partition_right_start = partition_start;

				for (unsigned int i = split.bucket + 1; i < NUM_BUCKETS; i++)
					partition_start += chunk_states[t].buckets[i].count;
			}

			parallel_for([&](unsigned int /*t*/, std::uint32_t start_t, std::uint32_t end_t) {
				std::copy(&context.infos[start_t], &context.infos[end_t], &context.infos_copy[start_t]);
			});

			parallel_for([&](unsigned int t, std::uint32_t start_t, std::uint32_t end_t) {
				std::uint32_t left_i = chunk_states[t].partition_left_start;
				std::uint32_t right_i = chunk_states[t].partition_right_start;

				for (std::uint32_t i = start_t; i < end_t; i++) {
					const auto &info = context.infos_copy[i];
					unsigned int b = bucket_index(info.centroid, centroids_aabb, split_axis);

					if (b <= split.bucket)
						context.infos[left_i++] = info;
					else
						context.infos[right_i++] = info;
				}
			});

			std::uint32_t mid = chunk_states[0].partition_right_start;
#else
			IntersectableInfo *pmid =
			        std::partition(&context.infos[start],
			                       &context.infos[end],
			                       [&](const IntersectableInfo &i) {
				                       unsigned int b =
				                               bucket_index(i.centroid, centroids_aabb, split_axis);
				                       return b <= split.bucket;
			                       });
			std::uint32_t mid = pmid - &context.infos[0];
#endif
			unsigned threads_left =
			        std::clamp(unsigned((std::uint64_t(mid - start) * threads) / count), 1U, threads - 1);
			unsigned int threads_right = threads - threads_left;

			auto future = context.thread_pool.async(
			        [&]() { return create_build_node(context, mid, end, threads_right); });
			const BuildNode *left = create_build_node(context, start, mid, threads_left);
			const BuildNode *right = future.get();

			node->aabb = AABB(left->aabb).merge(right->aabb);
			node->split_axis = split_axis;
			node->split.left = left;
			node->split.right = right;

			return node;
		}

		const BuildNode *create_build_node(BuildContext &context, std::uint32_t start, std::uint32_t end)
		{
			assert(start < end);

			BuildNode *node = next_build_node(context);
			std::uint32_t count = end - start;
			AABB aabb;
			AABB centroids_aabb;

			for (std::uint32_t i = start; i < end; i++) {
				aabb.merge(context.infos[i].aabb);
				centroids_aabb.merge(context.infos[i].centroid);
			}

			std::uint8_t split_axis = centroids_aabb.max_extent_axis();

			if (count == 1 || centroids_aabb.is_planar(split_axis) || context.cancellable.cancelled()) {
				node->aabb = aabb;
				node->split_axis = 3;
				node->leaf.start = start;
				node->leaf.end = end;
				return node;
			}

			std::uint32_t mid;

			if (count <= 2) {
				mid = (start + end) / 2;

				if (context.infos[mid].centroid[split_axis] < context.infos[start].centroid[split_axis])
					std::swap(context.infos[start], context.infos[mid]);
			} else {
				Bucket buckets[NUM_BUCKETS];

				for (std::uint32_t i = start; i < end; i++) {
					const auto &info = context.infos[i];
					unsigned int b = bucket_index(info.centroid, centroids_aabb, split_axis);
					buckets[b].count++;
					buckets[b].aabb.merge(info.aabb);
				}

				auto split = bucket_split(buckets, aabb);

				real_t leaf_cost = count;
				if (count <= MAX_LEAF_INTERSECTABLES && split.cost >= leaf_cost) {
					node->aabb = aabb;
					node->split_axis = 3;
					node->leaf.start = start;
					node->leaf.end = end;
					return node;
				}

				IntersectableInfo *pmid = std::partition(&context.infos[start],
				                                         &context.infos[end],
				                                         [&](const IntersectableInfo &i) {
					                                         unsigned int b =
					                                                 bucket_index(i.centroid,
					                                                              centroids_aabb,
					                                                              split_axis);
					                                         return b <= split.bucket;
				                                         });
				mid = pmid - &context.infos[0];
			}

			const BuildNode *left;
			const BuildNode *right;

			if (count > THREAD_MIN_INTERSECTABLES && context.thread_pool.threads_available() > 0) {
				auto future = context.thread_pool.async(
				        [&context, mid, end]() { return create_build_node(context, mid, end); });
				left = create_build_node(context, start, mid);
				right = future.get();
			} else {
				left = create_build_node(context, start, mid);
				right = create_build_node(context, mid, end);
			}

			node->aabb = AABB(left->aabb).merge(right->aabb);
			node->split_axis = split_axis;
			node->split.left = left;
			node->split.right = right;

			return node;
		}

		const BuildNode *create_build_node(BuildContext &context,
		                                   std::uint32_t start,
		                                   std::uint32_t end,
		                                   unsigned int threads)
		{
			if (threads >= THREAD_HORIZONTALLY_MIN_THREADS && end - start > THREAD_MIN_INTERSECTABLES)
				return create_build_node_thread_horizontally(context, start, end, threads);

			return create_build_node(context, start, end);
		}

		void build_node_to_nodes(const BuildContext &context,
		                         std::vector<const Intersectable *> &ordered_intersectables,
		                         std::vector<Node> &nodes,
		                         const BuildNode *build_node)
		{
			if (build_node->split_axis < 3) {
				auto pos = nodes.size();
				nodes.emplace_back(build_node->aabb, build_node->split_axis);

				build_node_to_nodes(context, ordered_intersectables, nodes, build_node->split.left);
				nodes[pos].set_right_offset(nodes.size() - pos);
				build_node_to_nodes(context, ordered_intersectables, nodes, build_node->split.right);
			} else {
				std::uint32_t offset = ordered_intersectables.size();

				for (std::uint32_t i = build_node->leaf.start; i < build_node->leaf.end; i++)
					ordered_intersectables.push_back(
					        context.intersectables[context.infos[i].index]);

				nodes.emplace_back(build_node->aabb,
				                   offset,
				                   build_node->leaf.end - build_node->leaf.start);
			}
		}

		void log_build_info(const Stopwatch &stopwatch,
		                    const std::vector<const Intersectable *> &intersectables,
		                    const std::vector<const Intersectable *> &ordered_intersectables,
		                    const std::vector<Node> &nodes)
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
			unsigned int leaf_intersectables_count[5] = {};

			PositionInfo current = {&nodes[0], 0};
			PositionInfo stack[ABSOLUTE_MAX_DEPTH];
			unsigned int stack_pos = 0;

			while (current.node) {
				if (current.node->is_leaf()) {
					min_depth = std::min(min_depth, current.depth);
					max_depth = std::max(max_depth, current.depth);
					leafs++;
					int count_index = std::min(int(current.node->intersectable_count()), 5) - 1;
					leaf_intersectables_count[count_index]++;

					current = stack_pos > 0 ? stack[--stack_pos] : PositionInfo{nullptr, 0};
				} else {
					current.depth++;
					assert(stack_pos < ABSOLUTE_MAX_DEPTH);
					stack[stack_pos++] = {current.node->right(), current.depth};
					current.node = current.node->left();
				}
			}

			static std::unordered_map<std::size_t, SavedInfo> saved_info;
			const AABB &aabb = nodes[0].aabb();
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

			double ordered_intersectables_mb =
			        double(ordered_intersectables.size() * sizeof(void *)) / (1024 * 1024);
			double nodes_mb = double(nodes.size() * sizeof(Node)) / (1024 * 1024);

			log_info("BVH build information:\n"
			         "  Time to build               : %s\n"
			         "  Average time to build       : %s (builds: %u)\n"
			         "  Intersectables              : %zu\n"
			         "  Ordered intersectables      : %zu (%.2fMb)\n"
			         "  Nodes                       : %zu (%.2fMb)\n"
			         "  Memory usage                : %.2fMb\n"
			         "  Min depth                   : %u\n"
			         "  Max depth                   : %u\n"
			         "  Leafs (ceil log2)           : %u (%d)\n"
			         "  Leafs with  1 intersectables: %u\n"
			         "  Leafs with  2 intersectables: %u\n"
			         "  Leafs with  3 intersectables: %u\n"
			         "  Leafs with  4 intersectables: %u\n"
			         "  Leafs with >4 intersectables: %u\n"
			         "  AABB minimum                : (%f, %f, %f)\n"
			         "  AABB maximum                : (%f, %f, %f)",
			         stopwatch.string().c_str(),
			         duration_format(si.total_time / si.total_count, {.seconds_precision = 3}).c_str(),
			         si.total_count,
			         intersectables.size(),
			         ordered_intersectables.size(),
			         ordered_intersectables_mb,
			         nodes.size(),
			         nodes_mb,
			         ordered_intersectables_mb + nodes_mb,
			         min_depth,
			         max_depth,
			         leafs,
			         int(std::ceil(std::log2(leafs))),
			         leaf_intersectables_count[0],
			         leaf_intersectables_count[1],
			         leaf_intersectables_count[2],
			         leaf_intersectables_count[3],
			         leaf_intersectables_count[4],
			         double(aabb.minimum().x()),
			         double(aabb.minimum().y()),
			         double(aabb.minimum().z()),
			         double(aabb.maximum().x()),
			         double(aabb.maximum().y()),
			         double(aabb.maximum().z()));
		}
	}

	std::unique_ptr<Intersectable> bvh_build(std::vector<const Intersectable *> &&intersectables,
	                                         const Cancellable &cancellable,
	                                         ThreadPool &thread_pool)
	{
		auto stopwatch = Stopwatch().start();

		BuildContext context(std::move(intersectables), cancellable, thread_pool);
		unsigned int num_threads = thread_pool.threads_available();

		prepare_build_context(context, num_threads);

		std::uint32_t num_intersectables = context.intersectables.size();
		const BuildNode *root = create_build_node(context, 0, num_intersectables, num_threads);

		unsigned int node_count = build_nodes_used(context);
		std::vector<const Intersectable *> ordered_intersectables;
		std::vector<Node> nodes;
		ordered_intersectables.reserve(num_intersectables);
		nodes.reserve(node_count);

		build_node_to_nodes(context, ordered_intersectables, nodes, root);

		assert(num_intersectables == ordered_intersectables.size()); // No reallocation should have occurred.
		assert(node_count == nodes.size());

		stopwatch.stop();

		if (!cancellable.cancelled())
			log_build_info(stopwatch, context.intersectables, ordered_intersectables, nodes);

		return std::make_unique<BVH>(std::move(ordered_intersectables), std::move(nodes));
	}
}
