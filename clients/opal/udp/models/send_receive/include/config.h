/** Compiled-in settings
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014, Institute for Automation of Complex Power Systems, EONERC
 * @file
 */
 
#ifndef _CONFIG_H_
#define _CONFIG_H_

#define PROGNAME	"VILLASnode-OPAL-UDP"
#define VERSION		"0.5"

#define MAX_VALUES	64
#define TOT_VALS_FROM_GTSKT 5
#define TOT_VALS_TO_GTSKT 5

#define DATA_FROM_VILLAS 0
#define DATA_FROM_GTSKT 1

#define DATA_SOURCE DATA_FROM_GTSKT

#endif /* _CONFIG_H_ */