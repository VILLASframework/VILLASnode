/** Vendor, Library, Name, Version (VLNV) tag.
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

/** @addtogroup fpga VILLASfpga
 * @{
 */

#ifndef _FPGA_VLNV_H_
#define _FPGA_VLNV_H_

/* Forward declarations */
struct list;

struct fpga_vlnv {
	char *vendor;
	char *library;
	char *name;
	char *version;
};

/** Return the first IP block in list \p l which matches the VLNV */
struct fpga_ip * fpga_vlnv_lookup(struct list *l, struct fpga_vlnv *v);

/** Check if IP block \p c matched VLNV. */
int fpga_vlnv_cmp(struct fpga_vlnv *a, struct fpga_vlnv *b);

/** Tokenizes VLNV \p vlnv and stores it into \p c */
int fpga_vlnv_parse(struct fpga_vlnv *c, const char *vlnv);

/** Release memory allocated by fpga_vlnv_parse(). */
int fpga_vlnv_destroy(struct fpga_vlnv *v);

#endif /** _FPGA_VLNV_H_ @} */