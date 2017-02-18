/** FPGA card
 *
 * This class represents a FPGA device.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2017, Steffen Vogel
 *   This file is part of VILLASnode. All Rights Reserved. Proprietary and confidential.
 *   Unauthorized copying of this file, via any medium is strictly prohibited. 
 */
/**
 * @addtogroup fpga VILLASfpga
 * @{
 */

#pragma once

enum fpga_card_state {
	FPGA_CARD_STATE_UNKOWN,
	FPGA_CARD_STATE_RESETTED,
	FPGA_CARD_STATE_INITIALIZED
};

struct fpga_card {
	char *name;			/**< The name of the FPGA card */

	enum fpga_card_state state;	/**< The state of this FPGA card. */

	struct pci_dev filter;		/**< Filter for PCI device. */
	struct vfio_dev vd;		/**< VFIO device handle. */

	int do_reset;			/**< Reset VILLASfpga during startup? */
	int affinity;			/**< Affinity for MSI interrupts */

	struct list ips;		/**< List of IP components on FPGA. */

	char *map;			/**< PCI BAR0 mapping for register access */

	size_t maplen;
	size_t dmalen;

	/* Some IP cores are special and referenced here */
	struct fpga_ip *intc;
	struct fpga_ip *reset;
	struct fpga_ip *sw;
	
	config_setting_t *cfg;
};

int fpga_card_parse(struct fpga_card *c, config_setting_t *cfg);

/** Initialize FPGA card and its IP components. */
int fpga_card_init(struct fpga_card *c);

int fpga_card_destroy(struct fpga_card *c);

/** Check if the FPGA card configuration is plausible. */
int fpga_card_check(struct fpga_card *c);

/** Reset the FPGA to a known state */
int fpga_card_reset(struct fpga_card *c);

/** @} */