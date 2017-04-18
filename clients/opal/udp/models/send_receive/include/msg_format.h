/** Message format
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
 *********************************************************************************/

#ifndef _MSG_FORMAT_H_
#define _MSG_FORMAT_H_

#include <stdint.h>

#ifdef __linux__
 #define _BSD_SOURCE    	1
 #include <endian.h>
#elif defined(__PPC__) /* Xilinx toolchain */
 #include <lwip/arch.h>
#endif

#include "config.h"

/** Maximum number of dword values in a message */
#define MSG_VALUES		MAX_VALUES

/** The current version number for the message format */
#define MSG_VERSION		1

/** @todo Implement more message types */
#define MSG_TYPE_DATA		0 /**< Message contains float values */
#define MSG_TYPE_START		1 /**< Message marks the beginning of a new simulation case */
#define MSG_TYPE_STOP		2 /**< Message marks the end of a simulation case */

#define MSG_ENDIAN_LITTLE	0 /**< Message values are in little endian format (float too!) */
#define MSG_ENDIAN_BIG		1 /**< Message values are in bit endian format */

#if BYTE_ORDER == LITTLE_ENDIAN
 #define MSG_ENDIAN_HOST	MSG_ENDIAN_LITTLE
#elif BYTE_ORDER == BIG_ENDIAN
 #define MSG_ENDIAN_HOST	MSG_ENDIAN_BIG
#else
 #error "Unknown byte order!"
#endif

/** The total length of a message */
#define MSG_LEN(msg)		(4 * ((msg)->length + 4))		/* @todo recheck this logic */
#define GTSKT_MSG_LEN(msg)		(4 * (TOT_VALS_TO_GTSKT + 3))

#define MSG_TS(msg) (struct timespec) {	\
	.tv_sec  = (msg)->ts.sec,	\
	.tv_nsec = (msg)->ts.nsec	\
}

/** Initialize a VILLASnode message */
#define MSG_INIT(i) (struct msg) {	\
	.version = MSG_VERSION,		\
	.type = MSG_TYPE_DATA,		\
	.endian = MSG_ENDIAN_HOST,	\
	.length = i,			\
	.sequence = 0,			\
	.rsvd1 = 0, .rsvd2 = 0		\
}

/** Initialize a GTSKT message */
#define GTSKT_MSG_INIT(i) (struct GTSKT_msg) {	\
	.sequence = 0,		\
	.ts.sec = 0,		\
	.ts.nsec = 0	\
}

/** This message format is used by all clients
 *
 * @diafile msg_format.dia
 **/
struct msg
{
#if BYTE_ORDER == BIG_ENDIAN
	unsigned version: 4;	/**< Specifies the format of the remaining message (see MGS_VERSION) */
	unsigned type	: 2;	/**< Data or control message (see MSG_TYPE_*) */
	unsigned endian	: 1;	/**< Specifies the byteorder of the message (see MSG_ENDIAN_*) */
	unsigned rsvd1	: 1;	/**< Reserved bits */
#elif BYTE_ORDER == LITTLE_ENDIAN
	unsigned rsvd1	: 1;	/**< Reserved bits */
	unsigned endian	: 1;	/**< Specifies the byteorder of the message (see MSG_ENDIAN_*) */
	unsigned type	: 2;	/**< Data or control message (see MSG_TYPE_*) */
	unsigned version: 4;	/**< Specifies the format of the remaining message (see MGS_VERSION) */
#endif
	unsigned rsvd2	: 8;	/**< Reserved bits */
	
	uint16_t length;	/**< The number of values in msg::data[]. Endianess is specified in msg::endian. */
	uint32_t sequence;	/**< The sequence number is incremented by one for consecutive messages. Endianess is specified in msg::endian. */
	
	/** A timestamp per message. Endianess is specified in msg::endian. */
	struct {
		uint32_t sec;	/**< Seconds     since 1970-01-01 00:00:00 */
		uint32_t nsec;	/**< Nanoseconds of the current second. */
	} ts;

	/** The message payload. Endianess is specified in msg::endian. */
	union {
		float    f;	/**< Floating point values (note msg::endian) */
		uint32_t i;	/**< Integer values (note msg::endian) */
	} data[MSG_VALUES];
} __attribute__((aligned(64), packed));

struct GTSKT_msg
{
	int32_t sequence;
	
	/** A timestamp per message */
	struct {
		uint32_t sec;	/**< Seconds     since 1970-01-01 00:00:00 */
		uint32_t nsec;	/**< Nanoseconds of the current second. */
	} ts;
	
	union {
		float   f;	/**< Floating point values*/
		uint32_t i;	/**< Integer values*/	/** @todo what if data sent is signed */
	} data[MSG_VALUES];
} __attribute__((aligned(64), packed));

#endif /* _MSG_FORMAT_H_ */
