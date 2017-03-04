/** General purpose helper functions.
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <ctype.h>

#include "config.h"
#include "cfg.h"
#include "utils.h"

void print_copyright()
{
	printf("VILLASnode %s (built on %s %s)\n",
		BLU(VERSION), MAG(__DATE__), MAG(__TIME__));
	printf(" copyright 2014-2016, Institute for Automation of Complex Power Systems, EONERC\n");
	printf(" Steffen Vogel <StVogel@eonerc.rwth-aachen.de>\n");
}

int version_parse(const char *s, struct version *v)
{
	return sscanf(s, "%u.%u", &v->major, &v->minor) != 2;
}

int version_cmp(struct version *a, struct version *b) {
	int major = a->major - b->major;
	int minor = a->minor - b->minor;

	return major ? major : minor;
}

int strftimespec(char *dst, size_t max, const char *fmt, struct timespec *ts)
{
	int s, d;
	char fmt2[64], dst2[64];

	for (s=0, d=0; fmt[s] && d < 64;) {
		char c = fmt2[d++] = fmt[s++];
		if (c == '%') {
			char n = fmt[s];
			if (n == 'u') {
				fmt2[d++] = '0';
				fmt2[d++] = '3';
			}
			else
				fmt2[d++] = '%';
		}
	}
	
	/* Print sub-second part to format string */
	snprintf(dst2, sizeof(dst2), fmt2, ts->tv_nsec / 1000000);

	/* Print timestamp */
	struct tm tm;
	if (localtime_r(&(ts->tv_sec), &tm) == NULL)
		return -1;

	return strftime(dst, max, dst2, &tm);
}

double box_muller(float m, float s)
{
	double x1, x2, y1;
	static double y2;
	static int use_last = 0;

	if (use_last) {		/* use value from previous call */
		y1 = y2;
		use_last = 0;
	}
	else {
		double w;
		do {
			x1 = 2.0 * randf() - 1.0;
			x2 = 2.0 * randf() - 1.0;
			w = x1*x1 + x2*x2;
		} while (w >= 1.0);

		w = sqrt(-2.0 * log(w) / w);
		y1 = x1 * w;
		y2 = x2 * w;
		use_last = 1;
	}

	return m + y1 * s;
}

double randf()
{
	return (double) random() / RAND_MAX;
}

void die()
{
	int zero = 0;
	log_outdent(&zero);
	abort();
}

char * strcatf(char **dest, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vstrcatf(dest, fmt, ap);
	va_end(ap);

	return *dest;
}

char * vstrcatf(char **dest, const char *fmt, va_list ap)
{
	char *tmp;
	int n = *dest ? strlen(*dest) : 0;
	int i = vasprintf(&tmp, fmt, ap);

	*dest = (char *)(realloc(*dest, n + i + 1));
	if (*dest != NULL)
		strncpy(*dest+n, tmp, i + 1);
	
	free(tmp);

	return *dest;
}

void cpuset_from_integer(uintmax_t set, cpu_set_t *cset)
{
	CPU_ZERO(cset);
	for (int i = 0; i < MIN(sizeof(set), CPU_SETSIZE) * 8; i++) {
		if (set & (1L << i))
			CPU_SET(i, cset);
	}
}

/* From: https://github.com/mmalecki/util-linux/blob/master/lib/cpuset.c */
static const char *nexttoken(const char *q,  int sep)
{
	if (q)
		q = strchr(q, sep);
	if (q)
		q++;
	return q;
}

int cpulist_parse(const char *str, cpu_set_t *set, int fail)
{
	const char *p, *q;
	int r = 0;

	q = str;
	CPU_ZERO(set);

	while (p = q, q = nexttoken(q, ','), p) {
		unsigned int a;	/* beginning of range */
		unsigned int b;	/* end of range */
		unsigned int s;	/* stride */
		const char *c1, *c2;
		char c;

		if ((r = sscanf(p, "%u%c", &a, &c)) < 1)
			return 1;
		b = a;
		s = 1;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');
		if (c1 != NULL && (c2 == NULL || c1 < c2)) {
			if ((r = sscanf(c1, "%u%c", &b, &c)) < 1)
				return 1;
			c1 = nexttoken(c1, ':');
			if (c1 != NULL && (c2 == NULL || c1 < c2)) {
				if ((r = sscanf(c1, "%u%c", &s, &c)) < 1)
					return 1;
				if (s == 0)
					return 1;
			}
		}

		if (!(a <= b))
			return 1;
		while (a <= b) {
			if (fail && (a >= CPU_SETSIZE))
				return 2;
			CPU_SET(a, set);
			a += s;
		}
	}

	if (r == 2)
		return 1;
	return 0;
}

char *cpulist_create(char *str, size_t len, cpu_set_t *set)
{
	size_t i;
	char *ptr = str;
	int entry_made = 0;

	for (i = 0; i < CPU_SETSIZE; i++) {
		if (CPU_ISSET(i, set)) {
			int rlen;
			size_t j, run = 0;
			entry_made = 1;
			for (j = i + 1; j < CPU_SETSIZE; j++) {
				if (CPU_ISSET(j, set))
					run++;
				else
					break;
			}
			if (!run)
				rlen = snprintf(ptr, len, "%zd,", i);
			else if (run == 1) {
				rlen = snprintf(ptr, len, "%zd,%zd,", i, i + 1);
				i++;
			} else {
				rlen = snprintf(ptr, len, "%zd-%zd,", i, i + run);
				i += run;
			}
			if (rlen < 0 || (size_t) rlen + 1 > len)
				return NULL;
			ptr += rlen;
			if (rlen > 0 && len > (size_t) rlen)
				len -= rlen;
			else
				len = 0;
		}
	}
	ptr -= entry_made;
	*ptr = '\0';

	return str;
}

#ifdef WITH_JANSSON
json_t * config_to_json(config_setting_t *cfg)
{
	switch (config_setting_type(cfg)) {
		case CONFIG_TYPE_INT:	 return json_integer(config_setting_get_int(cfg));
		case CONFIG_TYPE_INT64:	 return json_integer(config_setting_get_int64(cfg));
		case CONFIG_TYPE_FLOAT:  return json_real(config_setting_get_float(cfg));
		case CONFIG_TYPE_STRING: return json_string(config_setting_get_string(cfg));
		case CONFIG_TYPE_BOOL:	 return json_boolean(config_setting_get_bool(cfg));

		case CONFIG_TYPE_ARRAY:
		case CONFIG_TYPE_LIST: {
			json_t *json = json_array();
			
			for (int i = 0; i < config_setting_length(cfg); i++)
				json_array_append_new(json, config_to_json(config_setting_get_elem(cfg, i)));
			
			return json;
		}
		
		case CONFIG_TYPE_GROUP: {
			json_t *json = json_object();
			
			for (int i = 0; i < config_setting_length(cfg); i++)
				json_object_set_new(json,
					config_setting_name(config_setting_get_elem(cfg, i)),
					config_to_json(config_setting_get_elem(cfg, i))
				);
			
			return json;
		}
		
		default:
			return json_object();
	}
}
#endif

void * alloc(size_t bytes)
{
	void *p = malloc(bytes);
	if (!p)
		error("Failed to allocate memory");

	memset(p, 0, bytes);

	return p;
}

void * memdup(const void *src, size_t bytes)
{
	void *dst = alloc(bytes);
	
	memcpy(dst, src, bytes);
	
	return dst;
}

ssize_t read_random(char *buf, size_t len)
{
	int fd;
	ssize_t bytes, total;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
		return -1;

	bytes = 0;
	total = 0;
	while (total < len) {
		bytes = read(fd, buf + total, len - total);
		if (bytes < 0)
			break;

		total += bytes;
	}
	
	close(fd);
	
	return bytes;
}

void printb(void *mem, size_t len)
{
	uint8_t *mem8 = (uint8_t *) mem;

	for (int i = 0; i < len; i++) {
		printf("%02hx ", mem8[i]);

		if (i % 16 == 15)
			printf("\n");
	}
}

void printdw(void *mem, size_t len)
{
	int columns = 4;

	uint32_t *mem32 = (uint32_t *) mem;

	for (int i = 0; i < len; i++) {
		if (i % columns == 0)
			printf("%#x: ", i * 4);

		printf("%08x ", mem32[i]);
		
		char *memc = (char *) &mem32[i];
		printf("%c%c%c%c ",
			isprint(memc[0]) ? memc[0] : ' ',
			isprint(memc[1]) ? memc[1] : ' ',
			isprint(memc[2]) ? memc[2] : ' ',
			isprint(memc[3]) ? memc[3] : ' '
		);

		if ((i+1) % columns == 0)
			printf("\n");
	}
}

void rdtsc_sleep(uint64_t nanosecs, uint64_t start)
{
	uint64_t cycles;

	/** @todo Replace the hard coded CPU clock frequency */
	cycles = (double) nanosecs / (1e9 / 3392389000);
	
	if (start == 0)
		start = rdtsc();
	
	do {
		__asm__("nop");
	} while (rdtsc() - start < cycles);
}