/** Main routine of AsyncIP.
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

/* Standard ANSI C headers needed for this program */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

/* Define RTLAB before including OpalPrint.h for messages to be sent
 * to the OpalDisplay. Otherwise stdout will be used. */
#define RTLAB
#include "OpalPrint.h"
#include "AsyncApi.h"

/* This is the message format */
#include "config.h"
#include "msg.h"
#include "msg_format.h"
#include "socket.h"
#include "utils.h"

/* This is just for initializing the shared memory access to communicate
 * with the RT-LAB model. It's easier to remember the arguments like this */
#define	ASYNC_SHMEM_NAME	argv[1]
#define ASYNC_SHMEM_SIZE	atoi(argv[2])
#define PRINT_SHMEM_NAME	argv[3]

#ifdef _DEBUG // TODO: workaround

#define CPU_TICKS 3466948000
struct msg *msg_send = NULL;

void Tick(int sig, siginfo_t *si, void *ptr)
{
	Opal_GenAsyncParam_Ctrl *IconCtrlStruct;
	unsigned long long CpuTime, CpuTimeStart;
	double ModelTime;

	if (!msg_send)
		return;

	IconCtrlStruct = (Opal_GenAsyncParam_Ctrl*) si->si_value.sival_ptr;

	OpalGetAsyncStartExecCpuTime(IconCtrlStruct, &CpuTimeStart);
	OpalGetAsyncModelTime(IconCtrlStruct, &CpuTime, &ModelTime);

	OpalPrint("%s: CpuTime: %llu\tModelTime: %.3f\tSequence: %hu\tValue: %.2f\n",
		PROGNAME, (CpuTime - CpuTimeStart) / CPU_TICKS, ModelTime, msg_send->sequence, msg_send->data[0].f);
}
#endif /* _DEBUG */

static void * SendToIPPort(void *arg)
{
	unsigned int ModelState, SendID = 1, i, n;
	int nbSend = 0, ret;
	uint32_t seq = 0;

	/* Data from OPAL-RT model */
	double mdldata[MAX_VALUES];
	int mdldata_size;

	/* Data from VILLASnode */
	char buf[MSG_LEN(MAX_VALUES)];
	struct msg *msg = (struct msg *) buf;

#ifdef _DEBUG // TODO: workaround
	msg_send = msg;
#endif /* _DEBUG */

	OpalPrint("%s: SendToIPPort thread started\n", PROGNAME);

	OpalGetNbAsyncSendIcon(&nbSend);
	if (nbSend < 1) {
		OpalPrint("%s: SendToIPPort: No transimission block for this controller. Stopping thread.\n", PROGNAME);
		return NULL;
	}
	
	do {
		/* This call unblocks when the 'Data Ready' line of a send icon is asserted. */
		n = OpalWaitForAsyncSendRequest(&SendID);
		if (n != EOK) {
			ModelState = OpalGetAsyncModelState();
			if ((ModelState != STATE_RESET) && (ModelState != STATE_STOP)) {
				OpalSetAsyncSendIconError(n, SendID);
				OpalPrint("%s: OpalWaitForAsyncSendRequest(), errno %d\n", PROGNAME, n);
			}

			continue;
		}

		/* No errors encountered yet */
		OpalSetAsyncSendIconError(0, SendID);

		/* Get the size of the data being sent by the unblocking SendID */
		OpalGetAsyncSendIconDataLength(&mdldata_size, SendID);
		if (mdldata_size / sizeof(double) > MAX_VALUES) {
			OpalPrint("%s: Number of signals for SendID=%d exceeds allowed maximum (%d)\n",
				PROGNAME, SendID, MAX_VALUES);
			return NULL;
		}

		/* Read data from the model */
		OpalGetAsyncSendIconData(mdldata, mdldata_size, SendID);
		
		/* Get current time */
		struct timespec now;
		clock_gettime(CLOCK_REALTIME, &now);

		msg->length = mdldata_size / sizeof(double);
		for (i = 0; i < msg->length; i++)
			msg->data[i].f = (float) mdldata[i];

		msg->sequence = seq++;
		msg->ts.sec = now.tv_sec;
		msg->ts.nsec = now.tv_nsec;
		
		msg_hton(msg);

		/* Perform the actual write to the ip port */
		ret = SendPacket((char *) msg, MSG_LEN(msg->length));
		if (ret < 0)
			OpalSetAsyncSendIconError(errno, SendID);
		else
			OpalSetAsyncSendIconError(0, SendID);

		/* This next call allows the execution of the "asynchronous" process
		 * to actually be synchronous with the model. To achieve this, you
		 * should set the "Sending Mode" in the Async_Send block to
		 * NEED_REPLY_BEFORE_NEXT_SEND or NEED_REPLY_NOW. This will force
		 * the model to wait for this process to call this
		* OpalAsyncSendRequestDone function before continuing. */
		OpalAsyncSendRequestDone(SendID);

		/* Before continuing, we make sure that the real-time model
		 * has not been stopped. If it has, we quit. */
		ModelState = OpalGetAsyncModelState();
	} while ((ModelState != STATE_RESET) && (ModelState != STATE_STOP));

	OpalPrint("%s: SendToIPPort: Finished\n", PROGNAME);

	return NULL;
}

static void * RecvFromIPPort(void *arg)
{
	unsigned int ModelState, RecvID = 1, i, n;
	int nbRecv = 0, ret;

	/* Data from OPAL-RT model */
	double mdldata[MAX_VALUES];
	int mdldata_size;

	/* Data from VILLASnode */
	char buf[MSG_LEN(MAX_VALUES)];
	struct msg *msg = (struct msg *) buf;

	OpalPrint("%s: RecvFromIPPort thread started\n", PROGNAME);

	OpalGetNbAsyncRecvIcon(&nbRecv);
	if (nbRecv < 1) {
		OpalPrint("%s: RecvFromIPPort: No reception block for this controller. Stopping thread.\n", PROGNAME);
		return NULL;
	}

	do {
		/* Receive message */
		n  = RecvPacket((char *) msg, sizeof(buf), 1.0);
		if (n < 1) {
			ModelState = OpalGetAsyncModelState();
			if ((ModelState != STATE_RESET) && (ModelState != STATE_STOP)) {
				if (n ==  0) /* timeout, so we continue silently */
					OpalPrint("%s: Timeout while waiting for data\n", PROGNAME, errno);
				if (n == -1) /* a more serious error, so we print it */
					OpalPrint("%s: Error %d while waiting for data\n", PROGNAME, errno);
					
				continue;
			}
			break;
		}

		msg_ntoh(msg);
		
		ret = msg_verify(msg);
		if (ret) {
			OpalPrint("%s: Skipping invalid packet\n", PROGNAME);
			continue;
		}

		/* Update OPAL model */
		OpalSetAsyncRecvIconStatus(msg->sequence, RecvID);	/* Set the Status to the message ID */
		OpalSetAsyncRecvIconError(0, RecvID);			/* Set the Error to 0 */

		/* Get the number of signals to send back to the model */
		OpalGetAsyncRecvIconDataLength(&mdldata_size, RecvID);
		if (mdldata_size / sizeof(double) > MAX_VALUES) {
			OpalPrint("%s: Number of signals for RecvID=%d (%d) exceeds allowed maximum (%d)\n",
				PROGNAME, RecvID, mdldata_size / sizeof(double), MAX_VALUES);
			return NULL;
		}

		if (mdldata_size / sizeof(double) > msg->length)
			OpalPrint("%s: Number of signals for RecvID=%d (%d) exceeds what was received (%d)\n",
				PROGNAME, RecvID, mdldata_size / sizeof(double), msg->length);

		for (i = 0; i < msg->length; i++)
			mdldata[i] = (double) msg->data[i].f;

		OpalSetAsyncRecvIconData(mdldata, mdldata_size, RecvID);

		/* Before continuing, we make sure that the real-time model
		 * has not been stopped. If it has, we quit. */
		ModelState = OpalGetAsyncModelState();
	} while ((ModelState != STATE_RESET) && (ModelState != STATE_STOP));

	OpalPrint("%s: RecvFromIPPort: Finished\n", PROGNAME);

	return NULL;
}

int main(int argc, char *argv[])
{
	int ret;

	Opal_GenAsyncParam_Ctrl IconCtrlStruct;

	pthread_t tid_send, tid_recv;
	pthread_attr_t attr_send, attr_recv;

	OpalPrint("%s: This is %s client version %s\n", PROGNAME, PROGNAME, VERSION);

	/* Check for the proper arguments to the program */
	if (argc < 4) {
		printf("Invalid Arguments: 1-AsyncShmemName 2-AsyncShmemSize 3-PrintShmemName\n");
		exit(0);
	}

	/* Enable the OpalPrint function. This prints to the OpalDisplay. */
	if (OpalSystemCtrl_Register(PRINT_SHMEM_NAME) != EOK) {
		printf("%s: ERROR: OpalPrint() access not available\n", PROGNAME);
		exit(EXIT_FAILURE);
	}

	/* Open Share Memory created by the model. */
	ret = OpalOpenAsyncMem(ASYNC_SHMEM_SIZE, ASYNC_SHMEM_NAME);
	if (ret != EOK) {
		OpalPrint("%s: ERROR: Model shared memory not available\n", PROGNAME);
		exit(EXIT_FAILURE);
	}

	AssignProcToCpu0();

	/* Get IP Controler Parameters (ie: ip address, port number...) and
	 * initialize the device on the QNX node. */
	memset(&IconCtrlStruct, 0, sizeof(IconCtrlStruct));
	
	ret = OpalGetAsyncCtrlParameters(&IconCtrlStruct, sizeof(IconCtrlStruct));
	if (ret != EOK) {
		OpalPrint("%s: ERROR: Could not get controller parameters (%d).\n", PROGNAME, ret);
		exit(EXIT_FAILURE);
	}

	/* Initialize socket */
	ret = InitSocket(IconCtrlStruct);
	if (ret != EOK) {
		OpalPrint("%s: ERROR: Initialization failed.\n", PROGNAME);
		exit(EXIT_FAILURE);
	}

#ifdef _DEBUG
	/* Setup signals */
	struct sigaction sa_tick = {
		.sa_flags = SA_SIGINFO,
		.sa_sigaction = Tick
	};

	sigemptyset(&sa_tick.sa_mask);
	sigaction(SIGUSR1, &sa_tick, NULL);

	/* Setup timer */
	timer_t t;
	struct sigevent sev = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = SIGUSR1,
		.sigev_value.sival_ptr = &IconCtrlStruct
	};

	struct itimerspec its = {
		.it_interval = { 1, 0 },
		.it_value = { 0, 1 }
	};

	timer_create(CLOCK_REALTIME, &sev, &t);
	timer_settime(t, 0, &its, NULL);
#endif /* _DEBUG */

	/* Start send/receive threads */
	ret = pthread_create(&tid_send, NULL, SendToIPPort, NULL);
	if (ret == -1)
		OpalPrint("%s: ERROR: Could not create thread (SendToIPPort), errno %d\n", PROGNAME, errno);
	
	ret = pthread_create(&tid_recv, NULL, RecvFromIPPort, NULL);
	if (ret == -1)
		OpalPrint("%s: ERROR: Could not create thread (RecvFromIPPort), errno %d\n", PROGNAME, errno);

	/* Wait for both threads to finish */
	ret = pthread_join(tid_send, NULL);
	if (ret != 0)
		OpalPrint("%s: ERROR: pthread_join (SendToIPPort), errno %d\n", PROGNAME, ret);
	
	ret = pthread_join(tid_recv, NULL);
	if (ret != 0)
		OpalPrint("%s: ERROR: pthread_join (RecvFromIPPort), errno %d\n", PROGNAME, ret);

	/* Close the ip port and shared memories */
	CloseSocket(IconCtrlStruct);
	OpalCloseAsyncMem (ASYNC_SHMEM_SIZE, ASYNC_SHMEM_NAME);
	OpalSystemCtrl_UnRegister(PRINT_SHMEM_NAME);

#ifdef _DEBUG
	timer_delete(t);
#endif /* _DEBUG */

	return 0;
}
