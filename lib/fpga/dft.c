/** Moving window / Recursive DFT implementation based on HLS
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2015-2016, Steffen Vogel
 *   This file is part of S2SS. All Rights Reserved. Proprietary and confidential.
 *   Unauthorized copying of this file, via any medium is strictly prohibited.
 **********************************************************************************/

#include "log.h"
#include "plugin.h"

#include "fpga/ip.h"
#include "fpga/dft.h"

int dft_parse(struct ip *c)
{
	struct dft *dft = &c->dft;

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

int dft_init(struct ip *c)
{
	int ret;
	struct dft *dft = &c->dft;
	XHls_dft *xdft = &dft->inst;
	XHls_dft_Config xdft_cfg = {
		.Ctrl_BaseAddress = (uintptr_t) c->card->map + c->baseaddr
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

void dft_destroy(struct ip *c)
{
	struct dft *dft = &c->dft;
	XHls_dft *xdft = &dft->inst;

	XHls_dft_DisableAutoRestart(xdft);

	if (dft->fharmonics) {
		free(dft->fharmonics);
		dft->fharmonics = NULL;
	}
}

static struct plugin p = {
	.name		= "Discrete Fourier Transform",
	.description	= "Perfom Discrete Fourier Transforms with variable number of harmonics on the FPGA",
	.type		= PLUGIN_TYPE_FPGA_IP,
	.ip		= {
		.vlnv = { "acs.eonerc.rwth-aachen.de", "hls", "hls_dft", NULL },
		.init = dft_init,
		.destroy = dft_destroy,
		.parse = dft_parse
	}
};

REGISTER_PLUGIN(&p)