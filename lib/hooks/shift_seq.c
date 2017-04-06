/** Shift sequence number of samples
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

struct shift {
	int offset;
};

static int shift_seq_parse(struct hook *h, config_setting_t *cfg)
{
	struct shift *p = h->_vd;
	
	if (!config_setting_lookup_int(cfg, "offset", &p->offset))
		cerror(cfg, "Missing setting 'offset' for hook '%s'", plugin_name(h->_vt));
	
	return 0;
}

static int shift_seq_read(struct hook *h, struct sample *smps[], size_t *cnt)
{
	struct shift *p = h->_vd;

	for (int i = 0; i < *cnt; i++)
		smps[i]->sequence += p->offset;

	return 0;
}

static struct plugin p = {
	.name		= "shift_seq",
	.description	= "Shift sequence number of samples",
	.type		= PLUGIN_TYPE_HOOK,
	.hook		= {
		.priority = 99,
		.parse	= shift_seq_parse,
		.read	= shift_seq_read,
		.size	= sizeof(struct shift),
	}
};

REGISTER_PLUGIN(&p)

/** @} */