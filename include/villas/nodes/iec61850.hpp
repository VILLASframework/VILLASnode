/** Some helpers to libiec61850
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
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

/**
 * @addtogroup iec61850_sv IEC 61850-9-2 (Sampled Values) node type
 * @ingroup node
 * @{
 */

#pragma once

#include <cstdint>

#ifdef __APPLE__
  #include <net/ethernet.h>
#else
  #include <netinet/ether.h>
#endif

#include <libiec61850/hal_ethernet.h>
#include <libiec61850/goose_receiver.h>
#include <libiec61850/sv_subscriber.h>

#include <villas/list.h>
#include <villas/signal.h>

/* Forward declarations */
struct vnode;

enum class IEC61850Type {
	/* According to IEC 61850-7-2 */
	BOOLEAN,
	INT8,
	INT16,
	INT32,
	INT64,
	INT8U,
	INT16U,
	INT32U,
	INT64U,
	FLOAT32,
	FLOAT64,
	ENUMERATED,
	CODED_ENUM,
	OCTET_STRING,
	VISIBLE_STRING,
	OBJECTNAME,
	OBJECTREFERENCE,
	TIMESTAMP,
	ENTRYTIME,

	/* According to IEC 61850-8-1 */
	BITSTRING
};

struct iec61850_type_descriptor {
	const char *name;
	enum IEC61850Type iec_type;
	enum SignalType type;
	unsigned size;
	bool publisher;
	bool subscriber;
};

struct iec61850_receiver {
	char *interface;

	EthernetSocket socket;

	enum class Type {
		GOOSE,
		SAMPLED_VALUES
	} type;

	union {
		SVReceiver sv;
		GooseReceiver goose;
	};
};

/** @see node_type::type_start */
int iec61850_type_start(villas::node::SuperNode *sn);

/** @see node_type::type_stop */
int iec61850_type_stop();

const struct iec61850_type_descriptor * iec61850_lookup_type(const char *name);

int iec61850_parse_signals(json_t *json_signals, struct vlist *signals, struct vlist *node_signals);

struct iec61850_receiver * iec61850_receiver_lookup(enum iec61850_receiver::Type t, const char *intf);

struct iec61850_receiver * iec61850_receiver_create(enum iec61850_receiver::Type t, const char *intf);

int iec61850_receiver_start(struct iec61850_receiver *r);

int iec61850_receiver_stop(struct iec61850_receiver *r);

int iec61850_receiver_destroy(struct iec61850_receiver *r);

/** @} */
