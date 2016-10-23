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
 * @copyright 2014-2016, Institute for Automation of Complex Power Systems, EONERC
 *   This file is part of VILLASnode. All Rights Reserved. Proprietary and confidential.
 *   Unauthorized copying of this file, via any medium is strictly prohibited. 
 */
/**
 * @addtogroup hooks User-defined hook functions
 * @ingroup path
 * @{
 *********************************************************************************/
 
#ifndef _HOOKS_H_
#define _HOOKS_H_

#include <time.h>

#include "queue.h"
#include "list.h"

#define REGISTER_HOOK(nam, desc, prio, hist, fnc, typ)		\
__attribute__((constructor)) void __register_ ## fnc () {	\
	static struct hook h = {				\
		.name = nam,					\
		.description = desc,				\
		.priority = prio,				\
		.history = hist,				\
		.type = typ,					\
		.cb = fnc					\
	};							\
	list_push(&hooks, &h);					\
}

/* The configuration of hook parameters is done in "config.h" */

/* Forward declarations */
struct path;
struct hook;
struct sample;
struct settings;

/** This is a list of hooks which can be used in the configuration file. */
extern struct list hooks;

/** Optional parameters to hook callbacks */
struct hook_info {
	struct node *node;
	struct path *path;
	
	struct sample **smps;
	size_t cnt;

	struct list *paths;
	struct list *nodes;
	struct settings *settings;
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

enum hook_state {
	HOOK_DESTROYED,
	HOOK_INITIALIZED
};

/** The type of a hook defines when a hook will be exectuted. This is used as a bitmask. */
enum hook_type {
	HOOK_PATH_START		= 1 << 0,  /**< Called whenever a path is started; before threads are created. */
	HOOK_PATH_STOP		= 1 << 1,  /**< Called whenever a path is stopped; after threads are destoyed. */
	HOOK_PATH_RESTART	= 1 << 2,  /**< Called whenever a new simulation case is started. This is detected by a sequence no equal to zero. */

	HOOK_READ		= 1 << 3,  /**< Called for every single received samples. */
	HOOK_WRITE		= 1 << 4,  /**< Called for every single sample which will be sent. */

	HOOK_ASYNC		= 1 << 7,  /**< Called asynchronously with fixed rate (see path::rate). */
	HOOK_PERIODIC		= 1 << 8,  /**< Called periodically. Period is set by global 'stats' option in the configuration file. */

	HOOK_INIT		= 1 << 9,  /**< Called before path is started to parseHOOK_DESTROYs. */
	HOOK_DESTROY		= 1 << 10, /**< Called after path has been stopped to release memory allocated by HOOK_INIT */	

	HOOK_INTERNAL		= 1 << 11,  /**< Internal hooks are added to every path implicitely. */
	HOOK_PARSE		= 1 << 12, /**< Called for parsing hook arguments. */

	/** @{ Classes of hooks */
	/** Hooks which are using private data must allocate and free them propery. */	
	HOOK_STORAGE		= HOOK_INIT | HOOK_DESTROY,
	/** All path related actions */
	HOOK_PATH		= HOOK_PATH_START | HOOK_PATH_STOP | HOOK_PATH_RESTART,
	/** Hooks which are used to collect statistics. */
	HOOK_STATS		= HOOK_INTERNAL | HOOK_STORAGE | HOOK_PATH | HOOK_READ | HOOK_PERIODIC,

	/** All hooks */
	HOOK_ALL		= HOOK_INTERNAL - 1
	/** @} */
};

/** Descriptor for user defined hooks. See hooks[]. */
struct hook {
	const char *name;	/**< The unique name of this hook. This must be the first member! */
	const char *parameter;	/**< A parameter string for this hook. Can be used to configure the hook behaviour. */
	const char *description;/**< A short description of this hook function. */

	int priority;		/**< A priority to change the order of execution within one type of hook */
	int history;		/**< How many samples of history this hook requires. */
	enum hook_type type;	/**< The type of the hook as a bitfield */

	void *_vd;		/**< Private data for this hook. This pointer can be used to pass data between consecutive calls of the callback. */

	struct sample *last;
	struct sample *prev;

	hook_cb_t cb;		/**< The hook callback function as a function pointer. */
};

int hook_init(struct hook *h, struct list *nodes, struct list *paths, struct settings *settings);

void hook_destroy(struct hook *h);

int hook_copy(struct hook *h, struct hook *c);

/** Sort hook list according to the their priority. See hook::priority. */
int hooks_sort_priority(const void *a, const void *b);

/** Conditionally execute the hooks
 *
 * @param p A pointer to the path structure.
 * @param when Which type of hooks should be executed?
 * @param m An array to of (cnt) pointers to msgs.
 * @param cnt The size of the message array.
 * @retval 0 All registred hooks for the specified type have been executed successfully. 
 * @retval <0 On of the hook functions signalized, that the processing should be aborted; message should be skipped.
 */
int hook_run(struct path *p, struct sample *smps[], size_t cnt, int when);

/** Allocate & deallocate private memory per hook.
 *
 * Hooks which use this function must be flagged with HOOL_STORAGE.
 *
 * @param h A pointer to the hook structure.
 * @param when Which event cause the hook to be executed?
 * @param len The size of hook prvate memory allocation.
 * @return A pointer to the allocated memory region or NULL after it was released.
 */
void * hook_storage(struct hook *h, int when, size_t len);

int hook_print(struct hook *h, int when, struct hook_info *j);
int hook_ts(struct hook *h, int when, struct hook_info *j);
int hook_convert(struct hook *h, int when, struct hook_info *j);
int hook_decimate(struct hook *h, int when, struct hook_info *j);
int hook_skip_first(struct hook *h, int when, struct hook_info *j);

int hook_stats_send(struct hook *h, int when, struct hook_info *j);
int hook_stats(struct hook *h, int when, struct hook_info *j);

int hook_fix_ts(struct hook *h, int when, struct hook_info *j);
int hook_restart(struct hook *h, int when, struct hook_info *j);
int hook_drop(struct hook *h, int when, struct hook_info *j);

#endif /** _HOOKS_H_ @} */
