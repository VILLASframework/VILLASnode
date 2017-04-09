/** API session.
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

#include "api/session.h"

#include "web.h"
#include "plugin.h"

int api_session_init(struct api_session *s, struct api *a, enum api_mode m)
{
	s->mode = m;
	s->api = a;
	
	s->completed = false;
	
	web_buffer_init(&s->request.body,  s->mode == API_MODE_HTTP ? LWS_WRITE_HTTP : LWS_WRITE_TEXT);
	web_buffer_init(&s->response.body, s->mode == API_MODE_HTTP ? LWS_WRITE_HTTP : LWS_WRITE_TEXT);

	if (s->mode == API_MODE_HTTP)
		web_buffer_init(&s->response.headers, LWS_WRITE_HTTP_HEADERS);

	return 0;
}

int api_session_destroy(struct api_session *s)
{
	if (s->state == STATE_DESTROYED)
		return 0;
	
	web_buffer_destroy(&s->request.body);
	web_buffer_destroy(&s->response.body);
	
	if (s->mode == API_MODE_HTTP)
		web_buffer_destroy(&s->response.headers);
	
	s->state = STATE_DESTROYED;
	
	return 0;
}

int api_session_run_command(struct api_session *s, json_t *json_in, json_t **json_out)
{
	int ret;
	const char *action;
	char *id;
	struct plugin *p;
	
	json_t *json_args = NULL, *json_resp;
	
	ret = json_unpack(json_in, "{ s: s, s: s, s?: o }",
		"action", &action,
		"id", &id,
		"request", &json_args);
	if (ret) {
		ret = -100;
		*json_out = json_pack("{ s: s, s: i }",
				"error", "invalid request",
				"code", ret);
		goto out;
	}
	
	p = plugin_lookup(PLUGIN_TYPE_API, action);
	if (!p) {
		ret = -101;
		*json_out = json_pack("{ s: s, s: s, s: i, s: s }",
				"action", action,
				"id", id,
				"code", ret,
				"error", "command not found");
		goto out;
	}

	debug(LOG_API, "Running API request: %s", p->name);

	ret = p->api.cb(&p->api, json_args, &json_resp, s);
	if (ret)
		*json_out = json_pack("{ s: s, s: s, s: i, s: s }",
				"action", action,
				"id", id,
				"code", ret,
				"error", "command failed");
	else
		*json_out = json_pack("{ s: s, s: s, s: o }",
				"action", action,
				"id", id,
				"response", json_resp);

out:	debug(LOG_API, "API request completed with code: %d", ret);

	return 0;
}