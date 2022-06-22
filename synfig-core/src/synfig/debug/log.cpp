/* === S Y N F I G ========================================================= */
/*!	\file debug/log.cpp
**	\brief Template File
**
**	\legal
**	......... ... 2015 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <cstdlib>

#include <fstream>

#include "log.h"
#include "synfig/filesystemnative.h"

#include <ETL/stringf>
#include <synfig/general.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;
using namespace debug;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

std::mutex Log::mutex;

void Log::append_line_to_file(const String &logfile, const String &str)
{
	std::lock_guard<std::mutex> lock(mutex);
	std::ofstream f(FileSystemNative::path(logfile).c_str(), std::ios_base::app);
	f << str << std::endl;
}

void
Log::error(const String &logfile, const String &str)
{
	if (logfile.empty()) synfig::error(str); else append_line_to_file(logfile, str);
}

void
Log::warning(const String &logfile, const String &str)
{
	if (logfile.empty()) synfig::warning(str); else append_line_to_file(logfile, str);
}

void
Log::info(const String &logfile, const String &str)
{
	if (logfile.empty()) synfig::info(str); else append_line_to_file(logfile, str);
}

void
Log::error(const String &logfile, const char *format,...)
{
	va_list args;
	va_start(args,format);
	error(logfile, vstrprintf(format,args));
	va_end(args);
}

void
Log::warning(const String &logfile, const char *format,...)
{
	va_list args;
	va_start(args,format);
	warning(logfile, vstrprintf(format,args));
	va_end(args);
}

void
Log::info(const String &logfile, const char *format,...)
{
	va_list args;
	va_start(args,format);
	info(logfile, vstrprintf(format,args));
	va_end(args);
}
