/** Memory pool for fixed size objects.
 *
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

#include "utils.h"

#include "pool.h"
#include "memory.h"
#include "kernel/kernel.h"

int pool_init(struct pool *p, size_t cnt, size_t blocksz, const struct memtype *m)
{
	int ret;

	/* Make sure that we use a block size that is aligned to the size of a cache line */
	p->alignment = kernel_get_cacheline_size();
	p->blocksz = p->alignment * CEIL(blocksz, p->alignment);
	p->len = cnt * p->blocksz;
	p->mem = m;

	p->buffer = memory_alloc_aligned(m, p->len, p->alignment);
	if (!p->buffer)
		serror("Failed to allocate memory for memory pool");

	ret = queue_init(&p->queue, LOG2_CEIL(cnt), m);
	if (ret)
		return ret;
	
	for (int i = 0; i < cnt; i++)
		queue_push(&p->queue, (char *) p->buffer + i * p->blocksz);

	return 0;
}

int pool_destroy(struct pool *p)
{
	queue_destroy(&p->queue);	

	return memory_free(p->mem, p->buffer, p->len);
}