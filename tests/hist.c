/** Unit tests for histogram
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

#include <criterion/criterion.h>

#include "hist.h"
#include "utils.h"

const double test_data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

/* Histogram of test_data with 200 buckets between -100 and 100 */
const int hist_result[] = {};

Test(hist, simple) {
	struct hist h;
	int ret;
	
	ret = hist_create(&h, -100, 100, 1);
	cr_assert_eq(ret, 0);
	
	for (int i = 0; i < ARRAY_LEN(test_data); i++)
		hist_put(&h, test_data[i]);
	
	cr_assert_float_eq(hist_mean(&h), 5.5, 1e-6);
	cr_assert_float_eq(hist_var(&h), 9.1666, 1e-3,);
	cr_assert_float_eq(hist_stddev(&h), 3.027650, 1e-6);
	
//	for (int i = 0; i < ARRAY_LEN(hist_result); i++)
//		cr_assert_eq()
	
	hist_destroy(&h);
}