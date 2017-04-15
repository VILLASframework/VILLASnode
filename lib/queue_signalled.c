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

#include "queue_signalled.h"

int queue_signalled_init(struct queue_signalled *qs, size_t size, struct memtype *mem)
{
	int r = queue_init(&qs->q, size, mem);
	if (r < 0)
		return r;
	pthread_mutexattr_init(&qs->mtattr);
	pthread_mutexattr_setpshared(&qs->mtattr, PTHREAD_PROCESS_SHARED);
	pthread_condattr_init(&qs->readyattr);
	pthread_condattr_setpshared(&qs->readyattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&qs->mt, &qs->mtattr);
	pthread_cond_init(&qs->ready, &qs->readyattr);
	return 0;
}

int queue_signalled_destroy(struct queue_signalled *qs)
{
	int r = queue_destroy(&qs->q);
	if (r < 0)
		return r;
	pthread_cond_destroy(&qs->ready);
	pthread_mutex_destroy(&qs->mt);
	return 0;
}

int queue_signalled_push_many(struct queue_signalled *qs, void *ptr[], size_t cnt)
{
	int r = queue_push_many(&qs->q, ptr, cnt);
	if (r > 0) {
		pthread_mutex_lock(&qs->mt);
		pthread_cond_broadcast(&qs->ready);
		pthread_mutex_unlock(&qs->mt);
	}
	return r;
}

int queue_signalled_pull_many(struct queue_signalled *qs, void *ptr[], size_t cnt)
{
	/* Make sure that qs->mt is unlocked if this thread gets cancelled. */
	pthread_cleanup_push((void (*)(void*)) pthread_mutex_unlock, &qs->mt);
	pthread_mutex_lock(&qs->mt);
	pthread_cond_wait(&qs->ready, &qs->mt);
	pthread_mutex_unlock(&qs->mt);
	pthread_cleanup_pop(0);
	return queue_pull_many(&qs->q, ptr, cnt);
}
