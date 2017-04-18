/** Message related functions.
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

#ifdef __linux__
  #include <byteswap.h>
#elif defined(__PPC__) /* Xilinx toolchain */
  #include <xil_io.h>
  #define bswap_16(x)	Xil_EndianSwap16(x)
  #define bswap_32(x)	Xil_EndianSwap32(x)
#endif

#include "msg.h"

void msg_swap(struct msg *m)
{
	m->length   = bswap_16(m->length);
	m->sequence = bswap_32(m->sequence);
	m->ts.sec   = bswap_32(m->ts.sec);
	m->ts.nsec  = bswap_32(m->ts.nsec);
	
	for (int i = 0; i < m->length; i++)
		m->data[i].i = bswap_32(m->data[i].i);

	m->endian ^= 1;
}

void msg_fake_swap(struct GTSKT_msg *m)
{
	m->sequence = bswap_32(m->sequence);
	m->ts.sec   = bswap_32(m->ts.sec);
	m->ts.nsec  = bswap_32(m->ts.nsec);
	
	for (int i = 0; i < TOT_VALS_FROM_GTSKT; i++)
		m->data[i].i = bswap_32(m->data[i].i);
}

int msg_verify(struct msg *m)
{
	if      (m->version != MSG_VERSION)
		return -1;
	else if (m->type    != MSG_TYPE_DATA)
		return -2;
	else if ((m->length <= 0) || (m->length > MSG_VALUES))
		return -3;
	else if ((m->rsvd1 != 0)  || (m->rsvd2 != 0))
		return -4;
	else
		return 0;
}