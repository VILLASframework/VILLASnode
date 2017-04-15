#pragma once
/** JSON serializtion of various objects.
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

#ifdef WITH_JANSSON

#include <jansson.h>
#include <libconfig.h>

#include "sample.h"

/* Convert a libconfig object to a libjansson object */
json_t * config_to_json(config_setting_t *cfg);

int json_to_config(json_t *json, config_setting_t *parent);

int sample_io_json_pack(json_t **j, struct sample *s, int flags);

int sample_io_json_unpack(json_t *j, struct sample *s, int *flags);

int sample_io_json_fprint(FILE *f, struct sample *s, int flags);

int sample_io_json_fscan(FILE *f, struct sample *s, int *flags);
#endif
