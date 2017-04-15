/** Node-type for shared memory communication.
 *
 * @file
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

/**
 * @ingroup node
 * @addtogroup shmem Shared memory interface
 * @{
 */

#pragma once

#include "node.h"
#include "memory.h"
#include "pool.h"
#include "queue.h"
#include "shmem.h"

#define DEFAULT_SHMEM_QUEUESIZE 512

struct shmem {
	const char* name;		/**< Name of the shm object. */
	int sample_size;		/**< Number of data entries for each sample. */
	int insize, outsize;		/**< Size of ingoing and outgoing queue, respectively. */
	int cond_out;			/**< Whether to use a pthread_cond_t to signal if new samples are written to outqueue. */
	int cond_in;			/**< Whether to use a pthread_cond_t to signal if new samples are written to inqueue. */
	const char * const exec;	/**< External program to execute on start. */

	struct memtype *manager;	/**< Manager for the shared memory region. */
	int fd;				/**< Handle as returned by shm_open().*/
	void *base;			/**< Pointer to the shared memory region. */
	struct shmem_shared *shared;	/**< Shared datastructure. */
};

char *shmem_print(struct node *n);

int shmem_parse(struct node *n, config_setting_t *cfg);

int shmem_open(struct node *n);

int shmem_close(struct node *n);

int shmem_read(struct node *n, struct sample *smps[], unsigned cnt);

int shmem_write(struct node *n, struct sample *smps[], unsigned cnt);

/** @} */