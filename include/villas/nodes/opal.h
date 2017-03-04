/** Node type: OPAL (libOpalAsync API)
 *
 * This file implements the opal subtype for nodes.
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

/**
 * @ingroup node
 * @addtogroup opal OPAL-RT Async Process node type
 * @{
 */

#pragma once

#include <pthread.h>

#include "node.h"
#include "msg.h"

/* Define RTLAB before including OpalPrint.h for messages to be sent
 * to the OpalDisplay. Otherwise stdout will be used. */
#define RTLAB
#include "OpalPrint.h"
#include "AsyncApi.h"
#include "OpalGenAsyncParamCtrl.h"

struct opal {
	int reply;
	int mode;

	int send_id;
	int recv_id;
	
	Opal_SendAsyncParam send_params;
	Opal_RecvAsyncParam recv_params;
};

/** Initialize global OPAL settings and maps shared memory regions.
 *
 * @see node_vtable::init
 */
int opal_init(int argc, char *argv[], config_setting_t *cfg);

/** Free global OPAL settings and unmaps shared memory regions.
 *
 * @see node_vtable::deinit
 */
int opal_deinit();

/** @see node_vtable::parse */
int opal_parse(struct node *n, config_setting_t *cfg);

/** @see node_vtable::print */
char * opal_print(struct node *n);

/** Print global settings of the OPAL node type. */
int opal_print_global();

/** @see node_vtable::open */
int opal_open(struct node *n);

/** @see node_vtable::close */
int opal_close(struct node *n);

/** @see node_vtable::read */
int opal_read(struct node *n, struct sample *smps[], unsigned cnt);

/** @see node_vtable::write */
int opal_write(struct node *n, struct sample *smps[], unsigned cnt);

/** @} */