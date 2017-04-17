/** The "config" API ressource.
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

#include <libconfig.h>

#include "api.h"
#include "utils.h"
#include "plugin.h"
#include "json.h"

static int api_config(struct api_action *h, json_t *args, json_t **resp, struct api_session *s)
{
	config_setting_t *cfg_root = config_root_setting(&s->api->super_node->cfg);
	
	*resp = cfg_root ? config_to_json(cfg_root) : json_object();
	
	return 0;
}

static struct plugin p = {
	.name = "config",
	.description = "retrieve current VILLASnode configuration",
	.type = PLUGIN_TYPE_API,
	.api.cb = api_config
};

REGISTER_PLUGIN(&p)