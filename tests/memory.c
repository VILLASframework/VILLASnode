/** Unit tests for memory management
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
 *********************************************************************************/

#include <criterion/criterion.h>
#include <criterion/theories.h>

#include <errno.h>

#include "memory.h"
#include "utils.h"

TheoryDataPoints(memory, aligned) = {
	DataPoints(size_t, 1, 32, 55, 1 << 10, 1 << 20),
	DataPoints(size_t, 1, 8, 1 << 12),
	DataPoints(struct memtype *, &memtype_heap, &memtype_hugepage)
};

Theory((size_t len, size_t align, struct memtype *m), memory, aligned) {
	int ret;
	void *ptr;
	
	ptr = memory_alloc_aligned(m, len, align);
	cr_assert_neq(ptr, NULL, "Failed to allocate memory");
	
	cr_assert(IS_ALIGNED(ptr, align));

	if (m == &memtype_hugepage) {
		cr_assert(IS_ALIGNED(ptr, HUGEPAGESIZE));
	}

	ret = memory_free(m, ptr, len);
	cr_assert_eq(ret, 0, "Failed to release memory: ret=%d, ptr=%p, len=%zu: %s", ret, ptr, len, strerror(errno));
}

Test(memory, manager) {
    size_t total_size = 1 << 10;
    size_t max_block = total_size - sizeof(struct memtype_managed) - sizeof(struct memblock);
    void *p = memory_alloc(&memtype_heap, total_size);
    struct memtype *manager = memtype_managed_init(p, total_size);

    void *p1, *p2, *p3;
    p1 = memory_alloc(manager, 16);
    cr_assert(p1);

    p2 = memory_alloc(manager, 32);
    cr_assert(p2);

    cr_assert(memory_free(manager, p1, 16) == 0);

    p1 = memory_alloc_aligned(manager, 128, 128);
    cr_assert(p1);
    cr_assert(IS_ALIGNED(p1, 128));

    p3 = memory_alloc_aligned(manager, 128, 256);
    cr_assert(p3);
    cr_assert(IS_ALIGNED(p1, 256));

    cr_assert(memory_free(manager, p2, 32) == 0);
    cr_assert(memory_free(manager, p1, 128) == 0);
    cr_assert(memory_free(manager, p3, 128) == 0);


    p1 = memory_alloc(manager, max_block);
    cr_assert(p1);
    cr_assert(memory_free(manager, p1, max_block) == 0);
}
