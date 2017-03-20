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

struct decimate {
	int ratio;
	unsigned counter;
};

static int hook_decimate(struct hook *h, int when, struct hook_info *j)
{
	struct decimate *p = (struct decimate *) h->_vd;

	switch (when) {
		case HOOK_INIT:
			p->counter = 0;
			break;
		
		case HOOK_PARSE:
			if (!h->cfg)
				error("Missing configuration for hook: '%s'", plugin_name(h->_vt));
		
			if (!config_setting_lookup_int(h->cfg, "ratio", &p->ratio))
				cerror(h->cfg, "Missing setting 'ratio' for hook '%s'", plugin_name(h->_vt));
	
			break;
		
		case HOOK_READ:
			assert(j->samples);
		
			int i, ok;
			for (i = 0, ok = 0; i < j->count; i++) {
				if (p->counter++ % p->ratio == 0) {
					struct sample *tmp;
					
					tmp = j->samples[ok];
					j->samples[ok++] = j->samples[i];
					j->samples[i] = tmp;
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
		.size	= sizeof(struct decimate),
		.cb	= hook_decimate,
		.when	= HOOK_STORAGE | HOOK_PARSE | HOOK_DESTROY | HOOK_READ
	}
};

REGISTER_PLUGIN(&p)
	
/** @} */