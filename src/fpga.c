/** VILLASfpga utility for tests and benchmarks
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2016, Steffen Vogel
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

#include "config.h"

/* Declarations */
int fpga_benchmarks(int argc, char *argv[], struct fpga *f);
int fpga_tests(int argc, char *argv[], struct fpga *f);

struct cfg settings;

void usage(char *name)
{
	printf("Usage: %s CONFIGFILE CMD [OPTIONS]\n", name);
	printf("   Commands:\n");
	printf("      tests      Test functionality of VILLASfpga card\n");
	printf("      benchmarks Do benchmarks\n\n");
	printf("   Options:\n");
	printf("      -d    Set log level\n\n");

	print_copyright();

	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	int ret;
	struct fpga *fpga;
	config_t config;

	enum {
		FPGA_TESTS,
		FPGA_BENCH
	} subcommand;

	if (argc < 3)
		usage(argv[0]);
	if      (strcmp(argv[2], "tests") == 0)
		subcommand = FPGA_TESTS;
	else if (strcmp(argv[2], "benchmarks") == 0)
		subcommand = FPGA_BENCH;
	else
		usage(argv[0]);

	/* Parse arguments */
	char c, *endptr;
	while ((c = getopt(argc-1, argv+1, "d:")) != -1) {
		switch (c) {
			case 'd':
				log_setlevel(strtoul(optarg, &endptr, 10), ~0);
				break;	

			case '?':
			default:
				usage(argv[0]);
		}
	}

	info("Parsing configuration");
	cfg_parse(argv[1], &config, &settings, NULL, NULL);
	
	info("Initialize real-time system");
	rt_init(settings.affinity, settings.priority);

	/* Initialize VILLASfpga card */
	config_setting_t *cfg_root = config_root_setting(&config);
	ret = fpga_init(argc, argv, cfg_root);
	if (ret)
		error("Failed to initialize FPGA card");
	
	fpga = fpga_get();
	fpga_dump(fpga);

	/* Start subcommand */
	switch (subcommand) {
		case FPGA_TESTS: fpga_tests(argc-optind-1, argv+optind+1, fpga);      break;
		case FPGA_BENCH: fpga_benchmarks(argc-optind-1, argv+optind+1, fpga); break;
	}

	/* Shutdown */
	ret = fpga_deinit(&fpga);
	if (ret)
		error("Failed to de-initialize fpga card");
	
	cfg_destroy(&config);

	return 0;
}
