/** Logging and debugging routines
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2016, Institute for Automation of Complex Power Systems, EONERC
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
 
#ifndef _LOG_H_
#define _LOG_H_

#include <stdarg.h>
#include <libconfig.h>

#include "utils.h"

#ifdef __GNUC__
  #define INDENT	int __attribute__ ((__cleanup__(log_outdent), unused)) _old_indent = log_indent(1);
#else
  #define INDENT	;
#endif

/* The log level which is passed as first argument to print() */
#define LOG_LVL_DEBUG	GRY("Debug")
#define LOG_LVL_INFO	WHT("Info ") 
#define LOG_LVL_WARN	YEL("Warn ")
#define LOG_LVL_ERROR	RED("Error")
#define LOG_LVL_STATS	MAG("Stats")

/** Debug facilities.
 *
 * To be or-ed with the debug level
 */
enum debug_facilities {
	DBG_POOL =	(1 <<  8),
	DBG_QUEUE =	(1 <<  9),
	DBG_CONFIG =	(1 << 10),
	DBG_HOOK =	(1 << 11),
	DBG_PATH =	(1 << 12),
	DBG_MEM =	(1 << 13),
	
	/* Node-types */
	DBG_SOCKET =	(1 << 16),
	DBG_WEBSOCKET = (1 << 17),
	DBG_FILE =	(1 << 18),
	DBG_FPGA =	(1 << 19),
	DBG_NGSI =	(1 << 20),
	DBG_OPAL =	(1 << 21),
	DBG_NODES =  (0xFF << 16)
};

/** Change log indention  for current thread.
 *
 * The argument level can be negative!
 */
int log_indent(int levels);

/** A helper function the restore the previous log indention level.
 *
 * This function is usually called by a __cleanup__ handler (GCC C Extension).
 * See INDENT macro.
 */
void log_outdent(int *);

/** Reset the wallclock of debug messages.
 *
 * @param lvl The new debug level.
 * @param fac The new mask for debug facilities.
 */
void log_init(int lvl, int fac, const char *path);

int log_lookup_facility(const char *facility_name);

/** Logs variadic messages to stdout.
 *
 * @param lvl The log level
 * @param fmt The format string (printf alike)
 */
void log_print(const char *lvl, const char *fmt, ...)
	__attribute__ ((format(printf, 2, 3)));

/** Logs variadic messages to stdout.
 *
 * @param lvl The log level
 * @param fmt The format string (printf alike)
 * @param va The variadic argument list (see stdarg.h)
 */	
void log_vprint(const char *lvl, const char *fmt, va_list va);

/** Printf alike debug message with level. */
void debug(int lvl, const char *fmt, ...)
	__attribute__ ((format(printf, 2, 3)));

/** Print a horizontal line. */
void line();

/** Printf alike info message. */
void info(const char *fmt, ...)
	__attribute__ ((format(printf, 1, 2)));

/** Printf alike warning message. */
void warn(const char *fmt, ...)
	__attribute__ ((format(printf, 1, 2)));

/** Printf alike statistics message. */
void stats(const char *fmt, ...)
	__attribute__ ((format(printf, 1, 2)));

/** Print error and exit. */
void error(const char *fmt, ...)
	__attribute__ ((format(printf, 1, 2)));

/** Print error and strerror(errno). */
void serror(const char *fmt, ...)
	__attribute__ ((format(printf, 1, 2)));

/** Print configuration error and exit. */
void cerror(config_setting_t *cfg, const char *fmt, ...)
	__attribute__ ((format(printf, 2, 3)));

#endif /* _LOG_H_ */

