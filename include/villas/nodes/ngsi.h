/** Node type: OMA Next Generation Services Interface 10 (NGSI) (FIWARE context broker)
 *
 * This file implements the NGSI context interface. NGSI is RESTful HTTP is specified by
 * the Open Mobile Alliance (OMA).
 * It uses the standard operations of the NGSI 10 context information standard.
 *
 * @see https://forge.fiware.org/plugins/mediawiki/wiki/fiware/index.php/FI-WARE_NGSI-10_Open_RESTful_API_Specification
 * @see http://technical.openmobilealliance.org/Technical/Release_Program/docs/NGSI/V1_0-20120529-A/OMA-TS-NGSI_Context_Management-V1_0-20120529-A.pdf
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

/**
 * @addtogroup ngsi FIRWARE NGSI 9/10 RESTful HTTP API
 * @ingroup node
 * @{
 */

#pragma once

#include <curl/curl.h>
#include <jansson.h>

#include "list.h"
#include "msg.h"
#include "super_node.h"
#include "node.h"

struct node;

struct ngsi {
	const char *endpoint;		/**< The NGSI context broker endpoint URL. */
	const char *entity_id;		/**< The context broker entity id related to this node */
	const char *entity_type;	/**< The type of the entity */
	const char *access_token;	/**< An optional authentication token which will be sent as HTTP header. */

	double timeout;			/**< HTTP timeout in seconds */
	double rate;			/**< Rate used for polling. */

	int tfd;			/**< Timer */
	int ssl_verify;			/**< Boolean flag whether SSL server certificates should be verified or not. */

	struct curl_slist *headers;	/**< List of HTTP request headers for libcurl */

	CURL *curl;			/**< libcurl: handle */

	struct list mapping;		/**< A mapping between indices of the VILLASnode samples and the attributes in ngsi::context */
};

/** Initialize global NGSI settings and maps shared memory regions.
 *
 * @see node_type::init
 */
int ngsi_init(struct super_node *sn);

/** Free global NGSI settings and unmaps shared memory regions.
 *
 * @see node_type::deinit
 */
int ngsi_deinit();

/** @see node_type::parse */
int ngsi_parse(struct node *n, config_setting_t *cfg);

/** @see node_type::print */
char * ngsi_print(struct node *n);

/** @see node_type::open */
int ngsi_start(struct node *n);

/** @see node_type::close */
int ngsi_stop(struct node *n);

/** @see node_type::read */
int ngsi_read(struct node *n, struct sample *smps[], unsigned cnt);

/** @see node_type::write */
int ngsi_write(struct node *n, struct sample *smps[], unsigned cnt);

/** @} */