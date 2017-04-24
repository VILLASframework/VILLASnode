/** Websocket message related functions.
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

#include <endian.h>

#include "webmsg.h"
#include "webmsg_format.h"

void webmsg_ntoh(struct webmsg *m)
{
	webmsg_hdr_ntoh(m);

	for (int i = 0; i < m->length; i++)
		m->data[i].i = le32toh(m->data[i].i);
}

void webmsg_hton(struct webmsg *m)
{
	for (int i = 0; i < m->length; i++)
		m->data[i].i = htole32(m->data[i].i);

	webmsg_hdr_hton(m);
}

void webmsg_hdr_hton(struct webmsg *m)
{
	m->length   = htole16(m->length);
	m->sequence = htole32(m->sequence);
	m->ts.sec   = htole32(m->ts.sec);
	m->ts.nsec  = htole32(m->ts.nsec);
}

void webmsg_hdr_ntoh(struct webmsg *m)
{
	m->length   = le16toh(m->length);
	m->sequence = le32toh(m->sequence);
	m->ts.sec   = le32toh(m->ts.sec);
	m->ts.nsec  = le32toh(m->ts.nsec);
}

int webmsg_verify(struct webmsg *m)
{
	if      (m->version != WEBMSG_VERSION)
		return -1;
	else if (m->type    != WEBMSG_TYPE_DATA)
		return -2;
	else if (m->rsvd1 != 0)
		return -3;
	else
		return 0;
}