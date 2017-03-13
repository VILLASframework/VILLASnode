/** libcurl based remote IO
 *
 * Implements an fopen() abstraction allowing reading from URLs
 *
 * This file introduces a c library buffered I/O interface to
 * URL reads it supports fopen(), fread(), fgets(), feof(), fclose(),
 * rewind(). Supported functions have identical prototypes to their normal c
 * lib namesakes and are preceaded by a .
 *
 * Using this code you can replace your program's fopen() with afopen()
 * and fread() with afread() and it become possible to read remote streams
 * instead of (only) local files. Local files (ie those that can be directly
 * fopened) will drop back to using the underlying clib implementations
 *
 * This example requires libcurl 7.9.7 or later.
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

enum advio_flags {
	ADVIO_DIRTY	= (1 << 0)	/**< The file contents have been modified. We need to upload. */
};

struct advio {
	int flags;

	CURL *curl;
	FILE *file;

	const char *uri;
};

typedef struct advio AFILE;

AFILE *afopen(const char *url, const char *mode, int flags);

int afclose(AFILE *file);

int afflush(AFILE *file);

/* The remaining functions from stdio are just replaced macros */

#define afeof(af)		feof((af)->file)
#define aftell(af)		ftell((af)->file)
#define arewind(af)		rewind((af)->file)

size_t afread(void *restrict ptr, size_t size, size_t nitems, AFILE *restrict stream);
size_t afwrite(const void *restrict ptr, size_t size, size_t nitems, AFILE *restrict stream);