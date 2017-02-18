/** AXI Stream interconnect related helper functions
 *
 * These functions present a simpler interface to Xilinx' AXI Stream switch driver (XAxis_Switch_*)
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
 * @addtogroup fpga VILLASfpga
 * @{
 **********************************************************************************/

#pragma once

#include <xilinx/xaxis_switch.h>

#include "list.h"

/* Forward declaration */
struct ip;

struct sw_path {
	const char *in;
	const char *out;
};

struct sw {
	XAxis_Switch inst;

	int num_ports;
	struct list paths;
};

struct ip;

int switch_init(struct fpga_ip *c);

/** Initialize paths which have been parsed by switch_parse() */
int switch_init_paths(struct fpga_ip *c);

int switch_destroy(struct fpga_ip *c);

int switch_parse(struct fpga_ip *c);

int switch_connect(struct fpga_ip *c, struct fpga_ip *mi, struct fpga_ip *si);

int switch_disconnect(struct fpga_ip *c, struct fpga_ip *mi, struct fpga_ip *si);

/** @} */