/** Memory pool for fixed size objects.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014-2016, Institute for Automation of Complex Power Systems, EONERC
 *   This file is part of VILLASnode. All Rights Reserved. Proprietary and confidential.
 *   Unauthorized copying of this file, via any medium is strictly prohibited.
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
	p->blocksz = blocksz * CEIL(blocksz, p->alignment);
	p->len = cnt * p->blocksz;
	p->mem = m;

	p->buffer = memory_alloc_aligned(m, p->len, p->alignment);
	if (!p->buffer)
		serror("Failed to allocate memory for memory pool");
	else
		debug(LOG_POOL | 4, "Allocated %#zx bytes for memory pool", p->len);

	ret = queue_init(&p->queue, cnt, m);
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