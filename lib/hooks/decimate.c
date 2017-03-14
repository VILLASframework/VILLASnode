/** Decimate hook.
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

static int hook_decimate(struct hook *h, int when, struct hook_info *j)
{
	struct {
		unsigned ratio;
		unsigned counter;
	} *private = hook_storage(h, when, sizeof(*private), NULL, NULL);

	switch (when) {
		case HOOK_PARSE:
			if (!h->parameter)
				error("Missing parameter for hook: '%s'", plugin_name(h));
	
			private->ratio = strtol(h->parameter, NULL, 10);
			if (!private->ratio)
				error("Invalid parameter '%s' for hook 'decimate'", h->parameter);
		
			private->counter = 0;
			break;
		
		case HOOK_READ:
			assert(j->smps);
		
			int i, ok;
			for (i = 0, ok = 0; i < j->cnt; i++) {
				if (private->counter++ % private->ratio == 0) {
					struct sample *tmp;
					
					tmp = j->smps[ok];
					j->smps[ok++] = j->smps[i];
					j->smps[i] = tmp;
				}
			}

			return ok;
	}

	return 0;
}

static struct plugin p = {
	.name		= "decimate",
	.description	= "Downsamping by integer factor",
	.type		= PLUGIN_TYPE_HOOK,
	.hook		= {
		.priority = 99,
		.cb	= hook_decimate,
		.type	= HOOK_STORAGE | HOOK_DESTROY | HOOK_READ
	}
};

REGISTER_PLUGIN(&p)
	
/** @} */