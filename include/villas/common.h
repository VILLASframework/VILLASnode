/** Some common defines, enums and datastructures.
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

/* Common states for most objects in VILLASnode (paths, nodes, hooks, plugins) */
enum state {
	STATE_DESTROYED		= 0,
	STATE_INITIALIZED	= 1,
	STATE_PARSED		= 2,
	STATE_CHECKED		= 3,
	STATE_STARTED		= 4,
	STATE_LOADED		= 4, /* alias for STATE_STARTED used by plugins */
	STATE_STOPPED		= 5,
	STATE_UNLOADED		= 5 /* alias for STATE_STARTED used by plugins */
};