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

#include <unistd.h>

#include <criterion/criterion.h>
#include <criterion/logging.h>

#include <villas/utils.h>
#include <villas/advio.h>

/** This URI points to a Sciebo share which contains some test files.
 * The Sciebo share is read/write accessible via WebDAV. */
#define BASE_URI "https://1Nrd46fZX8HbggT:badpass@rwth-aachen.sciebo.de/public.php/webdav/node/tests"

Test(advio, download)
{
	AFILE *af;
	int ret;
	size_t len;
	char buffer[64];
	char expect[64] = "ook4iekohC2Teegoghu6ayoo1OThooregheebaet8Zod1angah0che7quai4ID7A";

	af = afopen(BASE_URI "/download" , "r");
	cr_assert(af, "Failed to download file");

	len = afread(buffer, 1, sizeof(buffer), af);
	cr_assert_gt(len, 0, "len=%zu, feof=%u", len, afeof(af));
	
	cr_assert_arr_eq(buffer, expect, sizeof(expect));

	ret = afclose(af);
	cr_assert_eq(ret, 0, "Failed to close file");
}

Test(advio, upload)
{
	AFILE *af;
	int ret;
	size_t len;
	
	char upload[64];
	char buffer[64];
	
	/* Get some random data to upload */
	len = read_random(upload, sizeof(upload));
	cr_assert_eq(len, sizeof(upload));
	
	/* Open file for writing */
	af = afopen(BASE_URI "/upload", "w+");
	cr_assert(af, "Failed to download file");
	
	len = afwrite(upload, 1, sizeof(upload), af);
	cr_assert_eq(len, sizeof(upload));
	
	ret = afclose(af);
	cr_assert_eq(ret, 0, "Failed to close/upload file");
	
	/* Open for reading and comparison */
	af = afopen(BASE_URI "/upload", "r");
	cr_assert(af, "Failed to download file");
	
	len = afread(buffer, 1, sizeof(upload), af);
	cr_assert_eq(len, sizeof(upload));
	
	cr_assert_arr_eq(buffer, upload, len);
	
	ret = afclose(af);
	cr_assert(ret == 0, "Failed to close file");
}

Test(advio, append)
{
	AFILE *af;
	int ret;
	size_t len;

	char append1[64] = "xa5gieTohlei9iu1uVaePae6Iboh3eeheeme5iejue5sheshae4uzisha9Faesei";
	char append2[64] = "bitheeRae7igee2miepahJaefoGad1Ooxeif0Mooch4eojoumueYahn4ohc9poo2";
	char expect[128] = "xa5gieTohlei9iu1uVaePae6Iboh3eeheeme5iejue5sheshae4uzisha9FaeseibitheeRae7igee2miepahJaefoGad1Ooxeif0Mooch4eojoumueYahn4ohc9poo2";
	char buffer[128];

	/* Open file for writing first chunk */
	af = afopen(BASE_URI "/append", "w+");
	cr_assert(af, "Failed to download file");

	/* The append file might already exist and be not empty from a previous run. */
	ret = ftruncate(afileno(af), 0);
	cr_assert_eq(ret, 0);
	
	char c;
	fseek(af->file, 0, SEEK_SET);
	if (af->file) {
	    while ((c = getc(af->file)) != EOF)
	        putchar(c);
	}

	len = afwrite(append1, 1, sizeof(append1), af);
	cr_assert_eq(len, sizeof(append1));

	ret = afclose(af);
	cr_assert_eq(ret, 0, "Failed to close/upload file");

	/* Open file for writing second chunk */
	af = afopen(BASE_URI "/append", "a");
	cr_assert(af, "Failed to download file");
	
	len = afwrite(append2, 1, sizeof(append2), af);
	cr_assert_eq(len, sizeof(append2));
	
	ret = afclose(af);
	cr_assert_eq(ret, 0, "Failed to close/upload file");
	
	/* Open for reading and comparison */
	af = afopen(BASE_URI "/append", "r");
	cr_assert(af, "Failed to download file");
	
	len = afread(buffer, 1, sizeof(buffer), af);
	cr_assert_eq(len, sizeof(buffer));
	
	ret = afclose(af);
	cr_assert(ret == 0, "Failed to close file");

	cr_assert_arr_eq(buffer, expect, sizeof(expect));
}