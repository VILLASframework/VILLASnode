/** Moving window / Recursive DFT implementation based on HLS
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

#include "log.h"
#include "plugin.h"

#include "fpga/ip.h"
#include "fpga/card.h"
#include "fpga/ips/dft.h"

int dft_parse(struct fpga_ip *c)
{
	struct dft *dft = (struct dft *) &c->_vd;

	config_setting_t *cfg_harms;
	
	if (!config_setting_lookup_int(c->cfg, "period", &dft->period))
		cerror(c->cfg, "DFT IP core requires 'period' setting");
	
	if (!config_setting_lookup_int(c->cfg, "decimation", &dft->decimation))
		cerror(c->cfg, "DFT IP core requires 'decimation' setting");

	cfg_harms = config_setting_get_member(c->cfg, "harmonics");
	if (!cfg_harms)
		cerror(c->cfg, "DFT IP core requires 'harmonics' setting!");
	
	if (!config_setting_is_array(cfg_harms))
		cerror(c->cfg, "DFT IP core requires 'harmonics' to be an array of integers!");
	
	dft->num_harmonics = config_setting_length(cfg_harms);
	if (dft->num_harmonics <= 0)
		cerror(cfg_harms, "DFT IP core requires 'harmonics' to contain at least 1 integer!");
	
	dft->fharmonics = alloc(sizeof(float) * dft->num_harmonics);

	for (int i = 0; i < dft->num_harmonics; i++)
		dft->fharmonics[i] = (float) config_setting_get_int_elem(cfg_harms, i) / dft->period;

	return 0;
}

int dft_init(struct fpga_ip *c)
{
	int ret;
	
	struct fpga_card *f = c->card;
	struct dft *dft = (struct dft *) &c->_vd;

	XHls_dft *xdft = &dft->inst;
	XHls_dft_Config xdft_cfg = {
		.Ctrl_BaseAddress = (uintptr_t) f->map + c->baseaddr
	};

	ret = XHls_dft_CfgInitialize(xdft, &xdft_cfg);
	if (ret != XST_SUCCESS)
		return ret;

	int max_harmonics = XHls_dft_Get_fharmonics_TotalBytes(xdft) / sizeof(dft->fharmonics[0]);

	if (dft->num_harmonics > max_harmonics)
		error("DFT IP core supports a maximum of %u harmonics", max_harmonics);

	XHls_dft_Set_num_harmonics_V(xdft, dft->num_harmonics);
	
	XHls_dft_Set_decimation_V(xdft, dft->decimation);

	memcpy((void *) (uintptr_t) XHls_dft_Get_fharmonics_BaseAddress(xdft), dft->fharmonics, dft->num_harmonics * sizeof(dft->fharmonics[0]));

	XHls_dft_EnableAutoRestart(xdft);
	XHls_dft_Start(xdft);
	
	return 0;
}

int dft_destroy(struct fpga_ip *c)
{
	struct dft *dft = (struct dft *) &c->_vd;

	XHls_dft *xdft = &dft->inst;

	XHls_dft_DisableAutoRestart(xdft);

	if (dft->fharmonics) {
		free(dft->fharmonics);
		dft->fharmonics = NULL;
	}
	
	return 0;
}

static struct plugin p = {
	.name		= "Discrete Fourier Transform",
	.description	= "Perfom Discrete Fourier Transforms with variable number of harmonics on the FPGA",
	.type		= PLUGIN_TYPE_FPGA_IP,
	.ip		= {
		.vlnv	= { "acs.eonerc.rwth-aachen.de", "hls", "hls_dft", NULL },
		.type	= FPGA_IP_TYPE_MATH,
		.init	= dft_init,
		.destroy = dft_destroy,
		.parse	= dft_parse,
		.size	= sizeof(struct dft)
	}
};

REGISTER_PLUGIN(&p)