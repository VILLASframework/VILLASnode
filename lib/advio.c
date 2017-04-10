/** libcurl based advanced IO aka ADVIO.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>

#include <curl/curl.h>

#include "utils.h"
#include "config.h"
#include "advio.h"

#define BAR_WIDTH 60 /**< How wide you want the progress meter to be. */

static int advio_xferinfo(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
	struct advio *af = (struct advio *) p;
	double curtime = 0;

	curl_easy_getinfo(af->curl, CURLINFO_TOTAL_TIME, &curtime);

	// ensure that the file to be downloaded is not empty
	// because that would cause a division by zero error later on
	if (dltotal <= 0.0)
		return 0;

	double frac = dlnow / dltotal;

	// part of the progressmeter that's already "full"
	int dotz = round(frac * BAR_WIDTH);

	// create the "meter"
	fprintf(stderr, "%3.0f%% in %f s (%" CURL_FORMAT_CURL_OFF_T " / %" CURL_FORMAT_CURL_OFF_T ") [", frac * 100, curtime, dlnow, dltotal);

	// part  that's full already
	int i = 0;
	for ( ; i < dotz; i++)
		fprintf(stderr, "=");

	// remaining part (spaces)
	for ( ; i < BAR_WIDTH; i++)
		fprintf(stderr, " ");

	// and back to line begin - do not forget the fflush to avoid output buffering problems!
	fprintf(stderr, "]\r");
	fflush(stderr);

	return 0;
}

AFILE * afopen(const char *uri, const char *mode)
{
	int ret;

	AFILE *af = alloc(sizeof(AFILE));
	
	strncpy(af->mode, mode, sizeof(af->mode));
	
	af->uri = strdup(uri);
	if (!af->uri)
		goto out2;

	af->file = tmpfile();
	if (!af->file)
		goto out2;
	
	af->curl = curl_easy_init();
	if (!af->curl)
		goto out1;

	/* Setup libcurl handle */
#if LIBCURL_VERSION_NUM >= 0x072d00
	curl_easy_setopt(af->curl, CURLOPT_DEFAULT_PROTOCOL, "file");
#endif
	curl_easy_setopt(af->curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(af->curl, CURLOPT_UPLOAD, 0L);
	curl_easy_setopt(af->curl, CURLOPT_USERAGENT, USER_AGENT);
	curl_easy_setopt(af->curl, CURLOPT_URL, uri);
	curl_easy_setopt(af->curl, CURLOPT_WRITEDATA, af->file);
	curl_easy_setopt(af->curl, CURLOPT_XFERINFOFUNCTION, advio_xferinfo);
	curl_easy_setopt(af->curl, CURLOPT_XFERINFODATA, af);
	curl_easy_setopt(af->curl, CURLOPT_NOPROGRESS, 0L);
	
	ret = adownload(af);
	if (ret)
		goto out0;

	return af;

out0:	curl_easy_cleanup(af->curl);
out1:	fclose(af->file);
out2:	free(af->uri);
	free(af);

	return NULL;
}

int afclose(AFILE *af)
{
	int ret;
	
	ret = afflush(af);

	curl_easy_cleanup(af->curl);
	fclose(af->file);
	
	free(af->uri);
	free(af);
	
	return ret;
}

int afflush(AFILE *af)
{
	bool dirty;
	unsigned char hash[SHA_DIGEST_LENGTH];
	
	/* Check if fle was modified on disk by comparing hashes */
	sha1sum(af->file, hash);
	dirty = memcmp(hash, af->hash, sizeof(hash));
	
	if (dirty)
		return aupload(af);

	return 0;
}

int aupload(AFILE *af)
{
	CURLcode res;
	long pos;
	int ret;

	ret = fflush(af->file);
	if (ret)
		return ret;

	curl_easy_setopt(af->curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(af->curl, CURLOPT_READDATA, af->file);

	pos = ftell(af->file); /* Remember old stream pointer */
	fseek(af->file, 0, SEEK_SET);

	res = curl_easy_perform(af->curl);

	fprintf(stderr, "\e[2K");
	fflush(stderr); /* do not continue in the same line as the progress bar */

	fseek(af->file, pos, SEEK_SET); /* Restore old stream pointer */
	
	if (res != CURLE_OK)
		return -1;

	sha1sum(af->file, af->hash);

	return 0;
}

int adownload(AFILE *af)
{
	CURLcode res;
	long code;
	int ret;

	fseek(af->file, 0, SEEK_SET);

	res = curl_easy_perform(af->curl);
	
	fprintf(stderr, "\e[2K");
	fflush(stderr); /* do not continue in the same line as the progress bar */
	
	switch (res) {
		case CURLE_OK:
			curl_easy_getinfo(af->curl, CURLINFO_RESPONSE_CODE, &code);
			switch (code) {
				case   0:
				case 200: goto exist;
				case 404: goto notexist;
				default:  return -1;
			}

		/* The following error codes indicate that the file does not exist
		 * Check the fopen mode to see if we should continue with an emoty file */ 
		case CURLE_FILE_COULDNT_READ_FILE:
		case CURLE_TFTP_NOTFOUND:
		case CURLE_REMOTE_FILE_NOT_FOUND:
			goto notexist;

		/* If libcurl does not know the protocol, we will try it with the stdio */
		case CURLE_UNSUPPORTED_PROTOCOL:
			af->file = fopen(af->uri, af->mode);
			if (!af->file)
				return -1;
		
		default:
			error("Failed to fetch file: %s: %s\n", af->uri, curl_easy_strerror(res));
			return -1;
	}


notexist: /* File does not exist */

	/* According to mode the file must exist! */
	if (af->mode[1] != '+' || (af->mode[0] != 'w' && af->mode[0] != 'a')) {
		errno = ENOENT;
		return -1;
	}

	/* If we receive a 404, we discard the already received error page
	 * and start with an empty file. */
	fflush(af->file);
	ret = ftruncate(fileno(af->file), 0);
	if (ret)
		return ret;

exist: /* File exists */
	if (af->mode[0] == 'a')
		fseek(af->file, 0, SEEK_END);
	else if (af->mode[0] == 'r' || af->mode[0] == 'w')
		fseek(af->file, 0, SEEK_SET);
	
	sha1sum(af->file, af->hash);

	return 0;
}
