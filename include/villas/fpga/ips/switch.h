/** AXI Stream interconnect related helper functions
 *
 * These functions present a simpler interface to Xilinx' AXI Stream switch driver (XAxis_Switch_*)
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2015-2016, Steffen Vogel
 *   This file is part of S2SS. All Rights Reserved. Proprietary and confidential.
 *   Unauthorized copying of this file, via any medium is strictly prohibited.
 */
/**
 * @addtogroup fpga VILLASfpga
 * @{
 */

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