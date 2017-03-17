/** Skip first hook.
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

#include <libconfig.h>

#include "hook.h"
#include "plugin.h"
#include "timing.h"

static int hook_skip_first(struct hook *h, int when, struct hook_info *j)
{
	struct {
		struct timespec skip;	/**< Time to wait until first message is not skipped */
		struct timespec until;	/**< Absolute point in time from where we accept samples. */
		enum {
			HOOK_SKIP_FIRST_STATE_STARTED,	/**< Path just started. First sample not received yet. */
			HOOK_SKIP_FIRST_STATE_SKIPPING,	/**< First sample received. Skipping samples now. */
			HOOK_SKIP_FIRST_STATE_NORMAL,	/**< All samples skipped. Normal operation. */
		} state;
	} *private = hook_storage(h, when, sizeof(*private), NULL, NULL);

	switch (when) {
		case HOOK_PARSE: {
			double wait;
			
			if (!h->cfg)
				error("Missing configuration for hook: '%s'", plugin_name(h->_vt));
			
			if (!config_setting_lookup_float(h->cfg, "seconds", &wait))
				cerror(h->cfg, "Missing setting 'seconds' for hook '%s'", plugin_name(h->_vt));
	
			private->skip = time_from_double(wait);
			break;
		}
		
		case HOOK_PATH_START:
		case HOOK_PATH_RESTART:
			private->state = HOOK_SKIP_FIRST_STATE_STARTED;
			break;
	
		case HOOK_READ:
			assert(j->samples);
			
			if (private->state == HOOK_SKIP_FIRST_STATE_STARTED) {
				private->until = time_add(&j->samples[0]->ts.received, &private->skip);
				private->state = HOOK_SKIP_FIRST_STATE_SKIPPING;
			}
			

			int i, ok;
			for (i = 0, ok = 0; i < j->count; i++) {
				if (time_delta(&private->until, &j->samples[i]->ts.received) > 0) {
					struct sample *tmp;

					tmp = j->samples[i];
					j->samples[i] = j->samples[ok];
					j->samples[ok++] = tmp;
				
				}

				/* To discard the first X samples in 'smps[]' we must
				 * shift them to the end of the 'smps[]' array.
				 * In case the hook returns a number 'ok' which is smaller than 'cnt',
				 * only the first 'ok' samples in 'smps[]' are accepted and further processed.
				 */
			}

			return ok;
	}

	return 0;
}

static struct plugin p = {
	.name		= "skip_first",
	.description	= "Skip the first samples",
	.type		= PLUGIN_TYPE_HOOK,
	.hook		= {
		.priority = 99,
		.cb	= hook_skip_first,
		.when	= HOOK_STORAGE |  HOOK_PARSE | HOOK_READ | HOOK_PATH
	}
};

REGISTER_PLUGIN(&p)
	
/** @} */