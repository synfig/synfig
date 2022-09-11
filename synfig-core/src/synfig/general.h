/* === S Y N F I G ========================================================= */
/*!	\file general.h
**	\brief General macros, classes, and procedure declarations
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2010 Carlos López
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GENERAL_H
#define __SYNFIG_GENERAL_H

/* === H E A D E R S ======================================================= */

#include <clocale>
#include "string.h"
#include "synfig_export.h"


/* === M A C R O S ========================================================= */

#define SYNFIG_COPYRIGHT "Copyright (c) 2001-2005 Robert B. Quattlebaum Jr., Adrian Bentley"

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

//! Change the Locale while this object exists
/// It's a guard class like std::lock_guard.
///
/// You should instantiate an object of this class to temporarily change the locale
///   until the end of its life cycle.
///
/// It is useful for reading/writing float numbers no matter the locale, for example.
class ChangeLocale {
	const String previous;
	const int category;
public:
	/// @param category - the locale category to be temporarily changed
	/// @param[in] locale - the temporary locale name
	ChangeLocale(int category, const char *locale)
	: // This backups the old locale
	  previous(setlocale(category,nullptr)), category(category)
	{
		// This effectively changes the locale
		setlocale(category, locale);
	}
	~ChangeLocale() {
		// This restores the locale
		setlocale(category,previous.c_str());
	}
};

//! If true, do not report "info"-level log, only errors and warnings
SYNFIG_EXPORT extern bool synfig_quiet_mode;

//! Reports an error
/*! Call this when an error occurs, describing what happened */
extern void error(const char *format,...);
extern void error(const String &str);

//! Reports a warning
/*! Call this when something questionable occurs, describing what happened */
extern void warning(const char *format,...);
extern void warning(const String &str);

//! Reports some information
/*! Call this to report various information. Please be sparse... */
extern void info(const char *format,...);
extern void info(const String &str);

//#define LOGGING_ENABLED
#ifdef LOGGING_ENABLED
#define DEBUG_LOG(logger, fmt, ...) if (getenv(logger)) printf(fmt __VA_OPT__(,) __VA_ARGS__);
#define DEBUG_GETENV(name) getenv(name)
#else
#define DEBUG_LOG(logger, fmt, ...) void (0)
#define DEBUG_GETENV(name) false
#endif

//! Returns absolute path to the binary
extern String get_binary_path(const String &fallback_path);

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
