/** Message related functions
 *
 * @file
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

#include <cstdlib>

#include <villas/format.hpp>

/* Forward declarations. */
struct sample;
struct msg;

namespace villas {
namespace node {

class VillasBinaryFormat : public BinaryFormat {

public:
	bool web;

	VillasBinaryFormat(int fl, bool w) :
		BinaryFormat(fl),
		web(w)
	{ }

	int sscan(const char *buf, size_t len, size_t *rbytes, struct sample * const smps[], unsigned cnt);
	int sprint(char *buf, size_t len, size_t *wbytes, const struct sample * const smps[], unsigned cnt);
};


template<bool web = false>
class VillasBinaryFormatPlugin : public FormatFactory {

public:
	using FormatFactory::FormatFactory;

	virtual Format * make()
	{
		return new VillasBinaryFormat((int) SampleFlags::HAS_TS_ORIGIN | (int) SampleFlags::HAS_SEQUENCE | (int) SampleFlags::HAS_DATA, web);
	}

	/// Get plugin name
	virtual std::string
	getName() const
	{
		std::stringstream ss;

		ss << "villas." << (web ? "web" : "binary");

		return ss.str();
	}

	/// Get plugin description
	virtual std::string
	getDescription() const
	{
		std::stringstream ss;

		ss << "VILLAS binary network format";

		if (web)
			ss << " for WebSockets";

		return ss.str();
	}
};

} /* namespace node */
} /* namespace villas */
