/** Path restart hook.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014-2019, Institute for Automation of Complex Power Systems, EONERC
 * @license GNU General Public License (version 3)
 *
 * VILLASnode
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************************/

/** @addtogroup hooks Hook functions
 * @{
 */

#include <inttypes.h>

#include <villas/hook.h>
#include <villas/plugin.h>
#include <villas/node.h>
#include <villas/sample.h>

struct restart {
	struct sample *prev;
};

static int restart_start(struct hook *h)
{
	struct restart *r = (struct restart *) h->_vd;

	r->prev = NULL;

	return 0;
}

static int restart_stop(struct hook *h)
{
	struct restart *r = (struct restart *) h->_vd;

	if (r->prev)
		sample_decref(r->prev);

	return 0;
}

static int restart_process(struct hook *h, struct sample *smp)
{
	struct restart *r = (struct restart *) h->_vd;

	assert(h->node);

	if (r->prev) {
		/* A wrap around of the sequence no should not be treated as a simulation restart */
		if (smp->sequence == 0 && r->prev->sequence != 0 && r->prev->sequence > UINT64_MAX - 16) {
			warning("Simulation from node %s restarted (previous->sequence=%" PRIu64 ", current->sequence=%" PRIu64 ")",
				node_name(h->node), r->prev->sequence, smp->sequence);

			smp->flags |= SAMPLE_IS_FIRST;

			/* Run restart hooks */
			for (size_t i = 0; i < vlist_length(&h->node->in.hooks); i++) {
				struct hook *k = (struct hook *) vlist_at(&h->node->in.hooks, i);

				hook_restart(k);
			}

			for (size_t i = 0; i < vlist_length(&h->node->out.hooks); i++) {
				struct hook *k = (struct hook *) vlist_at(&h->node->out.hooks, i);

				hook_restart(k);
			}
		}
	}

	sample_incref(smp);
	if (r->prev)
		sample_decref(r->prev);

	r->prev = smp;

	return HOOK_OK;
}

static struct plugin p = {
	.name		= "restart",
	.description	= "Call restart hooks for current node",
	.type		= PLUGIN_TYPE_HOOK,
	.hook		= {
		.flags		= HOOK_BUILTIN | HOOK_NODE_READ,
		.priority 	= 1,
		.process	= restart_process,
		.start		= restart_start,
		.stop		= restart_stop,
		.size		= sizeof(struct restart)
	}
};

REGISTER_PLUGIN(&p)

/** @} */
