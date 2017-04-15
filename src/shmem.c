/** Test "client" for the shared memory interface.
 *
 * Waits on the incoming queue, prints received samples and writes them
 * back to the other queue.
 *
 * @author Georg Martin Reinke <georg.reinke@rwth-aachen.de>
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

#include "config.h"
#include "log.h"
#include "node.h"
#include "nodes/shmem.h"
#include "pool.h"
#include "queue_signalled.h"
#include "sample.h"
#include "shmem.h"
#include "utils.h"

#include <string.h>

#define VECTORIZE 8

void *base;
struct shmem_shared *shared;

void usage()
{
	printf("Usage: villas-shmem SHM_NAME\n");
	printf("  SHMNAME name of the shared memory object\n");
}

void quit(int sig)
{
	shmem_shared_close(shared, base);
	exit(1);
}

int main(int argc, char* argv[])
{
	struct log log;
	
	log_init(&log, V, LOG_ALL);
	log_start(&log);
	
	int readcnt, writecnt, avail;

	if (argc != 2) {
		usage();
		return 1;
	}

	shared = shmem_shared_open(argv[1], &base);
	if (!shared)
		serror("Failed to open shmem interface");

	signal(SIGINT, quit);
	signal(SIGTERM, quit);
	struct sample *insmps[VECTORIZE], *outsmps[VECTORIZE];
	while (1) {

		readcnt = shmem_shared_read(shared, insmps, VECTORIZE);
		if (readcnt == -1) {
			printf("Node stopped, exiting\n");
			break;
		}
		
		avail = sample_alloc(&shared->pool, outsmps, readcnt);
		if (avail < readcnt)
			warn("Pool underrun: %d / %d\n", avail, readcnt);

		for (int i = 0; i < readcnt; i++)
			sample_io_villas_fprint(stdout, insmps[i], SAMPLE_IO_ALL);

		for (int i = 0; i < avail; i++) {
			outsmps[i]->sequence = insmps[i]->sequence;
			outsmps[i]->ts = insmps[i]->ts;

			int len = MIN(insmps[i]->length, outsmps[i]->capacity);
			memcpy(outsmps[i]->data, insmps[i]->data, SAMPLE_DATA_LEN(len));

			outsmps[i]->length = len;
		}

		for (int i = 0; i < readcnt; i++)
			sample_put(insmps[i]);

		writecnt = shmem_shared_write(shared, outsmps, avail);
		if (writecnt < avail)
			warn("Short write");
		
		info("Read / Write: %d / %d", readcnt, writecnt);
	}
}
