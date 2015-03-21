/** Send messages from stdin to server.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014, Institute for Automation of Complex Power Systems, EONERC
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "config.h"
#include "cfg.h"
#include "utils.h"
#include "node.h"
#include "msg.h"
#include "socket.h"

static struct settings set;
static struct msg  msg = MSG_INIT(0);
static struct node *node;
extern struct list nodes;

void quit(int sig, siginfo_t *si, void *ptr)
{
	node_stop(node);
	exit(EXIT_SUCCESS);
}

void usage(char *name)
{
	printf("Usage: %s [-r] CONFIG NODE\n", name);
	printf("  -r      swap local / remote address of socket based nodes)\n");
	printf("  CONFIG  path to a configuration file\n");
	printf("  NODE    name of the node which shoud be used\n");
	printf("Simulator2Simulator Server %s (built on %s %s)\n",
		BLU(VERSION), MAG(__DATE__), MAG(__TIME__));
	printf(" Copyright 2014, Institute for Automation of Complex Power Systems, EONERC\n");
	printf("   Steffen Vogel <stvogel@eonerc.rwth-aachen.de>\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	char c;
	int reverse = 0;

	struct config_t config;

	while ((c = getopt(argc, argv, "hr")) != -1) {
		switch (c) {
			case 'r': reverse = 1; break;
			case 'h':
			case '?': usage(argv[0]);
		}
	}
	
	if (argc - optind != 2)
		usage(argv[0]);

	/* Setup signals */
	struct sigaction sa_quit = {
		.sa_flags = SA_SIGINFO,
		.sa_sigaction = quit
	};

	sigemptyset(&sa_quit.sa_mask);
	sigaction(SIGTERM, &sa_quit, NULL);
	sigaction(SIGINT, &sa_quit, NULL);

	config_init(&config);
	config_parse(argv[optind], &config, &set, &nodes, NULL);
	
	node = node_lookup_name(argv[optind+1], &nodes);
	if (!node)
		error("There's no node with the name '%s'", argv[optind+1]);
	
	node->refcnt++;
	
	if (reverse)
		node_reverse(node);

	node_start(node);
	node_start_defer(node);

	while (!feof(stdin)) {
		msg_fscan(stdin, &msg);

#if 1 /* Preprend timestamp */
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		fprintf(stdout, "%17.3f\t", ts.tv_sec + ts.tv_nsec / 1e9);
#endif

		msg_fprint(stdout, &msg);
		node_write(node, &msg);	
	}

	return 0;
}
