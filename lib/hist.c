/** Histogram functions.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2016, Institute for Automation of Complex Power Systems, EONERC
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <time.h>

#include "utils.h"
#include "hist.h"

#define VAL(h, i)	((h)->low + (i) * (h)->resolution)
#define INDEX(h, v)	round((v - (h)->low) / (h)->resolution)

int hist_create(struct hist *h, double low, double high, double resolution)
{
	h->low = low;
	h->high = high;
	h->resolution = resolution;

	if (resolution > 0) {
		h->length = (high - low) / resolution;
		h->data = alloc(h->length * sizeof(hist_cnt_t));
	}
	else {
		h->length = 0;
		h->data = NULL;
	}

	hist_reset(h);
	
	return 0;
}

void hist_destroy(struct hist *h)
{
	if (h->data) {
		free(h->data);
		h->data = NULL;
	}
}

void hist_put(struct hist *h, double value)
{
	int idx = INDEX(h, value);
	
	h->last = value;

	/* Update min/max */
	if (value > h->highest)
		h->highest = value;
	if (value < h->lowest)
		h->lowest = value;

	/* Check bounds and increment */
	if      (idx >= h->length)
		h->higher++;
	else if (idx < 0)
		h->lower++;
	else if (h->data != NULL)
		h->data[idx]++;

	h->total++;

	/* Online / running calculation of variance and mean
	 *  by Donald Knuth’s Art of Computer Programming, Vol 2, page 232, 3rd edition */
	if (h->total == 1) {
		h->_m[1] = h->_m[0] = value;
		h->_s[1] = 0.0;
	}
	else {
		h->_m[0] = h->_m[1] + (value - h->_m[1]) / h->total;
		h->_s[0] = h->_s[1] + (value - h->_m[1]) * (value - h->_m[0]);

		// set up for next iteration
		h->_m[1] = h->_m[0];
		h->_s[1] = h->_s[0];
	}

}

void hist_reset(struct hist *h)
{
	h->total = 0;
	h->higher = 0;
	h->lower = 0;

	h->highest = DBL_MIN;
	h->lowest = DBL_MAX;

	if (h->data)
		memset(h->data, 0, h->length * sizeof(unsigned));
}

double hist_mean(struct hist *h)
{
	return (h->total > 0) ? h->_m[0] : 0.0;
}

double hist_var(struct hist *h)
{
	return (h->total > 1) ? h->_s[0] / (h->total - 1) : 0.0;
}

double hist_stddev(struct hist *h)
{
	return sqrt(hist_var(h));
}

void hist_print(struct hist *h, int details)
{ INDENT
	stats("Counted values: %ju (%ju between %f and %f)", h->total, h->total-h->higher-h->lower, h->high, h->low);
	stats("Highest: %f Lowest: %f", h->highest, h->lowest);
	stats("Mu: %f Sigma2: %f Sigma: %f", hist_mean(h), hist_var(h), hist_stddev(h));

	if (details > 0 && h->total - h->higher - h->lower > 0) {
		char *buf = hist_dump(h);
		stats("Matlab: %s", buf);
		free(buf);

		hist_plot(h);
	}
}

void hist_plot(struct hist *h)
{
	char buf[HIST_HEIGHT];
	memset(buf, '#', sizeof(buf));

	hist_cnt_t max = 1;

	/* Get highest bar */
	for (int i = 0; i < h->length; i++) {
		if (h->data[i] > max)
			max = h->data[i];
	}

	/* Print plot */
	stats("%9s | %5s | %s", "Value", "Count", "Plot");
	line();

	for (int i = 0; i < h->length; i++) {
		double value = VAL(h, i);
		hist_cnt_t cnt = h->data[i];
		int bar = HIST_HEIGHT * ((double) cnt / max);

		if (value > h->lowest || value < h->highest)
			stats("%+9g | %5ju | %.*s", value, cnt, bar, buf);
	}
}

char * hist_dump(struct hist *h)
{
	char *buf = alloc(128);
	
	strcatf(&buf, "[ ");

	for (int i = 0; i < h->length; i++)
		strcatf(&buf, "%ju ", h->data[i]);

	strcatf(&buf, "]");
	
	return buf;
}

#ifdef WITH_JANSSON
json_t * hist_json(struct hist *h)
{
	json_t *b = json_array();
	
	for (int i = 0; i < h->length; i++)
		json_array_append(b, json_integer(h->data[i]));
	
	return json_pack("{ s: f, s: f, s: i, s: i, s: i, s: f, s: f, s: f, s: f, s: f, s: o }",
		"low", h->low,
		"high", h->high,
		"total", h->total,
		"higher", h->higher,
		"lower", h->lower,
		"highest", h->highest,
		"lowest", h->lowest,
		"mean", hist_mean(h),
		"variance", hist_var(h),
		"stddev", hist_stddev(h),
		"buckets", b
	);
}

int hist_dump_json(struct hist *h, FILE *f)
{
	json_t *j = hist_json(h);
	
	int ret = json_dumpf(j, f, 0);
	
	json_decref(j);
	
	return ret;
}
#endif /* WITH_JANNSON */

int hist_dump_matlab(struct hist *h, FILE *f)
{
	char *buf = hist_dump(h);

	fprintf(f, "%lu = struct( ", time(NULL));
	fprintf(f, "'low', %f, ", h->low);
	fprintf(f, "'high', %f, ", h->high);
	fprintf(f, "'total', %ju, ", h->total);
	fprintf(f, "'higher', %ju, ", h->higher);
	fprintf(f, "'lower', %ju, ", h->lower);
	fprintf(f, "'highest', %f, ", h->highest);
	fprintf(f, "'lowest', %f, ", h->lowest);
	fprintf(f, "'mean', %f, ", hist_mean(h));
	fprintf(f, "'variance', %f, ", hist_var(h));
	fprintf(f, "'stddev', %f, ", hist_stddev(h));
	fprintf(f, "'buckets', %s ", buf);
	fprintf(f, "),\n");
	
	free(buf);
	
	return 0;
}
