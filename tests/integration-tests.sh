#!/bin/bash
#
# Run integration tests
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
##################################################################################

SCRIPT=$(realpath ${BASH_SOURCE[0]})
SCRIPTPATH=$(dirname $SCRIPT)

export SRCDIR=$(realpath ${SCRIPTPATH}/..)
export BUILDDIR=${SRCDIR}/build/release
export LOGDIR=${BUILDDIR}/tests/integration
export PATH=${BUILDDIR}:${PATH}

# Default values
VERBOSE=0
FILTER='*'

export NUM_SAMPLES=100

# Parse command line arguments
while getopts ":f:v" OPT; do
	case ${OPT} in
		f)
			FILTER=${OPTARG}
			;;
		v)
			VERBOSE=1
			;;
		\?)
			echo "Invalid option: -${OPTARG}" >&2
			;;
		:)
			echo "Option -$OPTARG requires an argument." >&2
			exit 1
			;;
	esac
done

TESTS=${SRCDIR}/tests/integration/${FILTER}.sh

# Preperations
mkdir -p ${LOGDIR}

NUM_PASS=0
NUM_FAIL=0

# Preamble
echo -e "Starting integration tests for VILLASnode/fpga:\n"

for TEST in ${TESTS}; do
	TESTNAME=$(basename -s .sh ${TEST})

	# Run test
	if (( ${VERBOSE} == 0 )); then
		${TEST} &> ${LOGDIR}/${TESTNAME}.log
	else
		${TEST}
	fi
	
	RC=$?

	if (( $RC != 0 )); then
		echo -e "\e[31m[Fail] \e[39m ${TESTNAME} with code $RC"
		NUM_FAIL=$((${NUM_FAIL} + 1))
	else
		echo -e "\e[32m[Pass] \e[39m ${TESTNAME}"
		NUM_PASS=$((${NUM_PASS} + 1))
	fi
done

# Show summary
if (( ${NUM_FAIL} > 0 )); then
	echo -e "\nSummary: ${NUM_FAIL} of $((${NUM_FAIL} + ${NUM_PASS})) tests failed."
	exit 1
else
	echo -e "\nSummary: all tests passed!"
	exit 0
fi