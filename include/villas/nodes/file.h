/** Node type: File
 *
 * This file implements the file type for nodes.
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

/**
 * @addtogroup file File-IO node type
 * @ingroup node
 * @{
 */

#pragma once

#include "advio.h"
#include "node.h"

#define FILE_MAX_PATHLEN	512

enum {
	FILE_READ,
	FILE_WRITE
};

struct file {
	struct file_direction {
		AFILE *handle;		/**< libc: stdio file handle */

		const char *mode;	/**< libc: fopen() mode */
		const char *fmt;	/**< Format string for file name. */

		char *uri;		/**< Real file name */
		
		int chunk;		/**< Current chunk number. */
		int split;		/**< Split file every file::split mega bytes. */
	} read, write;

	enum read_epoch_mode {
		EPOCH_DIRECT,
		EPOCH_WAIT,
		EPOCH_RELATIVE,
		EPOCH_ABSOLUTE
	} read_epoch_mode;		/**< Specifies how file::offset is calculated. */

	struct timespec read_first;	/**< The first timestamp in the file file::{read,write}::uri */
	struct timespec read_epoch;	/**< The epoch timestamp from the configuration. */
	struct timespec read_offset;	/**< An offset between the timestamp in the input file and the current time */

	int read_timer;			/**< Timer file descriptor. Blocks until 1 / rate seconds are elapsed. */
	double read_rate;		/**< The read rate. */
};

/** @see node_vtable::print */
char * file_print(struct node *n);

/** @see node_vtable::parse */
int file_parse(struct node *n, config_setting_t *cfg);

/** @see node_vtable::open */
int file_start(struct node *n);

/** @see node_vtable::close */
int file_stop(struct node *n);

/** @see node_vtable::read */
int file_read(struct node *n, struct sample *smps[], unsigned cnt);

/** @see node_vtable::write */
int file_write(struct node *n, struct sample *smps[], unsigned cnt);

/** @} */
