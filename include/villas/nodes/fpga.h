/** Node type: VILLASfpga
 *
 * This file implements the fpga node-type.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2017, Steffen Vogel
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

/**
 * @addtogroup fpga VILLASfpga
 * @ingroup node
 * @{
 */

#pragma once

#include "kernel/vfio.h"
#include "kernel/pci.h"

#include "fpga/ips/dma.h"

#include "node.h"
#include "list.h"

/* Forward declarations */
struct fpga_ip;

/** The node type */
struct fpga {
	struct fpga_ip *ip;
	
	struct pci *pci;
	struct vfio_container *vfio_container;

	int use_irqs;
	struct dma_mem dma;
};

/** @see node_vtable::init */
int fpga_init(struct super_node *sn);

/** @see node_type::deinit */
int fpga_deinit();

/** @see node_type::parse */
int fpga_parse(struct node *n, config_setting_t *cfg);

struct fpga_card * fpga_lookup_card(const char *name);

/** @see node_type::print */
char * fpga_print(struct node *n);

/** @see node_type::open */
int fpga_start(struct node *n);

/** @see node_type::close */
int fpga_stop(struct node *n);

/** @see node_type::read */
int fpga_read(struct node *n, struct sample *smps[], unsigned cnt);

/** @see node_type::write */
int fpga_write(struct node *n, struct sample *smps[], unsigned cnt);

/** Get pointer to internal VILLASfpga datastructure */
struct fpga * fpga_get();

/** Reset VILLASfpga */
int fpga_reset(struct fpga *f);

/** Dump some details about the fpga card */
void fpga_dump(struct fpga *f);

/** @} */
