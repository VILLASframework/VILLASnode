/** Timer related helper functions
 *
 * These functions present a simpler interface to Xilinx' Timer Counter driver (XTmrCtr_*)
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2017, Steffen Vogel
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
 **********************************************************************************/

#include "config.h"
#include "plugin.h"

#include "fpga/ip.h"
#include "fpga/card.h"
#include "fpga/ips/timer.h"

int timer_init(struct fpga_ip *c)
{
	struct fpga_card *f = c->card;
	struct timer *tmr = (struct timer *) &c->_vd;

	XTmrCtr *xtmr = &tmr->inst;
	XTmrCtr_Config xtmr_cfg = {
		.BaseAddress = (uintptr_t) f->map + c->baseaddr,
		.SysClockFreqHz = FPGA_AXI_HZ
	};

	XTmrCtr_CfgInitialize(xtmr, &xtmr_cfg, (uintptr_t) f->map + c->baseaddr);
	XTmrCtr_InitHw(xtmr);
	
	return 0;
}

static struct plugin p = {
	.name		= "Xilinx's programmable timer / counter",
	.description	= "",
	.type		= PLUGIN_TYPE_FPGA_IP,
	.ip		= {
		.vlnv	= { "xilinx.com", "ip", "axi_timer", NULL },
		.type	= FPGA_IP_TYPE_MISC,
		.init	= timer_init
		.size	= sizeof(struct timer)
	}
};

REGISTER_PLUGIN(&p)