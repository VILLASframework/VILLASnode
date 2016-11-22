/** Hook-releated functions.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2016, Institute for Automation of Complex Power Systems, EONERC
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

#include <string.h>
#include <math.h>

#include "timing.h"
#include "config.h"
#include "msg.h"
#include "hooks.h"
#include "path.h"
#include "utils.h"
#include "node.h"

struct list hooks;

/* Those references can be used inside the hook callbacks after initializing them with hook_init() */
struct list *hook_nodes = NULL;
struct list *hook_paths = NULL;
struct settings *hook_settings = NULL;

void hook_init(struct list *nodes, struct list *paths, struct settings *set)
{
	hook_nodes = nodes;
	hook_paths = paths;
	hook_settings = set;
}

int hooks_sort_priority(const void *a, const void *b) {
	struct hook *ha = (struct hook *) a;
	struct hook *hb = (struct hook *) b;
	
	return ha->priority - hb->priority;
}

int hook_run(struct path *p, struct sample *smps[], size_t cnt, int when)
{
	list_foreach(struct hook *h, &p->hooks) {
		if (h->type & when) {
			debug(DBG_HOOK | 22, "Running hook when=%u '%s' prio=%u, cnt=%zu", when, h->name, h->priority, cnt);

			cnt = h->cb(p, h, when, smps, cnt);
			if (cnt == 0)
				break;
		}
	}

	return cnt;
}

void * hook_storage(struct hook *h, int when, size_t len)
{
	switch (when) {
		case HOOK_INIT:
			h->_vd = alloc(len);
			break;
			
		case HOOK_DEINIT:
			free(h->_vd);
			h->_vd = NULL;
			break;
	}
	
	return h->_vd;
}