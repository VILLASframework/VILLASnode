/** AXI-PCIe Interrupt controller
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2016, Steffen Vogel
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
 **********************************************************************************/

#ifndef _INTC_H_
#define _INTC_H_

#include <xilinx/xintc.h>

enum intc_flags {
	INTC_ENABLED = (1 << 0),
	INTC_POLLING = (1 << 1)
};

struct intc {
	int num_irqs;		/**< Number of available MSI vectors */

	int efds[32];		/**< Event FDs */
	int nos[32];		/**< Interrupt numbers from /proc/interrupts */
	
	int flags[32];		/**< Mask of intc_flags */
};

int intc_init(struct ip *c);

void intc_destroy(struct ip *c);

int intc_enable(struct ip *c, uint32_t mask, int poll);

int intc_disable(struct ip *c, uint32_t mask);

uint64_t intc_wait(struct ip *c, int irq);

#endif /* _INTC_H_ */