/** Generate random packages on stdout.
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
 *
 * @addtogroup tools Test and debug tools
 * @{
 **********************************************************************************/

#include <unistd.h>
#include <math.h>
#include <string.h>

#include <villas/utils.h>
#include <villas/sample.h>
#include <villas/sample_io.h>
#include <villas/timing.h>

#include "config.h"

#define CLOCKID	CLOCK_REALTIME

enum SIGNAL_TYPE {
	TYPE_RANDOM,
	TYPE_SINE,
	TYPE_SQUARE,
	TYPE_TRIANGLE,
	TYPE_RAMP,
	TYPE_MIXED
};

void usage()
{
	printf("Usage: villas-signal SIGNAL [OPTIONS]\n");
	printf("  SIGNAL   is on of: 'mixed', 'random', 'sine', 'triangle', 'square', 'ramp'\n");
	printf("  -d LVL   set debug level\n");
	printf("  -v NUM   specifies how many values a message should contain\n");
	printf("  -r HZ    how many messages per second\n");
	printf("  -n       non real-time mode. do not throttle output.\n");
	printf("  -f HZ    the frequency of the signal\n");
	printf("  -a FLT   the amplitude\n");
	printf("  -D FLT   the standard deviation for 'random' signals\n");
	printf("  -l NUM   only send LIMIT messages and stop\n\n");

	print_copyright();
}

int main(int argc, char *argv[])
{
	struct log log;
	struct timespec start, now;
	
	enum {
		MODE_RT,
		MODE_NON_RT
	} mode = MODE_RT;

	/* Some default values */
	double rate = 10;
	double freq = 1;
	double ampl = 1;
	double stddev = 0.02;
	double running;
	int type = TYPE_MIXED;
	int values = 1;
	int limit = -1;	
	int counter, tfd, steps, level = V;

	if (argc < 2) {
		usage();
		exit(EXIT_FAILURE);
	}
		
	/* Parse signal type */
	if      (!strcmp(argv[1], "random"))
		type = TYPE_RANDOM;
	else if (!strcmp(argv[1], "sine"))
		type = TYPE_SINE;
	else if (!strcmp(argv[1], "square"))
		type = TYPE_SQUARE;
	else if (!strcmp(argv[1], "triangle"))
		type = TYPE_TRIANGLE;
	else if (!strcmp(argv[1], "ramp"))
		type = TYPE_RAMP;
	else if (!strcmp(argv[1], "mixed"))
		type = TYPE_MIXED;
	
	/* Parse optional command line arguments */
	char c, *endptr;
	while ((c = getopt(argc-1, argv+1, "hv:r:f:l:a:D:d:n")) != -1) {
		switch (c) {
			case 'd':
				level = strtoul(optarg, &endptr, 10);
				goto check;
			case 'l':
				limit = strtoul(optarg, &endptr, 10);
				goto check;
			case 'v':
				values = strtoul(optarg, &endptr, 10);
				goto check;
			case 'r':
				rate   = strtof(optarg, &endptr);
				goto check;
			case 'f':
				freq   = strtof(optarg, &endptr);
				goto check;
			case 'a':
				ampl   = strtof(optarg, &endptr);
				goto check;
			case 'D':
				stddev = strtof(optarg, &endptr);
				goto check;
			case 'n':
				mode = MODE_NON_RT;
				break;
			case 'h':
			case '?':
				usage();
				exit(c == '?' ? EXIT_FAILURE : EXIT_SUCCESS);
		}
		
		continue;
		
check:		if (optarg == endptr)
			error("Failed to parse parse option argument '-%c %s'", c, optarg);
	}
	
	log_init(&log, level, LOG_ALL);

	/* Allocate memory for message buffer */
	struct sample *s = alloc(SAMPLE_LEN(values));

	/* Print header */
	printf("# VILLASnode signal params: type=%s, values=%u, rate=%f, limit=%d, amplitude=%f, freq=%f\n",
		argv[1], values, rate, limit, ampl, freq);
	printf("# %-20s\t\t%s\n", "sec.nsec(seq)", "data[]");

	/* Setup timer */
	if (mode == MODE_RT) {
		tfd = timerfd_create_rate(rate);
		if (tfd < 0)
			serror("Failed to create timer");
	}
	else
		tfd = -1;

	start = time_now();

	counter = 0;
	while (limit < 0 || counter < limit) {
		if (mode == MODE_RT) {
			now = time_now();
			running = time_delta(&start, &now);
		}
		else {
			struct timespec offset;
			
			running = counter * 1.0 / rate;
			offset = time_from_double(running);
			
			now = time_add(&start, &offset);
		}

		s->ts.origin = now;
		s->sequence  = counter;
		s->length    = values;

		for (int i = 0; i < values; i++) {
			int rtype = (type != TYPE_MIXED) ? type : i % 4;			
			switch (rtype) {
				case TYPE_RANDOM:   s->data[i].f += box_muller(0, stddev); 					break;
				case TYPE_SINE:	    s->data[i].f = ampl *        sin(running * freq * 2 * M_PI);		break;
				case TYPE_TRIANGLE: s->data[i].f = ampl * (fabs(fmod(running * freq, 1) - .5) - 0.25) * 4;	break;
				case TYPE_SQUARE:   s->data[i].f = ampl * (    (fmod(running * freq, 1) < .5) ? -1 : 1);	break;
				case TYPE_RAMP:     s->data[i].f = fmod(counter, rate / freq); /** @todo send as integer? */	break;
			}
		}
			
		sample_io_villas_fprint(stdout, s, SAMPLE_IO_ALL & ~SAMPLE_IO_OFFSET);
		fflush(stdout);
		
		/* Throttle output if desired */
		if (mode == MODE_RT) {
			/* Block until 1/p->rate seconds elapsed */
			steps = timerfd_wait(tfd);
			if (steps > 1)
				warn("Missed steps: %u", steps);
			
			counter += steps;
		}
		else
			counter += 1;
	}

	if (mode == MODE_RT)
		close(tfd);

	free(s);

	return 0;
}

/** @} */
