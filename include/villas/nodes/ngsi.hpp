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
 * @copyright 2014-2020, Institute for Automation of Complex Power Systems, EONERC
 * @license GNU General Public License (version 3)
 *
 * VILLASnode
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************************/

/**
 * @addtogroup ngsi FIRWARE NGSI 9/10 RESTful HTTP API
 * @ingroup node
 * @{
 */

#pragma once

#include <curl/curl.h>
#include <jansson.h>

#include <villas/list.h>
#include <villas/task.hpp>

/* Forward declarations */
struct vnode;

struct ngsi {
	const char *endpoint;		/**< The NGSI context broker endpoint URL. */
	const char *entity_id;		/**< The context broker entity id related to this node */
	const char *entity_type;	/**< The type of the entity */
	const char *access_token;	/**< An optional authentication token which will be sent as HTTP header. */

	bool create;			/**< Weather we want to create the context element during startup. */
	bool remove;			/**< Weather we want to delete the context element during startup. */

	double timeout;			/**< HTTP timeout in seconds */
	double rate;			/**< Rate used for polling. */

	struct Task task;		/**< Timer for periodic events. */
	int ssl_verify;			/**< Boolean flag whether SSL server certificates should be verified or not. */

	struct curl_slist *headers;	/**< List of HTTP request headers for libcurl */

	struct {
		CURL *curl;		/**< libcurl: handle */
		struct vlist signals;	/**< A mapping between indices of the VILLASnode samples and the attributes in ngsi::context */
	} in, out;
};

/** Initialize global NGSI settings and maps shared memory regions.
 *
 * @see node_type::type_start
 */
int ngsi_type_start(villas::node::SuperNode *sn);

/** Free global NGSI settings and unmaps shared memory regions.
 *
 * @see node_type::type_stop
 */
int ngsi_type_stop();

/** @see node_type::parse */
int ngsi_parse(struct vnode *n, json_t *json);

/** @see node_type::print */
char * ngsi_print(struct vnode *n);

/** @see node_type::start */
int ngsi_start(struct vnode *n);

/** @see node_type::stop */
int ngsi_stop(struct vnode *n);

/** @see node_type::reverse */
int ngsi_reverse(struct vnode *n);

/** @see node_type::read */
int ngsi_read(struct vnode *n, struct sample * const smps[], unsigned cnt);

/** @see node_type::write */
int ngsi_write(struct vnode *n, struct sample * const smps[], unsigned cnt);

/** @} */
