/** Convert old style config to new JSON format.
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

#include <jansson.h>
#include <libconfig.h>

#include <villas/json.h>
#include <villas/utils.h>

void usage()
{
	printf("Usage: conf2json < input.conf > output.json\n\n");

	print_copyright();
}

int main(int argc, char *argv[])
{
	int ret;
	config_t cfg;
	config_setting_t *cfg_root;
	json_t *json;
	
	if (argc != 1) {
		usage();
		exit(EXIT_FAILURE);
	}
	
	config_init(&cfg);
	
	ret = config_read(&cfg, stdin);
	if (ret != CONFIG_TRUE)
		return ret;
	
	cfg_root = config_root_setting(&cfg);
	
	json = config_to_json(cfg_root);
	if (!json)
		return -1;
	
	ret = json_dumpf(json, stdout, JSON_INDENT(2)); fflush(stdout);
	if (ret)
		return ret;
	
	json_decref(json);
	config_destroy(&cfg);
	
	return 0;
}
