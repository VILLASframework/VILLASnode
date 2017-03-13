/** Time shift hook.
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

static int hook_shift(struct hook *h, int when, struct hook_info *k)
{
	struct {
		struct timespec offset;
		enum {
			TS_ORIGIN,
			TS_RECEIVED,
			TS_SENT
		} mode;
	} *private = hook_storage(h, when, sizeof(*private), NULL, NULL);

	switch (when) {
		case HOOK_PARSE:
			if (!h->parameter)
				error("Missing parameter for hook: '%s'", plugin_name(h));

			char *endptr;
			double offset;
			
			char *off;
			char *cpy = strdup(h->parameter);
			
			char *tok1 = strtok(cpy, ",");
			char *tok2 = strtok(NULL, ",");

			if (tok2) {
				off = tok2;

				if      (!strcmp(tok1, "origin"))
					private->mode = TS_ORIGIN;
				else if (!strcmp(tok1, "received"))
					private->mode = TS_RECEIVED;
				else if (!strcmp(tok1, "sent"))
					private->mode = TS_SENT;
				else
					error("Invalid mode parameter for hook '%s'", plugin_name(h));
			}
			else {
				off = tok1;

				private->mode = TS_ORIGIN;
			}
			
			offset = strtod(off, &endptr);
			if (endptr == off)
				error("Invalid offset parameter for hook '%s'", plugin_name(h));
				
			private->offset = time_from_double(offset);
			
			free(cpy);

			break;
		
		case HOOK_READ:
			for (int i = 0; i < k->cnt; i++) {
				struct timespec *ts = NULL;
				
				switch (private->mode) {
					case TS_ORIGIN:   ts = &k->smps[i]->ts.origin; break;
					case TS_RECEIVED: ts = &k->smps[i]->ts.received; break;
					case TS_SENT:     ts = &k->smps[i]->ts.sent; break;
				}
				
				if (ts)
					*ts = time_add(ts, &private->offset);
			}

			return k->cnt;
	}

	return 0;
}

static struct plugin p = {
	.name		= "shift",
	.description	= "Shift the origin timestamp of samples",
	.type		= PLUGIN_TYPE_HOOK,
	.hook		= {
		.priority = 99,
		.history = 0,
		.cb	= hook_shift,
		.type	= HOOK_STORAGE | HOOK_READ
	}
};

REGISTER_PLUGIN(&p)

/** @} */