/** Helper functions for sockets.
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

#ifndef _SOCKET_H_
#define _SOCKET_H_

#define RT
#include "OpalGenAsyncParamCtrl.h"

#define UDP_PROTOCOL	1
#define TCP_PROTOCOL	2
#define	EOK		0

int InitSocket(Opal_GenAsyncParam_Ctrl IconCtrlStruct);

int SendPacket(char* DataSend, int datalength);

int RecvPacket(char* DataRecv, int datalength, double timeout);

int CloseSocket(Opal_GenAsyncParam_Ctrl IconCtrlStruct);

#endif /* _SOCKET_H_ */
