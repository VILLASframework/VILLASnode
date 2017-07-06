/** Node-type for signal generation.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2017, Institute for Automation of Complex Power Systems, EONERC
 * @license GNU General Public License (version 3)
 *
 * VILLASnode
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************************/

#include <math.h>

#include "node.h"
#include "plugin.h"
#include "nodes/signal.h"

enum signal_type signal_lookup_type(const char *type)
{
	if      (!strcmp(type, "random"))
		return SIGNAL_TYPE_RANDOM;
	else if (!strcmp(type, "sine"))
		return SIGNAL_TYPE_SINE;
	else if (!strcmp(type, "square"))
		return SIGNAL_TYPE_SQUARE;
	else if (!strcmp(type, "triangle"))
		return SIGNAL_TYPE_TRIANGLE;
	else if (!strcmp(type, "ramp"))
		return SIGNAL_TYPE_RAMP;
	else if (!strcmp(type, "mixed"))
		return SIGNAL_TYPE_MIXED;
	else
		return -1;
}

int signal_parse(struct node *n, config_setting_t *cfg)
{
	struct signal *s = n->_vd;

	const char *type;

	if (!config_setting_lookup_string(cfg, "signal", &type))
		s->type = SIGNAL_TYPE_MIXED;
	else {
		s->type = signal_lookup_type(type);
		if (s->type == -1)
			cerror(cfg, "Unknown signal type '%s'", type);
	}
	
	if (!config_setting_lookup_bool(cfg, "realtime", &s->rt))
		s->rt = 1;

	if (!config_setting_lookup_int(cfg, "limit", &s->limit))
		s->limit = -1;

	if (!config_setting_lookup_int(cfg, "values", &s->values))
		s->values = 1;

	if (!config_setting_lookup_float(cfg, "rate", &s->rate))
		s->rate = 10;

	if (!config_setting_lookup_float(cfg, "frequency", &s->frequency))
		s->frequency = 1;

	if (!config_setting_lookup_float(cfg, "amplitude", &s->amplitude))
		s->amplitude = 1;

	if (!config_setting_lookup_float(cfg, "stddev", &s->stddev))
		s->stddev = 0.02;

	return 0;
}

int signal_open(struct node *n)
{
	struct signal *s = n->_vd;
	
	s->counter = 0;
	s->started = time_now();

	/* Setup timer */
	if (s->rt) {
		s->tfd = timerfd_create_rate(s->rate);
		if (s->tfd < 0)
			return -1;
	}
	else
		s->tfd = -1;

	return 0;
}

int signal_close(struct node *n)
{
	struct signal* s = n->_vd;
	
	close(s->tfd);

	return 0;
}

int signal_read(struct node *n, struct sample *smps[], unsigned cnt)
{
	struct signal *s = n->_vd;
	struct sample *t = smps[0];
	
	struct timespec now;
	
	assert(cnt == 1);
	
	/* Throttle output if desired */
	if (s->rt) {
		/* Block until 1/p->rate seconds elapsed */
		int steps = timerfd_wait(s->tfd);
		if (steps > 1)
			warn("Missed steps: %u", steps);

		s->counter += steps;
		
		now = time_now();
	}
	else {
		struct timespec offset = time_from_double(s->counter * 1.0 / s->rate);

		now = time_add(&s->started, &offset);
		
		s->counter += 1;
	}

	double running = time_delta(&s->started, &now);

	t->ts.origin = 
	t->ts.received = now;
	t->sequence = s->counter;
	t->length = s->values;

	for (int i = 0; i < MIN(s->values, t->capacity); i++) {
		int rtype = (s->type != SIGNAL_TYPE_MIXED) ? s->type : i % 4;
		switch (rtype) {
			case SIGNAL_TYPE_RANDOM:   t->data[i].f += box_muller(0, s->stddev);                                              break;
			case SIGNAL_TYPE_SINE:	   t->data[i].f = s->amplitude *        sin(running * s->frequency * 2 * M_PI);           break;
			case SIGNAL_TYPE_TRIANGLE: t->data[i].f = s->amplitude * (fabs(fmod(running * s->frequency, 1) - .5) - 0.25) * 4; break;
			case SIGNAL_TYPE_SQUARE:   t->data[i].f = s->amplitude * (    (fmod(running * s->frequency, 1) < .5) ? -1 : 1);   break;
			case SIGNAL_TYPE_RAMP:     t->data[i].f = fmod(s->counter, s->rate / s->frequency); /** @todo send as integer? */ break;
		}
	}

	if (s->limit > 0 && s->counter >= s->limit) {
		info("Reached limit");
		killme(SIGTERM);
	}

	return 1;
}

char * signal_print(struct node *n)
{
	struct signal *s = n->_vd;
	char *type, *buf = NULL;
	
	switch (s->type) {
		case SIGNAL_TYPE_MIXED:    type = "mixed";    break;
		case SIGNAL_TYPE_RAMP:     type = "ramp";     break;
		case SIGNAL_TYPE_TRIANGLE: type = "triangle"; break;
		case SIGNAL_TYPE_SQUARE:   type = "square";   break;
		case SIGNAL_TYPE_SINE:     type = "sine";     break;
		case SIGNAL_TYPE_RANDOM:   type = "random";   break;
		default: return NULL;
	}
	
	strcatf(&buf, "signal=%s, rate=%.2f, values=%d, frequency=%.2f, amplitude=%.2f, stddev=%.2f",
		type, s->rate, s->values, s->frequency, s->amplitude, s->stddev);
	
	if (s->limit > 0)
		strcatf(&buf, ", limit=%d", s->limit);

	return buf;
};

static struct plugin p = {
	.name = "signal",
	.description = "Signal generation",
	.type = PLUGIN_TYPE_NODE,
	.node = {
		.vectorize = 1,
		.size  = sizeof(struct signal),
		.parse = signal_parse,
		.print = signal_print,
		.start = signal_open,
		.stop  = signal_close,
		.read  = signal_read,
	}
};

REGISTER_PLUGIN(&p)
LIST_INIT_STATIC(&p.node.instances)