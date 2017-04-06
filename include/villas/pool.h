/** Memory pool for fixed size objects.
 *
 * This datastructure is based on a lock-less stack (lstack).
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

#ifndef _POOL_H_
#define _POOL_H_

#include <sys/types.h>

#include "queue.h"
#include "memory.h"

/** A thread-safe memory pool */
struct pool {
	off_t  buffer_off; /**< Offset from the struct address to the underlying memory area */
	struct memtype *mem;
	
	size_t len;		/**< Length of the underlying memory area */
	
	size_t blocksz;		/**< Length of a block in bytes */
	size_t alignment;	/**< Alignment of a block in bytes */
	
	struct queue queue; /**< The queue which is used to keep track of free blocks */
};

#define INLINE static inline __attribute__((unused)) 

/** Initiazlize a pool */
int pool_init(struct pool *p, size_t cnt, size_t blocksz, struct memtype *mem);

/** Destroy and release memory used by pool. */
int pool_destroy(struct pool *p);

/** Pop cnt values from the stack an place them in the array blocks */
INLINE ssize_t pool_get_many(struct pool *p, void *blocks[], size_t cnt)
{
	return queue_pull_many(&p->queue, blocks, cnt);
}

/** Push cnt values which are giving by the array values to the stack. */
INLINE ssize_t pool_put_many(struct pool *p, void *blocks[], size_t cnt)
{
	return queue_push_many(&p->queue, blocks, cnt);
}

/** Get a free memory block from pool. */
INLINE void * pool_get(struct pool *p)
{
	void *ptr;
	return queue_pull(&p->queue, &ptr) == 1 ? ptr : NULL;
}

/** Release a memory block back to the pool. */
INLINE int pool_put(struct pool *p, void *buf)
{
	return queue_push(&p->queue, buf);
}

#endif /* _POOL_H_ */
