#include "crypt.h"

#include <openssl/sha.h>

int sha1sum(FILE *f, unsigned char *sha1)
{
	SHA_CTX c;
	char buf[512];
	ssize_t bytes;
	long seek;
	
	seek = ftell(f);
	fseek(f, 0, SEEK_SET);

	SHA1_Init(&c);

	bytes = fread(buf, 1, 512, f);
	while (bytes > 0) {
		SHA1_Update(&c, buf, bytes);
		bytes = fread(buf, 1, 512, f);
	}

	SHA1_Final(sha1, &c);
	
	fseek(f, seek, SEEK_SET);

	return 0;
}

