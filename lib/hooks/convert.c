/** Convert hook.
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

static int hook_convert(struct hook *h, int when, struct hook_info *k)
{
	struct {
		enum {
			TO_FIXED,
			TO_FLOAT
		} mode;
	} *private = hook_storage(h, when, sizeof(*private), NULL, NULL);

	switch (when) {
		case HOOK_PARSE:
			if (!h->parameter)
				error("Missing parameter for hook: '%s'", plugin_name(h));

			if      (!strcmp(h->parameter, "fixed"))
				private->mode = TO_FIXED;
			else if (!strcmp(h->parameter, "float"))
				private->mode = TO_FLOAT;
			else
				error("Invalid parameter '%s' for hook 'convert'", h->parameter);
			break;
		
		case HOOK_READ:
			for (int i = 0; i < k->cnt; i++) {
				for (int j = 0; j < k->smps[i]->length; j++) {
					switch (private->mode) {
						case TO_FIXED: k->smps[i]->data[j].i = k->smps[i]->data[j].f * 1e3; break;
						case TO_FLOAT: k->smps[i]->data[j].f = k->smps[i]->data[j].i; break;
					}
				}
			}
			
			return k->cnt;
	}

	return 0;
}

static struct plugin p = {
	.name		= "convert",
	.description	= "Convert message from / to floating-point / integer",
	.type		= PLUGIN_TYPE_HOOK,
	.hook		= {
		.priority = 99,
		.history = 0,
		.cb	= hook_convert,
		.type	= HOOK_STORAGE | HOOK_READ
	}
};

REGISTER_PLUGIN(&p)

/** @} */