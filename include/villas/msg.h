/** Message related functions
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

#pragma once

#include <stdio.h>

#include "msg_format.h"

struct node;

/** Swaps the byte order of the header part of struct msg.
 *
 * Message can either be transmitted in little or big endian
 * format. The actual endianess for a message is defined by the
 * msg::endian field. This covers msg::length, msg::sequence, msg::data and msg::ts fields.
 * Received message are usally converted to the endianess of the host.
 * This is required for further sanity checks of the sequence number
 * or parsing of the data.
 *
 * @param m A pointer to the message
 */
void msg_hdr_swap(struct msg *m);

/** Check the consistency of a message.
 *
 * The functions checks the header fields of a message.
 *
 * @param m A pointer to the message
 * @retval 0 The message header is valid.
 * @retval <0 The message header is invalid.
 */
int msg_verify(struct msg *m);