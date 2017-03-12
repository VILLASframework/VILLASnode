## Main project Makefile
#
# The build system of this project is based on GNU Make and pkg-config
#
# To retain maintainability, the project is divided into multiple modules.
# Each module has its own Makefile which gets included.
#
# Please read "Recursive Make Considered Harmful" from Peter Miller
#  to understand the motivation for this structure.
#
# [1] http://aegis.sourceforge.net/auug97.pdf
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

# Project modules
MODULES = lib plugins src tests thirdparty tools

# Default prefix for install target
PREFIX ?= /usr/local

# Default out-of-source build path
BUILDDIR ?= build

# Default debug level for executables
V ?= 2

# Common flags
LDLIBS   =
CFLAGS  += -std=c11 -Iinclude -Iinclude/villas -I. -MMD -mcx16
CFLAGS  += -Wall -Werror -fdiagnostics-color=auto
CFLAGS  += -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE=1 -DV=$(V)
LDFLAGS += -L$(BUILDDIR)

# We must compile without optimizations for gcov!
ifdef DEBUG
	CFLAGS += -O0 -g
	VARIANTS += debug
else
	CFLAGS += -O3
	VARIANTS += release
endif

ifdef PROFILE
	CFLAGS += -pg
	LDFLAGS += -pg
	
	VARIANTS += profile
endif

ifdef COVERAGE
	CFLAGS  += -fprofile-arcs -ftest-coverage
	LDFLAGS += --coverage
	LDLIBS  += -lgcov
	
	LIB_LDFLAGS += -coverage
	LIB_LDLIBS += -gcov
	
	VARIANTS += coverage
endif

SPACE :=
SPACE +=
VARIANT = $(subst $(SPACE),-,$(strip $(VARIANTS)))

BUILDDIR := $(BUILDDIR)/$(VARIANT)
SRCDIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Add git revision and build variant defines
ifdef CI
	CFLAGS += -D_GIT_REV='"${CI_BUILD_REF:0:7}"'
	CFLAGS += -D_VARIANT='"-ci-$(VARIANT)"'
else
	GIT = $(shell type -p git)
	GIT_REV = $(shell git describe --tags --dirty)
	ifneq ($(GIT),)
		CFLAGS += -D_GIT_REV='"$(GIT_REV)"'
	endif
	CFLAGS += -D_VARIANT='"-$(VARIANT)"'
endif

# pkg-config dependencies
PKGS = libconfig

######## Targets ########

# Add flags by pkg-config
CFLAGS += $(addprefix -DWITH_, $(shell echo ${PKGS} | tr a-z- A-Z_ | tr -dc ' A-Z0-9_' ))
CFLAGS += $(shell pkg-config --cflags ${PKGS})
LDLIBS += $(shell pkg-config --libs ${PKGS})

all: src plugins | lib

# Build everything with different profiles: debug, coverage, ...
everything:
	$(MAKE) RELEASE=1
	$(MAKE) DEBUG=1
	$(MAKE) COVERAGE=1
	$(MAKE) PROFILE=1

docker:
	docker build -t villas .
	docker run -it -p 80:80 -p 443:443 -p 1234:1234 --privileged --cap-add sys_nic --ulimit memlock=1073741824 --security-opt seccomp:unconfined -v $(PWD):/villas villas

doc: | $(BUILDDIR)/doc/
	( cat Doxyfile ; echo "OUTPUT_DIRECTORY=$(BUILDDIR)/doc/" ) | doxygen -

# Create non-existent directories
%/:
	mkdir -p $@

.PHONY: all everything clean install docker doc
.PRECIOUS: %/
.SECONDEXPANSION:

install: $(addprefix install-,$(filter-out thirdparty,$(MODULES)))
clean: $(addprefix clean-,$(filter-out thirdparty,$(MODULES)))

-include $(wildcard $(BUILDDIR)/**/*.d)
-include $(addsuffix /Makefile.inc,$(MODULES))
