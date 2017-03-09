#!/bin/bash
# Wrapper to start tool of the VILLASnode suite
#
# This allows you to use VILLASnode tools like this:
#    $ villas node /etc/villas/test.cfg
#
# Install by:
#    $ make install
#
# @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
# @copyright 2016, Institute for Automation of Complex Power Systems, EONERC
# @license GNU Lesser General Public License v2.1
#
# VILLASnode - connecting real-time simulation equipment
#
# This application is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
#
# This application is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
###################################################################################

PREFIX=villas

# Get a list of all available tools
TOOLS=$(compgen -c | egrep "^$PREFIX-" | sort | cut -d- -f2 | paste -sd\|)

# First argument to wrapper is the tool which should be started
TOOL=$1

# Following arguments will be passed directly to tool
ARGS=${@:2}

# Check if tool is available
if ! [[ "$TOOL" =~ $(echo ^\($TOOLS\)$) ]]; then
	echo "Usage villas ($TOOLS)" 1>&2
	exit 1
fi

exec $PREFIX-$TOOL $ARGS