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

static int hook_convert(struct hook *h, int when, struct hook_info *j)
{
	struct {
		enum {
			TO_FIXED,
			TO_FLOAT
		} mode;
	} *private = hook_storage(h, when, sizeof(*private), NULL, NULL);

	const char *mode;

	switch (when) {
		case HOOK_PARSE:
			if (!h->cfg)
				error("Missing configuration for hook: '%s'", plugin_name(h->_vt));
			
			if (!config_setting_lookup_string(h->cfg, "mode", &mode))
				cerror(h->cfg, "Missing setting 'mode' for hook '%s'", plugin_name(h->_vt));
			
			if      (!strcmp(mode, "fixed"))
				private->mode = TO_FIXED;
			else if (!strcmp(mode, "float"))
				private->mode = TO_FLOAT;
			else
				error("Invalid parameter '%s' for hook 'convert'", mode);
			break;
		
		case HOOK_READ:
			for (int i = 0; i < j->count; i++) {
				for (int k = 0; k < j->samples[i]->length; k++) {
					switch (private->mode) {
						case TO_FIXED: j->samples[i]->data[k].i = j->samples[i]->data[k].f * 1e3; break;
						case TO_FLOAT: j->samples[i]->data[k].f = j->samples[i]->data[k].i; break;
					}
				}
			}
			
			return j->count;
	}

	return 0;
}

static struct plugin p = {
	.name		= "convert",
	.description	= "Convert message from / to floating-point / integer",
	.type		= PLUGIN_TYPE_HOOK,
	.hook		= {
		.priority = 99,
		.cb	= hook_convert,
		.when	= HOOK_STORAGE | HOOK_PARSE | HOOK_READ
	}
};

REGISTER_PLUGIN(&p)

/** @} */