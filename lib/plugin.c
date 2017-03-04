/** Loadable / plugin support.
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

#include "plugin.h"

int plugin_init(struct plugin *p, char *name, char *path)
{
	p->name = strdup(name);
	p->path = strdup(path);

	p->state = PLUGIN_STATE_UNLOADED;
}

int plugin_load(struct plugin *p)
{
	p->handle = dlopen(p->path, RTLD_NOW);
	if (!p->path)
		return -1;
	
	p->state = PLUGIN_STATE_LOADED;
	
	return 0;
}

int plugin_unload(struct plugin *p)
{
	int ret;
	
	if (p->state != PLUGIN_STATE_LOADED)
		return -1;
	
	ret = dlclose(p->handle);
	if (ret)
		return -1;
	
	p->state = UNLOADED;
	
	return 0;
}

int plugin_destroy(struct plugin *p)
{
	if (p->state == PLUGIN_STATE_LOADED)
		plugin_unload(p);
	
	free(p->name);
	free(p->path);
	
	return 0;
}

int plugin_parse(struct plugin *p, config_setting_t *cfg)
{
	const char *path;

	path = config_setting_get_string(cfg);
	if (!path)
		cerror(cfg, "Setting 'plugin' must be a string.");
		
	handle = dlopen(path, RTLD_NOW);
	if (!handle)
		error("Failed to load plugin %s", dlerror());
		
	list_push_back(&cfg->plugins, handle);

	return 0;
}

struct plugin * plugin_lookup(enum plugin_types type, const char *name)
{
	list_foreach(struct plugin *l, &plugins) {
		if (l->type == type && strcmp(l->name, name) == 0)
			return l;
	}
	
	return NULL;
}
