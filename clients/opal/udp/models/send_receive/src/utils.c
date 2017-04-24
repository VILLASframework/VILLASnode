/** Configure scheduler.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @author Mathieu Dubé-Dallaire
 * @copyright 2003, OPAL-RT Technologies inc
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

#include <errno.h>

/* Define RTLAB before including OpalPrint.h for messages to be sent
 * to the OpalDisplay. Otherwise stdout will be used. */
#define RTLAB
#include "OpalPrint.h"

#include "config.h"
#include "utils.h"

#if defined(__QNXNTO__)
  #include <process.h>
  #include <sys/sched.h>
  #include <devctl.h>
  #include <sys/dcmd_chr.h>
#elif defined(__linux__)
  #define _GNU_SOURCE   1
  #include <sched.h>
#endif

int AssignProcToCpu0(void)
{
#ifdef __linux__
	int ret;
	cpu_set_t bindSet;
	CPU_ZERO(&bindSet);
	CPU_SET(0, &bindSet);

	/* Changing process cpu affinity */
	ret = sched_setaffinity(0, sizeof(cpu_set_t), &bindSet);
	if (ret != 0) {
		OpalPrint("Unable to bind the process to CPU 0. (sched_setaffinity errno %d)\n", errno);
		return EINVAL;
	}

	return EOK;
#endif	/* __linux__ */
}
