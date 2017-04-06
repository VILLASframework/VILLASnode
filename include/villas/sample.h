/** The internal datastructure for a sample of simulation data.
 *
 * @file
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

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdatomic.h>
#include <time.h>
#include <sys/types.h>

/* Forward declarations */
struct pool;

/** The length of a sample datastructure with \p values values in bytes. */
#define SAMPLE_LEN(len)	(sizeof(struct sample) + SAMPLE_DATA_LEN(len))

/** The length of a sample data portion of a sample datastructure with \p values values in bytes. */
#define SAMPLE_DATA_LEN(len)	((len) * sizeof(float))

/** The offset to the beginning of the data section. */
#define SAMPLE_DATA_OFFSET(smp)	((char *) (smp) + offsetof(struct sample, data))

enum sample_data_format {
	SAMPLE_DATA_FORMAT_FLOAT= 0,
	SAMPLE_DATA_FORMAT_INT	= 1
};

struct sample {
	int sequence;		/**< The sequence number of this sample. */
	int length;		/**< The number of values in sample::values which are valid. */
	int capacity;		/**< The number of values in sample::values for which memory is reserved. */
	
	atomic_int refcnt;	/**< Reference counter. */
	off_t pool_off;	/**< This sample is belong to this memory pool (relative pointer). */
	struct node *source;	/**< The node from which this sample originates. */

	/** All timestamps are seconds / nano seconds after 1.1.1970 UTC */
	struct {
		struct timespec origin;		/**< The point in time when this data was sampled. */
		struct timespec received;	/**< The point in time when this data was received. */
		struct timespec sent;		/**< The point in time this data was send for the last time. */
	} ts;
	
	uint64_t format;	/**< A long bitfield indicating the number representation of the first 64 values in sample::data[] */

	/** The values. */
	union {
		float    f;	/**< Floating point values. */
		uint32_t i;	/**< Integer values. */
	} data[];		/**< Data is in host endianess! */
};

/** Request \p cnt samples from memory pool \p p and initialize them.
 *  This will leave the reference count of the sample to zero.
 *  Use the sample_get() function to increase it. */
int sample_alloc(struct pool *p, struct sample *smps[], int cnt);

/** Release an array of samples back to their pools */
void sample_free(struct sample *smps[], int cnt);

/** Increase reference count of sample */
int sample_get(struct sample *s);

/** Decrease reference count and release memory if last reference was held. */
int sample_put(struct sample *s);

/** Set number representation for a single value of a sample. */
int sample_get_data_format(struct sample *s, int idx);

/** Get number representation for a single value of a sample. */
int sample_set_data_format(struct sample *s, int idx, enum sample_data_format fmt);
