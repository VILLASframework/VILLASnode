/** Nodes.
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

#include <string.h>
#include <libconfig.h>

#include "sample.h"
#include "node.h"
#include "cfg.h"
#include "utils.h"
#include "config.h"
#include "plugin.h"

int node_read(struct node *n, struct sample *smps[], unsigned cnt)
{
	int nread = 0;

	if (!n->_vt->read)
		return -1;

	/* Send in parts if vector not supported */
	if (n->_vt->vectorize > 0 && n->_vt->vectorize < cnt) {
		while (cnt - nread > 0) {
			nread += n->_vt->read(n, &smps[nread], MIN(cnt - nread, n->_vt->vectorize));
		}
	}
	else {
		nread = n->_vt->read(n, smps, cnt);
	}
	
	for (int i = 0; i < nread; i++)
		smps[i]->source = n;
	
	return nread;
}

int node_write(struct node *n, struct sample *smps[], unsigned cnt)
{
	int nsent = 0;

	if (!n->_vt->write)
		return -1;

	/* Send in parts if vector not supported */
	if (n->_vt->vectorize > 0 && n->_vt->vectorize < cnt) {
		while (cnt - nsent > 0)
			nsent += n->_vt->write(n, &smps[nsent], MIN(cnt - nsent, n->_vt->vectorize));
	}
	else {
		nsent = n->_vt->write(n, smps, cnt);
	}
	
	return nsent;
}

int node_start(struct node *n)
{
	int ret;
	
	if (n->state != NODE_CREATED && n->state != NODE_STOPPED)
		return -1;
	
	n->state = NODE_STARTING;

	info("Starting node %s", node_name_long(n));
	{ INDENT
		ret = n->_vt->open ? n->_vt->open(n) : -1;
	}
	
	if (ret == 0)
		n->state = NODE_RUNNING;
	
	n->sequence = 0;
	
	return ret;
}

int node_stop(struct node *n)
{
	int ret;

	if (n->state != NODE_RUNNING)
		return -1;
	
	n->state = NODE_STOPPING;

	info("Stopping node %s", node_name(n));
	{ INDENT
		ret = n->_vt->close ? n->_vt->close(n) : -1;
	}
	
	if (ret == 0)
		n->state = NODE_STOPPED;

	return ret;
}

char * node_name(struct node *n)
{
	if (!n->_name)
		strcatf(&n->_name, RED("%s") "(" YEL("%s") ")", n->name, n->_vt->name);
		
	return n->_name;
}

char * node_name_long(struct node *n)
{
	if (!n->_name_long) {
		if (n->_vt->print) {
			char *name_long = n->_vt->print(n);
			strcatf(&n->_name_long, "%s: %s", node_name(n), name_long);
			free(name_long);
		}
		else
			n->_name_long = node_name(n);		
	}
		
	return n->_name_long;
}

const char * node_name_short(struct node *n)
{
	return n->name;
}

int node_reverse(struct node *n)
{
	return n->_vt->reverse ? n->_vt->reverse(n) : -1;
}

struct node * node_create(struct node_type *vt)
{
	struct node *n = alloc(sizeof(struct node));
	
	list_push(&vt->instances, n);
	
	n->_vt = vt;
	n->_vd = alloc(n->_vt->size);
	
	if (n->_vt->create)
		n->_vt->create(n);

	n->state = NODE_CREATED;
	
	return n;
}

int node_destroy(struct node *n)
{
	if (n->_vt->destroy)
		n->_vt->destroy(n);
	
	list_remove(&n->_vt->instances, n);

	free(n->_vd);
	free(n->_name);
	
	return 0;
}

int node_parse_list(struct list *list, config_setting_t *cfg, struct list *all) {
	const char *str;
	struct node *node;

	switch (config_setting_type(cfg)) {
		case CONFIG_TYPE_STRING:
			str = config_setting_get_string(cfg);
			if (str) {
				node = list_lookup(all, str);
				if (node)
					list_push(list, node);
				else
					cerror(cfg, "Unknown outgoing node '%s'", str);
			}
			else
				cerror(cfg, "Invalid outgoing node");
			break;

		case CONFIG_TYPE_ARRAY:
			for (int i = 0; i < config_setting_length(cfg); i++) {
				config_setting_t *elm = config_setting_get_elem(cfg, i);
				
				str = config_setting_get_string(elm);
				if (str) {
					node = list_lookup(all, str);
					if (!node)
						cerror(elm, "Unknown outgoing node '%s'", str);
					else if (node->_vt->write == NULL)
						cerror(cfg, "Output node '%s' is not supported as a sink.", node_name(node));

					list_push(list, node);
				}
				else
					cerror(cfg, "Invalid outgoing node");
			}
			break;

		default:
			cerror(cfg, "Invalid output node(s)");
	}

	return list_length(list);
}

int node_parse(struct node *n, config_setting_t *cfg)
{
	struct plugin *p;
	const char *type, *name;
	int ret;

	name = config_setting_name(cfg);
	
	if (!config_setting_lookup_string(cfg, "type", &type))
		cerror(cfg, "Missing node type");
	
	p = plugin_lookup(PLUGIN_TYPE_NODE, type);
	assert(&p->node == n->_vt);
	
	if (!config_setting_lookup_int(cfg, "vectorize", &n->vectorize))
		n->vectorize = 1;

	n->name = name;
	n->cfg = cfg;

	ret = n->_vt->parse ? n->_vt->parse(n, cfg) : 0;
	if (ret)
		cerror(cfg, "Failed to parse node '%s'", node_name(n));

	return ret;
}

int node_check(struct node *n)
{
	if (n->vectorize <= 0)
		error("Invalid `vectorize` value %d for node %s. Must be natural number!", n->vectorize, node_name(n));

	if (n->_vt->vectorize && n->_vt->vectorize < n->vectorize)
		error("Invalid value for `vectorize`. Node type requires a number smaller than %d!",
			n->_vt->vectorize);

	return 0;
}
