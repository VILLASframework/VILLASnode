/** Node type: Wrapper around RSCAD CBuilder model
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
 * @addtogroup cbuilder RTDS CBuilder model node
 * @ingroup node
 * @{ 
 */

#pragma once

#include <pthread.h>

#include "list.h"

/* Forward declaration */
struct cbuilder;

struct cbuilder_model {
	char *name;

	void (*code)();
	void (*ram)();
	
	int (*init)(struct cbuilder *cb);
	int (*read)(float inputs[], int len);
	int (*write)(float outputs[], int len);
};

struct cbuilder {
	unsigned long step, read;
	double timestep;

	struct cbuilder_model *model;
	
	float *params;
	int paramlen;

	/* This mutex and cv are used to protect model parameters, input & outputs
	 *
	 * The cbuilder_read() function will wait for the completion of a simulation step
	 * before returning.
	 * The simulation step is triggerd by a call to cbuilder_write().
	 */
	pthread_mutex_t mtx;
	pthread_cond_t cv;
};

/** @} */