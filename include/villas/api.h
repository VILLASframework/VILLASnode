/** REST-API-releated functions.
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

#pragma once

#include <libwebsockets.h>
#include <jansson.h>

#include "list.h"
#include "common.h"

#include "api/session.h"

/* Forward declarations */
struct lws;
struct super_node;

struct api;
struct api_action;

/** Callback type of command function
 *
 * @param[inout] c Command handle
 * @param[in] args JSON command arguments.
 * @param[out] resp JSON command response.
 * @param[in] i Execution context.
 */
typedef int (*api_cb_t)(struct api_action *c, json_t *args, json_t **resp, struct api_session *s);

struct api {
	struct list sessions;	/**< List of currently active connections */
	
	enum state state;
	
	struct super_node *super_node;
};

/** API action descriptor  */
struct api_action {
	api_cb_t cb;
};

/** Initalize the API.
 *
 * Save references to list of paths / nodes for command execution.
 */
int api_init(struct api *a, struct super_node *sn);

int api_destroy(struct api *a);

int api_start(struct api *a);

int api_stop(struct api *a);

/** Libwebsockets callback for "api" endpoint */
int api_ws_protocol_cb(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

int api_http_protocol_cb(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);