/** Compatibility code for GCC
 *
 * OPAL-RT's libSystem.a links against some Intel
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

#include <string.h>

size_t __intel_sse2_strlen(const char *s)
{
	return strlen(s);
}

void * _intel_fast_memset(void *b, int c, size_t len)
{
	return memset(b, c, len);
}

void * _intel_fast_memcpy(void *restrict dst, const void *restrict src, size_t n)
{
	return memcpy(dst, src, n);
}

int _intel_fast_memcmp(const void *s1, const void *s2, size_t n)
{
	return memcmp(s1, s2, n);
}
