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
#include "msg_format.h"

void webmsg_ntoh(struct webmsg *m)
{
	msg_hdr_ntoh(m);

	for (int i = 0; i < m->length; i++)
		m->data[i].i = ntohl(m->data[i].i);
}

void msg_hton(struct webmsg *m)
{
	for (int i = 0; i < m->length; i++)
		m->data[i].i = htonl(m->data[i].i);

	webmsg_hdr_hton(m);
}

void webmsg_hdr_hton(struct webmsg *m)
{
	m->length   = htole16(m->length);
	m->sequence = htonle32(m->sequence);
	m->ts.sec   = htonle32(m->ts.sec);
	m->ts.nsec  = htonle32(m->ts.nsec);
}

void webmsg_hdr_ntoh(struct webmsg *m)
{
	m->length   = le16tohs(m->length);
	m->sequence = le32tohl(m->sequence);
	m->ts.sec   = le32tohl(m->ts.sec);
	m->ts.nsec  = le32tohl(m->ts.nsec);
}

int webmsg_verify(struct webmsg *m)
{
	if      (m->version != WEBMSG_VERSION)
		return -1;
	else if (m->type    != WEBMSG_TYPE_DATA)
		return -2;
	else if ((m->rsvd1 != 0)  || (m->rsvd2 != 0))
		return -3;
	else
		return 0;
}