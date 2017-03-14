/** libcurl based advanced IO aka ADVIO.
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

#pragma once

#include <stdio.h>

#include <curl/curl.h>

#include "utils.h"

struct advio {
	CURL *curl;
	FILE *file;
	
	unsigned char hash[SHA_DIGEST_LENGTH];

	char mode[2];
	char *uri;
};

typedef struct advio AFILE;

/* The remaining functions from stdio are just replaced macros */

#define afeof(af)			feof((af)->file)
#define aftell(af)			ftell((af)->file)
#define arewind(af)			rewind((af)->file)
#define afread(ptr, sz, nitems, af)	fread(ptr, sz, nitems, (af)->file)
#define afwrite(ptr, sz, nitems, af)	fwrite(ptr, sz, nitems, (af)->file)

/* Extensions */
#define auri(af)			((af)->uri)
#define ahash(af)			((af)->hash)

AFILE *afopen(const char *url, const char *mode);

int afclose(AFILE *file);
int afflush(AFILE *file);
int adownload(AFILE *af);
int aupload(AFILE *af);
