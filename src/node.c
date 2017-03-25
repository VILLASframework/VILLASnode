/** Main routine.
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

#include <stdlib.h>
#include <unistd.h>

#include <villas/utils.h>
#include <villas/super_node.h>
#include <villas/memory.h>
#include <villas/node.h>
#include <villas/path.h>
#include <villas/api.h>
#include <villas/web.h>
#include <villas/timing.h>
#include <villas/plugin.h>
#include <villas/kernel/kernel.h>
#include <villas/kernel/rt.h>
#include <villas/hook.h>
#include <villas/stats.h>

#ifdef ENABLE_OPAL_ASYNC
  #include <villas/nodes/opal.h>
#endif

struct super_node sn;

static void quit(int signal, siginfo_t *sinfo, void *ctx)
{
	super_node_stop(&sn);
	super_node_destroy(&sn);

	info(GRN("Goodbye!"));

	_exit(EXIT_SUCCESS);
}

static void usage()
{
	printf("Usage: villas-node [CONFIG]\n");
	printf("  CONFIG is the path to an optional configuration file\n");
	printf("         if omitted, VILLASnode will start without a configuration\n");
	printf("         and wait for provisioning over the web interface.\n\n");
#ifdef ENABLE_OPAL_ASYNC
	printf("Usage: villas-node OPAL_ASYNC_SHMEM_NAME OPAL_ASYNC_SHMEM_SIZE OPAL_PRINT_SHMEM_NAME\n");
	printf("  This type of invocation is used by OPAL-RT Asynchronous processes.\n");
	printf("  See in the RT-LAB User Guide for more information.\n\n");
#endif
	printf("Supported node types:\n");
	plugin_dump(PLUGIN_TYPE_NODE);
	printf("\n");
	
	printf("Supported hooks:\n");
	plugin_dump(PLUGIN_TYPE_HOOK);
	printf("\n");
	
	printf("Supported API commands:\n");
	plugin_dump(PLUGIN_TYPE_API);
	printf("\n");

	print_copyright();

	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	/* Check arguments */
#ifdef ENABLE_OPAL_ASYNC
	if (argc != 4)
		usage(argv[0]);
	
	char *uri = "opal-shmem.conf";
#else
	if (argc == 2) {
		if (!strcmp(argv[1], "-h") ||
		    !strcmp(argv[1], "--help"))
			    usage();
	}
	else if (argc > 2)
		usage();
#endif

	super_node_init(&sn);

	info("This is VILLASnode %s (built on %s, %s)", BLD(YEL(VERSION)),
		BLD(MAG(__DATE__)), BLD(MAG(__TIME__)));

	/* Checks system requirements*/
	if (kernel_has_version(KERNEL_VERSION_MAJ, KERNEL_VERSION_MIN))
		error("Your kernel version is to old: required >= %u.%u", KERNEL_VERSION_MAJ, KERNEL_VERSION_MIN);

	signals_init(quit);

	super_node_parse_cli(&sn, argc, argv);
	super_node_check(&sn);
	super_node_start(&sn);

	if (sn.stats > 0)
		stats_print_header();

	struct timespec now, last = time_now();

	/* Run! Until signal handler is invoked */
	while (1) {
		now = time_now();
		if (sn.stats > 0 && time_delta(&last, &now) > sn.stats) {
			for (size_t i = 0; i < list_length(&sn.paths); i++) {
				struct path *p = list_at(&sn.paths, i);

				for (size_t j = 0; j < list_length(&p->hooks); j++) {
					struct hook *h = list_at(&p->hooks, j);

					hook_run(h, HOOK_PERIODIC, NULL, 0);
				}
			}

			last = time_now();
		}

		web_service(&sn.web); /** @todo Maybe we should move this to another thread */
	}

	return 0;
}
