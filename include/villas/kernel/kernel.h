/** Linux kernel related functions.
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

/** @addtogroup fpga Kernel @{ */

#pragma once

#include <stdint.h>

/* Forward declarations */
struct version;

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

/** Get the version of the kernel. */
int kernel_get_version(struct version *v);

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

/** Get the size of a standard page in bytes. */
int kernel_get_page_size();

/** Get the size of a huge page in bytes. */
int kernel_get_hugepage_size();

/** Set SMP affinity of IRQ */
int kernel_irq_setaffinity(unsigned irq, uintmax_t new, uintmax_t *old);

/** @} */