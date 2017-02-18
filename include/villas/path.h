/** Message paths
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2016, Institute for Automation of Complex Power Systems, EONERC
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
 */
/** A path connects one input node to multiple output nodes (1-to-n).
 *
 * @addtogroup path Path
 * @{
 *********************************************************************************/

#pragma once

#include <pthread.h>
#include <libconfig.h>

#include "list.h"
#include "config.h"
#include "hist.h"
#include "node.h"
#include "msg.h"
#include "hooks.h"
#include "queue.h"
#include "pool.h"

/** The datastructure for a path.
 *
 * @todo Add support for multiple outgoing nodes
 */
struct path
{
	enum {
		PATH_INVALID,		/**< Path is invalid. */
		PATH_CREATED,		/**< Path has been created. */
		PATH_RUNNING,		/**< Path is currently running. */
		PATH_STOPPED		/**< Path has been stopped. */
	} state;			/**< Path state */
	
	struct node *in;		/**< Pointer to the incoming node */
	
	struct queue queue;	/**< A ring buffer for all received messages (unmodified) */
	struct pool pool;		/**< Memory pool for messages / samples. */

	struct list destinations;	/**< List of all outgoing nodes */
	struct list hooks;		/**< List of function pointers to hooks */

	int samplelen;			/**< Maximum number of values per sample for this path. */
	int queuelen;			/**< Size of sample queue for this path. */
	int enabled;			/**< Is this path enabled */
	int tfd;			/**< Timer file descriptor for fixed rate sending */
	double rate;			/**< Send messages with a fixed rate over this path */

	pthread_t recv_tid;		/**< The thread id for this path */
	pthread_t sent_tid;		/**< A second thread id for fixed rate sending thread */

	config_setting_t *cfg;		/**< A pointer to the libconfig object which instantiated this path */
	
	char *_name;			/**< Singleton: A string which is used to print this path to screen. */
	
	/** The following fields are mostly managed by hook_ functions @{ */
	
	struct {
		struct hist owd;	/**< Histogram for one-way-delay (OWD) of received messages */
		struct hist gap_msg;	/**< Histogram for inter message timestamps (as sent by remote) */
		struct hist gap_recv;	/**< Histogram for inter message arrival time (as seen by this instance) */
		struct hist gap_seq;	/**< Histogram of sequence number displacement of received messages */
	} hist;

	/* Statistic counters */
	uintmax_t invalid;		/**< Counter for invalid messages */
	uintmax_t skipped;		/**< Counter for skipped messages due to hooks */
	uintmax_t dropped;		/**< Counter for dropped messages due to reordering */
	uintmax_t overrun;		/**< Counter of overruns for fixed-rate sending */
	
	/** @} */
};

/** Create a path by allocating dynamic memory. */
void path_init(struct path *p);

/** Destroy path by freeing dynamically allocated memory.
 *
 * @param i A pointer to the path structure.
 */
int path_destroy(struct path *p);

/** Initialize  pool queue and hooks.
 *
 * Should be called after path_init() and before path_start().
 */
int path_prepare(struct path *p);

/** Start a path.
 *
 * Start a new pthread for receiving/sending messages over this path.
 *
 * @param p A pointer to the path structure.
 * @retval 0 Success. Everything went well.
 * @retval <0 Error. Something went wrong.
 */
int path_start(struct path *p);

/** Stop a path.
 *
 * @param p A pointer to the path structure.
 * @retval 0 Success. Everything went well.
 * @retval <0 Error. Something went wrong.
 */
int path_stop(struct path *p);

/** Show some basic statistics for a path.
 *
 * @param p A pointer to the path structure.
 */
void path_print_stats(struct path *p);

/** Fills the provided buffer with a string representation of the path.
 *
 * Format: source => [ dest1 dest2 dest3 ]
 *
 * @param p A pointer to the path structure.
 * @return A pointer to a string containing a textual representation of the path.
 */
const char * path_name(struct path *p);

/** Check if node is used as source or destination of a path. */
int path_uses_node(struct path *p, struct node *n);

/** @} */