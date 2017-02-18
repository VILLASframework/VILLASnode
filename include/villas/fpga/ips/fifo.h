/** FIFO related helper functions
 *
 * These functions present a simpler interface to Xilinx' FIFO driver (XLlFifo_*)
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

#include <sys/types.h>

#include <xilinx/xstatus.h>
#include <xilinx/xllfifo.h>

struct fifo {
	XLlFifo inst;
	
	uint32_t baseaddr_axi4;
};

/* Forward declaration */
struct ip;

int fifo_init(struct fpga_ip *c);

ssize_t fifo_write(struct fpga_ip *c, char *buf, size_t len);

ssize_t fifo_read(struct fpga_ip *c, char *buf, size_t len);

/** @} */