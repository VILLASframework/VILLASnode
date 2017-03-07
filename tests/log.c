/** Unit tests for log functions
 *
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

#include <criterion/criterion.h>
#include <criterion/parameterized.h>

#include "log.h"

struct param {
	char *expression;
	long expected;
};

static struct log l;

static void init()
{
	log_init(&l, V, LOG_ALL);
}

static void fini()
{
	log_destroy(&l);
}

ParameterizedTestParameters(log, facility_expression)
{
	static struct param params[] = {
		{ "all,!pool",		LOG_ALL & ~LOG_POOL },
		{ "pool,!pool",		LOG_POOL & ~LOG_POOL },
		{ "pool,nodes,!socket",	(LOG_POOL | LOG_NODES) & ~LOG_SOCKET },
		{ "kernel",		LOG_KERNEL },
		{ "ngsi",		LOG_NGSI },
		{ "all",		LOG_ALL },
		{ "!all",		0 },
		{ "",			0 }
	};
	
	return cr_make_param_array(struct param, params, ARRAY_LEN(params));
}

ParameterizedTest(struct param *p, log, facility_expression, .init = init, .fini = fini)
{
	log_set_facility_expression(&l, p->expression);
	
	cr_assert_eq(l.facilities, p->expected, "log.faciltities is %#lx not %#lx", l.facilities, p->expected);
}