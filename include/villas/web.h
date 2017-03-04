/** LWS-releated functions.
 *
 * @file
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

/* Forward declarations */
struct api;

struct web {
	struct api *api;
	
	struct lws_context *context;	/**< The libwebsockets server context. */
	struct lws_vhost *vhost;	/**< The libwebsockets vhost. */

	int port;			/**< Port of the build in HTTP / WebSocket server. */
	const char *htdocs;		/**< The root directory for files served via HTTP. */
	const char *ssl_cert;		/**< Path to the SSL certitifcate for HTTPS / WSS. */
	const char *ssl_private_key;	/**< Path to the SSL private key for HTTPS / WSS. */
};

/** Initialize the web interface.
 *
 * The web interface is based on the libwebsockets library.
 */
int web_init(struct web *w, struct api *a);

int web_destroy(struct web *w);

/** Parse HTTPd and WebSocket related options */
int web_parse(struct web *w, config_setting_t *lcs);

/** De-initializes the web interface. */
int web_deinit(struct web *w);

/** libwebsockets service routine. Call periodically */
int web_service(struct web *w);