/** API buffer for sending and receiving data from libwebsockets.
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

#include <jansson.h>

#include "compat.h"
#include "assert.h"
#include "utils.h"
#include "web/buffer.h"

int web_buffer_init(struct web_buffer *b, enum lws_write_protocol prot)
{
	assert(b->state == STATE_DESTROYED);
	
	b->protocol = prot;
	b->size = 0;
	b->len = 0;
	b->buffer = NULL;
	b->prefix = b->protocol == LWS_WRITE_TEXT || b->protocol == LWS_WRITE_BINARY ? LWS_PRE : 0;
	
	b->state = STATE_INITIALIZED;

	return 0;
}

int web_buffer_destroy(struct web_buffer *b)
{
	if (b->state == STATE_DESTROYED)
		return 0;
	
	if (b->buffer)
		free(b->buffer);
	
	b->state = STATE_DESTROYED;
	
	return 0;
}

int web_buffer_write(struct web_buffer *b, struct lws *w)
{
	int ret, len, sent = 0;
	unsigned char *chunk;
	
	assert(b->state == STATE_INITIALIZED);
	
	if (b->len <= 0)
		return 0;
	
	do {
		chunk = (unsigned char *) b->buffer + b->prefix + sent;
		len = strlen(b->buffer + b->prefix);

		ret = lws_write(w, chunk, len, b->protocol);
		if (ret < 0)
			break;

		sent += ret + 1;
	} while (sent < b->len);

	web_buffer_read(b, NULL, sent); /* drop sent bytes from the buffer*/
	
	return sent;
}

int web_buffer_read(struct web_buffer *b, char *out, size_t len)
{
	assert(b->state == STATE_INITIALIZED);

	if (len > b->len)
		len = b->len;

	if (out)
		memcpy(out, b->buffer + b->prefix, len);
	
	memmove(b->buffer + b->prefix, b->buffer + b->prefix + len, b->len - len);
	b->len -= len;
	
	return 0;
}

int web_buffer_read_json(struct web_buffer *b, json_t **req)
{
	json_error_t e;

	assert(b->state == STATE_INITIALIZED);

	if (b->len <= 0)
		return -1;

	*req = json_loadb(b->buffer + b->prefix, b->len, JSON_DISABLE_EOF_CHECK, &e);
	if (!*req)
		return -2;
	
	web_buffer_read(b, NULL, e.position);

	return 1;
}

int web_buffer_append(struct web_buffer *b, const char *in, size_t len)
{
	assert(b->state == STATE_INITIALIZED);
	
	/* We append a \0 to split messages */
	len++;
	
	if (b->size < b->len + len) {
		b->buffer = realloc(b->buffer, b->prefix + b->len + len);
		if (!b->buffer)
			return -1;
		
		b->size = b->len + len;
	}
	
	memcpy(b->buffer + b->prefix + b->len, in, len);
	b->len += len;
	b->buffer[b->len+b->prefix] = 0;
	
	return 0;
}

int web_buffer_append_json(struct web_buffer *b, json_t *res)
{
	size_t len;

	assert(b->state == STATE_INITIALIZED);

retry:	len = json_dumpb(res, b->buffer + b->prefix + b->len, b->size - b->len, 0) + 1;
	if (b->size < b->len + len) {
		b->buffer = realloc(b->buffer, b->prefix + b->len + len);
		if (!b->buffer)
			return -1;
		
		b->size = b->len + len;
		goto retry;
	}

	b->len += len;
	b->buffer[b->len+b->prefix] = 0;

	return 0;
}
