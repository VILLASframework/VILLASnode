/** Linux PCI helpers
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

/** @addtogroup fpga Kernel @{ */

#pragma once

#include "list.h"

#define PCI_SLOT(devfn)		(((devfn) >> 3) & 0x1f)
#define PCI_FUNC(devfn)		((devfn) & 0x07)

struct pci_device {
	struct {
		int vendor;
		int device;
		int class;
	} id;
	
	struct {
		int domain;
		int bus;
		int device;
		int function;
	} slot;			/**< Bus, Device, Function (BDF) */
};

struct pci {
	struct list devices; /**< List of available PCI devices in the system (struct pci_device) */
};

/** Initialize Linux PCI handle.
 *
 * This search for all available PCI devices under /sys/bus/pci
 *
 * @retval 0 Success. Everything went well.
 * @retval <0 Error. Something went wrong.
 */
int pci_init(struct pci *p);

/** Destroy handle. */
int pci_destroy(struct pci *p);

int pci_device_parse_slot(struct pci_device *f, const char *str, const char **error);

int pci_device_parse_id(struct pci_device *f, const char *str, const char **error);

int pci_device_compare(const struct pci_device *d, const struct pci_device *f);

struct pci_device * pci_lookup_device(struct pci *p, struct pci_device *filter);

/** Bind a new LKM to the PCI device */
int pci_attach_driver(struct pci_device *d, const char *driver);

/** Return the IOMMU group of this PCI device or -1 if the device is not in a group. */
int pci_get_iommu_group(struct pci_device *d);

/** @} */