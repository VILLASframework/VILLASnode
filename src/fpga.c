/** VILLASfpga utility for tests and benchmarks
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2017, Steffen Vogel
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
 **********************************************************************************/
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include <villas/log.h>
#include <villas/cfg.h>
#include <villas/timing.h>
#include <villas/utils.h>
#include <villas/nodes/fpga.h>
#include <villas/kernel/rt.h>
#include <villas/kernel/pci.h>
#include <villas/kernel/kernel.h>

#include <villas/fpga/card.h>

#include "config.h"

/* Declarations */
int fpga_benchmarks(int argc, char *argv[], struct fpga_card *c);

void usage()
{
	printf("Usage: villas-fpga CONFIGFILE CARD [OPTIONS]\n\n");
	printf("   Options:\n");
	printf("      -d    Set log level\n\n");

	print_copyright();

	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	int ret;
	
	struct cfg cfg;
	struct fpga_card *card;

	if (argc < 3)
		usage(argv[0]);

	/* Parse arguments */
	char c, *endptr;
	while ((c = getopt(argc-1, argv+1, "d:")) != -1) {
		switch (c) {
			case 'd':
				cfg.log.level = strtoul(optarg, &endptr, 10);
				break;	

			case '?':
			default:
				usage();
		}
	}

	info("Parsing configuration");
	cfg_parse(&cfg, argv[1]);
	
	info("Initialize logging system");
	log_init(&cfg.log, cfg.log.level, cfg.log.facilities);

	info("Initialize real-time system");
	rt_init(cfg.priority, cfg.affinity);
	
	info("Initialize memory system");
	memory_init();

	/* Initialize VILLASfpga card */
	ret = fpga_init(argc, argv, config_root_setting(&cfg.cfg));
	if (ret)
		error("Failed to initialize FPGA card");
	
	card = fpga_lookup_card(argv[2]);
	if (!card)
		error("FPGA card '%s' does not exist", argv[2]);

	fpga_card_dump(card);

	/* Run benchmarks */
	fpga_benchmarks(argc-optind-1, argv+optind+1, card);

	/* Shutdown */
	ret = fpga_deinit();
	if (ret)
		error("Failed to de-initialize FPGA card");
	
	cfg_destroy(&cfg);

	return 0;
}
