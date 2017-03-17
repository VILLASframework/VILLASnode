/** Drop hook.
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
#include "stats.h"
#include "path.h"

static int hook_drop(struct hook *h, int when, struct hook_info *j)
{
	int i, ok, dist;
	
	assert(j->samples);

	for (i = 0, ok = 0; i < j->count; i++) {
		h->last = j->samples[i];
		
		if (h->prev) {
			dist = h->last->sequence - (int32_t) h->prev->sequence;
			if (dist <= 0) {
				warn("Dropped sample: dist = %d, i = %d", dist, i);
				if (j->path && j->path->stats)
					stats_update(j->path->stats->delta, STATS_DROPPED, dist);
			}
			else {
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
		else {
			struct sample *tmp;
		
			tmp = j->samples[i];
			j->samples[i] = j->samples[ok];
			j->samples[ok++] = tmp;
		}

		h->prev = h->last;
	}

	return ok;
}

static struct plugin p = {
	.name		= "drop",
	.description	= "Drop messages with reordered sequence numbers",
	.type		= PLUGIN_TYPE_HOOK,
	.hook		= {
		.priority = 3,
		.cb	= hook_drop,
		.when	= HOOK_AUTO | HOOK_READ
	}
};

REGISTER_PLUGIN(&p)

/** @} */