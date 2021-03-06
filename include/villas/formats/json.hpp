/** JSON serializtion sample data.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014-2020, Institute for Automation of Complex Power Systems, EONERC
 * @license GNU General Public License (version 3)
 *
 * VILLASnode
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************************/

#pragma once

#include <jansson.h>

#include <villas/format.hpp>

/* Forward declarations */
struct sample;

namespace villas {
namespace node {

class JsonFormat : public Format {

protected:
	static enum SignalType detect(const json_t *val);

	json_t * packTimestamps(const struct sample *smp);
	int unpackTimestamps(json_t *json_ts, struct sample *smp);

	virtual int packSample(json_t **j, const struct sample *smp);
	virtual int packSamples(json_t **j, const struct sample * const smps[], unsigned cnt);
	virtual int unpackSample(json_t *json_smp, struct sample *smp);
	virtual int unpackSamples(json_t *json_smps, struct sample * const smps[], unsigned cnt);

	int dump_flags;

public:
	JsonFormat(int fl) :
		Format(fl),
		dump_flags(0)
	{ }

	int sscan(const char *buf, size_t len, size_t *rbytes, struct sample * const smps[], unsigned cnt);
	int sprint(char *buf, size_t len, size_t *wbytes, const struct sample * const smps[], unsigned cnt);

	int print(FILE *f, const struct sample * const smps[], unsigned cnt);
	int scan(FILE *f, struct sample * const smps[], unsigned cnt);

	virtual void parse(json_t *json);
};

} /* namespace node */
} /* namespace villas */
