/* === S Y N F I G ========================================================= */
/*!	\file debug/log.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	......... ... 2015 Ivan Mahonin
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_DEBUG_LOG_H
#define __SYNFIG_DEBUG_LOG_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <synfig/string.h>
#include <mutex>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */


namespace synfig {
namespace debug {

class Log
{
private:
	static std::mutex mutex;

public:

static void append_line_to_file(const String &logfile, const String &str);


//! Reports an error
/*! Call this when an error occurs, describing what happened */
static void error(const String &logfile, const String &str);
static void error(const String &logfile, const char *format,...);

//! Reports a warning
/*! Call this when something questionable occurs, describing what happened */
static void warning(const String &logfile, const String &str);
static void warning(const String &logfile, const char *format,...);

//! Reports some information
/*! Call this to report various information. Please be sparse... */
static void info(const String &logfile, const String &str);
static void info(const String &logfile, const char *format,...);

};

}; // END of namespace debug
}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
