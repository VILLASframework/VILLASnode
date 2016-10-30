/** Helper to run a list of shell commands as Criterion tests
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014-2016, Institute for Automation of Complex Power Systems, EONERC
 *   This file is part of VILLASnode. All Rights Reserved. Proprietary and confidential.
 *   Unauthorized copying of this file, via any medium is strictly prohibited.
 *********************************************************************************/

#include <criterion/criterion.h>
#include <criterion/parameterized.h>

static struct param {
	char *cmd;
	char *test;
	int ret;
	enum {
		JQ = (1 << 0)
	} flags;
} tests[] = {
	{ "echo", NULL, 0 },
	{ "false", NULL, 1 },
	{ "true", NULL, 1 },
	{ "echo { \"test\" : 55 }", ".test == 55", JQ }
};

ParameterizedTestParameters(integration, shell)
{	
	return cr_make_param_array(struct param, tests, ARRAY_LEN(tests));
}

ParameterizedTest(struct param *p, integration, shell)
{
	FILE *p;
	int ret;
	char *cmd __attribute__ ((__cleanup__(free)));

	if (p.flags & JQ)
		asprintf(&cmd, "%s | jq -e '%s'", p->cmd, p->test);
	else
		cmd = strdup(p->cmd);
	
	cr_log_info("Running cmd: %s", cmd);

	ret = system(p->cmd);	
	cr_assert_eq(ret, 0);
	
	free(cmd);
}
