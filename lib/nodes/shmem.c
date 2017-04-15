/** Node-type for shared memory communication.
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

#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "kernel/kernel.h"
#include "log.h"
#include "nodes/shmem.h"
#include "plugin.h"
#include "shmem.h"
#include "utils.h"

int shmem_parse(struct node *n, config_setting_t *cfg) {
	struct shmem *shm = n->_vd;

	if (!config_setting_lookup_string(cfg, "name", &shm->name))
		cerror(cfg, "Missing shm object name");

	if (!config_setting_lookup_int(cfg, "insize", &shm->insize))
		shm->insize = DEFAULT_SHMEM_QUEUESIZE;
	if (!config_setting_lookup_int(cfg, "outsize", &shm->outsize))
		shm->outsize = DEFAULT_SHMEM_QUEUESIZE;
	if (!config_setting_lookup_int(cfg, "sample_size", &shm->sample_size))
		cerror(cfg, "Missing sample size setting");
	if (!config_setting_lookup_bool(cfg, "cond_out", &shm->cond_out))
		shm->cond_out = false;
	if (!config_setting_lookup_bool(cfg, "cond_in", &shm->cond_in))
		shm->cond_in = false;
	config_setting_t *exec_setting = config_setting_lookup(cfg, "exec");
	if (!exec_setting) {
		shm->exec = NULL;
	} else {
		if (!config_setting_is_array(exec_setting))
			cerror(exec_setting, "Invalid format for exec");
		shm->exec = malloc(sizeof(char*) * (config_setting_length(exec_setting) + 1));
		int i;
		for (i = 0; i < config_setting_length(exec_setting); i++) {
			const char* elm = config_setting_get_string_elem(exec_setting, i);
			if (!elm)
				cerror(exec_setting, "Invalid format for exec");
			shm->exec[i] = strdup(elm);
		}
		shm->exec[i] = NULL;
	}

	return 0;
}

int shmem_open(struct node *n) {
	struct shmem *shm = n->_vd;

	int r = shm_open(shm->name, O_RDWR|O_CREAT, 0600);
	if (r < 0)
		serror("Opening shared memory object failed");

	shm->fd = r;
	size_t len = shmem_total_size(shm->insize, shm->outsize, shm->sample_size);
	if (ftruncate(shm->fd, len) < 0)
		serror("Setting size of shared memory object failed");
	shm->base = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, shm->fd, 0);

	if (shm->base == MAP_FAILED)
		serror("Mapping shared memory failed");

	shm->manager = memtype_managed_init(shm->base, len);
	shm->shared = memory_alloc(shm->manager, sizeof(struct shmem_shared));
	if (!shm->shared)
		error("Shm shared struct allocation failed (not enough memory?)");
	memset(shm->shared, 0, sizeof(struct shmem_shared));
	shm->shared->len = len;
	shm->shared->cond_in = shm->cond_in;
	shm->shared->cond_out = shm->cond_out;
	if (shm->cond_in) {
		if (queue_signalled_init(&shm->shared->in.qs, shm->insize, shm->manager) < 0)
			error("Shm queue allocation failed (not enough memory?)");
	} else {
		if (queue_init(&shm->shared->in.q, shm->insize, shm->manager) < 0)
			error("Shm queue allocation failed (not enough memory?)");
	}
	if (shm->cond_out) {
		if (queue_signalled_init(&shm->shared->out.qs, shm->outsize, shm->manager) < 0)
			error("Shm queue allocation failed (not enough memory?)");
	} else {
		if (queue_init(&shm->shared->out.q, shm->outsize, shm->manager) < 0)
			error("Shm queue allocation failed (not enough memory?)");
	}
	if (pool_init(&shm->shared->pool, shm->insize+shm->outsize, SAMPLE_LEN(shm->sample_size), shm->manager) < 0)
		error("Shm pool allocation failed (not enough memory?)");

	pthread_barrierattr_init(&shm->shared->start_attr);
	pthread_barrierattr_setpshared(&shm->shared->start_attr, PTHREAD_PROCESS_SHARED);
	pthread_barrier_init(&shm->shared->start_bar, &shm->shared->start_attr, 2);

	if (shm->exec && !spawn(shm->exec[0], shm->exec))
		serror("Failed to spawn external program");

	pthread_barrier_wait(&shm->shared->start_bar);
	return 0;
}

int shmem_close(struct node *n) {
	struct shmem* shm = n->_vd;
	size_t len = shm->shared->len;
	atomic_store_explicit(&shm->shared->node_stopped, 1, memory_order_relaxed);
	if (shm->cond_out) {
		pthread_mutex_lock(&shm->shared->out.qs.mt);
		pthread_cond_broadcast(&shm->shared->out.qs.ready);
		pthread_mutex_unlock(&shm->shared->out.qs.mt);
	}
	/* Don't destroy the data structures yet, since the other process might
	 * still be using them. Once both processes are done and have unmapped the
	 * memory, it will be freed anyway. */
	int r = munmap(shm->base, len);
	if (r != 0)
		return r;
	return shm_unlink(shm->name);
}

int shmem_read(struct node *n, struct sample *smps[], unsigned cnt) {
	struct shmem *shm = n->_vd;
	int r;
	if (shm->cond_in)
		r = queue_signalled_pull_many(&shm->shared->in.qs, (void**) smps, cnt);
	else
		r = queue_pull_many(&shm->shared->in.q, (void**) smps, cnt);
	if (!r && atomic_load_explicit(&shm->shared->ext_stopped, memory_order_relaxed))
		return -1;
	return r;
}

int shmem_write(struct node *n, struct sample *smps[], unsigned cnt) {
	struct shmem *shm = n->_vd;

	/* Samples need to be copied to the shared pool first */
	struct sample *shared_smps[cnt];
	int avail = sample_alloc(&shm->shared->pool, shared_smps, cnt);
	if (avail != cnt)
		warn("Pool underrun for shmem node %s", shm->name);
	for (int i = 0; i < avail; i++) {
		/* Since the node isn't in shared memory, the source can't be accessed */
		shared_smps[i]->source = NULL;
		shared_smps[i]->sequence = smps[i]->sequence;
		shared_smps[i]->ts = smps[i]->ts;
		int len = MIN(smps[i]->length, shared_smps[i]->capacity);
		if (len != smps[i]->length)
			warn("Losing data because of sample capacity mismatch in shmem node %s", shm->name);
		memcpy(shared_smps[i]->data, smps[i]->data, len*sizeof(smps[0]->data[0]));
		shared_smps[i]->length = len;
		sample_get(shared_smps[i]);
	}
	int pushed;
	if (atomic_load_explicit(&shm->shared->ext_stopped, memory_order_relaxed)) {
		for (int i = 0; i < avail; i++)
			sample_put(shared_smps[i]);
		return -1;
	}
	if (shm->cond_out)
		pushed = queue_signalled_push_many(&shm->shared->out.qs, (void**) shared_smps, avail);
	else
		pushed = queue_push_many(&shm->shared->out.q, (void**) shared_smps, avail);
	if (pushed != avail)
		warn("Outqueue overrun for shmem node %s", shm->name);
	return pushed;
}

char *shmem_print(struct node *n) {
	struct shmem *shm = n->_vd;
	char *buf = NULL;
	strcatf(&buf, "name=%s, insize=%d, outsize=%d, sample_size=%d", shm->name, shm->insize, shm->outsize, shm->sample_size);
	return buf;
};

static struct plugin p = {
	.name = "shmem",
	.description = "use POSIX shared memory to interface with other programs",
	.type = PLUGIN_TYPE_NODE,
	.node = {
		.vectorize = 1,
		.size = sizeof(struct shmem),
		.parse = shmem_parse,
		.print = shmem_print,
		.start = shmem_open,
		.stop = shmem_close,
		.read = shmem_read,
		.write = shmem_write,
		.instances = LIST_INIT(),
	}
};

REGISTER_PLUGIN(&p)
