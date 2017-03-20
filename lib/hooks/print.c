/** Print hook.
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
#include "sample.h"

struct print {
	FILE *output;
	const char *uri;
};

static int hook_print(struct hook *h, int when, struct hook_info *j)
{
	struct print *p = (struct print *) h->_vd;
	
	switch (when) {
		case HOOK_INIT:
			p->output = stdout;
			p->uri = NULL;
			break;

		case HOOK_PATH_START:
			if (p->uri) {
				p->output = fopen(p->uri, "w+");
				if (!p->output)
					error("Failed to open file %s for writing", p->uri);
			}
			break;
		
		case HOOK_PATH_STOP:
			if (p->uri)
				fclose(p->output);
			break;
		
		case HOOK_PARSE:
			config_setting_lookup_string(h->cfg, "output", &p->uri);
			break;
		
		case HOOK_READ:
			assert(j->samples);
	
			for (int i = 0; i < j->count; i++)
				sample_fprint(p->output, j->samples[i], SAMPLE_ALL);
			break;
	}

	return j->count;
}

static struct plugin p = {
	.name		= "print",
	.description	= "Print the message to stdout",
	.type		= PLUGIN_TYPE_HOOK,
	.hook		= {
		.priority = 99,
		.size	= sizeof(struct print),
		.cb	= hook_print,
		.when	= HOOK_STORAGE | HOOK_PARSE | HOOK_READ | HOOK_PATH
	}
};

REGISTER_PLUGIN(&p)

/** @} */