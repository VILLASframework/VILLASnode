/** Node type: WebSockets
 *
 * This file implements the websocket type for nodes.
 * It's based on the libwebsockets library.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2016, Institute for Automation of Complex Power Systems, EONERC
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
 * @addtogroup websockets WebSockets node type
 * @ingroup node
 * @{
 *********************************************************************************/


#ifndef _WEBSOCKET_H_
#define _WEBSOCKET_H_

#include <libwebsockets.h>

#include "node.h"
#include "pool.h"
#include "queue.h"

/* Forward declaration */
struct lws;

/** Internal data per websocket node */
struct websocket {
	struct list connections;		/**< List of active libwebsocket connections in server mode (struct websocket_connection). */
	struct list destinations;		/**< List of websocket servers connect to in client mode (struct websocket_destination). */
	
	struct pool pool;
	struct queue queue;			/**< For samples which are received from WebSockets a */
	
	int id;					/**< The index of this node */
};

/** @see node_vtable::init */
int websocket_init(int argc, char * argv[], config_setting_t *cfg);

/** @see node_vtable::deinit */
int websocket_deinit();

/** @see node_vtable::open */
int websocket_open(struct node *n);

/** @see node_vtable::close */
int websocket_close(struct node *n);

/** @see node_vtable::close */
int websocket_destroy(struct node *n);

/** @see node_vtable::read */
int websocket_read(struct node *n, struct sample *smps[], unsigned cnt);

/** @see node_vtable::write */
int websocket_write(struct node *n, struct sample *smps[], unsigned cnt);

#endif /** _WEBSOCKET_H_ @} */