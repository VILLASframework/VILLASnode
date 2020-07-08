/** Node-type: CAN bus
 *
 * @author Niklas Eiling <niklas.eiling@eonerc.rwth-aachen.de>
 * @copyright 2014-2020, Institute for Automation of Complex Power Systems, EONERC
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

#include "villas/signal_data.h"
#include "villas/signal_list.h"
#include "villas/signal_type.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/sockios.h>

#include <villas/nodes/can.hpp>
#include <villas/utils.hpp>
#include <villas/sample.h>
#include <villas/plugin.h>
#include <villas/signal.h>
#include <villas/node.h>


/* Forward declarations */
static struct plugin p;

using namespace villas::node;
using namespace villas::utils;

int can_init(struct node *n)
{
	struct can *c = (struct can *) n->_vd;

	c->interface_name = nullptr;
	c->socket = 0;
	c->sample_buf = nullptr;
	c->sample_buf_num = 0;
	c->in = nullptr;
	c->out = nullptr;

	return 0;
}

int can_destroy(struct node *n)
{
	struct can *c = (struct can *) n->_vd;

	free(c->interface_name);
	if (c->socket != 0) {
		close(c->socket);
	}
	free(c->sample_buf);
	free(c->in);
	free(c->out);
	return 0;
}

int can_parse_signal(json_t *json, struct vlist *node_signals, struct can_signal *can_signals, size_t signal_index)
{
	const char *name = nullptr;
	uint64_t can_id = 0;
	int can_size = 8;
	int can_offset = 0;
	struct signal* sig = nullptr;
	int ret = 1;
	json_error_t err;

	ret = json_unpack_ex(json, &err, 0, "{ s?: s, s?: i, s?: i, s?: i }",
			     "name", &name,
			     "can_id", &can_id,
			     "can_size", &can_size,
			     "can_offset", &can_offset
			    );

	if (ret) {
		jerror(&err, "Failed to parse signal configuration for can");
		goto out;
	}

	if (can_size > 8 || can_size <= 0) {
		error("can_size of %d for signal \"%s\" is invalid. You must satisfy 0 < can_size <= 8.", can_size, name);
		goto out;
	}

	if (can_offset > 8 || can_offset < 0) {
		error("can_offset of %d for signal \"%s\" is invalid. You must satisfy 0 <= can_offset <= 8.", can_offset, name);
		goto out;
	}

	sig = (struct signal*)vlist_at(node_signals, signal_index);
	if ((!name && !sig->name) || (name && strcmp(name, sig->name) == 0)) {
		can_signals[signal_index].id = can_id;
		can_signals[signal_index].size = can_size;
		can_signals[signal_index].offset = can_offset;
		ret = 0;
		goto out;
	} else {
		error("Signal configuration inconsistency detected: Signal with index %zu (\"%s\") does not match can_signal \"%s\"\n", signal_index, sig->name, name);
	}
     out:
	return ret;
}

int can_parse(struct node *n, json_t *cfg)
{
	int ret = 1;
	struct can *c = (struct can *) n->_vd;
	size_t i;
	json_t *json_in_signals;
	json_t *json_out_signals;
	json_t *json_signal;
	json_error_t err;

	c->in = nullptr;
	c->out = nullptr;

	ret = json_unpack_ex(cfg, &err, 0, "{ s: s, s: F, s?: { s?: o }, s?: { s?: o } }",
			     "interface_name", &c->interface_name,
			     "sample_rate", &c->sample_rate,
			     "in",
				     "signals", &json_in_signals,
			     "out",
				     "signals", &json_out_signals
			    );
	if (ret) {
		jerror(&err, "Failed to parse configuration of node %s", node_name(n));
		goto out;
	}

	if ((c->in = (struct can_signal*)calloc(
			json_array_size(json_in_signals),
			sizeof(struct can_signal))) == nullptr) {
		error("failed to allocate memory for input ids");
		goto out;
	}
	if ((c->out = (struct can_signal*)calloc(
			 json_array_size(json_out_signals),
			 sizeof(struct can_signal))) == nullptr) {
		error("failed to allocate memory for output ids");
		goto out;
	}

	json_array_foreach(json_in_signals, i, json_signal) {
		if (can_parse_signal(json_signal, &n->in.signals, c->in, i) != 0) {
			error("at signal %zu in node %s.",i , node_name(n));
			goto out;
		}
	}
	json_array_foreach(json_out_signals, i, json_signal) {
		if (can_parse_signal(json_signal, &n->out.signals, c->out, i) != 0) {
			error("at signal %zu in node %s.",i , node_name(n));
			goto out;
		}
	}
	ret = 0;
     out:
	if (ret != 0) {
		free(c->in);
		free(c->out);
		c->in = nullptr;
		c->out = nullptr;
	}
	return ret;
}

char *can_print(struct node *n)
{
	struct can *c = (struct can *) n->_vd;

	return strf("interface_name=%s", c->interface_name);
}

int can_check(struct node *n)
{
	struct can *c = (struct can *) n->_vd;

	if (c->interface_name == nullptr || strlen(c->interface_name) == 0) {
		error("interface_name is empty. Please specify the name of the CAN interface!");
		return 1;
	}

	return 0;
}

int can_prepare(struct node *n)
{
	struct can *c = (struct can *) n->_vd;
	c->sample_buf = (union signal_data*) calloc(
				vlist_length(&n->in.signals),
			        sizeof(union signal_data));
	return (c->sample_buf != 0 ? 0 : 1);
}

int can_start(struct node *n)
{
	int ret = 1;
	struct sockaddr_can addr = {0};
	struct ifreq ifr;

	struct can *c = (struct can *) n->_vd;
	c->start_time = time_now();

	if((c->socket = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		error("Error while opening CAN socket");
		goto out;
	}

	strcpy(ifr.ifr_name, c->interface_name);
	if (ioctl(c->socket, SIOCGIFINDEX, &ifr) != 0) {
		error("Could not find interface with name \"%s\".", c->interface_name);
		goto out;
	}

	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if(bind(c->socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		error("Could not bind to interface with name \"%s\" (%d).", c->interface_name, ifr.ifr_ifindex);
		goto out;
	}
	ret = 0;
 out:
	return ret;
}

int can_stop(struct node *n)
{
	struct can *c = (struct can *) n->_vd;

	if (c->socket != 0) {
		close(c->socket);
		c->socket = 0;
	}

	return 0;
}

int can_conv_to_raw(union signal_data* sig, struct signal *from, void* to, int size)
{
	if (size <= 0 || size > 8) {
		error("signal size cannot be larger than 8!");
		return 1;
	}
	switch(from->type) {
	case SignalType::BOOLEAN:
		*(uint8_t*)to = sig->b;
		return 0;
	case SignalType::INTEGER:
		switch(size) {
		case 1:
			*(int8_t*)to = (int8_t)sig->i;
			return 0;
		case 2:
			*(int16_t*)to = (int16_t)sig->i;
			sig->i = (int64_t)*(int16_t*)from;
			return 0;
		case 3:
			*(int16_t*)to = (int16_t)sig->i;
			*((int8_t*)to+2) = (int8_t)(sig->i >> 16);
			return 0;
		case 4:
			*(int32_t*)to = (int32_t)sig->i;
			return 0;
		case 8:
			*(int64_t*)to = sig->i;
			return 0;
		default:
			goto fail;
		}
	case SignalType::FLOAT:
		switch(size) {
		case 4:
			assert(sizeof(float) == 4);
			*(float*)to = (float)sig->f;
			return 0;
		case 8:
			*(double*)to = sig->f;
			return 0;
		default:
			goto fail;
		}
	case SignalType::COMPLEX:
		if (size != 8) {
			goto fail;
		}
		*(float*)to = sig->z.real();
		*((float*)to+1) = sig->z.imag();
		return 0;
	default:
		goto fail;
	}
 fail:
	error("unsupported conversion to %s from raw (%p, %d)",
	      signal_type_to_str(from->type), to, size);
	return 1;
}

int can_conv_from_raw(union signal_data* sig, void* from, int size, struct signal *to)
{
	if (size <= 0 || size > 8) {
		error("signal size cannot be larger than 8!");
		return 1;
	}
	switch(to->type) {
	case SignalType::BOOLEAN:
		sig->b = (bool)*(uint8_t*)from;
		return 0;
	case SignalType::INTEGER:
		switch(size) {
		case 1:
			sig->i = (int64_t)*(int8_t*)from;
			return 0;
		case 2:
			sig->i = (int64_t)*(int16_t*)from;
			return 0;
		case 3:
			sig->i = (int64_t)*(int16_t*)from;
			sig->i += ((int64_t)*((int8_t*)(from)+2)) << 16;
			return 0;
		case 4:
			sig->i = (int64_t)*(int32_t*)from;
			return 0;
		case 8:
			sig->i = *(uint64_t*)from;
			return 0;
		default:
			goto fail;
		}
	case SignalType::FLOAT:
		switch(size) {
		case 4:
			assert(sizeof(float) == 4);
			sig->f = (double)*(float*)from;
			return 0;
		case 8:
			sig->f = *(double*)from;
			return 0;
		default:
			goto fail;
		}
	case SignalType::COMPLEX:
		if (size != 8) {
			goto fail;
		}
		sig->z = std::complex<float>(*(float*)from, *((float*)from+1));
		return 0;
	default:
		goto fail;
	}
 fail:
	error("unsupported conversion from %s to raw (%p, %d)",
	      signal_type_to_str(to->type), from, size);
	return 1;
}

int can_read(struct node *n, struct sample *smps[], unsigned cnt, unsigned *release)
{
	int ret = 0;
	int nbytes;
	unsigned nread = 0;
	struct can_frame frame;
	struct timeval tv;
	bool found_id = false;

	struct can *c = (struct can *) n->_vd;

	assert(cnt >= 1 && smps[0]->capacity >= 1);

	nbytes = read(c->socket, &frame, sizeof(struct can_frame));
	if (nbytes == -1) {
		error("CAN read() returned -1. Is the CAN interface up?");
		goto out;
	}
	if ((unsigned)nbytes != sizeof(struct can_frame)) {
		error("CAN read() error. read() returned %d bytes but expected %zu",
		      nbytes, sizeof(struct can_frame));
		goto out;
	}

	debug(0,"received can message: (id:%d, len:%u, data: 0x%x:0x%x)",
	      frame.can_id,
	      frame.can_dlc,
	      ((uint32_t*)&frame.data)[0],
	      ((uint32_t*)&frame.data)[1]);

	if (ioctl(c->socket, SIOCGSTAMP, &tv) == 0) {
		TIMEVAL_TO_TIMESPEC(&tv, &smps[nread]->ts.received);
		smps[nread]->flags |= (int) SampleFlags::HAS_TS_RECEIVED;
	}

	for (size_t i=0; i < vlist_length(&(n->in.signals)); i++) {
		if (c->in[i].id == frame.can_id) {
			if (can_conv_from_raw(&c->sample_buf[i],
				((uint8_t*)&frame.data) + c->in[i].offset,
				c->in[i].size,
				(struct signal*) vlist_at(&n->in.signals, i)) != 0) {
				goto out;
			}
			c->sample_buf_num++;
			found_id = true;
		}
	}
	if (!found_id) {
		error("did not find signal for can id %d\n", frame.can_id);
		return 0;
	}
	debug(0, "received %zu signals\n", c->sample_buf_num);
	/* Copy signal data to sample only when all signals have been received */
	if (c->sample_buf_num == vlist_length(&n->in.signals)) {
		smps[nread]->length = c->sample_buf_num;
		memcpy(smps[nread]->data, c->sample_buf, c->sample_buf_num*sizeof(union signal_data));
		c->sample_buf_num = 0;
		smps[nread]->flags |= (int) SampleFlags::HAS_DATA;
		ret = 1;
	} else {
		smps[nread]->length = 0;
		ret = 0;
	}
 out:
	/* Set signals, because other VILLASnode parts expect us to */
	smps[nread]->signals = &n->in.signals;
	return ret;
}

int can_write(struct node *n, struct sample *smps[], unsigned cnt, unsigned *release)
{
	int nbytes;
	unsigned nwrite;
	struct can_frame *frame;
	size_t fsize = 0; /* number of frames in use */

	struct can *c = (struct can *) n->_vd;

	assert(cnt >= 1 && smps[0]->capacity >= 1);

	frame = (struct can_frame*) calloc(sizeof(struct can_frame),
					   vlist_length(&(n->out.signals)));

	for (nwrite=0; nwrite < cnt; nwrite++) {
		for (size_t i=0; i < vlist_length(&(n->out.signals)); i++) {
			if (c->out[i].offset != 0) { /* frame is shared */
				continue;
			}
			frame[fsize].can_dlc = c->out[i].size;
			frame[fsize].can_id = c->out[i].id;
			can_conv_to_raw(
				&smps[nwrite]->data[i],
				(struct signal*)vlist_at(&(n->out.signals), i),
				&frame[fsize].data,
				c->out[i].size);
			fsize++;
		}
		for (size_t i=0; i < vlist_length(&(n->out.signals)); i++) {
			if (c->out[i].offset == 0) { /* frame already stored */
				continue;
			}
			for (size_t j=0; j < fsize; j++) {
				if (c->out[i].id != frame[j].can_id) {
					continue;
				}
				frame[j].can_dlc += c->out[i].size;
				can_conv_to_raw(
					&smps[nwrite]->data[i],
					(struct signal*)vlist_at(&(n->out.signals), i),
					(uint8_t*)&frame[j].data + c->out[i].offset,
					c->out[i].size);
				break;
			}
		}
		for (size_t j=0; j < fsize; j++) {
			debug(0,"writing can message: (id:%d, dlc:%u, data:0x%x:0x%x)", frame[j].can_id,
			      frame[j].can_dlc,
			      ((uint32_t*)&frame[j].data)[0],
			      ((uint32_t*)&frame[j].data)[1]);

			if ((nbytes = write(c->socket, &frame[j], sizeof(struct can_frame))) == -1) {
				error("CAN write() returned -1. Is the CAN interface up?");
				return nwrite;
			}
			if ((unsigned)nbytes != sizeof(struct can_frame)) {
				error("CAN write() error. write() returned %d bytes but expected %zu",
				      nbytes, sizeof(struct can_frame));
				return nwrite;
			}
		}
	}
	return nwrite;
}

	return nwrite;
}

int can_poll_fds(struct node *n, int fds[])
{
	struct can *c = (struct can *) n->_vd;
	fds[0] = c->socket;

	return 1; /* The number of file descriptors which have been set in fds */
}

__attribute__((constructor(110)))
static void register_plugin() {
	if (plugins.state == State::DESTROYED)
		vlist_init(&plugins);

	p.name			= "can";
	p.description		= "Receive CAN messages using the socketCAN driver";
	p.node.instances.state	= State::DESTROYED;
	p.node.vectorize	= 0;
	p.node.size		= sizeof(struct can);
	p.node.init		= can_init;
	p.node.destroy		= can_destroy;
	p.node.prepare		= can_prepare;
	p.node.parse		= can_parse;
	p.node.print		= can_print;
	p.node.check		= can_check;
	p.node.start		= can_start;
	p.node.stop		= can_stop;
	p.node.read		= can_read;
	p.node.write		= can_write;
	p.node.poll_fds		= can_poll_fds;

	vlist_init(&p.node.instances);
	vlist_push(&plugins, &p);
}

__attribute__((destructor(110)))
static void deregister_plugin() {
	if (plugins.state != State::DESTROYED)
		vlist_remove_all(&plugins, &p);
}
