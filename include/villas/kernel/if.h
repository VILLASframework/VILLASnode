/** Interface related functions
 *
 * These functions are used to manage a network interface.
 * Most of them make use of Linux-specific APIs.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2017, Institute for Automation of Complex Power Systems, EONERC
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

/** @addtogroup fpga Kernel @{ */

#pragma once

#include <sys/types.h>
#include <sys/socket.h>

#include "list.h"

#define IF_IRQ_MAX	3	/**< Maxmimal number of IRQs of an interface */

#ifndef SO_MARK
  #define SO_MARK	36	/**< Workaround: add missing constant for OPAL-RT Redhawk target */
#endif

struct socket;
struct nl_addr;
struct rtnl_link;

/** Interface data structure */
struct interface {
	struct rtnl_link *nl_link;	/**< libnl3: Handle of interface. */
	struct rtnl_qdisc *tc_qdisc;	/**< libnl3: Root priority queuing discipline (qdisc). */

	char irqs[IF_IRQ_MAX];		/**< List of IRQs of the NIC. */

	struct list sockets;		/**< Linked list of associated sockets. */
};

/** Add a new interface to the global list and lookup name, irqs...
 *
 * @param link The libnl3 link handle
 * @retval >0 Success. A pointer to the new interface.
 * @retval 0 Error. The creation failed.
 */
struct interface * if_create(struct rtnl_link *link);


/** Destroy interface by freeing dynamically allocated memory.
 *
 * @param i A pointer to the interface structure.
 */
int if_destroy(struct interface *i);

/** Start interface.
 *
 * This setups traffic controls queue discs, network emulation and
 * maps interface IRQs according to affinity.
 *
 * @param i A pointer to the interface structure.
 * @param affinity Set the IRQ affinity of this interface.
 * @retval 0 Success. Everything went well.
 * @retval <0 Error. Something went wrong.
 */
int if_start(struct interface *i, int affinity);

/** Stop interface
 *
 * This resets traffic qdiscs ant network emulation
 * and maps interface IRQs to all CPUs.
 *
 * @param i A pointer to the interface structure.
 * @retval 0 Success. Everything went well.
 * @retval <0 Error. Something went wrong.
 */
int if_stop(struct interface *i);

/** Lookup routing tables to get the interface on which packets for a certain destination
 *  will leave the system.
 *
 * @param[in] sa The destination address for outgoing packets.
 * @param[out] link The egress interface.
 * @retval 0 Success. Everything went well.
 * @retval <0 Error. Something went wrong.
 */
int if_get_egress(struct sockaddr *sa, struct rtnl_link **link);

/** Get all IRQs for this interface.
 *
 * Only MSI IRQs are determined by looking at:
 *  /sys/class/net/{ifname}/device/msi_irqs/
 *
 * @param i A pointer to the interface structure
 * @retval 0 Success. Everything went well.
 * @retval <0 Error. Something went wrong.
 */
int if_get_irqs(struct interface *i);

/** Change the SMP affinity of NIC interrupts.
 *
 * @param i A pointer to the interface structure
 * @param affinity A mask specifying which cores should handle this interrupt.
 * @retval 0 Success. Everything went well.
 * @retval <0 Error. Something went wrong.
 */
int if_set_affinity(struct interface *i, int affinity);

/** @} */