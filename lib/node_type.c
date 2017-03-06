/** Nodes.
 *
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

#include <string.h>
#include <libconfig.h>

#include "sample.h"
#include "node.h"
#include "cfg.h"
#include "utils.h"
#include "config.h"
#include "plugin.h"

int node_type_init(struct node_type *vt, int argc, char *argv[], config_setting_t *cfg)
{
	int ret;
	
	if (vt->state != NODE_TYPE_UNINITIALIZED)
		return -1;

	info("Initializing " YEL("%s") " node type", vt->name);
	{ INDENT
		ret = vt->init ? vt->init(argc, argv, cfg) : -1;
	}	

	if (ret == 0)
		vt->state = NODE_TYPE_INITIALIZED;

	return ret;
}

int node_type_deinit(struct node_type *vt)
{
	int ret;
	
	if (vt->state != NODE_TYPE_INITIALIZED)
		return -1;

	info("De-initializing " YEL("%s") " node type", vt->name);
	{ INDENT
		ret = vt->deinit ? vt->deinit() : -1;
	}
	
	if (ret == 0)
		vt->state = NODE_TYPE_UNINITIALIZED;

	return ret;
}
