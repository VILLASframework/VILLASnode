/** Internal hook functions.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2016, Institute for Automation of Complex Power Systems, EONERC
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

#include "hook.h"
#include "timing.h"
#include "sample.h"
#include "path.h"
#include "utils.h"

/**
 * Hook to calculate jitter between GTNET-SKT GPS timestamp and Villas node NTP timestamp.
 *
 * Drawbacks: No protection for out of order packets. Also always positive delay assumed,
 * so GPS timestamp should be earlier than NTP timestamp.
 */
 #define CALC_GPS_NTP_DELAY 1
 #define GPS_NTP_DELAY_WIN_SIZE 8

#if CALC_GPS_NTP_DELAY == 1
REGISTER_HOOK("ntp_jitter_calc", "Calc jitter b/w GPS and NTP timstamp", 0, 0, hook_jitter_ts, HOOK_AUTO | HOOK_READ)
#endif
int hook_jitter_ts(struct hook *h, int when, struct hook_info *j)
{
	/* @todo add data to each node, not just displaying on the screen, doesn't work for more than one node */
	struct timespec now = time_now();
	assert(j->smps);
	
	static int64_t jitter_val[GPS_NTP_DELAY_WIN_SIZE] = {0};	// no protection for out of order pkts
	static int64_t last_delta[GPS_NTP_DELAY_WIN_SIZE] = {0};
	static int64_t moving_avg[GPS_NTP_DELAY_WIN_SIZE] = {0};
	static int64_t moving_sqrd_avg[GPS_NTP_DELAY_WIN_SIZE] = {0};
	static int64_t moving_var[GPS_NTP_DELAY_WIN_SIZE] = {0};
	static int64_t temp_sum = 0, temp_sqrd_sum = 0;
	static int curr_count = 1;
	int64_t delay_sec, delay_nsec, curr_temp_delta;
	
	for(int i = 0; i < j->cnt; i++) {
		delay_sec = j->smps[i]->ts.origin.tv_sec - now.tv_sec;
		delay_nsec = j->smps[i]->ts.origin.tv_nsec - now.tv_nsec;
		
		curr_temp_delta = delay_sec*1000000000 + delay_nsec; // Assuming always positive values i.e. GPS TS older than NTP one
		temp_sum = temp_sum + curr_temp_delta - last_delta[curr_count];
		moving_avg[curr_count] = temp_sum/(GPS_NTP_DELAY_WIN_SIZE-1); // Will be valid after GPS_NTP_DELAY_WIN_SIZE-1 initial values
		
		temp_sqrd_sum = temp_sqrd_sum + (curr_temp_delta*curr_temp_delta) - (last_delta[curr_count]*last_delta[curr_count]);
		moving_sqrd_avg[curr_count] = temp_sqrd_sum/(GPS_NTP_DELAY_WIN_SIZE-1);
		moving_var[curr_count] = moving_sqrd_avg[curr_count] - (moving_avg[curr_count]*moving_avg[curr_count]); /* further test this logic */
		
		info("temp_sqrd_sum %ld, moving_sqrd_avg %ld, moving_var %ld, moving_avg sqrd %ld", temp_sqrd_sum, moving_sqrd_avg[curr_count], moving_var[curr_count], (moving_avg[curr_count]*moving_avg[curr_count]));
		
		last_delta[curr_count] = curr_temp_delta;
		
		/* Jitter calc formula as used in Wireshark according to RFC3550 (RTP)
			D(i,j) = (Rj-Ri)-(Sj-Si) = (Rj-Sj)-(Ri-Si)
			J(i) = J(i-1)+(|D(i-1,i)|-J(i-1))/16
		*/
		jitter_val[curr_count] = jitter_val[curr_count-1] + (abs(last_delta[curr_count]) - jitter_val[curr_count-1])/16;
		
		info("jitter value %ld nsec, last jitter val %ld nsec, delay value %ld nsec", jitter_val[curr_count], jitter_val[curr_count-1], last_delta[curr_count]);
		
		for(int ctmp = 0; ctmp < GPS_NTP_DELAY_WIN_SIZE; ctmp++) {
			info("%d jitter value %ld nsec, last delta %ld nsec, moving avg %ld, moving variance %ld, moving sqrd avg %ld", ctmp, jitter_val[ctmp], last_delta[ctmp], moving_avg[ctmp], moving_var[ctmp], moving_sqrd_avg[ctmp]);
		}
		
		if(curr_count == GPS_NTP_DELAY_WIN_SIZE-1) {
			jitter_val[0] = jitter_val[curr_count];
			last_delta[0] = last_delta[curr_count];
			moving_avg[0] = moving_avg[curr_count];
			moving_var[0] = moving_var[curr_count];
			moving_sqrd_avg[0] = moving_sqrd_avg[curr_count];
			curr_count = 0;
		}
		curr_count++;
	}
	return j->cnt;
}

REGISTER_HOOK("fix_ts", "Update timestamps of sample if not set", 0, 0, hook_fix_ts, HOOK_AUTO | HOOK_READ)
int hook_fix_ts(struct hook *h, int when, struct hook_info *j)
{
	struct timespec now = time_now();

	assert(j->smps);

	for (int i = 0; i < j->cnt; i++) {
		/* Check for missing receive timestamp
		 * Usually node_type::read() should update the receive timestamp.
		 * An example would be to use hardware timestamp capabilities of
		 * modern NICs.
		 */
		if ((j->smps[i]->ts.received.tv_sec ==  0 && j->smps[i]->ts.received.tv_nsec ==  0) ||
		    (j->smps[i]->ts.received.tv_sec == -1 && j->smps[i]->ts.received.tv_nsec == -1))
			j->smps[i]->ts.received = now;

		/* Check for missing origin timestamp */
		if ((j->smps[i]->ts.origin.tv_sec ==  0 && j->smps[i]->ts.origin.tv_nsec ==  0) ||
		    (j->smps[i]->ts.origin.tv_sec == -1 && j->smps[i]->ts.origin.tv_nsec == -1))
			j->smps[i]->ts.origin = now;
	}

	return j->cnt;
}

REGISTER_HOOK("restart", "Call restart hooks for current path", 1, 1, hook_restart, HOOK_AUTO | HOOK_READ)
int hook_restart(struct hook *h, int when, struct hook_info *j)
{
	assert(j->smps);
	assert(j->path);

	for (int i = 0; i < j->cnt; i++) {
		h->last = j->smps[i];
		
		if (h->prev) {
			if (h->last->sequence  == 0 &&
			    h->prev->sequence <= UINT32_MAX - 32) {
				warn("Simulation for path %s restarted (prev->seq=%u, current->seq=%u)",
					path_name(j->path), h->prev->sequence, h->last->sequence);

				hook_run(j->path, &j->smps[i], j->cnt - i, HOOK_PATH_RESTART);
			}
		}
		
		h->prev = h->last;
	}

	return j->cnt;
}

REGISTER_HOOK("drop", "Drop messages with reordered sequence numbers", 3, 1, hook_drop, HOOK_AUTO | HOOK_READ)
int hook_drop(struct hook *h, int when, struct hook_info *j)
{
	int i, ok, dist;
	
	assert(j->smps);

	for (i = 0, ok = 0; i < j->cnt; i++) {
		h->last = j->smps[i];
		
		if (h->prev) {
			dist = h->last->sequence - (int32_t) h->prev->sequence;
			if (dist <= 0) {
				warn("Dropped sample: dist = %d, i = %d", dist, i);
				if (j->path && j->path->stats)
					stats_update(j->path->stats->delta, STATS_DROPPED, dist);
			}
			else {
				struct sample *tmp;
			
				tmp = j->smps[i];
				j->smps[i] = j->smps[ok];
				j->smps[ok++] = tmp;
			}
		
			/* To discard the first X samples in 'smps[]' we must
			 * shift them to the end of the 'smps[]' array.
			 * In case the hook returns a number 'ok' which is smaller than 'cnt',
			 * only the first 'ok' samples in 'smps[]' are accepted and further processed.
			 */
		}
		else {
			struct sample *tmp;
		
			tmp = j->smps[i];
			j->smps[i] = j->smps[ok];
			j->smps[ok++] = tmp;
		}

		h->prev = h->last;
	}

	return ok;
}
