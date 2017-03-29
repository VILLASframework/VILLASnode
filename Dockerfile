# Dockerfile for VILLASnode dependencies.
#
# This Dockerfile builds an image which contains all library dependencies
# and tools to build VILLASnode.
# However, VILLASnode itself it not part of the image.
#
# @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
# @copyright 2017, Institute for Automation of Complex Power Systems, EONERC
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

FROM fedora:latest
MAINTAINER Steffen Vogel <stvogel@eonerc.rwth-aachen.de>

# Install dependencies
RUN dnf -y update && \
    dnf -y install \
	libconfig \
	libnl3 \
	libcurl \
	jansson

# Some additional tools required for running VILLASnode
RUN dnf -y update && \
    dnf -y install \
	iproute \
	openssl \
	kernel-modules-extra

# Install our own RPMs
COPY build/release/packaging/rpm/RPMS/ /rpms/
RUN rpm -i /rpms/x86_64/{libxil,libwebsockets,villas-node,villas-node-doc}-[0-9]*; rm -rf /rpms/

# For WebSocket / API access
EXPOSE 80
EXPOSE 443

ENTRYPOINT ["villas"]