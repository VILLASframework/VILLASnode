/** Node type: OPAL (AsyncApi)
 *
 * This file implements the opal subtype for nodes.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014-2016, Institute for Automation of Complex Power Systems, EONERC
 *   This file is part of VILLASnode. All Rights Reserved. Proprietary and confidential.
 *   Unauthorized copying of this file, via any medium is strictly prohibited.
 *********************************************************************************/

#include <stdlib.h>
#include <math.h>

#include "opal.h"
#include "utils.h"

/* Private static storage */
static char *async_shmem_name;		/**< Shared Memory identifiers and size, provided via argv. */
static char *print_shmem_name;		/**< Shared Memory identifiers and size, provided via argv. */
static int async_shmem_size;		/**< Shared Memory identifiers and size, provided via argv. */
	
static int send_icons, recv_icons;	/** Number of send blocks used in the running OPAL model. */
static int *send_ids, *recv_ids;	/** A dynamically allocated array of SendIDs. */

static Opal_GenAsyncParam_Ctrl params;	/** String and Float parameters, provided by the OPAL AsyncProcess block. */
	
static pthread_mutex_t lock;		/** Big Global Lock for libOpalAsync API */

int opal_init(int argc, char *argv[], config_setting_t *cfg)
{
	int err;

	if (argc != 4)
		return -1;

	pthread_mutex_init(&lock, NULL);

	async_shmem_name = argv[1];
	async_shmem_size = atoi(argv[2]);
	print_shmem_name = argv[3];

	/* Enable the OpalPrint function. This prints to the OpalDisplay. */
	err = OpalSystemCtrl_Register(print_shmem_name);
	if (err != EOK)
		error("OpalPrint() access not available (%d)", err);

	/* Open Share Memory created by the model. */
	err = OpalOpenAsyncMem(async_shmem_size, async_shmem_name);
	if (err != EOK)
		error("Model shared memory not available (%d)", err);

	err = OpalGetAsyncCtrlParameters(&params, sizeof(Opal_GenAsyncParam_Ctrl));
	if (err != EOK)
		error("Could not get OPAL controller parameters (%d)", err);

	/* Get list of Send and RecvIDs */
	err = OpalGetNbAsyncSendIcon(&send_icons);
	if (err != EOK)
		error("Failed to get number of send blocks (%d)", err);
	err = OpalGetNbAsyncRecvIcon(&recv_icons);
	if (err != EOK)
		error("Failed to get number of recv blocks (%d)", err);

	send_ids = alloc(send_icons * sizeof(int));
	recv_ids = alloc(recv_icons * sizeof(int));

	err = OpalGetAsyncSendIDList(send_ids, send_icons * sizeof(int));
	if (err != EOK)
		error("Failed to get list of send ids (%d)", err);
	err = OpalGetAsyncRecvIDList(recv_ids, recv_icons * sizeof(int));
	if (err != EOK)
		error("Failed to get list of recv ids (%d)", err);

	info("Started as OPAL Asynchronous process");
	info("This is VILLASnode %s (built on %s, %s)",
		VERSION, __DATE__, __TIME__);

	opal_print_global();

	return 0;
}

int opal_deinit()
{
	int err;

	err = OpalCloseAsyncMem(async_shmem_size, async_shmem_name);
	if (err != EOK)
		error("Failed to close shared memory area (%d)", err);

	debug(4, "Closing OPAL shared memory mapping");

	err = OpalSystemCtrl_UnRegister(print_shmem_name);
	if (err != EOK)
		error("Failed to close shared memory for system control (%d)", err);

	pthread_mutex_destroy(&lock);

	free(send_ids);
	free(recv_ids);

	return 0;
}

int opal_print_global()
{
	debug(2, "Controller ID: %u", params.controllerID);
	
	char *sbuf = alloc(send_icons * 5);
	char *rbuf = alloc(recv_icons * 5);

	for (int i = 0; i < send_icons; i++)
		strcatf(&sbuf, "%u ", send_ids[i]);
	for (int i = 0; i < recv_icons; i++)
		strcatf(&rbuf, "%u ", recv_ids[i]);

	debug(2, "Send Blocks: %s",    sbuf);
	debug(2, "Receive Blocks: %s", rbuf);
	
	free(sbuf);
	free(rbuf);

	debug(2, "Control Block Parameters:");
	for (int i = 0; i < GENASYNC_NB_FLOAT_PARAM; i++)
		debug(2, "FloatParam[]%u] = %f", i, params.FloatParam[i]);
	for (int i = 0; i < GENASYNC_NB_STRING_PARAM; i++)
		debug(2, "StringParam[%u] = %s", i, params.StringParam[i]);

	return 0;
}

int opal_parse(struct node *n, config_setting_t *cfg)
{
	struct opal *o = n->_vd;

	config_setting_lookup_int(cfg, "send_id", &o->send_id);
	config_setting_lookup_int(cfg, "recv_id", &o->recv_id);
	config_setting_lookup_bool(cfg, "reply", &o->reply);

	return 0;
}

char * opal_print(struct node *n)
{
	struct opal *o = n->_vd;

	/** @todo: Print send_params, recv_params */

	return strf("send_id=%u, recv_id=%u, reply=%u",
		o->send_id, o->recv_id, o->reply);
}

int opal_open(struct node *n)
{
	struct opal *o = n->_vd;

	/* Search for valid send and recv ids */
	int sfound = 0, rfound = 0;
	for (int i = 0; i < send_icons; i++)
		sfound += send_ids[i] == o->send_id;
	for (int i = 0; i < send_icons; i++)
		rfound += send_ids[i] == o->send_id;

	if (!sfound)
		error("Invalid send_id '%u' for node %s", o->send_id, node_name(n));
	if (!rfound)
		error("Invalid recv_id '%u' for node %s", o->recv_id, node_name(n));

	/* Get some more informations and paramters from OPAL-RT */
	OpalGetAsyncSendIconMode(&o->mode, o->send_id);
	OpalGetAsyncSendParameters(&o->send_params, sizeof(Opal_SendAsyncParam), o->send_id);
	OpalGetAsyncRecvParameters(&o->recv_params, sizeof(Opal_RecvAsyncParam), o->recv_id);

	return 0;
}

int opal_close(struct node *n)
{
	return 0;
}

int opal_read(struct node *n, struct pool *pool, int cnt)
{
	struct opal *o = n->_vd;

	int state, len, ret;
	unsigned id;

	struct msg *m = &pool[first % poolsize];

	double data[MSG_VALUES];
	
	if (cnt != 1)
		error("The OPAL-RT node type does not support combining!");

	/* This call unblocks when the 'Data Ready' line of a send icon is asserted. */
	do {
		ret = OpalWaitForAsyncSendRequest(&id);
		if (ret != EOK) {
			state = OpalGetAsyncModelState();
			if ((state == STATE_RESET) || (state == STATE_STOP))
				error("OpalGetAsyncModelState(): Model stopped or resetted!");

			return -1; // FIXME: correct return value
		}
	} while (id != o->send_id);

	/* No errors encountered yet */
	OpalSetAsyncSendIconError(0, o->send_id);

	/* Get the size of the data being sent by the unblocking SendID */
	OpalGetAsyncSendIconDataLength(&len, o->send_id);
	if (len > sizeof(data)) {
		warn("Ignoring the last %u of %u values for OPAL node %s (send_id=%u).",
		len / sizeof(double) - MSG_VALUES, len / sizeof(double), node_name(n), o->send_id);

		len = sizeof(data);
	}

	/* Read data from the model */
	OpalGetAsyncSendIconData(data, len, o->send_id);

	m->sequence = htons(o->seq_no++);
	m->length = len / sizeof(double);

	for (int i = 0; i < m->length; i++)
		m->data[i].f = (float) data[i]; /* OPAL provides double precission */

	/* This next call allows the execution of the "asynchronous" process
	 * to actually be synchronous with the model. To achieve this, you
	 * should set the "Sending Mode" in the Async_Send block to
	 * NEED_REPLY_BEFORE_NEXT_SEND or NEED_REPLY_NOW. This will force
	 * the model to wait for this process to call this
	 * OpalAsyncSendRequestDone function before continuing. */
	if (o->reply)
		OpalAsyncSendRequestDone(o->send_id);

	/* Before continuing, we make sure that the real-time model
	 * has not been stopped. If it has, we quit. */
	state = OpalGetAsyncModelState();
	if ((state == STATE_RESET) || (state == STATE_STOP))
		error("OpalGetAsyncModelState(): Model stopped or resetted!");

	return 1;
}

int opal_write(struct node *n, struct pool *pool, int cnt)
{
	struct opal *o = n->_vd;

	struct msg *m = &pool[first % poolsize];

	int state;
	int len;
	double data[m->length];
	
	if (cnt != 1)
		error("The OPAL-RT node type does not support combining!");

	state = OpalGetAsyncModelState();
	if ((state == STATE_RESET) || (state == STATE_STOP))
		error("OpalGetAsyncModelState(): Model stopped or resetted!");

	OpalSetAsyncRecvIconStatus(m->sequence, o->recv_id);	/* Set the Status to the message ID */
	OpalSetAsyncRecvIconError(0, o->recv_id);		/* Set the Error to 0 */

	/* Get the number of signals to send back to the model */
	OpalGetAsyncRecvIconDataLength(&len, o->recv_id);
	if (len > sizeof(data))
		warn("Node %s is expecting more signals (%u) than values in message (%u)", node_name(n), len / sizeof(double), m->length);

	for (int i = 0; i < m->length; i++)
		data[i] = (double) m->data[i].f; /* OPAL expects double precission */

	OpalSetAsyncRecvIconData(data, m->length * sizeof(double), o->recv_id);

	return 1;
}

static struct node_type vt = {
	.name		= "opal",
	.description	= "run as OPAL Asynchronous Process (libOpalAsyncApi)",
	.vectoroize	= 1,
	.size		= sizeof(struct opal),
	.parse		= opal_parse,
	.print		= opal_print,
	.open		= opal_open,
	.close		= opal_close,
	.read		= opal_read,
	.write		= opal_write,
	.init		= opal_init,
	.deinit		= opal_deinit
};

REGISTER_NODE_TYPE(&vt)