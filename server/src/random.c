/** Generate random packages on stdout.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014-2015, Institute for Automation of Complex Power Systems, EONERC
 *   This file is part of S2SS. All Rights Reserved. Proprietary and confidential.
 *   Unauthorized copying of this file, via any medium is strictly prohibited.
 *
 * @addtogroup tools Test and debug tools
 * @{
 **********************************************************************************/

#include <unistd.h>

#include "config.h"
#include "utils.h"
#include "msg.h"
#include "timing.h"

#define CLOCKID	CLOCK_REALTIME

int main(int argc, char *argv[])
{
	if (argc < 3 || argc > 4) {
		printf("Usage: %s VALUES RATE [LIMIT]\n", argv[0]);
		printf("  VALUES is the number of values a message contains\n");
		printf("  RATE   how many messages per second\n");
		printf("  LIMIT  only send LIMIT messages\n\n");

		printf("Simulator2Simulator Server %s (built on %s %s)\n",
			BLU(VERSION), MAG(__DATE__), MAG(__TIME__));
		printf(" copyright 2014-2015, Institute for Automation of Complex Power Systems, EONERC\n");
		printf(" Steffen Vogel <StVogel@eonerc.rwth-aachen.de>\n");

		exit(EXIT_FAILURE);
	}

	struct msg m = MSG_INIT(atoi(argv[1]));
	double rate = atof(argv[2]);
	int limit = argc >= 4 ? atoi(argv[3]) : -1;

	/* Setup timer */
	struct itimerspec its = {
		.it_interval = time_from_double(1 / rate),
		.it_value = { 1, 0 }
	};

	int tfd = timerfd_create(CLOCK_REALTIME, 0);
	if (tfd < 0)
		serror("Failed to create timer");

	if (timerfd_settime(tfd, 0, &its, NULL))
		serror("Failed to start timer");

	/* Print header */
	fprintf(stderr, "# %-20s\t\t%s\n", "sec.nsec(seq)", "data[]");

	/* Block until 1/p->rate seconds elapsed */
	while (limit-- > 0 || argc < 4) {
		m.sequence += timerfd_wait(tfd);

		struct timespec ts = time_now();

		m.ts.sec    = ts.tv_sec;
		m.ts.nsec   = ts.tv_nsec;

		msg_random(&m);
		msg_fprint(stdout, &m, MSG_PRINT_ALL & ~MSG_PRINT_OFFSET, 0);

		fflush(stdout);
	}

	close(tfd);

	return 0;
}

/** @} */
