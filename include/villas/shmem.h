/** Shared-memory interface: The interface functions that the external program should use.
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

#pragma once

#include "pool.h"
#include "queue.h"
#include "queue_signalled.h"

/** A signalled queue or a regular (polling) queue, depending on the polling setting. */
union shmem_queue {
	struct queue q;
	struct queue_signalled qs;
};

/** The structure that actually resides in the shared memory. */
struct shmem_shared {
	size_t len;                       /**< Total size of the shared memory region.*/

	int polling;                      /**< Whether to use a pthread_cond_t to signal if new samples are written to incoming queue. */

	union shmem_queue in;             /**< Queue for samples passed from external program to node.*/
	union shmem_queue out;            /**< Queue for samples passed from node to external program.*/

	struct pool pool;                 /**< Pool for the samples in the queues. */

	pthread_barrier_t start_bar;      /**< Barrier for synchronizing the start of both programs. */
	pthread_barrierattr_t start_attr;
	atomic_size_t node_stopped;       /**< Set to 1 by VILLASNode if it is stopped/killed. */
	atomic_size_t ext_stopped;        /**< Set to 1 by the external program if it is stopped/killed. */
};

/** Open the shared memory object and retrieve / initialize the shared data structures.
 * @param[in] name Name of the POSIX shared memory object.
 * @param[inout] base_ptr The base address of the shared memory region is written to this pointer.
 * @retval A valid shmem_shared* on success, or NULL with errno indicating the error on failure.
 */
struct shmem_shared * shmem_shared_open(const char* name, void **base_ptr);

/** Close and destroy the shared memory object and related structures.
 * @param[shm] The shared memory structure.
 * @param[base] The base address as returned by shmem_shared_open.
 */
int shmem_shared_close(struct shmem_shared *shm, void *base);

/** Read samples from VILLASNode.
 * @param[shm] The shared memory structure.
 * @param[smps] An array where the pointers to the samples will be written. The samples
 * must be freed with sample_put after use.
 * @param[cnt] Number of samples to be read.
 */
int shmem_shared_read(struct shmem_shared *shm, struct sample *smps[], unsigned cnt);

/** Write samples to VILLASNode.
 * @param[shm] The shared memory structure.
 * @param[smps] The samples to be written. Must be allocated from shm->pool.
 * @param[cnt] Number of samples to write.
 */
int shmem_shared_write(struct shmem_shared *shm, struct sample *smps[], unsigned cnt);

/** Returns the total size of the shared memory region with the given size of
 * the input/output queues (in elements) and the given number of data elements
 * per struct sample. */
size_t shmem_total_size(int insize, int outsize, int sample_size);
