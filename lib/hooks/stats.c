/** Statistic-related hook functions.
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

#include "hook.h"
#include "sample.h"
#include "path.h"
#include "utils.h"
#include "stats.h"

extern struct list *hook_nodes;

REGISTER_HOOK("stats", "Collect statistics for the current path", 2, 1, hook_stats, HOOK_STORAGE | HOOK_PATH | HOOK_READ | HOOK_PERIODIC)
int hook_stats(struct hook *h, int when, struct hook_info *j)
{
	struct stats *s = hook_storage(h, when, sizeof(struct stats), (ctor_cb_t) stats_init, (dtor_cb_t) stats_destroy);
	
	switch (when) {
		case HOOK_INIT:
			if (j->path)
				j->path->stats = s;
			break;

		case HOOK_READ:
			assert(j->smps);
		
			stats_collect(s->delta, j->smps, j->cnt);
			stats_commit(s, s->delta);
			break;

		case HOOK_PATH_STOP:
			stats_print(s, 1);
			break;

		case HOOK_PATH_RESTART:
			stats_reset(s);
			break;
			
		case HOOK_PERIODIC:
			assert(j->path);

			stats_print_periodic(s, j->path);
			break;
	}
	
	return j->cnt;
}

/** @todo This is untested */
REGISTER_HOOK("stats_send", "Send path statistics to another node", 99, 0, hook_stats_send, HOOK_STORAGE | HOOK_PATH | HOOK_READ)
int hook_stats_send(struct hook *h, int when, struct hook_info *j)
{
	struct private {
		struct node *dest;
		struct stats *stats;
		int ratio;
	} *private = hook_storage(h, when, sizeof(*private), NULL, NULL);
	
	switch (when) {
		case HOOK_INIT:
			assert(j->nodes);
			assert(j->path);
		
			if (!h->parameter)
				error("Missing parameter for hook '%s'", h->name);
			
			private->dest = list_lookup(j->nodes, h->parameter);
			if (!private->dest)
				error("Invalid destination node '%s' for hook '%s'", h->parameter, h->name);
			break;
			
		case HOOK_PATH_START:
			node_start(private->dest);
			break;

		case HOOK_PATH_STOP:
			node_stop(private->dest);
			break;

		case HOOK_READ:
			stats_send(private->stats, private->dest);
			break;
	}
	
	return 0;
}
