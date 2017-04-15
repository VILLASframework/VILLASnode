/** Wrapper around queue that uses POSIX CV's for signalling writes.
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

#include <pthread.h>

#include "queue.h"

/** Wrapper around queue that uses POSIX CV's for signalling writes. */
struct queue_signalled {
	struct queue q;			/**< Actual underlying queue. */
	pthread_cond_t ready;		/**< Condition variable to signal writes to the queue. */
	pthread_mutex_t mt;		/**< Mutex for ready. */
};

int queue_signalled_init(struct queue_signalled *qs, size_t size, struct memtype *mem);

int queue_signalled_destroy(struct queue_signalled *qs);

int queue_signalled_push_many(struct queue_signalled *qs, void *ptr[], size_t cnt);

int queue_signalled_pull_many(struct queue_signalled *qs, void *ptr[], size_t cnt);
