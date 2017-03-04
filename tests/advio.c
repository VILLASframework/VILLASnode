/** Unit tests for advio
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

#include <criterion/criterion.h>
#include <criterion/logging.h>

#include <villas/utils.h>
#include <villas/advio.h>

#include <errno.h>

Test(advio, download)
{
	AFILE *af;
	size_t len;
	int ret;
	char buffer[1024];
	
	const char *url = "http://www.textfiles.com/100/angela.art";

	af = afopen(url, "r", 0);
	cr_assert(af, "Failed to download file");
	cr_log_info("Opened file %s", url);

	while (!afeof(af)) {
		errno = 0;
		len = afread(buffer, 1, sizeof(buffer), af);
		cr_log_info("Read %zu bytes", len);

		fwrite(buffer, 1, len, stdout);
	}
	
	ret = afclose(af);
	cr_assert_eq(ret, 0, "Failed to close/upload file");
}

Test(advio, upload)
{
	AFILE *af;
	size_t len1, len2;
	int ret;
	
	const char *uri = "file:///tmp/test.txt";
	char rnd[128];
	char buffer[128];
	
	/* Get some random bytes */
	len1 = read_random(rnd, sizeof(rnd));

	/* Open file for writing */
	af = afopen(uri, "w+", 0);
	cr_assert(af, "Failed to download file");
	
	len2 = afwrite(rnd, 1, len1, af);
	cr_assert_eq(len1, len2);
	
	ret = afclose(af);
	cr_assert_eq(ret, 0, "Failed to close/upload file");
	
	/* Open for reading and comparison */
	af = afopen(uri, "r", 0);
	cr_assert(af, "Failed to download file");
	
	len2 = afread(buffer, 1, len1, af);
	cr_assert_arr_eq(rnd, buffer, len2);
	
	ret = afclose(af);
	cr_assert(ret == 0, "Failed to close file");
}