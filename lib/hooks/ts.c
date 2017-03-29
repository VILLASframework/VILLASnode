/** Timestamp hook.
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

static int ts_read(struct hook *h, struct sample *smps[], size_t *cnt)
{
	for (int i = 0; i < *cnt; i++)
		smps[i]->ts.origin = smps[i]->ts.received;

	return 0;
}

static struct plugin p = {
	.name		= "ts",
	.description	= "Overwrite origin timestamp of samples with receive timestamp",
	.type		= PLUGIN_TYPE_HOOK,
	.hook		= {
		.priority = 99,
		.read	= ts_read,
		.size	= 0
	}
};

REGISTER_PLUGIN(&p)
	
/** @} */