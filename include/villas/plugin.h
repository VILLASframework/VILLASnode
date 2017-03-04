/** Loadable / plugin support.
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

#include "hooks.h"
#include "api.h"

#include "fpga/ip.h"

#include "nodes/cbuilder.h"

#define REGISTER_PLUGIN(p)					\
__attribute__((constructor)) static void UNIQUE(__ctor)() {	\
	list_push(&plugins, p);					\
	(p)->load(p);						\
}								\
__attribute__((destructor)) static void UNIQUE(__dtor)() {	\
	(p)->unload(p);						\
	list_remove(&plugins, p);				\
}

extern struct list plugins;

enum plugin_type {
	PLUGIN_TYPE_HOOK,
	PLUGIN_TYPE_NODE,
	PLUGIN_TYPE_API,
	PLUGIN_TYPE_FPGA_IP,
	PLUGIN_TYPE_MODEL_CBUILDER
};

enum plugin_state {
	PLUGIN_STATE_DESTROYED,
	PLUGIN_STATE_UNLOADED,
	PLUGIN_STATE_LOADED
};

struct plugin {
	char *name;
	char *description;
	void *handle;
	char *path;
	
	enum plugin_type type;
	enum plugin_state state;
	
	int (*load)(struct plugin *p);
	int (*unload)(struct plugin *p);
	
	union {
		struct api_ressource	api;
		struct node_type	node;
		struct fpga_ip_type	ip;
		struct hook		hook;
		struct cbuilder_model	cb;
	};
};

int plugin_init(struct plugin *p, char *name, char *path);

int plugin_destroy(struct plugin *p);

int plugin_parse(struct plugin *p, config_setting_t *cfg);

int plugin_load(struct plugin *p);

int plugin_unload(struct plugin *p);

void plugin_dump(enum plugin_type type);

/** Find registered and loaded plugin with given name and type. */
struct plugin * plugin_lookup(enum plugin_type type, const char *name);