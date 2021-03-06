/** Memory allocators.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014-2020, Institute for Automation of Complex Power Systems, EONERC
 * @license GNU General Public License (version 3)
 *
 * VILLASnode
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************************/

#pragma once

#include <cstddef>
#include <cstdint>

/* Forward declarations */
struct memory_type;
struct vnode;

typedef struct memory_allocation * (*memory_allocator_t)(size_t len, size_t alignment, struct memory_type *mem);
typedef int (*memory_deallocator_t)(struct memory_allocation * ma, struct memory_type *mem);

enum class MemoryFlags {
	MMAP		= (1 << 0),
	DMA		= (1 << 1),
	HUGEPAGE	= (1 << 2),
	HEAP		= (1 << 3)
};

struct memory_type {
	const char *name;
	int flags;

	size_t alignment;

	memory_allocator_t alloc;
	memory_deallocator_t free;

	void *_vd; /**< Virtual data for internal state */
};

extern struct memory_type memory_heap;
extern struct memory_type memory_mmap;
extern struct memory_type memory_mmap_hugetlb;
extern struct memory_type *memory_default;

struct memory_type * memory_ib(struct vnode *n, struct memory_type *parent);
struct memory_type * memory_managed(void *ptr, size_t len);

int memory_mmap_init(int hugepages) __attribute__ ((warn_unused_result));
