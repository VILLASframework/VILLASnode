/** REST-API-releated functions.
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

#include <libwebsockets.h>

#include "plugin.h"
#include "api.h"
#include "log.h"
#include "config.h"

static int parse_request(struct api_buffer *b, json_t **req)
{
	json_error_t e;
	
	if (b->len <= 0)
		return -1;

	*req = json_loadb(b->buf, b->len, JSON_DISABLE_EOF_CHECK, &e);
	if (!*req)
		return -1;

	if (e.position < b->len) {
		void *dst = (void *) b->buf;
		void *src = (void *) (b->buf + e.position);
		
		memmove(dst, src, b->len - e.position);
		
		b->len -= e.position;
	}

	return 1;
}

#if JANSSON_VERSION_HEX < 0x020A00
size_t json_dumpb(const json_t *json, char *buffer, size_t size, size_t flags)
{
	char *str;
	size_t len;

	str = json_dumps(json, flags);
	if (!str)
		return 0;
	
	len = strlen(str) - 1; // not \0 terminated
	if (buffer && len <= size)
		memcpy(buffer, str, len);

	//free(str);

	return len;
}
#endif

static int unparse_response(struct api_buffer *b, json_t *res)
{
	size_t len;

retry:	len = json_dumpb(res, b->buf + b->len, b->size - b->len, 0);
	if (len > b->size - b->len) {
		b->buf = realloc(b->buf, b->len + len);
		b->len += len;
		goto retry;
	}
	
	return 0;
}

int api_session_run_command(struct api_session *s, json_t *req, json_t **resp)
{
	int ret;
	const char *rstr;
	struct plugin *p;
	
	json_t *args;
	
	ret = json_unpack(req, "{ s: s, s: o }",
		"command", &rstr,
		"arguments", &args);
	if (ret)
		*resp = json_pack("{ s: s, s: d }",
				"error", "invalid request",
				"code", -1);
	
	p = plugin_lookup(PLUGIN_TYPE_API, rstr);
	if (!p)
		*resp = json_pack("{ s: s, s: d, s: s }",
				"error", "command not found",
				"code", -2,
				"command", rstr);
				
	debug(LOG_API, "Running API request: %s with arguments: %s", p->name, json_dumps(args, 0));

	ret = p->api.cb(&p->api, args, resp, s);
	if (ret)
		*resp = json_pack("{ s: s, s: d }",
				"error", "command failed",
				"code", ret);

	debug(LOG_API, "API request completed with code: %d and output: %s", ret, json_dumps(*resp, 0));

	return 0;
}

int api_protocol_cb(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	int ret;

	struct api_session *s = (struct api_session *) user;

	switch (reason) {
		case LWS_CALLBACK_ESTABLISHED: {
			struct web *w = (struct web *) lws_context_user(lws_get_context(wsi));
			
			api_session_init(s, w->api, API_MODE_WS);
			break;
		}
		
		case LWS_CALLBACK_HTTP: {
			struct web *w = (struct web *) lws_context_user(lws_get_context(wsi));

			char *uri = (char *) in;

			/* Parse request URI */
			ret = sscanf(uri, "/api/v%d", (int *) &s->version);
			if (ret != 1)
				return -1;
						
			debug(LOG_API, "New REST API session initiated: version = %d", s->version);
			
			api_session_init(s, w->api, API_MODE_HTTP);

			/* Prepare HTTP response header */
			s->response.headers.len = 1 + asprintf(&s->response.headers.buf,
				"HTTP/1.1 200 OK\r\n"
				"Content-type: application/json\r\n"
				"User-agent: " USER_AGENT
				"\r\n"
			);

			/* book us a LWS_CALLBACK_HTTP_WRITEABLE callback */
			lws_callback_on_writable(wsi);

			break;
		}

		case LWS_CALLBACK_CLIENT_RECEIVE:
		case LWS_CALLBACK_RECEIVE:
		case LWS_CALLBACK_HTTP_BODY: {
			char *newbuf;
			
			newbuf = realloc(s->request.body.buf, s->request.body.len + len);
			
			s->request.body.buf = memcpy(newbuf + s->request.body.len, in, len);
			s->request.body.len += len;
			
			json_t *req, *resp;
			while (parse_request(&s->request.body, &req) == 1) {

				api_session_run_command(s, req, &resp);

				unparse_response(&s->response.body, resp);
				
				debug(LOG_WEB, "Sending response: %s len=%zu", s->response.body.buf, s->response.body.len);

				lws_callback_on_writable(wsi);
			}
			
			break;
		}
			
		case LWS_CALLBACK_HTTP_BODY_COMPLETION:
			s->completed = true;
			break;

		case LWS_CALLBACK_SERVER_WRITEABLE:
		case LWS_CALLBACK_HTTP_WRITEABLE: {
			int sent;			
			if (s->mode == API_MODE_HTTP && s->response.headers.len > 0) {
				sent = lws_write(wsi, s->response.headers.buf, s->response.headers.len, LWS_WRITE_HTTP_HEADERS);
				if (sent > 0) {
					memmove(s->response.headers.buf, s->response.headers.buf + sent, sent);
					s->response.headers.len -= sent;
				}
			}
			else if (s->response.body.len > 0) {
				sent = lws_write(wsi, s->response.body.buf, s->response.body.len, LWS_WRITE_HTTP);
				if (sent > 0) {
					memmove(s->response.body.buf, s->response.body.buf + sent, sent);
					s->response.body.len -= sent;
				}
			}
			
			if (s->completed && s->response.body.len == 0)
				return -1;

			break;
		}

		default:
			return 0;
	}

	return 0;
}

int api_init(struct api *a, struct cfg *cfg)
{
	list_init(&a->sessions);

	a->cfg = cfg;

	return 0;
}

int api_destroy(struct api *a)
{
	return 0;
}

int api_deinit(struct api *a)
{
	list_destroy(&a->sessions, (dtor_cb_t) api_session_destroy, false);
	
	return 0;
}

int api_session_init(struct api_session *s, struct api *a, enum api_mode m)
{
	s->mode = m;
	s->api = a;
	
	s->completed = false;
	
	s->request.body = 
	s->response.body =
	s->response.headers = (struct api_buffer) { 0 };
	
	return 0;
}

int api_session_destroy(struct api_session *s)
{
	return 0;
}
