/** Advanced IO
 *
 */

#ifndef _ADVIO_H_
#define _ADVIO_H_

#include <stdio.h>

#include <curl/curl.h>

enum advio_flags {
	ADVIO_MEM	= (1 << 0),	/**< Instead of a real file, a memory backed stdio stream is used. */
	ADVIO_DIRTY	= (1 << 1)	/**< The file contents have been modified. We need to upload. */
};

struct advio {
	int flags;

	CURL *curl;
	FILE *file;
	
	const char *url;
	
};

typedef struct advio AFILE;

AFILE *afopen(const char *url, const char *mode, int flags);

int afclose(AFILE *file);

int afflush(AFILE *file);

/* The remaining functions from stdio are just replaced macros */

#define afeof(af)		feof(af->file)

size_t afread(void *restrict ptr, size_t size, size_t nitems, AFILE *restrict stream);
size_t afwrite(const void *restrict ptr, size_t size, size_t nitems, AFILE *restrict stream);

#endif /* _ADVIO_H_ */