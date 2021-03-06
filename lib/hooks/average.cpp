/** Average hook.
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

/** @addtogroup hooks Hook functions
 * @{
 */

#include <bitset>
#include <cstring>

#include <villas/hook.hpp>
#include <villas/node/exceptions.hpp>
#include <villas/signal.h>
#include <villas/sample.h>

namespace villas {
namespace node {

class AverageHook : public MultiSignalHook {

protected:
	unsigned offset;

public:
	AverageHook(struct vpath *p, struct vnode *n, int fl, int prio, bool en = true) :
		MultiSignalHook(p, n, fl, prio, en),
		offset(0)
	{ }

	virtual void prepare()
	{
		int ret;
		struct signal *avg_sig;

		assert(state == State::CHECKED);

		MultiSignalHook::prepare();

		/* Add averaged signal */
		avg_sig = signal_create("average", nullptr, SignalType::FLOAT);
		if (!avg_sig)
			throw RuntimeError("Failed to create new signal");

		ret = vlist_insert(&signals, offset, avg_sig);
		if (ret)
			throw RuntimeError("Failed to intialize list");

		state = State::PREPARED;
	}

	virtual void parse(json_t *json)
	{
		int ret;
		json_error_t err;

		assert(state != State::STARTED);

		MultiSignalHook::parse(json);

		ret = json_unpack_ex(json, &err, 0, "{ s: i }",
			"offset", &offset
		);
		if (ret)
			throw ConfigError(json, err, "node-config-hook-average");

		state = State::PARSED;
	}

	virtual Hook::Reason process(sample *smp)
	{
		double avg, sum = 0;
		int n = 0;

		assert(state == State::STARTED);

		for (unsigned index : signalIndices) {
			switch (sample_format(smp, index)) {
				case SignalType::INTEGER:
					sum += smp->data[index].i;
					break;

				case SignalType::FLOAT:
					sum += smp->data[index].f;
					break;

				case SignalType::INVALID:
				case SignalType::COMPLEX:
				case SignalType::BOOLEAN:
					return Hook::Reason::ERROR; /* not supported */
			}

			n++;
		}

		avg = sum / n;

		if (offset >= smp->length)
			return Reason::ERROR;

		sample_data_insert(smp, (union signal_data *) &avg, offset, 1);

		return Reason::OK;
	}
};

/* Register hook */
static char n[] = "average";
static char d[] = "Calculate average over some signals";
static HookPlugin<AverageHook, n , d, (int) Hook::Flags::PATH | (int) Hook::Flags::NODE_READ | (int) Hook::Flags::NODE_WRITE> p;

} /* namespace node */
} /* namespace villas */

/** @} */
