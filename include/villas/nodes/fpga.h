/** Node type: VILLASfpga
 *
 * This file implements the fpga node-type.
 *
 * @file
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
 */
/**
 * @addtogroup fpga VILLASnode
 * @ingroup node
 * @{
 *********************************************************************************/

#ifndef _FPGA_H_
#define _FPGA_H_

#include "kernel/vfio.h"
#include "kernel/pci.h"

#include "fpga/dma.h"
#include "fpga/ip.h"
#include "fpga/intc.h"

#include "node.h"
#include "list.h"

struct fpga {
	struct pci_dev filter;		/**< Filter for PCI device. */
	struct vfio_dev vd;		/**< VFIO device handle. */

	int do_reset;			/**< Reset VILLASfpga during startup? */
	int affinity;			/**< Affinity for MSI interrupts */

	struct list ips;		/**< List of IP components on FPGA. */

	char *map;			/**< PCI BAR0 mapping for register access */

	size_t maplen;
	size_t dmalen;

	/* Some IP cores are special and referenced here */
	struct ip *intc;
	struct ip *reset;
	struct ip *sw;
	
	config_setting_t *cfg;
};

struct fpga_dm {
	struct ip *ip;
	const char *ip_name;

	int use_irqs;

	struct dma_mem dma;

	enum {
		FPGA_DM_DMA,
		FPGA_DM_FIFO
	} type;

	struct fpga *card;
};

/** @see node_vtable::init */
int fpga_init(int argc, char * argv[], config_setting_t *cfg);

/** @see node_vtable::deinit */
int fpga_deinit();

/** @see node_vtable::parse */
int fpga_parse(struct node *n, config_setting_t *cfg);

/** Parse the 'fpga' section in the configuration file */
int fpga_parse_card(struct fpga *v, int argc, char * argv[], config_setting_t *cfg);

/** @see node_vtable::print */
char * fpga_print(struct node *n);

/** @see node_vtable::open */
int fpga_open(struct node *n);

/** @see node_vtable::close */
int fpga_close(struct node *n);

/** @see node_vtable::read */
int fpga_read(struct node *n, struct sample *smps[], unsigned cnt);

/** @see node_vtable::write */
int fpga_write(struct node *n, struct sample *smps[], unsigned cnt);

/** Get pointer to internal VILLASfpga datastructure */
struct fpga * fpga_get();

/** Reset VILLASfpga */
int fpga_reset(struct fpga *f);

/** Dump some details about the fpga card */
void fpga_dump(struct fpga *f);

#endif /** _FPGA_H_ @} */
