/** Memory allocators.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2016, Institute for Automation of Complex Power Systems, EONERC
 * @license GNU Lesser General Public License v2.1
 *
 * VILLASnode - connecting real-time simulation equipment
 *
 * This application is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stddef.h>
#include <stdint.h>

#ifndef _MEMORY_H_
#define _MEMORY_H_

#define HUGEPAGESIZE	(1 << 21)

typedef void *(*memzone_allocator_t)(size_t len);
typedef int (*memzone_deallocator_t)(void *ptr, size_t len);

enum memtype_flags {
	MEMORY_MMAP	= (1 << 0),
	MEMORY_DMA	= (1 << 1),
	MEMORY_HUGEPAGE	= (1 << 2),
	MEMORY_HEAP	= (1 << 3)
};

struct memtype {
	const char *name;
	int flags;
	
	size_t alignment;
	
	memzone_allocator_t alloc;
	memzone_deallocator_t free;
};

/** @todo Unused for now */
struct memzone {
	struct memtype * const type;
	
	void *addr;
	uintptr_t physaddr;
	size_t len; 
};

/** Allocate \p len bytes memory of type \p m.
 *
 * @retval NULL If allocation failed.
 * @retval <>0  If allocation was successful.
 */
void * memory_alloc(const struct memtype *m, size_t len);

void * memory_alloc_aligned(const struct memtype *m, size_t len, size_t alignment);

int memory_free(const struct memtype *m, void *ptr, size_t len);

extern const struct memtype memtype_heap;
extern const struct memtype memtype_hugepage;

#endif /* _MEMORY_H_ */