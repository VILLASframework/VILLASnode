/** Hook funktions
 *
 * Every path can register a hook function which is called for every received
 * message. This can be used to debug the data flow, get statistics
 * or alter the message.
 *
 * This file includes some examples.
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
 */
/**
 * @addtogroup hooks User-defined hook functions
 * @ingroup path
 * @{
 *********************************************************************************/
 
#pragma once

#include <time.h>
#include <string.h>

#include "queue.h"
#include "list.h"
#include "super_node.h"
#include "common.h"

/* Forward declarations */
struct path;
struct hook;
struct sample;
struct super_node;

/** Optional parameters to hook callbacks */
struct hook_info {
	struct path *path;
	
	struct list *nodes;
	struct list *paths;

	struct sample **samples;
	size_t count;
};

/** Callback type of hook function
 *
 * @param h The hook datastructure which contains parameter, name and private context for the hook.
 * @param when Provides the type of hook for which this occurence of the callback function was executed. See hook_type for possible values.
 * @param i The hook_info structure contains references to the current node, path or samples. Some fields of this structure can be NULL.
 * @retval 0 Success. Continue processing and forwarding the message.
 * @retval <0 Error. Drop the message.
 */
typedef int (*hook_cb_t)(struct hook *h, int when, struct hook_info *i);

/** Destructor callback for hook_storage()
 *
 * @param data A pointer to the data which should be destroyed.
 */
typedef int (*dtor_cb_t)(void *);

/** Constructor callback for hook_storage() */
typedef int (*ctor_cb_t)(void *);

enum hook_state {
	HOOK_DESTROYED,
	HOOK_INITIALIZED
};

/** The type of a hook defines when a hook will be exectuted. This is used as a bitmask. */
enum hook_when {
	HOOK_PATH_START		= 1 << 0,  /**< Called whenever a path is started; before threads are created. */
	HOOK_PATH_STOP		= 1 << 1,  /**< Called whenever a path is stopped; after threads are destoyed. */
	HOOK_PATH_RESTART	= 1 << 2,  /**< Called whenever a new simulation case is started. This is detected by a sequence no equal to zero. */

	HOOK_READ		= 1 << 3,  /**< Called for every single received samples. */
	HOOK_WRITE		= 1 << 4,  /**< Called for every single sample which will be sent. */

	HOOK_ASYNC		= 1 << 7,  /**< Called asynchronously with fixed rate (see path::rate). */
	HOOK_PERIODIC		= 1 << 8,  /**< Called periodically. Period is set by global 'stats' option in the configuration file. */

	HOOK_INIT		= 1 << 9,  /**< Called before path is started to parseHOOK_DESTROYs. */
	HOOK_DESTROY		= 1 << 10, /**< Called after path has been stopped to release memory allocated by HOOK_INIT */	

	HOOK_AUTO		= 1 << 11,  /**< Internal hooks are added to every path implicitely. */
	HOOK_PARSE		= 1 << 12, /**< Called for parsing hook arguments. */

	/** @{ Classes of hooks */
	/** Hooks which are using private data must allocate and free them propery. */	
	HOOK_STORAGE		= HOOK_INIT | HOOK_DESTROY,
	/** All path related actions */
	HOOK_PATH		= HOOK_PATH_START | HOOK_PATH_STOP | HOOK_PATH_RESTART
	/** @} */
};

struct hook_type {
	enum hook_when when;	/**< The type of the hook as a bitfield */
	hook_cb_t cb;		/**< The hook callback function as a function pointer. */
	int priority;		/**< Default priority of this hook type. */
};

/** Descriptor for user defined hooks. See hooks[]. */
struct hook {
	enum state state;

	struct sample *prev, *last;
	
	struct hook_type *_vt;	/**< C++ like Vtable pointer. */
	void *_vd;		/**< Private data for this hook. This pointer can be used to pass data between consecutive calls of the callback. */
	
	int priority;		/**< A priority to change the order of execution within one type of hook. */
	
	config_setting_t *cfg;
};

/** Save references to global nodes, paths and settings */
int hook_init(struct hook *h, struct hook_type *vt, struct super_node *sn);

/** Parse a single hook.
 *
 * A hook definition is composed of the hook name and optional parameters
 * seperated by a colon.
 *
 * Examples:
 *   "print:stdout"
 */
int hook_parse(struct hook *h, config_setting_t *cfg);

int hook_run(struct hook *h, int when, struct hook_info *i);

int hook_destroy(struct hook *h);

/** Compare two hook functions with their priority. Used by list_sort() */
int hook_cmp_priority(const void *a, const void *b);

/** Allocate & deallocate private memory per hook.
 *
 * Hooks which use this function must be flagged with HOOL_STORAGE.
 *
 * @param h A pointer to the hook structure.
 * @param when Which event cause the hook to be executed?
 * @param len The size of hook prvate memory allocation.
 * @return A pointer to the allocated memory region or NULL after it was released.
 */
void * hook_storage(struct hook *h, int when, size_t len, ctor_cb_t ctor, dtor_cb_t dtor);

/** Parses an object of hooks
 *
 * Example:
 *
 * {
 *    stats = {
 *       output = "stdout"
 *    },
 *    skip_first = {
 *       seconds = 10
 *    },
 *    hooks = [ "print" ]
 * }
 */
int hook_parse_list(struct list *list, config_setting_t *cfg, struct super_node *sn);