/** A simple example hook function which can be loaded as a plugin.
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

#include <stddef.h>
#include <villas/log.h>
#include <villas/plugin.h>

struct hook;
struct path;
struct sample;

static int hook_example(struct path *p, struct hook *h, int when, struct sample *smps[], size_t cnt)
{
	info("Hello world from example hook!");
	
	return 0;
}

static struct plugin p = {
	.name		= "example",
	.description	= "This is just a simple example hook",
	.type		= LOADABLE_TYPE_HOOK,
	.hook		= {
		.priority = 99,
		.history = 0,
		.cb	= hook_example,
		.type	= HOOK_PATH_START
	}
};

REGISTER_PLUGIN(&p)