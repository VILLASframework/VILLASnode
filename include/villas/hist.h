/** Histogram functions.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014-2016, Institute for Automation of Complex Power Systems, EONERC
 *   This file is part of VILLASnode. All Rights Reserved. Proprietary and confidential.
 *   Unauthorized copying of this file, via any medium is strictly prohibited. 
 *********************************************************************************/

#ifndef _HIST_H_
#define _HIST_H_

#include <stdio.h>

#include "config.h"

#ifdef WITH_JANSSON
  #include <jansson.h>
#endif

#define HIST_HEIGHT	(LOG_WIDTH - 55)
#define HIST_SEQ	17

typedef unsigned hist_cnt_t;

/** Histogram structure used to collect statistics. */
struct hist {
	double resolution;	/**< The distance between two adjacent buckets. */
	
	double high;		/**< The value of the highest bucket. */
	double low;		/**< The value of the lowest bucket. */
	
	double highest;		/**< The highest value observed (may be higher than #high). */
	double lowest;		/**< The lowest value observed (may be lower than #low). */
	double last;		/**< The last value which has been put into the buckets */
	
	int length;		/**< The number of buckets in #data. */

	hist_cnt_t total;	/**< Total number of counted values. */
	hist_cnt_t higher;	/**< The number of values which are higher than #high. */ 
	hist_cnt_t lower;	/**< The number of values which are lower than #low. */

	hist_cnt_t *data;	/**< Pointer to dynamically allocated array of size length. */
	
	double _m[2], _s[2];	/**< Private variables for online variance calculation */
};

/** Initialize struct hist with supplied values and allocate memory for buckets. */
void hist_create(struct hist *h, double start, double end, double resolution);

/** Free the dynamically allocated memory. */
void hist_destroy(struct hist *h);

/** Reset all counters and values back to zero. */
void hist_reset(struct hist *h);

/** Count a value within its corresponding bucket. */
void hist_put(struct hist *h, double value);

/** Calcluate the variance of all counted values. */
double hist_var(struct hist *h);

/** Calculate the mean average of all counted values. */
double hist_mean(struct hist *h);

/** Calculate the standard derivation of all counted values. */
double hist_stddev(struct hist *h);

/** Print all statistical properties of distribution including a graphilcal plot of the histogram. */
void hist_print(struct hist *h);

/** Print ASCII style plot of histogram */
void hist_plot(struct hist *h);

/** Dump histogram data in Matlab format.
 *
 * @return The string containing the dump. The caller is responsible to free() the buffer.
 */
char * hist_dump(struct hist *h);

/** Prints Matlab struct containing all infos to file. */
int hist_dump_matlab(struct hist *h, FILE *f);

#ifdef WITH_JANSSON
int hist_dump_json(struct hist *h, FILE *f);

json_t * hist_json(struct hist *h);
#endif

#endif /* _HIST_H_ */
