/** FPGA card
 *
 * This class represents a FPGA device.
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

/** @addtogroup fpga VILLASfpga
 * @{
 */

#pragma once

#include <libconfig.h>

#include "common.h"

/* Forward declarations */
struct fpga_ip;
struct vfio_container;

struct fpga_card {
	char *name;			/**< The name of the FPGA card */

	enum state state;		/**< The state of this FPGA card. */

	struct pci_device filter;		/**< Filter for PCI device. */
	struct vfio_device vfio_device;	/**< VFIO device handle. */

	int do_reset;			/**< Reset VILLASfpga during startup? */
	int affinity;			/**< Affinity for MSI interrupts */

	struct list ips;		/**< List of IP components on FPGA. */

	char *map;			/**< PCI BAR0 mapping for register access */

	size_t maplen;
	size_t dmalen;

	/* Some IP cores are special and referenced here */
	struct fpga_ip *intc;
	struct fpga_ip *reset;
	struct fpga_ip *sw;
	
	config_setting_t *cfg;
};

int fpga_card_parse(struct fpga_card *c, config_setting_t *cfg);

void fpga_card_dump(struct fpga_card *c);

/** Initialize FPGA card and its IP components. */
int fpga_card_init(struct fpga_card *c, struct pci *pci, struct vfio_container *vc);

int fpga_card_destroy(struct fpga_card *c);

/** Check if the FPGA card configuration is plausible. */
int fpga_card_check(struct fpga_card *c);

/** Reset the FPGA to a known state */
int fpga_card_reset(struct fpga_card *c);

/** @} */