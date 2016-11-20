/** Linux kernel related functions.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014-2016, Institute for Automation of Complex Power Systems, EONERC
 *   This file is part of VILLASnode. All Rights Reserved. Proprietary and confidential.
 *   Unauthorized copying of this file, via any medium is strictly prohibited.
 *********************************************************************************/

#ifndef _LINUX_H_
#define _LINUX_H_

#include <stdint.h>

#include "config.h"

//#include <sys/capability.h>

/** Check if current process has capability \p cap.
 *
 * @retval 0 If capabilty is present.
 * @retval <0 If capability is not present.
 */
//int kernel_check_cap(cap_value_t cap);

/** Get number of reserved hugepages. */
int kernel_get_nr_hugepages();

/** Set number of reserved hugepages. */
int kernel_set_nr_hugepages(int nr);

/** Checks for realtime (PREEMPT_RT) patched kernel.
 *
 * See https://rt.wiki.kernel.org
 *
 * @retval 0 Kernel is patched.
 * @reval <>0 Kernel is not patched.
 */
int kernel_is_rt();

/** Get kernel cmdline parameter
 *
 * See https://www.kernel.org/doc/Documentation/kernel-parameters.txt
 *
 * @param param The cmdline parameter to look for.
 * @param buf The string buffer to which the parameter value will be copied to.
 * @param len The length of the buffer \p value
 * @retval 0 Parameter \p key was found and value was copied to \p value
 * @reval <>0 Kernel was not booted with parameter \p key
 */
int kernel_get_cmdline_param(const char *param, char *buf, size_t len);

/** Check if kernel is version is sufficient
 *
 * @retval 0 Kernel version is sufficient.
 * @reval <>0 Kernel version is not sufficient.
 */
int kernel_has_version(int maj, int min);

/** Checks if a kernel module is loaded
 *
 * @param module the name of the module
 * @retval 0 Module is loaded.
 * @reval <>0 Module is not loaded.
 */
int kernel_module_loaded(const char *module);

/** Load kernel module via modprobe */
int kernel_module_load(const char *module);

/** Set parameter of loaded kernel module */
int kernel_module_set_param(const char *module, const char *param, const char *value);

/** Get cacheline size in bytes */
int kernel_get_cacheline_size();

/** Set SMP affinity of IRQ */
int kernel_irq_setaffinity(unsigned irq, uintmax_t new, uintmax_t *old);

#endif /* _LINUX_H_ */
