/** Path restart hook.
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

/** @addtogroup hooks Hook functions
 * @{
 */

#include "hook.h"
#include "plugin.h"
#include "path.h"

static int hook_restart(struct hook *h, int when, struct hook_info *j)
{
	assert(j->smps);
	assert(j->path);

	for (int i = 0; i < j->cnt; i++) {
		h->last = j->smps[i];
		
		if (h->prev) {
			if (h->last->sequence  == 0 &&
			    h->prev->sequence <= UINT32_MAX - 32) {
				warn("Simulation for path %s restarted (prev->seq=%u, current->seq=%u)",
					path_name(j->path), h->prev->sequence, h->last->sequence);

				hook_run(j->path, &j->smps[i], j->cnt - i, HOOK_PATH_RESTART);
			}
		}
		
		h->prev = h->last;
	}

	return j->cnt;
}

static struct plugin p = {
	.name		= "restart",
	.description	= "Call restart hooks for current path",
	.type		= PLUGIN_TYPE_HOOK,
	.hook		= {
		.priority = 1,
		.history = 1,
		.cb	= hook_restart,
		.type	= HOOK_AUTO | HOOK_READ
	}
};

REGISTER_PLUGIN(&p)

/** @} */