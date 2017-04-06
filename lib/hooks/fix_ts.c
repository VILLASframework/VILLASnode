/** Fix timestamp hook.
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
#include "timing.h"

int fix_ts_read(struct hook *h, struct sample *smps[], size_t *cnt)
{
	struct timespec now;

	now = time_now();

	for (int i = 0; i < *cnt; i++) {
		/* Check for missing receive timestamp
		 * Usually node_type::read() should update the receive timestamp.
		 * An example would be to use hardware timestamp capabilities of
		 * modern NICs.
		 */
		if ((smps[i]->ts.received.tv_sec ==  0 && smps[i]->ts.received.tv_nsec ==  0) ||
		    (smps[i]->ts.received.tv_sec == -1 && smps[i]->ts.received.tv_nsec == -1))
			smps[i]->ts.received = now;

		/* Check for missing origin timestamp */
		if ((smps[i]->ts.origin.tv_sec ==  0 && smps[i]->ts.origin.tv_nsec ==  0) ||
		    (smps[i]->ts.origin.tv_sec == -1 && smps[i]->ts.origin.tv_nsec == -1))
			smps[i]->ts.origin = now;
	}

	return 0;
}

static struct plugin p = {
	.name		= "fix_ts",
	.description	= "Update timestamps of sample if not set",
	.type		= PLUGIN_TYPE_HOOK,
	.hook		= {
		.priority = 0,
		.builtin = true,
		.read	= fix_ts_read,
	}
};

REGISTER_PLUGIN(&p)

/** @} */